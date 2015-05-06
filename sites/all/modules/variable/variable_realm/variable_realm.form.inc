<?php
/**
 * @file
 * Administrative forms for variable realms.
 */

/**
 * Select variables for realm.
 */
function variable_realm_select_variables_form($form, &$form_state, $realm_name) {
  $controller = variable_realm_controller($realm_name);
  $current = $controller->getEnabledVariables();
  $optional = $controller->getAvailableVariables();
  // The final list will be the sum of both lists. We may have unknown variables
  // we want to preserve.
  $list = array_unique(array_merge($optional, $current));
  $form['realm_name'] = array('#type' => 'value', '#value' => $realm_name);
  $form['message']['select']['#markup'] = '<h3>' . t('Select variables to be set for this realm.') . '</h3>';
  if ($current) {
    $form['message']['current']['#markup']  = '<p>' . t('Currently selected variables are: <em>!variables</em>', array('!variables' => variable_list_name($current))) . '</p>';
  }
  $form['variables'] = array(
    '#type' => 'vertical_tabs',
    '#tree' => TRUE,
  );
  $variable_groups = variable_group_variables($list);
  foreach ($variable_groups as $group => $group_list) {
    $group_info = variable_get_group($group);
    $group_current = array_intersect($group_list, $current);
    $form['variables'][$group] = array(
      '#type' => 'fieldset',
      '#title' => $group_info['title'],
      '#theme' => 'variable_table_select',
      '#collapsible' => TRUE, '#collapsed' => TRUE,
    );
    foreach ($group_list as $name) {
      // Variable names may clash with form element names, so we need to replace '[' and ']'
      $safename = str_replace(array('[', ']'), array('<', '>'), $name);
      $form['variables'][$group][$safename] = array(
        '#type' => 'checkbox',
        '#default_value' => in_array($name, $group_current),
        '#variable_name' => $name,
        '#parents' => array('variables', $safename),
      );
    }
  }
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));
  return $form;
}

/**
 * Select variables for realm.
 */
function variable_realm_select_variables_form_submit($form, &$form_state) {
  // Get realm name and current list of variables.
  $realm_name = $form_state['values']['realm_name'];
  $controller = variable_realm_controller($realm_name);
  $old_variables = $controller->getEnabledVariables();
  // Get main variable names
  $variables = $form_state['values']['variables'];
  unset($variables['variables__active_tab']);
  $variables = array_keys(array_filter($variables));
  // Translate variable names
  foreach ($variables as $index => $name) {
    $variables[$index] = str_replace(array('<', '>'), array('[', ']'), $name);
  }
  // Hook for modules to alter this variable list.
  drupal_alter('variable_realm_variable_list', $variables, $realm_name);
  // And save the list to a variable.
  $controller->setRealmVariable('list', $variables);
  // Spawn multiple variables and translate into actual variables
  $new_list = variable_children($variables);
  // Delete variables from realm that are not in the new list.
  $old_list = variable_children($old_variables);
  foreach (array_diff($old_list, $new_list) as $name) {
    $controller->deleteVariable($name);
    drupal_set_message(t('Deleted existing values of %name from realm variables.', array('%name' => $name)));
  }
}

/**
 * Edit variables for realm.
 */
