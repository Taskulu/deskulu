<?php

/**
 * @file
 * Find/replace text using REGEX.
 */

$plugin = array(
  'form'     => 'feeds_tamper_find_replace_regex_form',
  'callback' => 'feeds_tamper_find_replace_regex_callback',
  'validate' => 'feeds_tamper_find_replace_regex_validate',
  'name'     => 'Find replace REGEX',
  'multi'    => 'direct',
  'category' => 'Text',
);

function feeds_tamper_find_replace_regex_form($importer, $element_key, $settings) {
  $form = array();

  $form['find'] = array(
    '#type' => 'textfield',
    '#title' => t('REGEX to find'),
    '#default_value' => isset($settings['find']) ? $settings['find'] : '',
    '#description' => t('A regular expression in the form: @regex', array('@regex' => '/<your regex here>/')),
    '#maxlength' => 1024,
  );

  $form['replace'] = array(
    '#type' => 'textfield',
    '#title' => t('Replacement pattern'),
    '#default_value' => isset($settings['replace']) ? $settings['replace'] : '',
    '#description' => t('The replacement pattern.'),
    '#maxlength' => 1024,
  );

  $form['limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Limit number of replacements'),
    '#default_value' => isset($settings['limit']) ? $settings['limit'] : '',
    '#description' => t('This sets an optional limit. Leave it blank for no limit.'),
  );

  return $form;
}

function feeds_tamper_find_replace_regex_validate(&$settings) {
  // Test the regex.
  $test = @preg_replace($settings['find'], '', 'asdfsadf');
  if ($test === NULL) {
    form_set_error('settings][find', 'Invalid regular expression.');
  }

  $settings['limit'] = trim($settings['limit']);

  if (empty($settings['limit'])) {
    $settings['real_limit'] = -1;
  }
  else {
    $settings['real_limit'] = $settings['limit'];
  }
  if (!is_numeric($settings['real_limit'])) {
    form_set_error('settings][limit', 'Limit must be an integer.');
  }
}

function feeds_tamper_find_replace_regex_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = preg_replace($settings['find'], $settings['replace'], $field, $settings['real_limit']);
}
