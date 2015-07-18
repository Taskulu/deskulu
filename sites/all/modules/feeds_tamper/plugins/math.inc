<?php

/**
 * @file
 * Performs basic mathematical calculations on the imported value.
 */

$plugin = array(
  'form' => 'feeds_tamper_math_form',
  'validate' => 'feeds_tamper_math_validate',
  'callback' => 'feeds_tamper_math_callback',
  'name' => 'Mathematical operation',
  'category' => 'Number',
  'multi' => 'loop',
);

function feeds_tamper_math_form($importer, $element_key, $settings) {
  $form = array();

  $form['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#description' => t('The operation to apply to the imported value.'),
    '#required' => TRUE,
    '#options' => array(
      'addition' =>  '+',
      'subtraction' =>  '-',
      'multiplication' =>  '*',
      'division' =>  '/',
    ),
    '#default_value' => isset($settings['operation']) ? $settings['operation'] : '',
  );

  $form['flip'] = array(
    '#type' => 'checkbox',
    '#title' => t('Flip'),
    '#description' => t('Normally, the feed item will be processed like feed-value / setting-value. This option switches the order so that it is setting-value / feed-value.'),
    '#states' => array(
      'visible' => array(
        ':input[name="settings[operation]"]' => array(
          array('value' => 'subtraction'),
          array('value' => 'division'),
        ),
      ),
    ),
    '#default_value' => isset($settings['flip']) ? $settings['flip'] : FALSE,
  );

  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#required' => TRUE,
    '#description' => t('A numerical value.'),
    '#default_value' => isset($settings['value']) ? $settings['value'] : '',
  );

  $form['log'] = array(
    '#type' => 'checkbox',
    '#default_value' => !empty($settings['log']),
    '#title' => t('Log'),
    '#description' => t('Log to the Feed log and print a message when an invalid value is found.'),
  );

  return $form;
}

function feeds_tamper_math_validate(&$settings) {
  if (!is_numeric($settings['value'])) {
    form_set_error('settings][value', t('The value must be numeric.'));
  }
  elseif ($settings['operation'] === 'division' && empty($settings['flip']) && $settings['value'] == 0) {
    form_set_error('settings][value', t('Cannot divide by zero.'));
  }
}

function feeds_tamper_math_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if ($field === TRUE || $field === FALSE || $field === NULL) {
    $field = (int) $field;
  }

  if (!is_numeric($field)) {

    if (!empty($settings['log'])) {
      $source->log('feeds_tamper:math', 'Math plugin failed because @key was not numeric. Value: @field', array('@key' => $element_key, '@field' => $field));
      drupal_set_message(t('Math plugin failed because @key was not numeric. Value: @field', array('@key' => $element_key, '@field' => $field)));
    }

    return;
  }

  if (!empty($settings['flip'])) {

    switch ($settings['operation']) {
      case 'subtraction':
        $field = $settings['value'] - $field;
        return;

      case 'division':
        // Avoid divide by zero.
        if (empty($field)) {
          if (!empty($settings['log'])) {
            $source->log('feeds_tamper:math', 'Math plugin failed because @key was zero.', array('@key' => $element_key));
            drupal_set_message(t('Math plugin failed because @key was zero.', array('@key' => $element_key)));
          }

          return;
        }

        $field = $settings['value'] / $field;
        return;
    }
  }

  switch ($settings['operation']) {
    case 'addition':
      $field = $field + $settings['value'];
      return;

    case 'subtraction':
      $field = $field - $settings['value'];
      return;

    case 'multiplication':
      $field = $field * $settings['value'];
      return;

    case 'division':
      $field = $field / $settings['value'];
      return;
  }
}
