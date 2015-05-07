<?php
/**
 * @file
 * Views translation page
 */

// This needs i18n_string page library and some views admin functions
require_once drupal_get_path('module', 'i18n_string') . '/i18n_string.pages.inc';
require_once drupal_get_path('module', 'views') . '/includes/admin.inc';

/**
 * Provide a better organized views translation page grouping strings by display.
 *
 * @see i18n_string_translate_page_object()
 *
 * @param $view
 *   View object
 * @param $langcode
 *   Language code
 */
function i18nviews_object_translate_page($view, $langcode) {
  // Build i18n wrapper and get all strings
  $object = i18n_object('views', $view);
  $strings = $object->get_strings(array('empty' => TRUE));

  drupal_set_title(t('Translate to @language', array('@language' => i18n_language_name($langcode))));
  // For each string object, objectid = display
  return drupal_get_form('i18nviews_translate_page_form', $view, $strings, $langcode);
}

/**
 * Form builder callback for in-place string translation.
 */
function i18nviews_translate_page_form($form, &$form_state, $view, $strings, $langcode) {
  // Group strings by display
  $groups = array();
  foreach ($strings as $key => $i18nstring) {
    $groups[$i18nstring->objectid][$key] = $i18nstring;
  }
  // The base form adds proper validation and submit callbacks
  $form = i18n_string_translate_page_form_base($form, $langcode);
  // Build the form with one vertical tab per display
  $form['display'] = array(
    '#type' => 'vertical_tabs',
  );
  foreach ($groups as $display_id => $display_strings) {
    $form['groups'][$display_id] = array(
      '#type' => 'fieldset',
      '#title' => views_ui_get_display_label($view, $display_id),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'display',
    );
    $form['groups'][$display_id]['strings'] = i18n_string_translate_page_form_strings($display_strings, $langcode);
  }
  $form['submit']['#weight'] = 1;
  $form['#submit'][] = 'views_invalidate_cache';
  return $form;
}
