<?php
/**
 * @file
 * Variable information
 */

/**
 * Implements hook_variable_info().
 */
function i18n_variable_variable_info($options = array()) {
  $variables['i18n_variable_conf'] = array(
    'title' => t('Multilingual variables, main variable names.', array(), $options),
    'type' => 'array',
    'group' => 'i18n',
  );
  $variables['i18n_variable_list'] = array(
    'title' => t('Multilingual variables, real variable names.', array(), $options),
    'type' => 'array',
    'group' => 'i18n',
  );
  return $variables;
}



