<?php

/**
 * @file
 * Set field to default value.
 */

$plugin = array(
  'form'     => 'feeds_tamper_default_value_form',
  'callback' => 'feeds_tamper_default_value_callback',
  'name'     => 'Set value or default value',
  'multi'    => 'direct',
  'category' => 'Other',
);

function feeds_tamper_default_value_form($importer, $element_key, $settings) {
  $form = array();

  $form['default_value'] = array(
    '#type' => 'textarea',
    '#title' => t('Value'),
    '#default_value' => isset($settings['default_value']) ? $settings['default_value'] : '',
    '#description' => t('This field will be set to the value specified.'),
  );

  $form['only_if_empty'] = array(
    '#type' => 'checkbox',
    '#title' => t('Only if empty'),
    '#default_value' => isset($settings['only_if_empty']) ? $settings['only_if_empty'] : 0,
    '#description' => t('This field will be set to the value specified only if the imported field is empty.'),
  );

  return $form;
}

function feeds_tamper_default_value_callback($source, $item_key, $element_key, &$field, $settings, $source) {
  // Setting a default value.
  if (!empty($settings['only_if_empty']) && !$field) {
    $field = $settings['default_value'];
  }
  elseif (empty($settings['only_if_empty'])) {
    $field = $settings['default_value'];
  }
}
