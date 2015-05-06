<?php
/**
 * @file
 * Drush commands for Variable Realm.
 */

/**
 * Implements hook_drush_command().
 */
function variable_realm_drush_command() {
  $items['variable-realm-get'] = array(
    'description' => 'Get a variable for a specific realm and key.',
    'examples' => array(
      'drush variable-realm-get language es site_frontpage',
      'drush variable-realm-get language es',
    ),
    'arguments' => array(
      'domain_name' => 'The realm name for the variable.',
      'domain_key' => 'The domain key for the variable.',
      'variable_name' => 'Optional variable name variable to get. If not present list all variables for that realm and key.',
    ),
  );
  $items['variable-realm-set'] = array(
    'description' => 'Set a variable for a specific realm and key.',
    'examples' => array(
      'drush variable-realm-set language es site_frontpage /home ',
    ),
    'arguments' => array(
      'domain_name' => 'The realm name for the variable.',
      'domain_key' => 'The domain key for the variable.',
      'variable_name' => 'The name variable to set (e.g. site_frontpage).',
      'value' => 'The value to set for this variable.'
    ),
  );
  $items['variable-realm-del'] = array(
    'description' => 'Delete a variable for a specific realm and key.',
    'examples' => array(
      'drush variable-realm-del language es site_frontpage',
    ),
    'arguments' => array(
      'domain_name' => 'The realm name for the variable.',
      'domain_key' => 'The domain key for the variable.',
      'variable_name' => 'The name variable to set (e.g. site_frontpage).',
    ),
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function variable_realm_drush_help($section) {
  $items = variable_realm_drush_command();
  $name = str_replace('domain:', '', $section);
  if (isset($items[$name])) {
    return dt($items[$name]['description']);
  }
}

/**
 * Drush command callback.
 * Set realm's variables.
 */
function drush_variable_realm_set($realm_name, $realm_key, $variable_name, $value) {
  variable_realm_set($realm_name, $realm_key, $variable_name, $value);
  drush_print('Variable set.');
}

/**
 * Drush command callback.
 * Validate parameters
 */
function drush_variable_realm_set_validate($realm_name = NULL, $realm_key = NULL, $variable_name = NULL, $value = NULL) {
  if (is_null($realm_name) | is_null($realm_key) || is_null($variable_name) || is_null($value)) {
    return drush_set_error('variable_realm', dt('Please enter all four required arguments, "dc-set realm_name realm_key variable_name value".'));
  }
}

/**
 * Drush command callback.
 * Delete realm's variables.
 */
function drush_variable_realm_del($realm_name, $realm_key, $variable_name) {
  variable_realm_del($realm_name, $realm_key, $variable_name);
  drush_print('Variable deleted.');
}

/**
 * Drush command callback.
 * List realm's variables.
 */
function drush_variable_realm_get($realm_name, $realm_key, $variable_name) {
  if ($variable_name) {
    $variables[$variable_name] = variable_realm_get($realm_name, $realm_key, $variable_name);
  }
  else {
    $variables = variable_realm($realm_name, $realm_key)->variable_list();
  }

  foreach ($variables as $name => $value) {
    drush_print_pipe(drush_format($value, $name, 'export'));
    drush_print(drush_format($value, $name));
    $returns[$name] = $value;
  }

  if (empty($variables)) {
    return drush_set_error('No matching variable found.');
  }
  else {
    return $returns;
  }
}
