<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18n_menu_i18n_object_info() {
  $info['menu'] = array(
    'title' => t('Menu'),
    'key' => 'menu_name',
    'load callback' => 'menu_load',
    'base path' => 'admin/structure/menu/manage',
    'placeholders' => array(
      '%menu' => 'menu_name',
    ),
    'edit path' => 'admin/structure/menu/manage/%menu/edit',
    // Auto-generate translate tab.
    'translate tab' => 'admin/structure/menu/manage/%menu/translate',
    // We can easily list all these objects
    'list callback' => 'menu_load_all',
    // Metadata for string translation
    'string translation' => array(
      'textgroup' => 'menu',
      'type' => 'menu',
      'properties' => array(
        'title' => t('Title'),
        'description' => t('Description'),
      ),
    ),
    'translation container' => array(
      'name' => t('menu'),
      'item type' => 'menu_link',
      'item name' => t('menu items'),
      'options' => array(I18N_MODE_NONE, I18N_MODE_MULTIPLE, I18N_MODE_LANGUAGE),
    ),
  );
  $info['menu_link'] = array(
    'title' => t('Menu link'),
    'class' => 'i18n_menu_link',
    'key' => 'mlid',
    'load callback' => 'menu_link_load',
    'base path' => 'admin/structure/menu/item',
    'edit path' => 'admin/structure/menu/item/%menu_link/edit',
    // Auto-generate translate tab
    'translate tab' => 'admin/structure/menu/item/%menu_link/translate',
    'placeholders' => array(
      '%menu_link' => 'mlid',
      '%menu' => 'menu_name',
    ),
    'string translation' => array(
      'textgroup' => 'menu',
      'type' => 'item',
      'properties' => array(
        'title' => array(
          'title' => t('Title'),
          'field' => 'link_title',
        ),
        'description' => array(
          'title' => t('Description'),
          'field' => 'options.attributes.title',
        ),
      ),
    ),
    'translation set' => TRUE,
  );
  return $info;
}

/**
 * Implements hook_i18n_translation_set_info()
 */
function i18n_menu_i18n_translation_set_info() {
  $info['menu_link'] = array(
    'title' => t('Menu link'),
    'class' => 'i18n_menu_link_translation_set',
    'table' => 'menu_links',
    'field' => 'i18n_tsid',
    'parent' => 'menu',
    'placeholder' => '%i18n_menu_translation',
    'list path' => 'admin/structure/menu/manage/translation',
    'edit path' => 'admin/structure/menu/manage/translation/edit/%i18n_menu_translation',
    'delete path' => 'admin/structure/menu/manage/translation/delete/%i18n_menu_translation',
    'page callback' => 'i18n_menu_item_translation_page',
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info()
 */
function i18n_menu_i18n_string_info() {
  $groups['menu'] = array(
    'title' => t('Menu'),
    'description' => t('Translatable menu items: title and description.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_objects()
 */
function i18n_menu_i18n_string_objects($type) {
  if ($type == 'menu_link') {
    // All menu items that have no language and are customized.
    return db_select('menu_links', 'm')
      ->fields('m')
      ->condition('language', LANGUAGE_NONE)
      ->condition('customized', 1)
      ->execute()
      ->fetchAllAssoc('mlid', PDO::FETCH_ASSOC);
  }
}

/**
 * Callback for menu item translation tab.
 */
function i18n_menu_item_translation_page($type, $item) {
  module_load_include('admin.inc', 'i18n_menu');
  // If the item has a language code, we can only support translation sets.
  $translation_set = !empty($item['i18n_tsid']) ? i18n_translation_set_load($item['i18n_tsid']) : NULL;
  $overview = i18n_menu_translation_item_overview($item, $translation_set);
  $translation_form = drupal_get_form('i18n_menu_translation_form', $translation_set, $item);
  return $overview + $translation_form;
}

