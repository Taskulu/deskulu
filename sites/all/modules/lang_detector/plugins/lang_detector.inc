<?php


$plugin = array(
  'form' => 'lang_detector_plugin_form',
  // Optional validation callback.
  'callback' => 'lang_detector_plugin_callback',
  'name' => 'Detect language of text',
  'multi' => 'loop',
  'category' => 'Text',
);

function lang_detector_plugin_form($importer, $element_key, $settings) {
  $form = array();
  $form['help']['#value'] = t('Detects the language of the given text.');
  return $form;
}

function lang_detector_plugin_callback($source, $item_key, $element_key, &$field, $settings) {
  $lang = lang_detector_detect($field);
  if ($lang == 'fa') {
    $field = $lang;
  }
  else {
    $field = 'en';
  }
}