function variable_realm_edit_variables_form($form, &$form_state, $realm_name, $realm_key) {
  $controller = variable_realm_controller($realm_name);
  $form['realm_name'] = array('#type' => 'value', '#value' => $realm_name);
  $form['realm_key'] = array('#type' => 'value', '#value' => $realm_key);
  $options['realm'] = variable_realm($realm_name, $realm_key);

  if ($variable_list = $controller->getEnabledVariables()) {
    $form = variable_base_form($form, $form_state, $variable_list, $options);
    // variable_base_form() adds its own submit handler overriding the default,
    // so we need to add it as a explicit submit callback here.
    $form['#submit'][] = 'variable_realm_edit_variables_form_submit';

    // Group variables by variable group for vertical tabls
    $group_list = array();
    foreach ($variable_list as $variable_name) {
      $variable_info = variable_get_info($variable_name, $options);
      $group = $variable_info['group'];
      $group_list[$group][] = $variable_name;
    }
    $form['variables'] = array(
      '#type' => 'vertical_tabs',
      '#tree' => TRUE,
    );
    foreach ($group_list as $group => $group_variables) {
      $group_info = variable_get_group($group);
      $form['variables'][$group] = array(
        '#type' => 'fieldset',
        '#title' => $group_info['title'],
        '#collapsible' => TRUE, '#collapsed' => TRUE,
      );
      // Set form parents for this variable / group.
      $options['form parents'] = array('variables', $group);
      $form['variables'][$group] += variable_edit_subform($group_variables, $options);
    }
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));
  }
  else {
    $form['message']['#markup'] = '<h3>' . t('No variables have been selected as %realm_name specific.', array('%realm_name' => $controller->getTitle())) . '</h3>';
  }
  if (!empty($realm_info['select path'])) {
    $form['select']['#markup'] = t('To select variables to be configured, see <a href="!select-url">%realm_name variable settings</a>.', array(
      '%realm_name' => $realm_info['title'],
      '!select-url' => url($realm_info['select path'])
    ));
  }
  return $form;
}

/**
 * Edit variables for realm.
 */
function variable_realm_edit_variables_form_submit($form, &$form_state) {
  $realm_name = $form_state['values']['realm_name'];
  $realm_key = $form_state['values']['realm_key'];
  foreach ($form_state['values']['variables'] as $group => $group_variables) {
    if (is_array($group_variables)) {
      foreach ($group_variables as $name => $value) {
        $current = variable_realm_get($realm_name, $realm_key, $name);
        if ($current !== $value) {
          variable_realm_set($realm_name, $realm_key, $name, $value);
        }
      }
    }
  }
  // Redirect later depending on query string parameters.
  _variable_realm_form_submit_redirect($form, $form_state);
}

/**
 * Key selector for realm forms.
 */
function variable_realm_form_key_selector($realm_name, $current_key) {
  $element_name = VARIABLE_REALM_FORM_SWITCHER . $realm_name;
  $query_name = 'variable_realm_' . $realm_name . '_key';
  $controller = variable_realm_controller($realm_name);
  $keys = $controller->getAllKeys();
  // Don't show selector if there aren't any keys to select.
  if (empty($keys)) {
    return array();
  }
  $form[$element_name] = array(
    '#type' => 'fieldset',
    '#title' => t('There are %name variables in this form', array('%name' => $controller->getVariableName())),
    '#weight' => -100,
    '#description' => t('Check you are editing the variables for the right %realm value or select the desired %realm.', array('%realm' => $controller->getTitle())),
  );
  // Replace only this element on current query string, there may be others.
  $current_query = $_GET;
  unset($current_query['q']);
  foreach ($keys as $realm_key => $key_name) {
    $query[VARIABLE_REALM_QUERY_STRING . $realm_name] = $realm_key;
    $link =  l($key_name, $_GET['q'], array('query' => $query + $current_query));
    $items[] = $current_key == $realm_key ? '<strong>' . $link . '</strong>' : $link;
  }
  $form[$element_name]['select_key'] = array(
    '#type' => 'item',
    '#markup' => implode(' | ', $items),
  );
  return $form;
}

/**
 * Get current realm key from query string or from current realm value.
 */
function variable_realm_form_key_current($realm_name) {
  $realm_controller = variable_realm_controller($realm_name);
  if ($key = variable_realm_params($realm_name)) {
    return $key;
  }
  elseif ($key = $realm_controller->getKey()) {
    return $key;
  }
  elseif ($key = $realm_controller->getRequestKey()) {
    return $key;
  }
  else {
    return $realm_controller->getDefaultKey();
  }
}

/**
 * Alter settings form and return list of found variables.
 */
