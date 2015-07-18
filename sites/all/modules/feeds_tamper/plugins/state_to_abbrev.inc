<?php

/**
 * @file
 * Convert a full U.S. state name to a 2-letter abbreviation.
 */

$plugin = array(
  'form' => 'feeds_tamper_state_to_abbrev_form',
  'callback' => 'feeds_tamper_state_to_abbrev_callback',
  'name' => 'Full U.S. state name to abbrev.',
  'multi' => 'loop',
  'category' => 'Other',
  'default description' => 'Full state name to abbrev',
);

function feeds_tamper_state_to_abbrev_form($importer, $element_key, $settings) {
  $form = array();
  $form['help'] = array(
    '#markup' => t('Converts this field from a full state name name string to the two character abbreviation.'),
  );

  return $form;
}

function feeds_tamper_state_to_abbrev_callback($result, $item_key, $element_key, &$field, $settings) {
  static $states;

  if (!$states) {
    $states = _feeds_tamper_state_to_abbrev_states_get_list();
  }

  // If it's already a valid state abbreviation, leave it alone.
  if (in_array($field, $states)) {
    return;
  }

  // Trim whitespace, set to lowercase.
  $state = drupal_strtolower(trim($field));

  if (isset($states[$state])) {
    $field = $states[$state];
  }
  else {
    // If state name cannot be found, return nothing.
    $field = '';
  }
}

function _feeds_tamper_state_to_abbrev_states_get_list() {
  $states = array(
    'alabama' => 'AL',
    'alaska' => 'AK',
    'arizona' => 'AZ',
    'american samoa' => 'AS',
    'arkansas' => 'AR',
    'california' => 'CA',
    'colorado' => 'CO',
    'connecticut' => 'CT',
    'delaware' => 'DE',
    'district of columbia' => 'DC',
    'federated states of micronesia' => 'FM',
    'florida' => 'FL',
    'georgia' => 'GA',
    'guam' => 'GU',
    'hawaii' => 'HI',
    'idaho' => 'ID',
    'illinois' => 'IL',
    'indiana' => 'IN',
    'iowa' => 'IA',
    'kansas' => 'KS',
    'kentucky' => 'KY',
    'louisiana' => 'LA',
    'maine' => 'ME',
    'maryland' => 'MD',
    'massachusetts' => 'MA',
    'marshall islands' => 'MH',
    'michigan' => 'MI',
    'minnesota' => 'MN',
    'mississippi' => 'MS',
    'missouri' => 'MO',
    'montana' => 'MT',
    'nebraska' => 'NE',
    'nevada' => 'NV',
    'new hampshire' => 'NH',
    'new jersey' => 'NJ',
    'new mexico' => 'NM',
    'new york' => 'NY',
    'north carolina' => 'NC',
    'north dakota' => 'ND',
    'northern mariana islands' => 'mp',
    'ohio' => 'OH',
    'oklahoma' => 'OK',
    'oregon' => 'OR',
    'pennsylvania' => 'PA',
    'palau' => 'PW',
    'puerto rico' => 'PR',
    'rhode island' => 'RI',
    'south carolina' => 'SC',
    'south dakota' => 'SD',
    'tennessee' => 'TN',
    'texas' => 'TX',
    'utah' => 'UT',
    'vermont' => 'VT',
    'virginia' => 'VA',
    'virgin islands' => 'VI',
    'washington' => 'WA',
    'west virginia' => 'WV',
    'wisconsin' => 'WI',
    'wyoming' => 'WY',
  );

  return $states;
}
