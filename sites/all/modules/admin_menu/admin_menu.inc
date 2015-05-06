<?php

/**
 * @file
 * Menu builder functions for Administration menu.
 */

/**
 * Build the full administration menu tree from static and expanded dynamic items.
 *
 * @param $menu_name
 *   The menu name to use as base for the tree.
 */
function admin_menu_tree($menu_name) {
  // Get placeholder expansion arguments from hook_admin_menu_map()
  // implementations.
  module_load_include('inc', 'admin_menu', 'admin_menu.map');
  $expand_map = module_invoke_all('admin_menu_map');
  // Allow modules to alter the expansion map.
  drupal_alter('admin_menu_map', $expand_map);

  $new_map = array();
  foreach ($expand_map as $path => $data) {
    // Convert named placeholders to anonymous placeholders, since the menu
    // system stores paths using anonymous placeholders.
    $replacements = array_fill_keys(array_keys($data['arguments'][0]), '%');
    $data['parent'] = strtr($data['parent'], $replacements);
    $new_map[strtr($path, $replacements)] = $data;
  }
  $expand_map = $new_map;
  unset($new_map);

  // Retrieve dynamic menu link tree for the expansion mappings.
  // @todo Skip entire processing if initial $expand_map is empty and directly
  //   return $tree?
  if (!empty($expand_map)) {
    $tree_dynamic = admin_menu_tree_dynamic($expand_map);
  }
  else {
    $tree_dynamic = array();
  }

  // Merge local tasks with static menu tree.
  $tree = menu_tree_all_data($menu_name);
  admin_menu_merge_tree($tree, $tree_dynamic, array());

  return $tree;
}

/**
 * Load menu link trees for router paths containing dynamic arguments.
 *
 * @param $expand_map
 *   An array containing menu router path placeholder expansion argument
 *   mappings.
 *
 * @return
 *   An associative array whose keys are the parent paths of the menu router
 *   paths given in $expand_map as well as the parent paths of any child link
 *   deeper down the tree. The parent paths are used in admin_menu_merge_tree()
 *   to check whether anything needs to be merged.
 *
 * @see hook_admin_menu_map()
 */
function admin_menu_tree_dynamic(array $expand_map) {
  $p_columns = array();
  for ($i = 1; $i <= MENU_MAX_DEPTH; $i++) {
    $p_columns[] = 'p' . $i;
  }

  // Fetch p* columns for all router paths to expand.
  $router_paths = array_keys($expand_map);
  $plids = db_select('menu_links', 'ml')
    ->fields('ml', $p_columns)
    ->condition('router_path', $router_paths)
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  // Unlikely, but possible.
  if (empty($plids)) {
    return array();
  }

  // Use queried plid columns to query sub-trees for the router paths.
  $query = db_select('menu_links', 'ml');
  $query->join('menu_router', 'm', 'ml.router_path = m.path');
  $query
    ->fields('ml')
    ->fields('m', array_diff(drupal_schema_fields_sql('menu_router'), drupal_schema_fields_sql('menu_links')));

  // The retrieved menu link trees have to be ordered by depth, so parents
  // always come before their children for the storage logic below.
  foreach ($p_columns as $column) {
    $query->orderBy($column, 'ASC');
  }

  $db_or = db_or();
  foreach ($plids as $path_plids) {
    $db_and = db_and();
    // plids with value 0 may be ignored.
    foreach (array_filter($path_plids) as $column => $plid) {
      $db_and->condition($column, $plid);
    }
    $db_or->condition($db_and);
  }
  $query->condition($db_or);
  $result = $query
    ->execute()
    ->fetchAllAssoc('mlid', PDO::FETCH_ASSOC);

  // Store dynamic links grouped by parent path for later merging and assign
  // placeholder expansion arguments.
  $tree_dynamic = array();
  foreach ($result as $mlid => $link) {
    // If contained in $expand_map, then this is a (first) parent, and we need
    // to store by the defined 'parent' path for later merging, as well as
    // provide the expansion map arguments to apply to the dynamic tree.
    if (isset($expand_map[$link['path']])) {
      $parent_path = $expand_map[$link['path']]['parent'];
      $link['expand_map'] = $expand_map[$link['path']]['arguments'];
    }
    // Otherwise, just store this link keyed by its parent path; the expand_map
    // is automatically derived from parent paths.
    else {
      $parent_path = $result[$link['plid']]['path'];
    }

    $tree_dynamic[$parent_path][] = $link;
  }

  return $tree_dynamic;
}

