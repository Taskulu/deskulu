<?php
/**
 * @file
 * bef_test_content.features.inc
 */

/**
 * Implements hook_views_api().
 */
function bef_test_content_views_api() {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function bef_test_content_node_info() {
  $items = array(
    'bef_test' => array(
      'name' => t('bef_test'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
