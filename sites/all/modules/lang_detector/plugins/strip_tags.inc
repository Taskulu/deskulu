<?php


$plugin = array(
  'form' => 'lang_detector_strip_tags',
  // Optional validation callback.
  'callback' => 'lang_detector_strip_tags_callback',
  'name' => 'Strip tags and style attributes',
  'multi' => 'loop',
  'category' => 'HTML',
);

function lang_detector_strip_tags($importer, $element_key, $settings) {
  $form = array();
  $form['help']['#value'] = t('Strip tags and style attributes.');
  return $form;
}

function lang_detector_strip_tags_callback($source, $item_key, $element_key, &$field, $settings) {
  $field = incoming_mail_strip_tags($field);
}