/**
 * Walk through the entire menu tree and merge in expanded dynamic menu links.
 *
 * @param &$tree
 *   A menu tree structure as returned by menu_tree_all_data().
 * @param $tree_dynamic
 *   A dynamic menu tree structure as returned by admin_menu_tree_dynamic().
 * @param $expand_map
 *   An array containing menu router path placeholder expansion argument
 *   mappings.
 *
 * @see hook_admin_menu_map()
 * @see admin_menu_tree_dynamic()
 * @see menu_tree_all_data()
 */
function admin_menu_merge_tree(array &$tree, array $tree_dynamic, array $expand_map) {
  foreach ($tree as $key => $data) {
    $path = $data['link']['router_path'];

    // Recurse into regular menu tree.
    if ($tree[$key]['below']) {
      admin_menu_merge_tree($tree[$key]['below'], $tree_dynamic, $expand_map);
    }
    // Nothing to merge, if this parent path is not in our dynamic tree.
    if (!isset($tree_dynamic[$path])) {
      continue;
    }

    // Add expanded dynamic items.
    foreach ($tree_dynamic[$path] as $link) {
      // If the dynamic item has custom placeholder expansion parameters set,
      // use them, otherwise keep current.
      if (isset($link['expand_map'])) {
        // If there are currently no expansion parameters, we may use the new
        // set immediately.
        if (empty($expand_map)) {
          $current_expand_map = $link['expand_map'];
        }
        else {
          // Otherwise we need to filter out elements that differ from the
          // current set, i.e. that are not in the same path.
          $current_expand_map = array();
          foreach ($expand_map as $arguments) {
            foreach ($arguments as $placeholder => $value) {
              foreach ($link['expand_map'] as $new_arguments) {
                // Skip the new argument if it doesn't contain the current
                // replacement placeholders or if their values differ.
                if (!isset($new_arguments[$placeholder]) || $new_arguments[$placeholder] != $value) {
                  continue;
                }
                $current_expand_map[] = $new_arguments;
              }
            }
          }
        }
      }
      else {
        $current_expand_map = $expand_map;
      }

      // Skip dynamic items without expansion parameters.
      if (empty($current_expand_map)) {
        continue;
      }

      // Expand anonymous to named placeholders.
      // @see _menu_load_objects()
      $path_args = explode('/', $link['path']);
      $load_functions = unserialize($link['load_functions']);
      if (is_array($load_functions)) {
        foreach ($load_functions as $index => $function) {
          if ($function) {
            if (is_array($function)) {
              list($function,) = each($function);
            }
            // Add the loader function name minus "_load".
            $placeholder = '%' . substr($function, 0, -5);
            $path_args[$index] = $placeholder;
          }
        }
      }
      $path_dynamic = implode('/', $path_args);

      // Create new menu items using expansion arguments.
      foreach ($current_expand_map as $arguments) {
        // Create the cartesian product for all arguments and create new
        // menu items for each generated combination thereof.
        foreach (admin_menu_expand_args($arguments) as $replacements) {
          $newpath = strtr($path_dynamic, $replacements);
          // Skip this item, if any placeholder could not be replaced.
          // Faster than trying to invoke _menu_translate().
          if (strpos($newpath, '%') !== FALSE) {
            continue;
          }
          $map = explode('/', $newpath);
          $item = admin_menu_translate($link, $map);
          // Skip this item, if the current user does not have access.
          if (empty($item)) {
            continue;
          }
          // Build subtree using current replacement arguments.
          $new_expand_map = array();
          foreach ($replacements as $placeholder => $value) {
            $new_expand_map[$placeholder] = array($value);
          }
          admin_menu_merge_tree($item, $tree_dynamic, array($new_expand_map));
          $tree[$key]['below'] += $item;
        }
      }
    }
    // Sort new subtree items.
    ksort($tree[$key]['below']);
  }
}

/**
 * Translate an expanded router item into a menu link suitable for rendering.
 *
 * @param $router_item
 *   A menu router item.
 * @param $map
 *   A path map with placeholders replaced.
 */
