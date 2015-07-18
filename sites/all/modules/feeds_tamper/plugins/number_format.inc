<?php

/**
 * @file
 * Format a number with grouped thousands and decimal point.
 */

$plugin = array(
  'form' => 'feeds_tamper_number_format_form',
  'callback' => 'feeds_tamper_number_format_callback',
  'validate' => 'feeds_tamper_number_format_validate',
  'name' => 'Format a number',
  'multi' => 'loop',
  'category' => 'Number',
);

function feeds_tamper_number_format_form($importer, $element_key, $settings) {
  $form = array();
  $form['decimals'] = array(
    '#type' => 'textfield',
    '#title' => t('Decimals'),
    '#default_value' => isset($settings['decimals']) ? $settings['decimals'] : 0,
    '#description' => t('The number of decimal places.'),
  );
  $form['dec_point'] = array(
    '#type' => 'textfield',
    '#title' => t('Decimal point'),
    '#default_value' => isset($settings['dec_point']) ? $settings['dec_point'] : '.',
    '#description' => t('The character to use as the decimal point.'),
  );
  $form['thousands_sep'] = array(
    '#type' => 'textfield',
    '#title' => t('Thousands separator'),
    '#default_value' => isset($settings['thousands_sep']) ? $settings['thousands_sep'] : ',',
    '#description' => t('The character to use as the thousands separator.'),
  );
  return $form;
}

function feeds_tamper_number_format_validate(&$settings) {
  $settings['decimals'] = trim($settings['decimals']);

  if (!is_int($settings['decimals']) && ($settings['decimals'] !== (string) (int) $settings['decimals'])) {
    form_set_error('settings][decimals', t('Decimals field must be an integer.'));
  }
  else {
    $settings['decimals'] = (int) $settings['decimals'];
  }
}

function feeds_tamper_number_format_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if (is_numeric($field)) {
    $field = number_format($field, $settings['decimals'], $settings['dec_point'], $settings['thousands_sep']);
  }
}
