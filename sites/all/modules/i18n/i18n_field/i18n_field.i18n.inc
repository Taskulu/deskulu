<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18n_field_i18n_object_info() {
  $info['field'] = array(
    'title' => t('Field'),
    'class' => 'i18n_field',
    'key' => 'field_name',
    'load callback' => 'field_info_field',
    'placeholders' => array(
      '%field_ui_menu' => 'field_name',
      '%field_type' => 'type',
    ),
    'edit path' => 'admin/structure/types/manage/%bundle/fields/%field_ui_menu/field-settings',
    // We can easily list all these objects
    'list callback' => 'field_read_fields',
    'string translation' => array(
      'textgroup' => 'field',
      'properties' => array(
        'label' => array(
          'title' => t('Label'),
        ),
      ),
      //'translate path' => 'admin/structure/block/manage/%module/%delta/translate/%i18n_language',
    )
  );
  $info['field_instance'] = array(
    'title' => t('Field instance'),
    'class' => 'i18n_field_instance',
    'key' => array('field_name', 'bundle'),
    'placeholders' => array(
      '%bundle' => 'bundle',
      '%field_ui_menu' => 'field_name',
    ),
    'edit path' => 'admin/structure/types/manage/%bundle/fields/%field_ui_menu',
    // We can easily list all these objects.
    'list callback' => 'field_read_instances',
    // Metadata for string translation.
    'string translation' => array(
      'textgroup' => 'field',
      'properties' => array(
        'label' => array(
          'title' => t('Label'),
        ),
        'description' => array(
          'title' => t('Description'),
          'format' => 'format',
        ),
        'default_value' => array(
          'title' => t('Default value'),
          'format' => 'format',
        ),
      ),
      //'translate path' => 'admin/structure/types/manage/%bundle/fields/%field_ui_menu/translate/%i18n_language',
    )
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info().
 */
function i18n_field_i18n_string_info() {
  $groups['field'] = array(
    'title' => t('Fields'),
    'description' => t('Configurable fields descriptions, defaults, options, etc.'),
    'format' => FALSE, // This group doesn't have formatted strings
    'list' => TRUE, // This group can list all strings
    'class' => 'i18n_string_textgroup_cached',
  );
  return $groups;
}

/**
 * Implements hook_i18n_field_info().
 */
function i18n_field_i18n_field_info() {
  $info['text'] = $info['text_long'] = $info['text_with_summary'] = array(
    'translate_default' => 'i18n_field_translate_default',
  );
  $info['list_text'] = $info['list_boolean'] = $info['list_integer'] = array(
    'translate_options' => 'i18n_field_translate_allowed_values',
  );
  return $info;
}
