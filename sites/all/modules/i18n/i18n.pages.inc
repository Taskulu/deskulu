<?php
/**
 * @file
 * Generic translation pages
 * 
 * It handles generic 'translation' tabs, redirecting to the right module depending on
 * object type and properties.
 */

/**
 * Create menu items for translatable objecs
 */
function i18n_page_menu_items() {
  $items = array();
  // Menus are rebuilt right after other modules are disabled, if no reset we get tabs for disabled modules.
  drupal_static_reset('i18n_object_info');
  foreach (i18n_object_info() as $type => $info) {
    // These objects should have a 'translate tab' property
    if (!empty($info['translate tab'])) {
      $path = $info['translate tab'];
      $localize = module_exists('i18n_string') && !empty($info['string translation']);
      $translate = module_exists('i18n_translation') && i18n_translation_set_info($type);
      if ($translate && $localize) {
        $page_callback = 'i18n_page_translate_tab';
      }
      elseif ($translate) {
        $page_callback = 'i18n_page_translate_translation';
      }
      elseif ($localize) {
        $page_callback = 'i18n_page_translate_localize';
      }
      // Find the position for the object placeholder. We assume the first one.
      $placeholder = key($info['placeholders']);
      $parts = explode('/', $path);
      $position = array_search($placeholder, $parts);
      // Now create items with the right callbacks
      if ($translate || $localize) {
        $items[$path] = array(
          'title' => 'Translate',
          'page callback' => $page_callback,
          'page arguments' => array($type, $position),
          'access callback' => 'i18n_object_translate_access',
          'access arguments' => array($type, $position),
          'type' => MENU_LOCAL_TASK,
          'file' => 'i18n.pages.inc',
          'weight' => 10,
        );
      }
      if ($localize) {
        $items[$path . '/%i18n_language'] = array(
          'title' => 'Translate',
          'page callback' => $page_callback,
          'page arguments' => array($type, $position, count($parts)),
          'access callback' => 'i18n_object_translate_access',
          'access arguments' => array($type, $position),
          'type' => MENU_CALLBACK,
          'file' => 'i18n.pages.inc',
        );
      }
    }
    
  }
  return $items;
}

/**
 * Translate or localize page for object
 */
function i18n_page_translate_tab($type, $object, $language = NULL) {
   // Check whether object should be part of a translation set
   switch (i18n_object($type, $object)->get_translate_mode()) {
     case I18N_MODE_TRANSLATE:
       return i18n_page_translate_translation($type, $object);
     case I18N_MODE_LOCALIZE:
       return i18n_page_translate_localize($type, $object, $language);
     default:
       drupal_access_denied();
   }
}

/**
 * Translate object, create translation set
 */
function i18n_page_translate_translation($type, $object) {
  module_load_include('pages.inc', 'i18n_translation');
  return i18n_translation_object_translate_page($type, $object);
}

/**
 * Translate object, string translation
 */
function i18n_page_translate_localize($type, $object, $language = NULL) {
  module_load_include('pages.inc', 'i18n_string');
  return i18n_string_object_translate_page($type, $object, $language);
}