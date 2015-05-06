<?php

/**
 * @file
 * Examples and test folder for migration into date fields.
 */

/**
 * Implements hook_node_info().
 */
function date_migrate_example_node_info() {
  $items = array(
    'date_migrate_example' => array(
      'name' => t('Migrate example - dates'),
      'base' => 'node_content',
      'description' => t('This content type is used for demonstrating and testing migration into Date fields.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