function _variable_realm_variable_settings_form_alter(&$form, $realm_name, $variables) {
  $result = array();
  foreach (element_children($form) as $field) {
    if (count(element_children($form[$field])) && empty($form[$field]['#tree'])) {
      // Rewrite fieldsets recursively.
      $result += _variable_realm_variable_settings_form_alter($form[$field], $realm_name, $variables);
    }
    elseif (in_array($field, $variables)) {
      if (isset($form[$field]['#variable_realm'])) {
        // Oh-oh, variable already taken by another realm.
        _variable_realm_variable_settings_form_conflict($field);
        $form[$field]['#disabled'] = TRUE;
      }
      else {
        // Mark variable as already taken by a realm
        $form[$field]['#variable_realm'] = $realm_name;
      }
      _variable_realm_variable_settings_form_mark($realm_name, $form[$field]);
      // Addd field => name to result
      $result[$field] = !empty($form[$field]['#title']) ? $form[$field]['#title'] : $field;
    }
  }
  return $result;
}

/**
 * Mark variable as belonging to a realm.
 */
function _variable_realm_variable_settings_form_mark($realm_name, &$element) {
  $realm_info = variable_realm_info($realm_name);
  // Add form field class (i18n-variable) and description text.
  if (!empty($realm_info['variable class'])) {
    $element['#attributes']['class'][] = $realm_info['variable class'];
  }
  $element['#description'] = !empty($element['#description']) ? $element['#description'] : '';
  $element['#description'] .= ' <strong>' . t('This is a @name variable.', array('@name' => $realm_info['variable name'])) . '</strong> ';
}

/**
 * Warning about variable conflict.
 */
function _variable_realm_variable_settings_form_conflict($variable) {
  static $warnings;
  if (!isset($warnings[$variable])) {
    $warnings[$variable] = TRUE;
    drupal_set_message(t('There are conflicting realm variables in the form. The variable %name is enabled for more than one realm. Review your realm settings', array('%name' => variable_name($variable))), 'warning');
  }
}

/**
 * Save realm variables and remove them from form.
 */
function variable_realm_variable_settings_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  foreach ($form['#realm_keys'] as $realm_name => $realm_key) {
    $realm_controller = variable_realm_controller($realm_name);
    //$language = i18n_language($form_state['values']['i18n_variable_language']);
    //unset($form_state['values']['i18n_variable_language']);
    $variables = array_keys($form['#realm_variables'][$realm_name]);
    foreach ($variables as $variable_name) {
      if (isset($form_state['values'][$variable_name])) {
        if ($op == t('Reset to defaults')) {
          variable_realm_del($realm_name, $realm_key, $variable_name);
        }
        else {
          $value = $form_state['values'][$variable_name];
          if (is_array($value) && isset($form_state['values']['array_filter'])) {
            $value = array_keys(array_filter($value));
          }
          variable_realm_set($realm_name, $realm_key, $variable_name, $value);
        }
        // If current is not default realm key, we don't set any global variable (without realm)
        if ($realm_key != $realm_controller->getDefaultKey()) {
          unset($form_state['values'][$variable_name]);
        }
      }
    }
  }
  // Redirect later depending on query string parameters.
  _variable_realm_form_submit_redirect($form, $form_state);
  // The form will go now through system_settings_form_submit()
}

/**
 * Process system_theme_settings form submissions.
 *
 * @see system_theme_settings_submit()
 */