function admin_menu_translate($router_item, $map) {
  _menu_translate($router_item, $map, TRUE);

  // Run through hook_translated_menu_link_alter() to add devel information,
  // if configured.
  $router_item['menu_name'] = 'management';
  // @todo Invoke as usual like _menu_link_translate().
  admin_menu_translated_menu_link_alter($router_item, NULL);

  if ($router_item['access']) {
    // Override mlid to make this item unique; since these items are expanded
    // from dynamic items, the mlid is always the same, so each item would
    // replace any other.
    // @todo Doing this instead leads to plenty of duplicate links below
    //   admin/structure/menu; likely a hidden recursion problem.
    // $router_item['mlid'] = $router_item['href'] . $router_item['mlid'];
    $router_item['mlid'] = $router_item['href'];
    // Turn menu callbacks into regular menu items to make them visible.
    if ($router_item['type'] == MENU_CALLBACK) {
      $router_item['type'] = MENU_NORMAL_ITEM;
    }

    // @see _menu_tree_check_access()
    $key = (50000 + $router_item['weight']) . ' ' . $router_item['title'] . ' ' . $router_item['mlid'];
    return array($key => array(
      'link' => $router_item,
      'below' => array(),
    ));
  }

  return array();
}

/**
 * Create the cartesian product of multiple varying sized argument arrays.
 *
 * @param $arguments
 *   A two dimensional array of arguments.
 *
 * @see hook_admin_menu_map()
 */
function admin_menu_expand_args($arguments) {
  $replacements = array();

  // Initialize line cursors, move out array keys (placeholders) and assign
  // numeric keys instead.
  $i = 0;
  $placeholders = array();
  $new_arguments = array();
  foreach ($arguments as $placeholder => $values) {
    // Skip empty arguments.
    if (empty($values)) {
      continue;
    }
    $cursor[$i] = 0;
    $placeholders[$i] = $placeholder;
    $new_arguments[$i] = $values;
    $i++;
  }
  $arguments = $new_arguments;
  unset($new_arguments);

  if ($rows = count($arguments)) {
    do {
      // Collect current argument from each row.
      $row = array();
      for ($i = 0; $i < $rows; ++$i) {
        $row[$placeholders[$i]] = $arguments[$i][$cursor[$i]];
      }
      $replacements[] = $row;

      // Increment cursor position.
      $j = $rows - 1;
      $cursor[$j]++;
      while (!array_key_exists($cursor[$j], $arguments[$j])) {
        // No more arguments left: reset cursor, go to next line and increment
        // that cursor instead. Repeat until argument found or out of rows.
        $cursor[$j] = 0;
        if (--$j < 0) {
          // We're done.
          break 2;
        }
        $cursor[$j]++;
      }
    } while (1);
  }

  return $replacements;
}

/**
 * Build the administration menu as renderable menu links.
 *
 * @param $tree
 *   A data structure representing the administration menu tree as returned from
 *   menu_tree_all_data().
 *
 * @return
 *   The complete administration menu, suitable for theme_admin_menu_links().
 *
 * @see theme_admin_menu_links()
 * @see admin_menu_menu_alter()
 */
function admin_menu_links_menu($tree) {
  $links = array();
  foreach ($tree as $data) {
    // Skip items that are inaccessible, invisible, or link to their parent.
    // (MENU_DEFAULT_LOCAL_TASK), and MENU_CALLBACK-alike items that should only
    // appear in the breadcrumb.
    if (!$data['link']['access'] || $data['link']['type'] & MENU_LINKS_TO_PARENT || $data['link']['type'] == MENU_VISIBLE_IN_BREADCRUMB || $data['link']['hidden'] == 1) {
      continue;
    }
    // Hide 'Administer' and make child links appear on this level.
    // @todo Make this configurable.
    if ($data['link']['router_path'] == 'admin') {
      if ($data['below']) {
        $links = array_merge($links, admin_menu_links_menu($data['below']));
      }
      continue;
    }
    // Omit alias lookups.
    $data['link']['localized_options']['alias'] = TRUE;
    // Remove description to prevent mouseover tooltip clashes.
    unset($data['link']['localized_options']['attributes']['title']);

    // Make action links (typically "Add ...") appear first in dropdowns.
    // They might appear first already, but only as long as there is no link
    // that comes alphabetically first (e.g., a node type with label "Ad").
    if ($data['link']['type'] & MENU_IS_LOCAL_ACTION) {
      $data['link']['weight'] -= 1000;
    }

    $links[$data['link']['href']] = array(
      '#title' => $data['link']['title'],
      '#href' => $data['link']['href'],
      '#options' => $data['link']['localized_options'],
      '#weight' => $data['link']['weight'],
    );

    // Recurse to add any child links.
    $children = array();
    if ($data['below']) {
      $children = admin_menu_links_menu($data['below']);
      $links[$data['link']['href']] += $children;
    }

    // Handle links pointing to category/overview pages.
    if ($data['link']['page_callback'] == 'system_admin_menu_block_page' || $data['link']['page_callback'] == 'system_admin_config_page') {
      // Apply a marker for others to consume.
      $links[$data['link']['href']]['#is_category'] = TRUE;
      // Automatically hide empty categories.
      // Check for empty children first for performance. Only when non-empty
      // (typically 'admin/config'), check whether children are accessible.
      if (empty($children) || !element_get_visible_children($children)) {
        $links[$data['link']['href']]['#access'] = FALSE;
      }
    }
  }
  return $links;
}

