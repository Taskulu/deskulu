<?php

/**
 * @file
 * Pad a string to a certain length with another string.
 */

$plugin = array(
  'form' => 'feeds_tamper_str_pad_form',
  'callback' => 'feeds_tamper_str_pad_callback',
  'validate' => 'feeds_tamper_str_pad_validate',
  'name' => 'Pad a string',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_str_pad_form($importer, $element_key, $settings) {
  $form = array();
  $form['pad_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Pad length'),
    '#default_value' => isset($settings['pad_length']) ? $settings['pad_length'] : 10,
    '#description' => t('If the input value has a length less than this, it will use the string below to increase the length.'),
  );
  $form['pad_string'] = array(
    '#type' => 'textfield',
    '#title' => t('Pad string'),
    '#default_value' => isset($settings['pad_string']) ? $settings['pad_string'] : '',
    '#description' => t('The string to use for padding. If blank, a space will be used.'),
  );
  $form['pad_type'] = array(
    '#type' => 'radios',
    '#title' => t('Pad type'),
    '#options' => array(STR_PAD_RIGHT => t('Right'), STR_PAD_LEFT => t('Left'), STR_PAD_BOTH => t('Both')),
    '#default_value' => isset($settings['pad_type']) ? $settings['pad_type'] : STR_PAD_RIGHT,
  );
  return $form;
}

function feeds_tamper_str_pad_validate(&$settings) {
  $settings['pad_length'] = trim($settings['pad_length']);

  if (!is_int($settings['pad_length']) && ($settings['pad_length'] !== (string) (int) $settings['pad_length'])) {
    form_set_error('settings][pad_length', t('Pad length field must be an integer.'));
  }
  else {
    $settings['pad_length'] = (int) $settings['pad_length'];
  }
  if ($settings['pad_string'] === '') {
    $settings['real_pad_string'] = ' ';
  }
  else {
    $settings['real_pad_string'] = $settings['pad_string'];
  }
}

function feeds_tamper_str_pad_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = str_pad($field, $settings['pad_length'], $settings['real_pad_string'], $settings['pad_type']);
}
