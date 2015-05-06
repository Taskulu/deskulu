<?php

/**
 * @file
 * Helper functions for menu administration.
 */


/**
 * Produces a menu translation form.
 */
function i18n_menu_translation_form($form, $form_state, $translation_set = NULL, $item = NULL) {
  $translation_set = $translation_set ? $translation_set : i18n_translation_set_build('menu_link');
  $form['translation_set'] = array('#type' => 'value', '#value' => $translation_set);
  $translations = $translation_set->get_translations();
  // What to do with title? Let's make it a hidden field for now, some tests relay on it
  $form['title'] = array('#type' => 'hidden', '#default_value' => $translation_set->title);
  if ($item && ($lang = i18n_object_langcode($item))) {
    $translations[$lang] = $item;
  }
  $item = $item ? $item : array('mlid' => 0, 'menu_name' => '', 'plid' => 0);
  $item_lang = i18n_object_langcode($item);
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translations'),
    '#tree' => TRUE,
    '#description' => t('Enter items that will be considered as translations of each other.'),
  );
  foreach (i18n_language_list() as $langcode => $language_name) {
    if ($langcode == $item_lang) {
      // We've got a predefined item for this language
      $form['translations'][$langcode] = array('#type' => 'value', '#value' => $item['menu_name'] . ':' . $item['mlid']);
      $form['translations']['display'] = array(
        '#type' => 'item',
        '#title' => $language_name,
        '#markup' => check_plain($item['link_title']),
      );
    }
    else {
      // Generate a list of possible parents (not including this link or descendants).
      $options = i18n_menu_parent_options(menu_get_menus(), $item, $langcode);
      $default = isset($translations[$langcode]) ? $translations[$langcode]['menu_name'] . ':' . $translations[$langcode]['mlid'] : 'navigation:0';
      if (!isset($options[$default])) {
        $default = 'navigation:0';
      }
      $form['translations'][$langcode] = array(
        '#type' => 'select',
        '#title' => $language_name,
        '#default_value' => $default,
        '#options' => $options,
        '#description' => t('The maximum depth for a link and all its children is fixed at !maxdepth. Some menu links may not be available as parents if selecting them would exceed this limit.', array('!maxdepth' => MENU_MAX_DEPTH)),
        '#attributes' => array('class' => array('menu-title-select')),
      );
    }
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['update'] = array('#type' => 'submit', '#value' => t('Save'));
  if (!empty($translation_set->tsid)) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
  }
  return $form;
}

/**
 * Process form validation
 */
function i18n_menu_translation_form_validate($form, &$form_state)  {
  if ($form_state['values']['op'] == t('Save')) {
    $selected = 0;
    // example array('en' => 'navigation:0')
    $mlids = array_filter($form_state['values']['translations']);
    foreach ($mlids as $lang => $item_name) {
      list($menu_name, $mlid) = explode(':', $item_name);
      if ($mlid && ($item = menu_link_load($mlid)) && i18n_object_langcode($item)) {
        $selected++;
      }
      else {
        unset($form_state['values']['translations'][$lang]);
      }
    }
    if ($selected < 1) {
      form_set_error('translations', t('There are no translations to save.'));
    }
  }
}

/**
 * Menu item translation form submission
 */
function i18n_menu_translation_form_submit($form, &$form_state) {
  $translation_set = $form_state['values']['translation_set'];

  switch ($form_state['values']['op']) {
    case t('Save'):
      $mlids = array_filter($form_state['values']['translations']);
      $translation_set->reset_translations();
      foreach ($mlids as $lang => $item_name) {
        list($menu_name, $mlid) = explode(':', $item_name);
        $item = menu_link_load($mlid);
        $translation_set->add_item($item, $lang);
      }
      $translation_set->title = !empty($form_state['values']['title']) ? $form_state['values']['title'] : '';
      $translation_set->save(TRUE);
      drupal_set_message(t('The item translation has been saved.'));
      break;
    case t('Delete'):
      $translation_set->delete(TRUE);
      drupal_set_message(t('The item translation has been deleted.'));
      break;
  }

  $form_state['redirect'] = 'admin/structure/menu';
}

/**
 * Return a list of menu items that are valid possible parents for the given menu item.
 *
 * @param $menus
 *   An array of menu names and titles, such as from menu_get_menus().
 * @param $item
 *   The menu item or the node type for which to generate a list of parents.
 *   If $item['mlid'] == 0 then the complete tree is returned.
 * @return
 *   An array of menu link titles keyed on the a string containing the menu name
 *   and mlid. The list excludes the given item and its children.
 *
 * @todo This has to be turned into a #process form element callback. The
 *   'menu_override_parent_selector' variable is entirely superfluous.
 */