/**
 * Build icon menu links; mostly containing maintenance helpers.
 *
 * @see theme_admin_menu_links()
 */
function admin_menu_links_icon() {
  $destination = drupal_get_destination();

  $links = array(
    '#theme' => 'admin_menu_links',
    '#wrapper_attributes' => array('id' => 'admin-menu-icon'),
    '#weight' => -100,
  );
  $links['icon'] = array(
    '#title' => theme('admin_menu_icon'),
    '#attributes' => array('class' => array('admin-menu-icon')),
    '#href' => '<front>',
    '#options' => array(
      'html' => TRUE,
    ),
  );
  // Add link to manually run cron.
  $links['icon']['cron'] = array(
    '#title' => t('Run cron'),
    '#weight' => 50,
    '#access' => user_access('administer site configuration'),
    '#href' => 'admin/reports/status/run-cron',
  );
  // Add link to run update.php.
  $links['icon']['update'] = array(
    '#title' => t('Run updates'),
    '#weight' => 50,
    // @see update_access_allowed()
    '#access' => $GLOBALS['user']->uid == 1 || !empty($GLOBALS['update_free_access']) || user_access('administer software updates'),
    '#href' => base_path() . 'update.php',
    '#options' => array(
      'external' => TRUE,
    ),
  );
  // Add link to drupal.org.
  $links['icon']['drupal.org'] = array(
    '#title' => 'Drupal.org',
    '#weight' => 100,
    '#access' => user_access('display drupal links'),
    '#href' => 'http://drupal.org',
  );
  // Add links to project issue queues.
  foreach (module_list(FALSE, TRUE) as $module) {
    $info = drupal_parse_info_file(drupal_get_path('module', $module) . '/' . $module . '.info');
    if (!isset($info['project']) || isset($links['icon']['drupal.org'][$info['project']])) {
      continue;
    }
    $links['icon']['drupal.org'][$info['project']] = array(
      '#title' => t('@project issue queue', array('@project' => $info['name'])),
      '#weight' => ($info['project'] == 'drupal' ? -10 : 0),
      '#href' => 'http://drupal.org/project/issues/' . $info['project'],
      '#options' => array(
        'query' => array('version' => (isset($info['core']) ? $info['core'] : 'All')),
      ),
    );
  }
  // Add items to flush caches.
  $links['icon']['flush-cache'] = array(
    '#title' => t('Flush all caches'),
    '#weight' => 20,
    '#access' => user_access('flush caches'),
    '#href' => 'admin_menu/flush-cache',
    '#options' => array(
      'query' => $destination + array('token' => drupal_get_token('admin_menu/flush-cache')),
    ),
  );
  $caches = module_invoke_all('admin_menu_cache_info');
  foreach ($caches as $name => $cache) {
    $links['icon']['flush-cache'][$name] = array(
      '#title' => $cache['title'],
      '#href' => 'admin_menu/flush-cache/' . $name,
      '#options' => array(
        'query' => $destination + array('token' => drupal_get_token('admin_menu/flush-cache/' . $name)),
      ),
    );
  }

  // Add Devel module menu links.
  if (module_exists('devel')) {
    $devel_tree = menu_build_tree('devel');
    $devel_links = admin_menu_links_menu($devel_tree);
    if (element_get_visible_children($devel_links)) {
      $links['icon']['devel'] = array(
        '#title' => t('Development'),
        '#weight' => 30,
      ) + $devel_links;
    }
  }

  return $links;
}

/**
 * Builds the account links.
 *
 * @see theme_admin_menu_links()
 */
