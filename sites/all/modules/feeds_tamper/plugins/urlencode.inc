<?php

/**
 * @file
 * URL Encode text.
 */

$plugin = array(
  'form' => 'feeds_tamper_urlencode_form',
  'callback' => 'feeds_tamper_urlencode_callback',
  'validate' => 'feeds_tamper_urlencode_validate',
  'multi' => 'loop',
  'name' => t('URL Encode'),
  'category' => t('Text'),
 );

function feeds_tamper_urlencode_form($importer, $element_key, array $settings, array &$form_state) {
  $form = array();
  $form['help']['#markup'] = t('Run values through the <a href="!url">urlencode()</a> function.', array('!url' => 'http://us3.php.net/urlencode'));
  $form['method'] = array(
    '#type' => 'radios',
    '#title' => t('Encode method'),
    '#options' => array(
      'raw' => t('Raw'),
      'legacy' => t('Legacy: Encodes spaces into + symbols.'),
    ),
    '#default_value' => !empty($settings['method']) ? $settings['method'] : 'raw',
  );
  return $form;
}

function feeds_tamper_urlencode_validate(array &$settings) {
  if ($settings['method'] === 'raw') {
    $settings['function'] = 'rawurlencode';
  }
  else {
    $settings['function'] = 'urlencode';
  }
}

function feeds_tamper_urlencode_callback($source, $item_key, $element_key, &$field, array $settings) {
  $field = $settings['function']($field);
}
