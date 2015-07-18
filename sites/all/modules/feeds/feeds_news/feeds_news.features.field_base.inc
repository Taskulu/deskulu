<?php
/**
 * @file
 * feeds_news.features.field_base.inc
 */

/**
 * Implements hook_field_default_field_bases().
 */
function feeds_news_field_default_field_bases() {
  $field_bases = array();

  // Exported field_base: 'field_feed_item_description'
  $field_bases['field_feed_item_description'] = array(
    'active' => 1,
    'cardinality' => 1,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'field_feed_item_description',
    'foreign keys' => array(
      'format' => array(
        'columns' => array(
          'format' => 'format',
        ),
        'table' => 'filter_format',
      ),
    ),
    'indexes' => array(
      'format' => array(
        0 => 'format',
      ),
    ),
    'locked' => 0,
    'module' => 'text',
    'settings' => array(),
    'translatable' => 1,
    'type' => 'text_with_summary',
  );

  return $field_bases;
}
