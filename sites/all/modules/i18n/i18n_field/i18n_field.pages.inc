<?php
/**
 * @file
 * Translation page for fields.
 */

/**
 * Field translation page
 *
 * We need to translate field and field instance.
 */
function i18n_field_page_translate($instance, $language = NULL) {
  module_load_include('inc', 'i18n_string', 'i18n_string.pages');
  if (!$language) {
    // Overview page will be the regular one
    return i18n_string_translate_page_object('field_instance', $instance);
  }
  else {
    // Because of some weird menu mapping for comment fields language object loader is not working.
    $language = i18n_language_object($language);
    drupal_set_title(t('Translate to !language', array('!language' => i18n_language_name($language->language))));
    //return drupal_get_form('i18n_field_page_translate_form', $instance, $language->language);
    // Create form with two tabs, one for instance, once for field.
    $groups = array(
      'instance' => t('Field instance'),
      'field' => t('Field settings'),
    );
    // Field instance
    $instance_object = i18n_object('field_instance', $instance);
    $strings['instance'] = $instance_object->get_strings(array('empty' => TRUE));
    // Field settings
    $field_object = i18n_object('field', $instance);
    $strings['field'] = $field_object->get_strings(array('empty' => TRUE));
    return drupal_get_form('i18n_string_translate_page_form', $strings, $language->language, $groups);
  }
}
