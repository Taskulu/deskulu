<?php

/**
 * @file
 * Rewrite a field using tokens.
 */

$plugin = array(
  'form'     => 'feeds_tamper_rewrite_form',
  'callback' => 'feeds_tamper_rewrite_callback',
  'name'     => 'Rewrite',
  'multi'    => 'skip',
  'category' => 'Other',
);

function feeds_tamper_rewrite_form($importer, $element_key, $settings) {
  $form = array();
  $replace = array();

  foreach (feeds_tamper_get_unique_source_list($importer) as $source) {
    $replace[] = '[' . $source . ']';
  }

  $form['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Replacement pattern'),
    '#default_value' => isset($settings['text']) ? $settings['text'] : '',
  );
  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available Replacement Patterns'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
    '#value' => theme('item_list', array('items' => $replace)),
  );
  return $form;
}

function feeds_tamper_rewrite_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $trans = array();
  $item = $result->items[$item_key];
  foreach ($item as $key => $value) {
    $trans['[' . $key . ']'] = is_array($value) ? reset($value) : $value;
  }
  $field = strtr($settings['text'], $trans);
}
