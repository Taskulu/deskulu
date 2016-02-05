<?php

/**
 * Implements hook_views_data_alter().
 */
function views_bulk_operations_views_data_alter(&$data) {
  foreach (entity_get_info() as $entity_type => $info) {
    if (isset($info['base table']) && isset($data[$info['base table']]['table'])) {
      $data[$info['base table']]['views_bulk_operations'] = array(
        'title' => $data[$info['base table']]['table']['group'],
        'group' => t('Bulk operations'),
        'help' => t('Provide a checkbox to select the row for bulk operations.'),
        'real field' => $info['entity keys']['id'],
        'field' => array(
          'handler' => 'views_bulk_operations_handler_field_operations',
          'click sortable' => FALSE,
        ),
      );
    }
    if (isset($info['revision table']) && isset($data[$info['revision table']]['table'])) {
      $data[$info['revision table']]['views_bulk_operations'] = array(
        'title' => $data[$info['revision table']]['table']['group'],
        'group' => t('Bulk operations'),
        'help' => t('Provide a checkbox to select the row for bulk operations.'),
        'real field' => $info['entity keys']['revision'],
        'field' => array(
          'handler' => 'views_bulk_operations_handler_field_operations',
          'click sortable' => FALSE,
        ),
      );
    }
  }
}