function admin_menu_links_account() {
  $links = array(
    '#theme' => 'admin_menu_links',
    '#wrapper_attributes' => array('id' => 'admin-menu-account'),
    '#weight' => 100,
  );
  $links['account'] = array(
    '#title' => format_username($GLOBALS['user']),
    '#weight' => -99,
    '#attributes' => array('class' => array('admin-menu-action', 'admin-menu-account')),
    '#href' => 'user/' . $GLOBALS['user']->uid,
  );
  $links['logout'] = array(
    '#title' => t('Log out'),
    '#weight' => -100,
    '#attributes' => array('class' => array('admin-menu-action')),
    '#href' => 'user/logout',
  );
  // Add Devel module switch user links.
  $switch_links = module_invoke('devel', 'switch_user_list');
  if (!empty($switch_links) && count($switch_links) > 1) {
    foreach ($switch_links as $uid => $link) {
      $links['account'][$link['title']] = array(
        '#title' => $link['title'],
        '#description' => $link['attributes']['title'],
        '#href' => $link['href'],
        '#options' => array(
          'query' => $link['query'],
          'html' => !empty($link['html']),
        ),
      );
    }
  }
  return $links;
}

/**
 * Builds user counter.
 *
 * @see theme_admin_menu_links()
 */
function admin_menu_links_users() {
  $links = array(
    '#theme' => 'admin_menu_links',
    '#wrapper_attributes' => array('id' => 'admin-menu-users'),
    '#weight' => 150,
  );
  // Add link to show current authenticated/anonymous users.
  $links['user-counter'] = array(
    '#title' => admin_menu_get_user_count(),
    '#description' => t('Current anonymous / authenticated users'),
    '#weight' => -90,
    '#attributes' => array('class' => array('admin-menu-action', 'admin-menu-users')),
    '#href' => (user_access('administer users') ? 'admin/people/people' : 'user'),
  );
  return $links;
}

/**
 * Build search widget.
 *
 * @see theme_admin_menu_links()
 */
function admin_menu_links_search() {
  $links = array(
    '#theme' => 'admin_menu_links',
    '#wrapper_attributes' => array('id' => 'admin-menu-search'),
    '#weight' => 180,
  );
  $links['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Search'),
    '#title_display' => 'attribute',
    '#attributes' => array(
      'placeholder' => t('Search'),
      'class' => array('admin-menu-search'),
    ),
  );
  return $links;
}

/**
 * Form builder function for module settings.
 */
function admin_menu_theme_settings() {
  $form['admin_menu_margin_top'] = array(
    '#type' => 'checkbox',
    '#title' => t('Adjust top margin'),
    '#default_value' => variable_get('admin_menu_margin_top', 1),
    '#description' => t('Shifts the site output down by approximately 20 pixels from the top of the viewport. If disabled, absolute- or fixed-positioned page elements may be covered by the administration menu.'),
  );
  $form['admin_menu_position_fixed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Keep menu at top of page'),
    '#default_value' => variable_get('admin_menu_position_fixed', 1),
    '#description' => t('Displays the administration menu always at the top of the browser viewport (even when scrolling the page).'),
  );
  // @todo Re-confirm this with latest browser versions.
  $form['admin_menu_position_fixed']['#description'] .= '<br /><strong>' . t('In some browsers, this setting may result in a malformed page, an invisible cursor, non-selectable elements in forms, or other issues.') . '</strong>';

  $form['advanced'] = array(
    '#type' => 'vertical_tabs',
    '#title' => t('Advanced settings'),
  );

  $form['plugins'] = array(
    '#type' => 'fieldset',
    '#title' => t('Plugins'),
    '#group' => 'advanced',
  );
  $form['plugins']['admin_menu_components'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled components'),
    '#options' => array(
      'admin_menu.icon' => t('Icon menu'),
      'admin_menu.menu' => t('Administration menu'),
      'admin_menu.search' => t('Search bar'),
      'admin_menu.users' => t('User counts'),
      'admin_menu.account' => t('Account links'),
    ),
  );
  $form['plugins']['admin_menu_components']['#default_value'] = array_keys(array_filter(variable_get('admin_menu_components', $form['plugins']['admin_menu_components']['#options'])));

  $process = element_info_property('checkboxes', '#process', array());
  $form['plugins']['admin_menu_components']['#process'] = array_merge(array('admin_menu_settings_process_components'), $process);
  $form['#attached']['js'][] = drupal_get_path('module', 'admin_menu') . '/admin_menu.admin.js';

  $form['tweaks'] = array(
    '#type' => 'fieldset',
    '#title' => t('System tweaks'),
    '#group' => 'advanced',
  );
  $form['tweaks']['admin_menu_tweak_modules'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collapse module groups on the <a href="!modules-url">%modules</a> page', array(
      '%modules' => t('Modules'),
      '!modules-url' => url('admin/modules'),
    )),
    '#default_value' => variable_get('admin_menu_tweak_modules', 0),
  );
  if (module_exists('util')) {
    $form['tweaks']['admin_menu_tweak_modules']['#description'] .= '<br /><strong>' . t('If the Utility module was installed for this purpose, it can be safely disabled and uninstalled.') . '</strong>';
  }
  $form['tweaks']['admin_menu_tweak_permissions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collapse module groups on the <a href="@permissions-url">%permissions</a> page', array(
      '%permissions' => t('Permissions'),
      '@permissions-url' => url('admin/people/permissions'),
    )),
    '#default_value' => variable_get('admin_menu_tweak_permissions', 0),
  );
  $form['tweaks']['admin_menu_tweak_tabs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Move local tasks into menu'),
    '#default_value' => variable_get('admin_menu_tweak_tabs', 0),
    '#description' => t('Moves the tabs on all pages into the administration menu. Only possible for themes using the CSS classes <code>tabs primary</code> and <code>tabs secondary</code>.'),
  );

  $form['performance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Performance'),
    '#group' => 'advanced',
  );
  $form['performance']['admin_menu_cache_client'] = array(
    '#type' => 'checkbox',
    '#title' => t('Cache menu in client-side browser'),
    '#default_value' => variable_get('admin_menu_cache_client', 1),
  );

  return system_settings_form($form);
}

