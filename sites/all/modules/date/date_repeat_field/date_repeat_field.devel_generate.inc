<?php
/*
 * @file
 * Handling of devel generate functionality for repeating dates.
 */

/**
 * Implements hook_date_field_insert().
 *
 * A substitute for hook_devel_generate to handle repeating dates.
 */
function date_repeat_field_date_field_insert(&$items, $context) {

  $entity_type = $context['entity_type'];
  $entity = $context['entity'];
  $field = $context['field'];
  $instance = $context['instance'];
  $langcode = $context['langcode'];

  // The first value was already created by the regular Devel Generate code.
  // Skipping doing anything if there is no value for this field.
  if (empty($items)) {
    return;
  }
  $item = $items[0];

  // Unset any previous dates past the first one.
  $count = count($items);
  for ($i = 1; $i < $count; $i++) {
    unset($items[$i]);
  }

  // Compute repeating date values.
  module_load_include('inc', 'date_repeat', 'date_repeat_calc');
  module_load_include('inc', 'date_api', 'date_api_ical');

  $increment = $instance['widget']['settings']['increment'];
  $timezone = date_get_timezone($field['settings']['tz_handling'], $item['timezone']);
  $timezone_db = date_get_timezone_db($field['settings']['tz_handling']);

  switch ($field['type']) {
    case 'date':
      $format = DATE_FORMAT_ISO;
      break;
    case 'datestamp':
      $format = DATE_FORMAT_UNIX;
      break;
    case 'datetime':
      $format = DATE_FORMAT_DATETIME;
      break;
  }

  $start = new dateObject($item['value'], $timezone_db, $format);
  $start2 = new dateObject($item['value2'], $timezone_db, $format);

  // Create a repeating date rule.
  $duration = $start->difference($start2);
  $form_values = array();

  // Create the default case more frequently than case 1 or 2.
  $which = mt_rand(0, 10);

  $max_items = mt_rand(3, 10);

  $intervals = array_keys(date_repeat_interval_options());
  unset($intervals[0]);
  $interval = $intervals[mt_rand(1, 3)];
  switch ($which) {
    case 1:
      $mo = mt_rand(1, 28);
      $options = array('YEARLY', 'MONTHLY');
      $freq = date_content_generate_key($options);
      $freq = $options[$freq];
      $form_values['FREQ'] = $freq;
      // Make sure we'll find a match in our range.
      if ($freq == 'YEARLY') {
        $interval = 1;
      }
      $form_values['BYMONTHDAY'] = array($mo);
      break;
    case 2:
      $mo = mt_rand(1, 12);
      $options = array('YEARLY', 'MONTHLY');
      $freq = date_content_generate_key($options);
      $freq = $options[$freq];
      $form_values['FREQ'] = $freq;
      // Make sure we'll find a match in our range.
      if ($freq == 'YEARLY') {
        $interval = 1;
      }
      $form_values['BYMONTH'] = array($mo);
      break;
    default:
      $dows = array_keys(date_content_repeat_dow_options());
      $day = date_content_generate_key($dows);
      $dow = $dows[$day];
      $options = array('MONTHLY', 'DAILY', 'WEEKLY');
      $freq = date_content_generate_key($options);
      $freq = $options[$freq];
      $form_values['FREQ'] = $freq;
      $form_values['BYDAY'] = array($dow);
      break;
  }

  $form_values['INTERVAL'] = $interval;

  switch ($freq) {
    case 'YEARLY':
      $period = 'year';
      break;
    case 'MONTHLY':
      $period = 'month';
      break;
    case 'WEEKLY':
      $period = 'week';
      break;
    default:
      $period = 'day';
      break;

  }

  $form_values['UNTIL'] = array();
  $form_values['COUNT'] = $max_items;

  $rrule = date_api_ical_build_rrule($form_values);
  $items[0]['rrule'] = $rrule;

  $values = date_repeat_build_dates($rrule, $form_values, $field, $item);

  $items += $values;

}

function date_content_generate_key($array) {
  $keys = array_keys($array);
  $min = array_shift($keys);
  $max = array_pop($keys);
  return mt_rand($min, $max);
}

/**
 * Helper function for BYDAY options.
 *
 * Creates options like -1SU and 2TU
 * Omit options that won't find many matches, like 5th Sunday.
 */
function date_content_repeat_dow_options() {
  $options = array();
  foreach (date_repeat_dow_count_options() as $count_key => $count_value) {
    foreach (date_repeat_dow_day_options() as $dow_key => $dow_value) {
      if ($count_key != 5 && $count_key != -5) {
        $options[$count_key . $dow_key] = $count_value . ' ' . $dow_value;
      }
    }
  }
  return $options;
}