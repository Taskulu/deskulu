<?php

/**
 * @file
 * Convert timestamp to date with custom format.
 */

$plugin = array(
  'form' => 'feeds_tamper_timetodate_form',
  'callback' => 'feeds_tamper_timetodate_callback',
  'validate' => 'feeds_tamper_timetodate_validate',
  'name' => 'Unix timestamp to Date',
  'multi' => 'loop',
  'category' => 'Date/time',
);

function feeds_tamper_timetodate_form($importer, $element_key, $settings) {
  $form = array();
  $form['date_format'] = array(
    '#type' => 'textfield',
    '#title' => t('Date format'),
    '#default_value' => isset($settings['date_format']) ? $settings['date_format'] : '',
    '#description' => t('A user-defined php date format string like "m-d-Y H:i". See the <a href="@url">PHP manual</a> for available options.', array('@url' => 'http://www.php.net/manual/function.date.php')),
  );
  return $form;
}

function feeds_tamper_timetodate_validate(&$settings) {
  if (empty($settings['date_format'])) {
    form_set_error('settings][date_format', t('Please enter a valid date format string.'));
  }
}

function feeds_tamper_timetodate_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = date($settings['date_format'], $field);
}
