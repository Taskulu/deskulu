<?php
/**
 * @file
 * Variable API module - Admin UI
 */

/**
 * Variable overview, by group
 */
function variable_admin_page_group($group = NULL) {
  variable_include();
  $build['message']['#markup'] = _variable_admin_realm_message();
  $realm_options = _variable_admin_realm_options();
  if ($group && ($group_info = variable_get_group($group))) {
    $group_info += array('description' => '');
    drupal_set_title($group_info['title']);
    $list = variable_list_group($group);
    $build['description']['#markup'] = '<h3>' . $group_info['description'] . '</h3>';
    $build['form'] = drupal_get_form('variable_edit_form', array_keys($list), $realm_options);
    return $build;
  }
  else {
    // Groups overview
    $build['groups'] = array(
      '#type' => 'vertical_tabs',
    );
    foreach (variable_get_group() as $group => $info) {
      if ($group_variables = variable_list_group($group)) {
        $build['groups'][$group] = array(
          '#type' => 'fieldset',
          '#title' => $info['title'],
          '#collapsible' => TRUE, '#collapsed' => TRUE,
          '#description' => (!empty($info['description']) ? $info['description'] . ' ' : '') . l(t('Edit group variables'), 'admin/config/system/variable/group/' . $group),
        );
        $build['groups'][$group]['list'] = variable_admin_group_list($group_variables, $realm_options);
      }
    }
    return $build;
  }
}

/**
 * Build list of variables for group
 *
 * @return Renderable array
 */
function variable_admin_group_list($variables, $options = array()) {
  $list = array();
  foreach ($variables as $name => $variable) {
    $type = variable_get_type($variable['type']);
    $list[$name] = array(
      '#type' => 'item',
      '#title' => l($variable['title'], 'admin/config/system/variable/edit/' . $name) . ' (' . $type['title'] . ')',
      '#markup' => variable_format_value($variable, $options), //
      '#description' => isset($variable['description']) ? $variable['description'] : '',
    );
  }
  return $list;
  return array(
    '#theme' => 'table',
    '#rows' => $list,
  );
}

/**
 * Variable overview, by group
 */
function variable_admin_page_module($name = NULL) {
  variable_include();
  $build['message']['#markup'] = _variable_admin_realm_message();
  $realm_options = _variable_admin_realm_options();
  if ($name && (in_array($name, module_list()))) {
    $modules = system_list('module_enabled');
    $module = $modules[$name];
    drupal_set_title($module->info['name']);
    $list = variable_list_module($name);
    $build['description']['#markup'] = $module->info['description'];
    $build['form'] = drupal_get_form('variable_edit_form', array_keys($list), $realm_options);
    return $build;
  }
  else {
    // Groups overview
    $build['modules'] = array(
      '#type' => 'vertical_tabs',

    );
    foreach (system_list('module_enabled') as $module) {
      if ($variables = variable_list_module($module->name)) {
        $build['modules'][$module->name] = array(
          '#type' => 'fieldset',
          '#title' => $module->info['name'],
          '#collapsible' => TRUE, '#collapsed' => TRUE,
          '#description' => $module->info['description'] . ' ' . l(t('Edit module variables'), 'admin/config/system/variable/module/' . $module->name),
          '#group' => 'modules',
        );
        $build['modules'][$module->name]['list'] = variable_admin_group_list($variables, $realm_options);
      }
    }
    return $build;
  }
}

/**
 * Lists available realms.
 */
function variable_admin_realm_overview($form, $form_state) {
  $header = array(
    t('Realm name'),
    t('Weight'),
    t('Options'),
  );

  $realms = array();
  foreach (variable_realm_info() as $realm => $info) {
    $options = array();
    // Add list of configured variables + configuration link
    if (!empty($info['select'])) {
      $options[] = l(t('Configure'), 'admin/config/system/variable/realm/' . $realm . '/configure');
      $options[] = l(t('Edit'), 'admin/config/system/variable/realm/' . $realm . '/edit');
    }
    // Get weight from realm controller.
    $controller = variable_realm_controller($realm);
    $realms[] = array(
      l($info['title'], 'admin/config/system/variable/realm/' . $realm),
      $controller->getDefaultWeight(),
      implode(' | ' , $options)
    );
  }

  $form['realms'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $realms,
    '#empty' => t('No realms available.'),
  );

  return $form;
}

/**
 * Lists available realms.
 */
function variable_admin_realm_info($realm_name) {
  $controller = variable_realm_controller($realm_name);
  $build['title'] = array(
    '#type' => 'item',
    '#title' => t('Name'),
    '#markup' => $controller->getTitle(),
  );
  $build['keys'] = array(
    '#theme' => 'item_list',
    '#title' => t('Keys'),
    '#items' => $controller->getAllKeys(),
  );
  $build['variables'] = array(
    '#theme' => 'item_list',
    '#title' => t('Variables'),
    '#items' => $controller->getEnabledVariables(),
  );
  if ($controller->getInfo('select')) {
    $build['options'] = array(
      '#theme' => 'item_list',
      '#title' => t('Options'),
      '#items' => array(
        l(t('Configure'), 'admin/config/system/variable/realm/' . $realm_name . '/configure'),
        l(t('Edit'), 'admin/config/system/variable/realm/' . $realm_name . '/edit'),
      ),
    );
  }
  return $build;
}

/**
 * Edit variables for realm.
 */
function variable_admin_realm_edit($realm_name) {
  module_load_include('form.inc', 'variable_realm');
  $realm_key = variable_realm_form_key_current($realm_name);
  $build['select'] = variable_realm_form_key_selector($realm_name, $realm_key);
  $build['form'] = drupal_get_form('variable_realm_edit_variables_form', $realm_name, $realm_key);
  return $build;
}

/**
 * Message for forms viewing / editing global variables.
 */
function _variable_admin_realm_message() {
  if (module_exists('variable_realm')) {
    return '<p>' . t('These are global default variables. To edit variables for a specific realm go to <a href="@variable-realm">Variable Realms</a>', array('@variable-realm' => url('admin/config/system/variable/realm'))) .'</p>';
  }
  else {
    return '';
  }
}

/**
 * Options for forms viewing / editing global variables.
 */
function _variable_admin_realm_options() {
  if (module_exists('variable_realm')) {
    $options['realm'] = variable_realm('global', 'default');
    $options['realm_name'] = 'global';
    $options['realm_key'] = 'default';
    return $options;
  }
  else {
    return array();
  }
}
