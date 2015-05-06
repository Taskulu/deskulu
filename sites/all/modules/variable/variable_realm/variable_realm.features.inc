<?php

/**
 * @file
 * Features support for Variable store.
 */

/**
 * Implements hook_features_export_options().
 */
function variable_realm_features_export_options() {
  foreach (variable_realm_info() as $name => $realm) {
    $controller = variable_realm_controller($name);
    $realm_keys = $controller->getAllKeys();
    if (count($realm_keys) > 1) {
      $options[$name] = $controller->getTitle() . ': ' . t('all');
    }
    foreach ($realm_keys as $key => $title) {
      $options[$name . ':' . $key] = $controller->getTitle() . ': ' . $title;
    }
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function variable_realm_features_export($data, &$export, $module_name) {
  $export['dependencies']['variable_realm'] = 'variable_realm';
  $list = variable_realm_features_selection($data);
  foreach ($list as $machine_name) {
    // Add module that provides the exported realm
    list($realm, $key) = explode(':', $machine_name, 2); // limit 2 because keys can contain colons
    if ($realm_info = variable_realm_info($realm)) {
      $export['dependencies'][$realm_info['module']] = $realm_info['module'];
    }
    $export['features']['variable_realm'][$machine_name] = $machine_name;
  }
  return array();
}

/**
 * Processes export data selections consistently.
 *
 * @param $data
 *   Array of selections from the features component form.
 *
 * @return
 *   An array of realms, keyed by machine_name.
 */
function variable_realm_features_selection($data) {
  $list = array();
  foreach ($data as $machine_name) {
    if (strpos($machine_name, ':')) {
      $list[] = $machine_name;
    }
    else {
      foreach (variable_realm_keys($machine_name) as $key => $title) {
        $list[] = "$machine_name:$key";
      }
    }
  }
  return $list;
}

/**
 * Implements hook_features_export_render().
 */
function variable_realm_features_export_render($module_name, $data, $export = NULL) {
  variable_realm_features_load($module_name, 'variable_realm_default_variables', FALSE);
  $code = array();
  $code[] = '$realm_variables = array();';
  foreach ($data as $machine_name) {
    list($realm, $key) = explode(':', $machine_name, 2); // limit 2 because keys can contain colons
    $variable_realm = variable_realm($realm, $key);
    $variables = $variable_realm->variable_list();
    $code[] = "  \$realm_variables['{$realm}']['{$key}'] = " . features_var_export($variables) .";";
  }
  $code[] = "\nreturn \$realm_variables;";
  $output = implode("\n", $code);
  return array('variable_realm_default_variables' => $output);
}

/**
 * Implements hook_features_revert().
 */
function variable_realm_features_revert($module) {
  return variable_realm_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 */
function variable_realm_features_rebuild($module) {
  if ($defaults = variable_realm_features_load($module, 'variable_realm_default_variables', TRUE)) {
    foreach ($defaults as $realm => $realm_data) {
      foreach ($realm_data as $key => $variables) {
        $variable_realm = variable_realm($realm, $key);
        foreach ($variables as $name => $value) {
          $variable_realm->variable_set($name, $value);
        }
      }
    }
  }
}

/**
 * Features doesn't know how to load custom includes.
 *
 * @param $module
 *  The name of the feature to load.
 * @param $hook
 *  The name of the domain hook.
 * @param $return
 *  Boolean indicator to return the results of the function.
 *
 * @return
 *  The results of the $hook implemenation, if requested.
 */
function variable_realm_features_load($module, $hook, $return = TRUE) {
  // Features does not handle module loading of custom files.
  module_load_include('inc', $module, $module . '.variable');
  $function = $module . '_' . $hook;
  if ($return && function_exists($function)) {
    return $function();
  }
}
