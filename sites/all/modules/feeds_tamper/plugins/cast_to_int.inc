<?php

/**
 * @file
 * Cast the value to an integer.
 */

$plugin = array(
  'form' => 'feeds_tamper_cast_to_int_form',
  'callback' => 'feeds_tamper_cast_to_int_callback',
  'name' => 'Cast to integer',
  'multi' => 'loop',
  'category' => 'Number',
);

function feeds_tamper_cast_to_int_form($importer, $element_key, $settings) {
  $form = array();
  $form['info'] = array(
    '#markup' => t('This plugin will convert any value to its integer form.'),
  );
  return $form;
}

function feeds_tamper_cast_to_int_callback($result, $item_key, $element_key, &$field, $settings) {
  $field = (int) $field;
}
