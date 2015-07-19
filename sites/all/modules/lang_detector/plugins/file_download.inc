<?php


$plugin = array(
  'form' => 'lang_detector_file_download_form',
  // Optional validation callback.
  'callback' => 'lang_detector_file_download_callback',
  'name' => 'Download file',
  'multi' => 'loop',
  'category' => 'Other',
);

function lang_detector_file_download_form($importer, $element_key, $settings) {
  $form = array();
  $form['help']['#value'] = t('Downloads file from URL and replaces the URL with Drupal file URI.');
  return $form;
}

function lang_detector_file_download_callback($source, $item_key, $element_key, &$field, $settings) {
  $file = system_retrieve_file($field, 'private://ticket', TRUE);
  if ($file != FALSE) {
    $field = $file->uri;
  }
}

