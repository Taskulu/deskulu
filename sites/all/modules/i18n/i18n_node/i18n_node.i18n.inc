<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */


/**
 * Implements hook_i18n_object_info().
 */
function i18n_node_i18n_object_info() {
  $info['node_type'] = array(
    'title' => t('Node type'),
    'key' => 'type',
    'load callback' => 'node_type_get_type',
    'placeholders' => array(
      '%node_type' => 'type',
    ),
    'edit path' => 'admin/structure/types/manage/%node_type',
    'translate tab' => 'admin/structure/types/manage/%node_type/translate',
    // We can easily list all these objects
    'list callback' => 'node_type_get_types',
    // Metadata for string translation
    'string translation' => array(
      'textgroup' => 'node',
      'type' => 'type',
      'properties' => array(
        'name' => t('Name'),
        'title_label' => t('Title label'),
        'description' => t('Description'),
        'help' => t('Help text'),
      ),
      'translate path' => 'admin/structure/types/manage/%node_type/translate/%i18n_language',
    )
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info()
 */
function i18n_node_i18n_string_info() {
  $groups['node'] = array(
    'title' => t('Node types'),
    'description' => t('Content type names, descriptions, help texts.'),
    //'format' => TRUE, // This group has strings with format (block body)
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}
