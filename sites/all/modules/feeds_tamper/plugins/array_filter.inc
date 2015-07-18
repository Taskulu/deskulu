<?php

/**
 * @file
 * Filters out empty values from an array.
 */

$plugin = array(
  'form' => 'feeds_tamper_array_filter_form',
  'callback' => 'feeds_tamper_array_filter_callback',
  'name' => 'Filter empty items',
  'multi' => 'direct',
  'category' => 'List',
);

function feeds_tamper_array_filter_form($importer, $element_key, $settings) {
  $form = array();
  $form['help'] = array(
    '#markup' => t('Filter out empty values from a list.'),
  );
  return $form;
}

function feeds_tamper_array_filter_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if (is_array($field)) {
    $field = array_values(array_filter($field));
  }
}
