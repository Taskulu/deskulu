<?php

/**
 * @file
 * Internationalization (i18n) package. Synchronization of translations
 *
 * Implements hook_i18n_sync_node_fields() for several core modules.
 */

/**
 * Book module. Implements hook_i18n_sync_options().
 */
function book_i18n_sync_options($entity_type, $bundle_name) {
  if ($entity_type == 'node') {
    return array(
      'parent' => array(
        'title' => t('Book outline'),
        'description' => t('Set the translated parent for each translation if possible.')
      ),
    );
  }
}

/**
 * Comment module. Implements hook_i18n_sync_options().
 */
function comment_i18n_sync_options($entity_type, $bundle_name) {
  if ($entity_type == 'node') {
    $fields['comment'] = array('title' =>  t('Comment settings'));
    return $fields;
  }
}

/**
 * Field module. Implements hook_i18n_sync_options().
 */
function field_i18n_sync_options($entity_type, $bundle_name) {
  $sync_fields = array();
  if ($bundle_name) {
    $instances = field_info_instances($entity_type, $bundle_name);
    foreach ($instances as $name => $instance) {
      $sync_fields[$name] = array(
        'title' => $instance['label'],
        'description' => $instance['description'],
        'field_name' => $instance['field_name'],
        'group' => 'fields',
      );
    }
  }
  return $sync_fields;
}

/**
 * Node module. Implements hook_i18n_sync_options().
 */
function node_i18n_sync_options($entity_type, $bundle_name) {
  if ($entity_type == 'node') {
    return array(
      'uid' => array('title' => t('Author')),
      'status' => array('title' => t('Status')),
      'created' => array('title' => t('Post date')),
      'promote' => array('title' => t('Promote')),
      'moderate' =>  array('title' => t('Moderate')),
      'sticky' => array('title' => t('Sticky')),
      'revision' =>  array('title' => t('Revision'), 'description' => t('Create also new revision for translations')),
    );
  }
}
