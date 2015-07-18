<?php
/**
 * @file
 * feeds_news.features.field_instance.inc
 */

/**
 * Implements hook_field_default_field_instances().
 */
function feeds_news_field_default_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'node-feed_item-field_feed_item_description'
  $field_instances['node-feed_item-field_feed_item_description'] = array(
    'bundle' => 'feed_item',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'hidden',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 0,
      ),
      'full' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
      'rss' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
      'teaser' => array(
        'label' => 'hidden',
        'module' => 'text',
        'settings' => array(
          'trim_length' => 600,
        ),
        'type' => 'text_trimmed',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_feed_item_description',
    'label' => 'Description',
    'required' => 0,
    'settings' => array(
      'display_summary' => 0,
      'text_processing' => 1,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 20,
        'summary_rows' => 5,
      ),
      'type' => 'text_textarea_with_summary',
      'weight' => -4,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Description');

  return $field_instances;
}
