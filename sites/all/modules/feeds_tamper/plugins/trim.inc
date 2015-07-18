<?php

/**
 * @file
 * Remove text or whitespace from the beginning, middle, or both sides of text.
 */

$plugin = array(
  'form' => 'feeds_tamper_trim_form',
  'callback' => 'feeds_tamper_trim_callback',
  'name' => 'Trim',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_trim_form($importer, $element_key, $settings) {
  $form = array();
  $form['mask'] = array(
    '#type' => 'textfield',
    '#title' => t('Characters to trim'),
    '#default_value' => isset($settings['mask']) ? $settings['mask'] : '',
    '#description' => t('The characters to remove from the string. If blank, then whitespace will be removed.'),
  );
  $form['side'] = array(
    '#type' => 'radios',
    '#title' => t('Side'),
    '#default_value' => isset($settings['side']) ? $settings['side'] : 'trim',
    '#options' => array('trim' => t('Both'), 'ltrim' => t('Left'), 'rtrim' => t('Right')),
  );
  return $form;
}

function feeds_tamper_trim_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if (!empty($settings['mask'])) {
    $field = $settings['side']($field, $settings['mask']);
  }
  else {
    $field = $settings['side']($field);
  }
}