/**
 * #process callback for component plugin form element in admin_menu_theme_settings().
 */
function admin_menu_settings_process_components($element) {
  // Assign 'rel' attributes to all options to achieve a live preview.
  // Unfortunately, #states relies on wrapping .form-wrapper classes, so it
  // cannot be used here.
  foreach ($element['#options'] as $key => $label) {
    if (!isset($element[$key]['#attributes']['rel'])) {
      $id = preg_replace('/[^a-z]/', '-', $key);
      $element[$key]['#attributes']['rel'] = '#' . $id;
    }
  }
  return $element;
}

/**
 * Form validation handler for admin_menu_theme_settings().
 */
function admin_menu_theme_settings_validate(&$form, &$form_state) {
  // Change the configured components to Boolean values.
  foreach ($form_state['values']['admin_menu_components'] as $component => &$enabled) {
    $enabled = (bool) $enabled;
  }
}

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * Extends Devel module with Administration menu developer settings.
 */
function _admin_menu_form_devel_admin_settings_alter(&$form, $form_state) {
  // Shift system_settings_form buttons.
  $weight = isset($form['buttons']['#weight']) ? $form['buttons']['#weight'] : 0;
  $form['buttons']['#weight'] = $weight + 1;

  $form['admin_menu'] = array(
    '#type' => 'fieldset',
    '#title' => t('Administration menu settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $display_options = array('mid', 'weight', 'pid');
  $display_options = array(0 => t('None'), 'mlid' => t('Menu link ID'), 'weight' => t('Weight'), 'plid' => t('Parent link ID'));
  $form['admin_menu']['admin_menu_display'] = array(
    '#type' => 'radios',
    '#title' => t('Display additional data for each menu item'),
    '#default_value' => variable_get('admin_menu_display', 0),
    '#options' => $display_options,
    '#description' => t('Display the selected items next to each menu item link.'),
  );
  $form['admin_menu']['admin_menu_show_all'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display all menu items'),
    '#default_value' => variable_get('admin_menu_show_all', 0),
    '#description' => t('If enabled, all menu items are displayed regardless of your site permissions. <em>Note: Do not enable on a production site.</em>'),
  );
}

/**
 * Flush all caches or a specific one.
 *
 * @param $name
 *   (optional) Name of cache to flush.
 */
function admin_menu_flush_cache($name = NULL) {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], current_path())) {
    return MENU_ACCESS_DENIED;
  }
  if (isset($name)) {
    $caches = module_invoke_all('admin_menu_cache_info');
    if (!isset($caches[$name])) {
      return MENU_NOT_FOUND;
    }
  }
  else {
    $caches[$name] = array(
      'title' => t('Every'),
      'callback' => 'drupal_flush_all_caches',
    );
  }
  // Pass the cache to flush forward to the callback.
  $function = $caches[$name]['callback'];
  $function($name);

  drupal_set_message(t('!title cache cleared.', array('!title' => $caches[$name]['title'])));

  // The JavaScript injects a destination request parameter pointing to the
  // originating page, so the user is redirected back to that page. Without
  // destination parameter, the redirect ends on the front page.
  drupal_goto();
}

