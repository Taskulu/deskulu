<?php

/**
 * @file
 * Encode HTML entities.
 */

$plugin = array(
  'form' => 'feeds_tamper_html_entity_encode_form',
  'callback' => 'feeds_tamper_html_entity_encode_callback',
  'name' => 'HTML entity encode',
  'multi' => 'loop',
  'category' => 'HTML',
);

function feeds_tamper_html_entity_encode_form($importer, $element_key, $settings) {
  $form = array();
  $form['html'] = array(
    '#markup' => t('This will convert all HTML special characters such as &gt; and &amp; to &amp;gt; and &amp;apm;.'),
  );
  return $form;
}

function feeds_tamper_html_entity_encode_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  $field = check_plain($field);
}