function variable_realm_variable_theme_form_submit($form, &$form_state) {
  // Regular theme submission without variable set.
  $values = $form_state['values'];

  // If the user uploaded a new logo or favicon, save it to a permanent location
  // and use it in place of the default theme-provided file.
  if ($file = $values['logo_upload']) {
    unset($values['logo_upload']);
    $filename = file_unmanaged_copy($file->uri);
    $values['default_logo'] = 0;
    $values['logo_path'] = $filename;
    $values['toggle_logo'] = 1;
  }
  if ($file = $values['favicon_upload']) {
    unset($values['favicon_upload']);
    $filename = file_unmanaged_copy($file->uri);
    $values['default_favicon'] = 0;
    $values['favicon_path'] = $filename;
    $values['toggle_favicon'] = 1;
  }

  // If the user entered a path relative to the system files directory for
  // a logo or favicon, store a public:// URI so the theme system can handle it.
  if (!empty($values['logo_path'])) {
    $values['logo_path'] = _system_theme_settings_validate_path($values['logo_path']);
  }
  if (!empty($values['favicon_path'])) {
    $values['favicon_path'] = _system_theme_settings_validate_path($values['favicon_path']);
  }

  if (empty($values['default_favicon']) && !empty($values['favicon_path'])) {
    $values['favicon_mimetype'] = file_get_mimetype($values['favicon_path']);
  }
  $key = $values['var'];

  // Exclude unnecessary elements before saving.
  unset($values['var'], $values['submit'], $values['reset'], $values['form_id'], $values['op'], $values['form_build_id'], $values['form_token']);

  // Set realm variable.
  $variable_name = $key;
  $realm_name = $form['#realm_theme'];
  $realm_key = $form['#realm_keys'][$realm_name];
  variable_realm_set($realm_name, $realm_key, $variable_name, $values);

  // If current is default realm key, set global variable too.
  $realm_controller = variable_realm_controller($realm_name);
  if ($realm_key == $realm_controller->getDefaultKey()) {
    variable_set($variable_name, $values);
  }
  // Confirmation, clear cache, taken from system_theme_settings_submit()
  drupal_set_message(t('The configuration options have been saved.'));
  cache_clear_all();
  // Redirect later depending on query string parameters.
  _variable_realm_form_submit_redirect($form, $form_state);
}

/**
 * Get variable list for settings forms handling multiple realms.
 *
 * Variables available for more than one realm, will be kept only in the list
 * for the realm with the higher weight.
 */
function _variable_realm_variable_settings_form_list() {
  $list = array();
  foreach (variable_realm_list_all() as $realm_name => $controller) {
    if ($controller->getInfo('form settings') && ($realm_list = $controller->getEnabledVariables())) {
      // Remove from previous realms with lower weight.
      foreach ($list as $name => $variables) {
        $list[$name] = array_diff($variables, $realm_list);
      }
      $list[$realm_name] = $realm_list;
    }
  }
  return $list;
}

/**
 * Redirect to current page after form submission, using query string parameters.
 */
function _variable_realm_form_submit_redirect($form, &$form_state) {
  if ($query_params = variable_realm_params()) {
    foreach ($query_params as $realm => $value) {
      $query[VARIABLE_REALM_QUERY_STRING . $realm] = $value;
    }
    // Parameters to be passed to drupal_goto().
    $form_state['redirect'] = array(current_path(), array('query' => $query));
  }
}

/**
 * Add realm switcher to the form.
 */
function _variable_realm_variable_settings_form_switcher(&$form) {
  // Add switchers for current realms and current key.
  // Add realm values and subform realm / key selector.
  foreach (array_keys($form['#realm_variables']) as $realm_name) {
    $current_key = variable_realm_form_key_current($realm_name);
    $info = variable_realm_info($realm_name);
    if (!empty($info['form switcher'])) {
      $form += variable_realm_form_key_selector($realm_name, $current_key);
    }
    $form['#realm_keys'][$realm_name] = $current_key;
  }
  // Make sure realm switchers are added for all parent realms of current ones.
  foreach (variable_realm_list_all() as $realm_name => $realm_controller) {
    if (($parent_realms = $realm_controller->getParentRealms()) && !empty($form['#realm_variables'][$realm_name]) && empty($form[VARIABLE_REALM_FORM_SWITCHER . $realm_name])) {
      // Check we have selectors for the other realms.
      foreach ($parent_realms as $realm) {
        $info = variable_realm_info($realm);
        if (!empty($info['form switcher']) && empty($form[VARIABLE_REALM_FORM_SWITCHER . $realm])) {
          $current_key = variable_realm_form_key_current($realm);
          $form += variable_realm_form_key_selector($realm, $current_key);
        }
      }
    }
  }
}
