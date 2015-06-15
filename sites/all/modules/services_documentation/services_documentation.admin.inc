<?php
/**
 * @file
 * services_documentation.admin.inc
 * Provides the Services Documentation administrative interface.
 */

/**
 * Configuration settings for the Services Documentation module.
 */
function services_documentation_configure() {
  $form = array();

  // Designate documentation basepath.
  $form['services_documentation_base_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Set your documentation base path here'),
    '#default_value' => variable_get('services_documentation_base_path', 'developers'),
    '#description' => 'If you enter "developers" here, your documentation will appear at /developers/1000, /developers/2000, /developers/3000. Your documentation homepage will be /developers.',
  );

  $form['services_documentation_intro'] = array(
    '#type' => 'textarea',
    '#title' => t('API Introduction'),
    '#default_value' => variable_get('services_documentation_intro'),
    '#description' => t('This text will be displayed at the top of each API version documentation page.'),
  );

  $form['services_documentation_toc'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display table of contents'),
    '#default_value' => variable_get('services_documentation_toc', TRUE),
    '#description' => t('If enabled, a table of contents will be displayed after the introduction of each API version documentation page.'),
  );

  $form['services_documentation_display_bundle_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display method bundle titles'),
    '#default_value' => variable_get('services_documentation_display_bundle_title', FALSE),
    '#description' => t('If enabled, method bundle titles will be displayed. E.g., operations, relationships, targeted actions, etc.'),
  );

  // Designate verbs to use.
  $form['services_documentation_verbs'] = array(
    '#type' => 'radios',
    '#title' => t('Services Documentation Verbs'),
    '#default_value' => variable_get('services_documentation_verbs', 'services'),
    '#options' => array(
      'services' => t('Services'),
      'http' => t('HTTP'),
      'crud' => t('CRUD'),
      'custom' => t('Custom'),
    ),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom verbs.
  $form['services_documentation_verbs_custom'] = array(
    '#type' => 'fieldset',
    '#title' => t('Services Documentation Custom Verbs'),
    '#description' => t('Select verbs used to describe operations.'),
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="services_documentation_verbs"]' => array('value' => 'custom'),
      ),
    ),
  );

  // Designate custom retrieve verb.
  $form['services_documentation_verbs_custom']['retrieve'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Retrieve Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_retrieve', t('retrieve')),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom create verb.
  $form['services_documentation_verbs_custom']['create'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Create Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_create', t('create')),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom update verb.
  $form['services_documentation_verbs_custom']['update'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Update Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_update', t('update')),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom delete verb.
  $form['services_documentation_verbs_custom']['delete'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Delete Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_delete', t('delete')),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom index verb.
  $form['services_documentation_verbs_custom']['index'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Index Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_index', t('index')),
    '#description' => t('Select verbs used to describe operations.'),
  );

  // Designate custom actions verb.
  $form['services_documentation_verbs_custom']['actions'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Action Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_action', t('action')),
    '#description' => t('Select verbs used to describe actions.'),
  );

  // Designate custom targeted actions verb.
  $form['services_documentation_verbs_custom']['targeted_actions'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Targeted Action Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_targeted_actions', t('targeted action')),
    '#description' => t('Select verbs used to describe targeted action.'),
  );

  // Designate custom relationships verb.
  $form['services_documentation_verbs_custom']['relationships'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Relationship Verb'),
    '#default_value' => variable_get('services_documentation_verbs_custom_relationships', t('relationships')),
    '#description' => t('Select verbs used to describe relationships.'),
  );

  $form['services_documentation_css'] = array(
  '#type' => 'checkbox',
  '#title' => t('Add default CSS styling'),
  '#default_value' => variable_get('services_documentation_css', TRUE),
  '#description' => t('If enabled, default CSS styling will be applied to the documentation.'),
  );

  // Use custom submit handler, rather than system_settings_form.
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['#submit'][] = 'services_documentation_config_submit';

  return $form;
}

/**
 * Submit handler for services_documentation_configure form.
 *
 * Check to see if values have changed for base_path and homepage alias.
 *
 * If they have, remove old path alias, save new values, and save new path
 * aliases.
 */
function services_documentation_config_submit($form, &$form_state) {
  $services_documentation_base_path = $form_state['values']['services_documentation_base_path'];
  if (variable_get('services_documentation_base_path', 'developers') != $services_documentation_base_path) {
    variable_set('services_documentation_base_path', $services_documentation_base_path);
    drupal_set_message('You will need to clear menu caches before changes to the base path take effect.');
  }

  // Get verb-related values from form.
  $verbs = array(
    'services_documentation_verbs' => $form_state['values']['services_documentation_verbs'],
    'services_documentation_verbs_custom_retrieve' => $form_state['values']['services_documentation_verbs_custom']['retrieve'],
    'services_documentation_verbs_custom_update' => $form_state['values']['services_documentation_verbs_custom']['update'],
    'services_documentation_verbs_custom_delete' => $form_state['values']['services_documentation_verbs_custom']['delete'],
    'services_documentation_verbs_custom_index' => $form_state['values']['services_documentation_verbs_custom']['index'],
  );

  // Save updated verb-related variable values.
  foreach ($verbs as $variable => $value) {
    if ($value != variable_get($variable, '')) {
      variable_set($variable, check_plain($value));
    }
  }

  // Simple variable saves.
  variable_set('services_documentation_intro', $form_state['values']['services_documentation_intro']);
  variable_set('services_documentation_toc', $form_state['values']['services_documentation_toc']);
}


/**
 * Configuration settings for specific resource operations.
 */
function services_documentation_op_docs_form($form, &$form_state, $endpoint, $resource_name, $class, $operation_name) {

  $form_state['endpoint'] = $endpoint;
  $form_state['resource_name'] = $resource_name;
  $form_state['class'] = $class;
  $form_state['operation_name'] = $operation_name;

  $docs_settings = array();
  if (!empty($endpoint->server_settings['docs'][$resource_name][$class][$operation_name])) {
    $docs_settings = $endpoint->server_settings['docs'][$resource_name][$class][$operation_name];
  }

  $form['enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable display of custom documentation'),
    '#description' => t('If checked, custom documentation for this operation will be displayed.'),
    '#default_value' => services_documentation_get_default_value($docs_settings, 'enable'),
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('A description of this operation.'),
    '#default_value' => services_documentation_get_default_value($docs_settings, 'description'),
  );
  $form['request_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Request URL'),
    '#description' => t('An example Request URL for this operation. E.g., /myapi/myresource?myvar=myvalue.'),
    '#default_value' => services_documentation_get_default_value($docs_settings, 'request_url'),
  );
  $form['response'] = array(
    '#type' => 'textarea',
    '#title' => t('Response'),
    '#description' => t('An example response. E.g., {}.'),
    '#default_value' => services_documentation_get_default_value($docs_settings, 'response'),
  );
  $form['auth'] = array(
    '#type' => 'checkbox',
    '#title' => t('Authorization'),
    '#description' => t('Check this box is authorization is required for this operation.'),
    '#default_value' => services_documentation_get_default_value($docs_settings, 'auth'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validation callback for services_documentation_op_docs_form.
 */
function services_documentation_op_docs_form_validate(&$form, &$form_state) {

}

/**
 * Submit callback for services_documentation_op_docs_form.
 */
function services_documentation_op_docs_form_submit($form, &$form_state) {
  $resource_name = $form_state['resource_name'];
  $class = $form_state['class'];
  $operation_name = $form_state['operation_name'];

  $values = $form_state['values'];
  // Unsetting unneeded values allows other modules to form_alter() the
  // docs for and foist new data into $endpoint->server_settings.
  unset($values['submit']);
  unset($values['op']);
  unset($values['form_build_id']);
  unset($values['form_token']);
  unset($values['form_id']);

  $endpoint = $form_state['endpoint'];
  $endpoint->server_settings['docs'][$resource_name][$class][$operation_name] = $values;
  services_endpoint_save($endpoint);
}

/**
 * Returns a key from $settings, or else returns a default value.
 *
 * @param array $settings
 *   An array of settings.
 *
 * @param string $key
 *   A key within the settings array.
 *
 * @param mixed $default
 *   (optional) The default value to be returned if the key is not found in
 *   the settings.
 *
 * @return mixed
 *   The value settings value or default value.
 */
function services_documentation_get_default_value($settings, $key, $default = NULL) {
  if (!empty($settings[$key])) {
    return $settings[$key];
  }
  elseif (isset($default)) {
    return $default;
  }

  return '';
}

/**
 * Title callback for services_documentation_op_docs_form menu router item.
 *
 * @param string $endpoint_name
 *   The endpoint name. E.g., api_v1.
 *
 * @param string $resource_name
 *   The resource name. E.g., node.
 *
 * @param string $op_name
 *   The operation name. E.g., index.
 *
 * @return string
 *   The page title.
 */
function services_documentation_op_docs_form_title($endpoint_name, $resource_name, $method_bundle, $op_name) {
  $title = t('Services -> @endpoint -> @resource -> @method_bundle -> @operation -> Docs', array(
    '@endpoint' => $endpoint_name,
    '@resource' => $resource_name,
    '@method_bundle' => $method_bundle,
    '@operation' => $op_name,
  ));

  return $title;
}
