<?php
/**
 * Implements hook_views_data_alter().
 *
 * Replace field default handler (views_handler_field_field).
 */
function title_field_views_data_alter(&$data) {
  foreach (entity_get_info() as $entity_type => $entity_info) {
    if (!empty($entity_info['field replacement'])) {
      foreach ($entity_info['field replacement'] as $replacement) {
        if (isset($replacement['field']['field_name'])) {
          $field = field_info_field($replacement['field']['field_name']);
          $table = _field_sql_storage_tablename($field);
          if (isset($data[$table][$field['field_name']])) {
            $data[$table][$field['field_name']]['field']['handler'] = 'views_handler_title_field';
          }
        }
      }
    }
  }
}
