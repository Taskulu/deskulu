<?php
/**
 * @file
 * Variable information
 */

/**
 * Implements hook_variable_info().
 */
function i18n_sync_variable_info($options = array()) {
  $variables['i18n_sync_node_type_[node_type]'] = array(
    'title' => t('Synchronize fields for node type.', array(), $options),
    'type' => 'multiple',
    'repeat' => array(
      'type' => 'array',
    ),
    'group' => 'i18n',
  );
  return $variables;
}