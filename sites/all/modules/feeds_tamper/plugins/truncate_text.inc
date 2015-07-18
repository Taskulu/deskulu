<?php

/**
 * @file
 * Truncate text to a specified length.
 */

$plugin = array(
  'form' => 'feeds_tamper_truncate_text_form',
  'callback' => 'feeds_tamper_truncate_text_callback',
  'validate' => 'feeds_tamper_truncate_text_validate',
  'name' => 'Truncate',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_truncate_text_form($importer, $element_key, $settings) {
  $form = array();
  $form['num_char'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of characters'),
    '#default_value' => isset($settings['num_char']) ? $settings['num_char'] : '',
    '#description' => t('The number of characters the text will be limited to.'),
  );
  $form['ellipses'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ellipses'),
    '#default_value' => isset($settings['ellipses']) ? $settings['ellipses'] : FALSE,
    '#description' => t('Add ellipses (...) to the end of the truncated text.'),
  );
  $form['wordsafe'] = array(
    '#type' => 'checkbox',
    '#title' => t('Truncate on a word boundary'),
    '#default_value' => isset($settings['wordsafe']) ? $settings['wordsafe'] : FALSE,
    '#description' => t('Attempt to truncate on a word boundary.'),
  );
  return $form;
}

function feeds_tamper_truncate_text_validate($settings) {
  $settings['num_char'] = trim($settings['num_char']);
  if (!is_int($settings['num_char']) && ($settings['num_char'] !== (string) (int) $settings['num_char'])) {
    form_set_error('settings][num_char', t('Needs to be an integer.'));
  }
}

function feeds_tamper_truncate_text_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if (!empty($settings['wordsafe'])) {
    $field = truncate_utf8($field, $settings['num_char'], $settings['wordsafe'], $settings['ellipses']);
  }
  else {
    $field = truncate_utf8($field, $settings['num_char'], FALSE, $settings['ellipses']);
  }
}
