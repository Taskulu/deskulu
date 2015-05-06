<?php
/**
 * @file
 * Variable API module. Definition for Drupal core variables
 */

/**
 * Implements hook_variable_info().
 */
function locale_variable_info($options) {
  // This variable will be altered by translation module. See translation.variable.inc
  $variables['language_content_type_[node_type]'] = array(
    'type' => 'multiple',
    'title' => t('Multilingual support'),
    'repeat' => array(
      'type' => 'enable',
       // Set options here so translation module can add some more
      'options' => array(t('Disabled'), t('Enabled')),
    ),
    'description' => t('Enable multilingual support for this content type. If enabled, a language selection field will be added to the editing form, allowing you to select from one of the <a href="!languages">enabled languages</a>. If disabled, new posts are saved with the default language. Existing content will not be affected by changing this option.', array('!languages' => url('admin/config/regional/language', $options)), $options),
    'group' => 'node_type_settings',
  );
  $variables['language_default'] = array(
    'title' => t('Site default language'),
    'type' => 'select',
    'group' => 'regional_settings',
    'options callback' => 'locale_variable_options_language',
    'format callback' => 'locale_variable_language_default_format',
    'element callback' => 'locale_variable_language_default_element',
    'default callback' => 'locale_variable_language_default',
  );
  return $variables;
}

/**
 * Implements hook_variable_type_info().
 */
function locale_variable_type_info() {
  // Language code
  $type['language'] = array(
    'title' => t('Language'),
    'options callback' => 'locale_variable_options_language',
    'type' => 'select',
  );
  return $type;
}

/**
 * Callback for all languages
 */
function locale_variable_options_language($variable, $options) {
  return locale_language_list('name', TRUE);
}

/**
 * Form element for site default language
 */
function locale_variable_language_default_element($variable, $options) {
  $element = variable_form_element_options($variable, $options);
  // Default needs to be language code for the radios to work.
  $element['#default_value'] = $element['#default_value']->language;
  // Since 'value callback' is useless as it is run before validation, we use validate
  // callback to transform language code into an object.
  $element['#element_validate'][] = 'locale_variable_language_element_validate';
  return $element;
}

/**
 * Get language default.
 */
function locale_variable_language_default($variable, $options) {
  return language_default();
}

/**
 * Format language object variable
 */
function locale_variable_language_default_format($variable, $options = array()) {
  return !empty($variable['value']) ? check_plain($variable['value']->name) : t('None');
}


/**
 * Replace language code by language object on submission.
 *
 * This runs alter default element validation so we know it is a valid checkbox option.
 */
function locale_variable_language_element_validate($element, &$form_state, $form) {
  $languages = language_list();
  $language = $languages[$element['#value']];
  form_set_value($element, $language, $form_state);
}
