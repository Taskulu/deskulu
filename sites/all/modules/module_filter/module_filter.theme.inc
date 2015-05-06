<?php

/**
 * @file
 *
 * @author greenSkin
 */

function theme_module_filter($variables) {
  $element = $variables['element'];
  return '<div class="module-filter-inputs-wrapper">' . drupal_render_children($element) . '</div>';
}

/**
 * Theme callback for the modules tabbed form.
 */
function theme_module_filter_system_modules_tabs($variables) {
  if (module_exists('views_ui')) {
    // Hack to get consistent style with views ctools dropbutton.
    if (module_load_include('inc', 'views_ui', 'includes/admin')) {
      foreach (views_ui_get_admin_css() as $file => $options) {
        drupal_add_css($file, $options);
      }
    }
  }

  $form = $variables['form'];

  if (!module_exists('page_actions')) {
    $form['actions']['#prefix'] = '<div id="module-filter-submit">';
    $form['actions']['#suffix'] = '</div>';
  }

  $header = array(
    array('data' => '', 'class' => array('checkbox')),
    array('data' => t('Name'), 'class' => array('name')),
    array('data' => t('Description'), 'class' => array('description')),
    array('data' => t('Links'), 'class' => array('links')),
  );
  $package_ids = array('all');
  $enabled['all'] = array();

  if (variable_get('module_filter_track_recent_modules', 1)) {
    $recent_modules = array_filter(variable_get('module_filter_recent_modules', array()), 'module_filter_recent_filter');
    // Save the filtered results.
    variable_set('module_filter_recent_modules', $recent_modules);

    $package_ids[] = 'recent';
    $enabled['recent'] = array();
  }

  // Determine what modules are new (within a week).
  $new_modules = module_filter_new_modules();
  $package_ids[] = 'new';
  $enabled['new'] = array();

  $rows = array();
  $flip = array('even' => 'odd', 'odd' => 'even');
  foreach (element_children($form['modules']) as $package) {
    $package_id = module_filter_get_id($package);
    $package_ids[] = $package_id;

    // Package title and header.
    $rows[] = array('data' => array(array('data' => '<h3>' . $form['modules'][$package]['#title'] . '</h3>', 'colspan' => 4)), 'id' => $package_id . '-package', 'class' => array('admin-package-title'));
    $rows[] = array('data' => $header, 'class' => array('admin-package-header'));

    $stripe = 'odd';
    $enabled[$package_id] = array();
    foreach (element_children($form['modules'][$package]) as $key) {
      $module = &$form['modules'][$package][$key];

      $is_enabled = isset($module['enable']['#default_value']) ? $module['enable']['#default_value'] : '';
      $enabled['all'][] = $enabled[$package_id][] = $is_enabled;
      if (isset($recent_modules[$key])) {
        $enabled['recent'][] = $is_enabled;
      }
      if (isset($new_modules[$key])) {
        $enabled['new'][] = $is_enabled;
      }

      $row = array();

      $version = !empty($module['version']['#markup']);
      $requires = !empty($module['#requires']);
      $required_by = !empty($module['#required_by']);

      $toggle_enable = '';
      if (isset($module['enable']['#type']) && $module['enable']['#type'] == 'checkbox') {
        unset($module['enable']['#title']);
        $class = ($is_enabled ? 'enabled' : 'off');
        if (!empty($module['enable']['#disabled'])) {
          $class .= ' disabled';
        }
        $toggle_enable = '<div class="js-hide toggle-enable ' . $class . '"><div>&nbsp;</div></div>';
      }
      $row[] = array('class' => array('checkbox'), 'data' => $toggle_enable . drupal_render($module['enable']));

      $label = '<label';
      if (isset($module['enable']['#id'])) {
        $label .= ' for="' . $module['enable']['#id'] . '"';
      }
      $row[] = array('class' => array('name'), 'data' => $label . '><strong>' . drupal_render($module['name']) . '</strong> <span class="module-machine-name">(' . $key . ')</span></label>');

      // Add the description, along with any modules it requires.
      $description = '<span class="details"><span class="text">' . drupal_render($module['description']) . '</span></span>';
      if ($version || $requires || $required_by) {
        $description .= '<div class="requirements">';
        if ($version) {
          $description .= '<div class="admin-requirements">' . t('Version: !module-version', array('!module-version' => drupal_render($module['version']))) . '</div>';
        }
        if ($requires) {
          $description .= '<div class="admin-requirements">' . t('Requires: !module-list', array('!module-list' => implode(', ', $module['#requires']))) . '</div>';
        }
        if ($required_by) {
          $description .= '<div class="admin-requirements">' . t('Required by: !module-list', array('!module-list' => implode(', ', $module['#required_by']))) . '</div>';
        }
        $description .= '</div>';
      }
      $row[] = array('data' => '<div class="inner expand" role="button">' . $description . '</div>', 'class' => array('description'));

      $operations = (module_exists('ctools')) ? theme('module_filter_operations', array('links' => $module['links'], 'dropbutton' => TRUE)) : theme('module_filter_operations', array('links' => $module['links']));
      $row[] = array('data' => '<div class="links">' . $operations . '</div>', 'class' => array('links'));

      $class = array(module_filter_get_id($package) . '-tab', 'module', $stripe);
      if (isset($recent_modules[$key])) {
        $class[] = 'recent-module';
      }
      if (isset($new_modules[$key])) {
        $class[] = 'new-module';
      }
      $rows[] = array('data' => $row, 'no_striping' => TRUE, 'class' => $class);
      $stripe = $flip[$stripe];
    }

    // Set the package as printed.
    $form['modules'][$package]['#printed'] = TRUE;
  }

  if (variable_get('module_filter_count_enabled', 1)) {
    $enabled_counts = array();
    foreach ($enabled as $package_id => $value) {
      $enabled_counts[$package_id] = array(
        'enabled' => count(array_filter($value)),
        'total' => count($value),
      );
    }
    drupal_add_js(array(
      'moduleFilter' => array(
        'packageIDs' => $package_ids,
        'enabledCounts' => $enabled_counts,
      )
    ), 'setting');
  }

  // Add first and last class to rows.
  $rows[0]['class'][] = 'first';
  $rows[count($rows) - 1]['class'][] = 'last';

  $output = '<div id="module-filter-wrapper">';
  $output .= '<div id="module-filter-modules">' . drupal_render($form['module_filter']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);
  $output .= '</div>';
  $output .= '</div>';
  return $output;
}

function theme_module_filter_operations(&$vars) {
  $links = &$vars['links'];
  $dropbutton = $vars['dropbutton'];

  $operations = array();
  foreach (element_children($links) as $key) {
    if ($dropbutton) {
      hide($links[$key]);
      if (!empty($links[$key]['#href'])) {
        $operations[] = array(
          'title' => $links[$key]['#title'],
          'href' => $links[$key]['#href'],
        );
      }
    }
    else {
      $data = drupal_render($links[$key]);
      if (!empty($data)) {
        $operations[] = array('data' => $data);
      }
    }
  }
  if (!empty($operations)) {
    if ($dropbutton) {
      return '<div class="admin-operations">' . theme('links__ctools_dropbutton', array('title' => t('Operations'), 'links' => $operations, 'attributes' => array('class' => array('links')))) . '</div>';
    }
    return '<div class="admin-operations">' . theme('item_list', array('items' => $operations, 'attributes' => array('class' => array('links', 'inline')))) . '</div>';
  }
}