/**
 * Implements hook_admin_menu_cache_info().
 */
function admin_menu_admin_menu_cache_info() {
  $caches['admin_menu'] = array(
    'title' => t('Administration menu'),
    'callback' => '_admin_menu_flush_cache',
  );
  return $caches;
}

/**
 * Implements hook_admin_menu_cache_info() on behalf of System module.
 */
function system_admin_menu_cache_info() {
  $caches = array(
    'assets' => t('CSS and JavaScript'),
    'cache' => t('Page and else'),
    'menu' => t('Menu'),
    'registry' => t('Class registry'),
    'theme' => t('Theme registry'),
  );
  foreach ($caches as $name => $cache) {
    $caches[$name] = array(
      'title' => $cache,
      'callback' => '_admin_menu_flush_cache',
    );
  }
  return $caches;
}

/**
 * Implements hook_admin_menu_cache_info() on behalf of Update module.
 */
function update_admin_menu_cache_info() {
  $caches['update'] = array(
    'title' => t('Update data'),
    'callback' => '_update_cache_clear',
  );
  return $caches;
}

/**
 * Flush all caches or a specific one.
 *
 * @param $name
 *   (optional) Name of cache to flush.
 *
 * @see system_admin_menu_cache_info()
 */
function _admin_menu_flush_cache($name = NULL) {
  switch ($name) {
    case 'admin_menu':
      admin_menu_flush_caches();
      break;

    case 'menu':
      menu_rebuild();
      break;

    case 'registry':
      registry_rebuild();
      // Fall-through to clear cache tables, since registry information is
      // usually the base for other data that is cached (e.g. SimpleTests).
    case 'cache':
      // Don't clear cache_form - in-progress form submissions may break.
      // Ordered so clearing the page cache will always be the last action.
      // @see drupal_flush_all_caches()
      $core = array('cache', 'cache_bootstrap', 'cache_filter', 'cache_page');
      $cache_tables = array_merge(module_invoke_all('flush_caches'), $core);
      foreach ($cache_tables as $table) {
        cache_clear_all('*', $table, TRUE);
      }
      break;

    case 'assets':
      // Change query-strings on css/js files to enforce reload for all users.
      _drupal_flush_css_js();

      drupal_clear_css_cache();
      drupal_clear_js_cache();

      // Clear the page cache, since cached HTML pages might link to old CSS and
      // JS aggregates.
      cache_clear_all('*', 'cache_page', TRUE);
      break;

    case 'theme':
      system_rebuild_theme_data();
      drupal_theme_rebuild();
      break;
  }
}

/**
 * Preprocesses variables for theme_admin_menu_icon().
 */
function template_preprocess_admin_menu_icon(&$variables) {
  // Image source might have been passed in as theme variable.
  if (!isset($variables['src'])) {
    if (theme_get_setting('toggle_favicon')) {
      $variables['src'] = theme_get_setting('favicon');
    }
    else {
      $variables['src'] = base_path() . 'misc/favicon.ico';
    }
  }
  // Strip the protocol without delimiters for transient HTTP/HTTPS support.
  // Since the menu is cached on the server-side and client-side, the cached
  // version might contain a HTTP link, whereas the actual page is on HTTPS.
  // Relative paths will work fine, but theme_get_setting() returns an
  // absolute URI.
  $variables['src'] = preg_replace('@^https?:@', '', $variables['src']);
  $variables['src'] = check_plain($variables['src']);
  $variables['alt'] = t('Home');
}

/**
 * Renders an icon to display in the administration menu.
 *
 * @ingroup themeable
 */
function theme_admin_menu_icon($variables) {
  return '<img class="admin-menu-icon" src="' . $variables['src'] . '" width="16" height="16" alt="' . $variables['alt'] . '" />';
}

