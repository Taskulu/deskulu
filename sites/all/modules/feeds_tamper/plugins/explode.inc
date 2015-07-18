<?php

/**
 * @file
 * Break up text on every specified delimeter.
 */

$plugin = array(
  'form' => 'feeds_tamper_explode_form',
  'callback' => 'feeds_tamper_explode_callback',
  'validate' => 'feeds_tamper_explode_validate',
  'name' => 'Explode',
  'multi' => 'direct',
  'category' => 'List',
);

function feeds_tamper_explode_form($importer, $element_key, $settings) {
  $form = array();
  $form['separator'] = array(
    '#type' => 'textfield',
    '#title' => t('String separator'),
    '#default_value' => isset($settings['separator']) ? $settings['separator'] : ',',
    '#description' => t('This will break up sequenced data into an array. For example, "a, b, c" would get broken up into the array(\'a\', \'b\', \'c\').
                        A space can be represented by %s, tabs by %t, and newlines by %n.'),
  );
  $form['limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Limit'),
    '#default_value' => isset($settings['limit']) ? $settings['limit'] : '',
    '#description' => t('If limit is set and positive, the returned items will contain a maximum of limit with the last item containing the rest of string.
                        If limit is negative, all components except the last -limit are returned. If the limit parameter is zero, then this is treated as 1. If limit is not set, then there will be no limit on the number of items returned.'),
  );
  return $form;
}

function feeds_tamper_explode_validate(&$settings) {
  $settings['real_separator'] = str_replace(array('%s', '%t', '%n'), array(' ', "\t", "\n"), $settings['separator']);
  $settings['limit'] = trim($settings['limit']);

  if (strlen($settings['limit']) && !is_numeric($settings['limit'])) {
    form_set_error('settings][limit', t('Limit must be an integer or blank.'));
  }
  elseif (strlen($settings['limit'])) {
    $settings['limit'] = (int) $settings['limit'];
  }
}

function feeds_tamper_explode_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if (!is_array($field)) {
    $field = array($field);
  }
  $out = array();
  foreach ($field as $f) {
    if ($settings['limit'] === '') {
      $out = array_merge($out, explode($settings['real_separator'], $f));
    }
    else {
      $out = array_merge($out, explode($settings['real_separator'], $f, $settings['limit']));
    }
  }
  $field = $out;
}
