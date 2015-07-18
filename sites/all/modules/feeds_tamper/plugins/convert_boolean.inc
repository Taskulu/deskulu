<?php

/**
 * @file
 * Convert text value to boolean value.
 */

$plugin = array(
  'form' => 'feeds_tamper_convert_boolean_form',
  'callback' => 'feeds_tamper_convert_boolean_callback',
  'validate' => 'feeds_tamper_convert_boolean_validate',
  'name' => 'Convert to boolean',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_convert_boolean_form($importer, $element_key, $settings) {
  $form = array();
  $form['true_value'] = array(
    '#type' => 'textfield',
    '#title' => t('Truth value'),
    '#default_value' => isset($settings['true_value']) ? $settings['true_value'] : 'true',
  );
  $form['false_value'] = array(
    '#type' => 'textfield',
    '#title' => t('False value'),
    '#default_value' => isset($settings['false_value']) ? $settings['false_value'] : 'false',
  );
  $form['match_case'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match case'),
    '#default_value' => isset($settings['match_case']) ? $settings['match_case'] : FALSE,
  );
  $form['no_match'] = array(
    '#type' => 'radios',
    '#title' => t('If no match'),
    '#default_value' => isset($settings['no_match']) ? $settings['no_match'] : 'false',
    '#options' => array('true' => ('True'), 'false' => t('False'), 'null' => t('Null'), 'pass' => t('Do not modify'), 'other' => t('Other')),
    '#description' => t('The value to set if the true and false values do not match.'),
  );
  $form['other_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Other text'),
    '#default_value' => isset($settings['other_text']) ? $settings['other_text'] : '',
    '#states' => array(
      'visible' => array(
        'input[name="settings[no_match]"]' => array('value' => 'other'),
      ),
    ),
  );
  return $form;
}

function feeds_tamper_convert_boolean_validate(&$settings) {
  if (!$settings['match_case']) {
    $settings['false_value'] = drupal_strtolower($settings['false_value']);
    $settings['true_value'] = drupal_strtolower($settings['true_value']);
  }
  switch ($settings['no_match']) {
    case 'true':
      $settings['no_match_value'] = TRUE;
      break;
    case 'false':
      $settings['no_match_value'] = FALSE;
      break;
    case 'null':
      $settings['no_match_value'] = NULL;
      break;
    case 'other':
      $settings['no_match_value'] = $settings['other_text'];
      break;
  }
}

function feeds_tamper_convert_boolean_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  // Copy field value in case 'pass' is set.
  $match_field = $field;
  if (!$settings['match_case']) {
    $match_field = drupal_strtolower($match_field);
  }
  if ($match_field == $settings['true_value']) {
    $field = 1;
    return;
  }
  if ($match_field == $settings['false_value']) {
    $field = 0;
    return;
  }
  if ($settings['no_match'] == 'pass') {
    return;
  }
  $field = $settings['no_match_value'];
}
