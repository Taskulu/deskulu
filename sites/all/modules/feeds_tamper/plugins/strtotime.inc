<?php

/**
 * @file
 * Convert string to timestamp.
 */

$plugin = array(
  'form' => 'feeds_tamper_strtotime_form',
  'callback' => 'feeds_tamper_strtotime_callback',
  'name' => 'String to Unix timestamp',
  'multi' => 'loop',
  'category' => 'Date/time',
);

function feeds_tamper_strtotime_form($importer, $element_key, $settings) {
  $form = array();
  $form['html'] = array(
    '#markup' => t('This will take a string containing an English date format and convert it into a Unix Timestamp.'),
  );
  return $form;
}

function feeds_tamper_strtotime_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = strtotime($field);
}
