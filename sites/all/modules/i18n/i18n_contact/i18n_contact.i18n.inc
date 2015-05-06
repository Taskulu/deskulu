<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_contact_i18n_string_info() {
  $groups['contact'] = array(
    'title' => t('Contact forms'),
    'description' => t('Configurable contact form categories.'),
    'format' => FALSE,
    'list' => TRUE,
  );
  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_contact_i18n_object_info() {
  $info['contact_category'] = array(
    // Generic object title.
    'title' => t('Contact category'),
    // The object key field.
    'key' => 'cid',
    // The object load callback.
    'load callback' => 'contact_load',
    // Placeholders for automatic paths.
    'placeholders' => array(
      '%contact' => 'cid',
    ),
    // To produce edit links automatically.
    'edit path' => 'admin/structure/contact/edit/%contact',
    // Auto-generate translate tab.
    'translate tab' => 'admin/structure/contact/edit/%contact/translate',
    // Properties for string translation.
    'string translation' => array(
      // Text group that will handle this object's strings.
      'textgroup' => 'contact',
      // Object type property for string translation.
      'type' => 'category',
      // Table where the object is stored, to automate string lists
      'table' => 'contact',
      // Translatable properties of these objects.
      'properties' => array(
        'category' => t('Category'),
        'reply' => t('Auto-reply'),
      ),
      // Path to translate strings to every language.
      'translate path' => 'admin/structure/contact/edit/%contact/translate/%i18n_language',
    )
  );
  return $info;
}