function i18n_menu_parent_options($menus, $item, $langcode) {
  // The menu_links table can be practically any size and we need a way to
  // allow contrib modules to provide more scalable pattern choosers.
  // hook_form_alter is too late in itself because all the possible parents are
  // retrieved here, unless menu_override_parent_selector is set to TRUE.
  if (variable_get('i18n_menu_override_parent_selector', FALSE)) {
    return array();
  }
  // If no menu item, create a dummy one
  $item = $item ? $item : array('mlid' => 0);
  // Get menus that have a language or have language for terms
  $available_menus = array();
  foreach (menu_load_all() as $name => $menu) {
    if ($menu['i18n_mode'] & I18N_MODE_TRANSLATE) {
      $available_menus[$name] = $menu;
    }
    elseif ($menu['i18n_mode'] & I18N_MODE_LANGUAGE && $menu['language'] == $langcode) {
      $available_menus[$name] = $menu;
    }
  }
  // Disable i18n selection, enable after the query.
  $previous = i18n_select(FALSE);
  $options = _i18n_menu_get_options($menus, $available_menus, $item, $langcode);
  i18n_select($previous);
  return $options;
}

/**
 * Helper function to get the items of the given menu.
 */
function _i18n_menu_get_options($menus, $available_menus, $item, $langcode) {
  // If the item has children, there is an added limit to the depth of valid parents.
  if (isset($item['parent_depth_limit'])) {
    $limit = $item['parent_depth_limit'];
  }
  else {
    $limit = _menu_parent_depth_limit($item);
  }

  $options = array();
  foreach ($menus as $menu_name => $title) {
    if (isset($available_menus[$menu_name])) {
      if ($tree = i18n_menu_tree_all_data($menu_name, $langcode, NULL)) {
        $options[$menu_name . ':0'] = '<' . $title . '>';
        _menu_parents_recurse($tree, $menu_name, '--', $options, $item['mlid'], $limit);
      }
    }
  }
  return $options;
}

/**
 * Filter out menu items that have a different language
 */
function i18n_menu_tree_all_data($menu_name, $langcode, $link = NULL, $max_depth = NULL) {
  $tree = menu_tree_all_data($menu_name, $link, $max_depth);
  return _i18n_menu_tree_filter_items($tree, $langcode);
}

/**
 * Filter out menu items that have a different language
 */
function _i18n_menu_tree_filter_items($tree, $langcode) {
  $result = array();
  foreach ($tree as $key => $item) {
    $lang = i18n_object_langcode($item['link']);
    if (!empty($item['below'])) {
      $item['below'] = _i18n_menu_tree_filter_items($item['below'], $langcode);
    }
    if (!empty($item['link']['customized']) && $lang == $langcode) {
      $result[$key] = $item;
    }
    elseif (!empty($item['below'])) {
      // Keep for the tree but mark as unselectable.
      $item['link']['title'] = '(' . $item['link']['title'] . ')';
      $result[$key] = $item;
    }
  }
  return $result;
}

/**
 * Callback for menu translation tab.
 */
function i18n_menu_translation_item_overview($item, $translation_set = NULL) {
  if ($item['i18n_tsid']) {
    // Already part of a set, grab that set.
    $translation_set = i18n_translation_set_load($item['i18n_tsid']);
    $translations = $translation_set->get_translations();
  }
  else {
    // We have no translation source mlid, this could be a new set, emulate that.
    $translations = array($item['language'] => $item);
  }

  $type = variable_get('translation_language_type', LANGUAGE_TYPE_INTERFACE);
  $header = array(t('Language'), t('Title'), t('Operations'));
  $rows = array();

  foreach (i18n_language_list() as $langcode => $language_name) {
    $options = array();
    if (isset($translations[$langcode])) {
      // Existing translation in the translation set: display status.
      $translation_item = menu_link_load($translations[$langcode]['mlid']);
      $title = l($translation_item['link_title'], $translation_item['link_path']);
      $path = 'admin/structure/menu/item/' . $translation_item['mlid'];
      $options[] = l(t('edit'), $path);

      if ($translation_item['mlid'] == $item['mlid']) {
        $language_name = t('<strong>@language_name</strong> (source)', array('@language_name' => $language_name));
      }
    }
    else {
      // No such translation in the set yet: help user to create it.
      $title = t('n/a');
      $options[] = l(t('add translation'), 'admin/structure/menu/manage/' . $item['menu_name'] . '/add', array('query' => array('translation' => $item['mlid'], 'target' => $langcode) + drupal_get_destination()));
    }
    $rows[$langcode] = array(
      'language' => $language_name,
      'title' => $title,
      'operations' => implode(" | ", $options)
    );
  }

  drupal_set_title(t('Translations of menu item %title', array('%title' => $item['link_title'])), PASS_THROUGH);

  $build['translation_overview'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}
