<?php
/**
 * @file
 * Variable hooks.
 */

/**
 * Implements hook_variable_info().
 */
function variable_realm_variable_info($options) {
  $variables['variable_realm_list_[variable_realm]'] = array(
    'type' => 'multiple',
    'group' => 'variable',
    'multiple' => 'variable_realm',
    'title' => t('Realm list', array(), $options),
    'description' => t('List of variables that can be set for a realm.', array(), $options),
    'repeat' => array(
      'type' => 'array',
    ),
  );
  $variables['variable_realm_weight_[variable_realm]'] = array(
    'type' => 'multiple',
    'group' => 'variable',
    'multiple' => 'variable_realm',
    'title' => t('Realm weight', array(), $options),
    'description' => t('Override default weight for realm variables.', array(), $options),
    'repeat' => array(
      'type' => 'number',
    ),
  );
  return $variables;
}

/**
 * Implements hook_variable_type_info().
 */
function variable_realm_variable_type_info() {
  $type['variable_realm'] = array(
    'title' => t('Variable realm'),
    'type' => 'select',
    'options callback' => 'variable_realm_list',
  );
  return $type;
}

/**
 * Implements hook_variable_settings_form_alter().
 */
function variable_realm_variable_settings_form_alter(&$form, &$form_state, $form_id) {
  form_load_include($form_state, 'form.inc', 'variable_realm');
  foreach (_variable_realm_variable_settings_form_list() as $realm_name => $variables) {
    if ($realm_variables = _variable_realm_variable_settings_form_alter($form, $realm_name, variable_children($variables))) {
      $form['#realm_variables'][$realm_name] = $realm_variables;
    }
  }
  if (!empty($form['#realm_variables'])) {
    array_unshift($form['#submit'], 'variable_realm_variable_settings_form_submit');
    // Add form switcher and current key for each realm.
    _variable_realm_variable_settings_form_switcher($form);
  }
}
