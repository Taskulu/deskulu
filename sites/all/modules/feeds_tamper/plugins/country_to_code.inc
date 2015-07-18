<?php

/**
 * @file
 * Convert a country name to ISO code.
 */

$plugin = array(
  'form' => 'feeds_tamper_country_to_code_form',
  'callback' => 'feeds_tamper_country_to_code_callback',
  'name' => 'Country to ISO code',
  'multi' => 'loop',
  'category' => 'Other',
  'default description' => 'Convert country to ISO code',
);

function feeds_tamper_country_to_code_form($importer, $element_key, $settings) {
  $form = array();
  $form['help'] = array(
    '#markup' => t('Converts this field from a country name string to the two character ISO 3166-1 alpha-2 code.'),
  );
  return $form;
}

function feeds_tamper_country_to_code_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  include_once DRUPAL_ROOT . '/includes/locale.inc';
  static $countries = array();

  if (empty($countries)) {
    $countries = country_get_list();
    array_walk($countries, '_feeds_tamper_country_to_code_strtolower');
    $countries = array_flip($countries);
  }

  // If it's already a valid country code, leave it alone.
  if (in_array($field, $countries)) {
    return;
  }

  // Trim whitespace, set to lowercase.
  $country = t(drupal_strtolower(trim($field)));
  if (isset($countries[$country])) {
    $field = $countries[$country];
  }
  else {
    // If country name cannot be found, return nothing.
    $field = '';
  }
}

/**
 * Callback for array_walk().
 */
function _feeds_tamper_country_to_code_strtolower(&$word) {
  $word = drupal_strtolower($word);
}
