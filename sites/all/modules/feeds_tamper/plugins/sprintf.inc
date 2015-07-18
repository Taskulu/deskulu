<?php

/**
 * @file
 * Format a string with printf format.
 */

$plugin = array(
  'form' => 'feeds_tamper_sprintf_form',
  'callback' => 'feeds_tamper_sprintf_callback',
  'name' => 'Format string',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_sprintf_form($importer, $element_key, $settings) {
  $form = array();
  $form['format'] = array(
    '#type' => 'textfield',
    '#title' => t('Format'),
    '#default_value' => '%s',
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => FALSE,
    '#description' => t('See the <a href="http://www.php.net/manual/en/function.sprintf.php">sprintf</a> documentation for more details.'),
  );
  return $form;
}

function feeds_tamper_sprintf_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = sprintf($settings['format'], $field);
}
