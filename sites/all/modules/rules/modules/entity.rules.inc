<?php

/**
 * @file General entity related rules integration
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implements hook_rules_file_info() on behalf of the entity module.
 * @see rules_core_modules()
 */
function rules_entity_file_info() {
  return array('modules/entity.eval');
}

/**
 * Implements hook_rules_category_info() on behalf of the pseudo entity module.
 */
function rules_entity_category_info() {
  return array(
    'rules_entity' => array(
      'label' => t('Entities'),
      'equals group' => t('Entities'),
      'weight' => -50,
    ),
  );
}

/**
 * Implements hook_rules_action_info() on behalf of the entity module.
 * @see rules_core_modules()
 */
function rules_entity_action_info() {
  if (rules_entity_action_type_options('entity_fetch')) {
    $return['entity_fetch'] = array(
      'label' => t('Fetch entity by id'),
      'parameter' => array(
        'type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
          'options list' => 'rules_entity_action_type_options',
          'description' => t('Specifies the type of entity that should be fetched.'),
          'restriction' => 'input',
        ),
        'id' => array('type' => 'unknown', 'label' => t('Identifier')),
      ),
      'provides' => array(
        'entity_fetched' => array('type' => 'unknown', 'label' => t('Fetched entity')),
      ),
      'group' => t('Entities'),
      'access callback' => 'rules_entity_action_access',
      'base' => 'rules_action_entity_fetch',
      'callbacks' => array(
        'access' => 'rules_action_entity_createfetch_access',
        'form_alter' => 'rules_action_type_form_alter',
      ),
    );
    $return['entity_query'] = array(
      'label' => t('Fetch entity by property'),
      'parameter' => array(
        'type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
          'options list' => 'rules_entity_action_type_options',
          'description' => t('Specifies the type of the entity that should be fetched.'),
          'restriction' => 'input',
        ),
        'property' => array(
          'type' => 'text',
          'label' => t('Property'),
          'description' => t('The property by which the entity is to be selected.'),
          'restriction' => 'input',
        ),
        'value' => array(
          'type' => 'unknown',
          'label' => t('Value'),
          'description' => t('The property value of the entity to be fetched.'),
        ),
        'limit' => array(
          'type' => 'integer',
          'label' => t('Limit result count'),
          'description' => t('Limit the maximum number of fetched entities.'),
          'optional' => TRUE,
          'default value' => '10',
        ),
      ),
      'provides' => array(
        'entity_fetched' => array('type' => 'list', 'label' => t('Fetched entity')),
      ),
      'group' => t('Entities'),
      'access callback' => 'rules_entity_action_access',
      'base' => 'rules_action_entity_query',
      'callbacks' => array(
        'form_alter' => 'rules_action_type_form_alter',
      ),
    );
  }

  if (rules_entity_action_type_options('entity_create')) {
    $return['entity_create'] = array(
      'label' => t('Create a new entity'),
      'named parameter' => TRUE,
      'parameter' => array(
        'type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
          'options list' => 'rules_entity_action_type_options',
          'description' => t('Specifies the type of the entity that should be created.'),
          'restriction' => 'input',
        ),
        // Further needed parameter depends on the type.
      ),
      'provides' => array(
        'entity_created' => array(
          'type' => 'unknown',
          'label' => t('Created entity'),
          'save' => TRUE,
        ),
      ),
      'group' => t('Entities'),
      'access callback' => 'rules_entity_action_access',
      'base' => 'rules_action_entity_create',
      'callbacks' => array(
        'access' => 'rules_action_entity_createfetch_access',
        'form_alter' => 'rules_action_type_form_alter',
        'validate' => 'rules_action_create_type_validate',
      ),
    );
  }

  $return['entity_save'] = array(
    'label' => t('Save entity'),
    'parameter' => array(
      'data' => array(
        'type' => 'entity',
        'label' => t('Entity'),
        'description' => t('Specifies the entity, which should be saved permanently.'),
        'restriction' => 'selector',
        'wrapped' => TRUE,
      ),
      'immediate' => array(
        'type' => 'boolean',
        'label' => t('Force saving immediately'),
        'description' => t('Usually saving is postponed till the end of the evaluation, so that multiple saves can be fold into one. If this set, saving is forced to happen immediately.'),
        'default value' => FALSE,
        'optional' => TRUE,
        'restriction' => 'input',
      ),
    ),
    'group' => t('Entities'),
    'access callback' => 'rules_entity_action_access',
    'base' => 'rules_action_entity_save',
    'callbacks' => array(
      'access' => 'rules_action_entity_savedelete_access',
    ),
  );

  $return['entity_delete'] = array(
    'label' => t('Delete entity'),
    'parameter' => array(
      'data' => array(
        'type' => 'entity',
        'label' => t('Entity'),
        'description' => t('Specifies the entity, which should be deleted permanently.'),
        'restriction' => 'selector',
        'wrapped' => TRUE,
      ),
    ),
    'group' => t('Entities'),
    'access callback' => 'rules_entity_action_access',
    'base' => 'rules_action_entity_delete',
    'callbacks' => array(
      'access' => 'rules_action_entity_savedelete_access',
    ),
  );
  return $return;
}

/**
 * Custom access callback for data create and fetch action.
 */
function rules_action_entity_createfetch_access(RulesAbstractPlugin $element) {
  $op = $element->getElementName() == 'entity_create' ? 'create' : 'view';
  return entity_access($op, $element->settings['type']);
}

/**
 * Custom access callback for the data query action.
 */
function rules_action_entity_query_access(RulesAbstractPlugin $element) {
  if (!rules_action_entity_createfetch_access($element)) {
    return FALSE;
  }
  $properties = entity_get_all_property_info($element->settings['type']);
  if (isset($element->settings['property']) && isset($properties[$element->settings['property']]['access callback'])) {
    return call_user_func($properties[$element->settings['property']]['access callback'], 'view', $element->settings['property'], $element->settings['type'], NULL, NULL);
  }
  return TRUE;
}

/**
 * Options list callback for a parameter of entity_create.
 */
function rules_action_entity_parameter_options_list(RulesPlugin $element, $param_name) {
  // Remove the parameter name prefix 'param_'.
  $property_name = substr($param_name, 6);
  $wrapper = entity_metadata_wrapper($element->settings['type']);
  // The possible values of the "value" parameter are those of the data param.
  return $wrapper->$property_name->optionsList();
}

/**
 * Custom access callback for data save and delete action.
 */
function rules_action_entity_savedelete_access(RulesAbstractPlugin $element) {
  if ($wrapper = $element->applyDataSelector($element->settings['data:select'])) {
    $op = $element->getElementName() == 'entity_save' ? 'save' : 'delete';
    return $wrapper instanceof EntityDrupalWrapper && $wrapper->entityAccess($op);
  }
  return FALSE;
}

/**
 * Returns the options list for choosing a property of an entity type.
 */
function rules_action_entity_query_property_options_list(RulesAbstractPlugin $element) {
  $element->settings += array('type' => NULL);
  if ($element->settings['type']) {
    $properties = entity_get_all_property_info($element->settings['type']);
    return rules_extract_property($properties, 'label');
  }
}

/**
 * Returns the options list specified for the chosen property.
 */
function rules_action_entity_query_value_options_list(RulesAbstractPlugin $element) {
  // Get the possible values for the selected property.
  $element->settings += array('type' => NULL, 'property' => NULL);
  if ($element->settings['type'] && $element->settings['property']) {
    $wrapper = rules_get_entity_metadata_wrapper_all_properties($element);

    if (isset($wrapper->{$element->settings['property']}) && $property = $wrapper->{$element->settings['property']}) {
      return $property->optionsList('view');
    }
  }
}

/**
 * Options list callback for data actions.
 *
 * @param $element
 *   The element to return options for.
 * @param $param
 *   The name of the parameter to return options for.
 */
function rules_entity_action_type_options($element, $name = NULL) {
  // We allow calling this function with just the element name too. That way
  // we ease manual re-use.
  $name = is_object($element) ? $element->getElementName() : $element;
  return ($name == 'entity_create') ? rules_entity_type_options('create') : rules_entity_type_options();
}

/**
 * Returns options containing entity types having the given key set in the info.
 *
 * Additionally, we exclude all entity types that are marked as configuration.
 */
function rules_entity_type_options($key = NULL) {
  $info = entity_get_info();
  $types = array();
  foreach ($info as $type => $entity_info) {
    if (empty($entity_info['configuration']) && empty($entity_info['exportable'])) {
      if (!isset($key) || entity_type_supports($type, $key)) {
        $types[$type] = $entity_info['label'];
      }
    }
  }
  return $types;
}

/**
 * Options list callback for getting a list of possible entity bundles.
 *
 * @param $element
 *   The element to return options for.
 */
function rules_entity_bundle_options(RulesAbstractPlugin $element) {
  $bundles = array();
  if (isset($element->settings['type'])) {
    $entity_info = entity_get_info($element->settings['type']);
    foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
      $bundles[$bundle_name] = $bundle_info['label'];
    }
  }
  return $bundles;
}

/**
 * Entity actions access callback.
 *
 * Returns TRUE if at least one type is available for configuring the action.
 */
function rules_entity_action_access($type, $name) {
  if ($name == 'entity_fetch' || $name == 'entity_create' || $name == 'entity_query') {
    $types = array_keys(rules_entity_action_type_options($name));
    $op = $name == 'entity_create' ? 'create' : 'view';
  }
  elseif ($name == 'entity_save' || $name == 'entity_delete') {
    $types = array_keys(entity_get_info());
    $op = $name == 'entity_save' ? 'save' : 'delete';
  }
  foreach ($types as $key => $type) {
    if (!entity_access($op, $type)) {
      unset($types[$key]);
    }
  }
  return !empty($types);
}

/**
 * Implements hook_rules_condition_info() on behalf of the entity module.
 * @see rules_core_modules()
 */
function rules_entity_condition_info() {
  return array(
    'entity_is_new' => array(
      'label' => t('Entity is new'),
      'parameter' => array(
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
          'description' => t('Specifies the entity for which to evaluate the condition.'),
          'restriction' => 'selector',
        ),
      ),
      'group' => t('Entities'),
      'base' => 'rules_condition_entity_is_new',
    ),
    'entity_has_field' => array(
      'label' => t('Entity has field'),
      'parameter' => array(
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
          'description' => t('Specifies the entity for which to evaluate the condition.'),
          'restriction' => 'selector',
        ),
        'field' => array(
          'type' => 'text',
          'label' => t('Field'),
          'description' => t('The name of the field to check for.'),
          'options list' => 'rules_condition_entity_has_field_options',
          'restriction' => 'input',
        ),
      ),
      'group' => t('Entities'),
      'base' => 'rules_condition_entity_has_field',
    ),
    'entity_is_of_type' => array(
      'label' => t('Entity is of type'),
      'parameter' => array(
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
          'description' => t('Specifies the entity for which to evaluate the condition.'),
        ),
        'type' => array(
          'type' => 'token',
          'label' => t('Entity type'),
          'description' => t('The entity type to check for.'),
          'options list' => 'rules_entity_action_type_options',
          'restriction' => 'input',
        ),
      ),
      'group' => t('Entities'),
      'base' => 'rules_condition_entity_is_of_type',
    ),
    'entity_is_of_bundle' => array(
      'label' => t('Entity is of bundle'),
      'parameter' => array(
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
          'description' => t('Specifies the entity for which to evaluate the condition.'),
        ),
        'type' => array(
          'type' => 'token',
          'label' => t('Entity type'),
          'description' => t('The type of the checked entity.'),
          'options list' => 'rules_entity_action_type_options',
          'restriction' => 'input',
        ),
        'bundle' => array(
          'type' => 'list<text>',
          'label' => t('Entity bundle'),
          'description' => t('The condition is met if the entity is of one of the selected bundles.'),
          'options list' => 'rules_entity_bundle_options',
          'restriction' => 'input',
        ),
      ),
      'group' => t('Entities'),
      'base' => 'rules_condition_entity_is_of_bundle',
    ),
    'entity_field_access' => array(
      'label' => t('User has field access'),
      'parameter' => array(
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
          'description' => t('Specifies the entity for which to evaluate the condition.'),
          'restriction' => 'selector',
          'wrapped' => TRUE,
        ),
        'field' => array(
          'type' => 'token',
          'label' => t('Field name'),
          'description' => t('The name of the field to check for.'),
          'options list' => 'rules_condition_entity_has_field_options',
          'restriction' => 'input',
        ),
        'op' => array(
          'type' => 'text',
          'label' => t('Access operation'),
          'options list' => 'rules_condition_entity_field_access_op_options',
          'restriction' => 'input',
          'optional' => TRUE,
          'default value' => 'view',
        ),
        'account' => array(
          'type' => 'user',
          'label' => t('User account'),
          'description' => t('Specifies the user account for which to check access. If left empty, the currently logged in user will be used.'),
          'restriction' => 'selector',
          'optional' => TRUE,
          'default value' => NULL,
        ),
      ),
      'group' => t('Entities'),
      'base' => 'rules_condition_entity_field_access',
    ),
  );
}

/**
 * Help callback for condition entity_is_new.
 */
function rules_condition_entity_is_new_help() {
  return t('This condition determines whether the specified entity has just been created and has not yet been saved to the database.');
}

/**
 * Returns options for choosing a field for the selected entity.
 */
function rules_condition_entity_has_field_options(RulesAbstractPlugin $element) {
  // The field_info_field_map() function was introduced in Drupal 7.22 (see
  // https://drupal.org/node/1915646).
  if (function_exists('field_info_field_map')) {
    $fields = field_info_field_map();
  }
  else {
    $fields = field_info_fields();
  }
  return drupal_map_assoc(array_keys($fields));
}

/**
 * Returns options for choosing a field_access() operation.
 */
function rules_condition_entity_field_access_op_options(RulesAbstractPlugin $element) {
  return array(
    'view' => t('View'),
    'edit' => t('Edit'),
  );
}

/**
 * Assert that the entity has the field, if there is metadata for the field.
 */
function rules_condition_entity_has_field_assertions($element) {
  // Assert the field is there if the condition matches.
  if ($wrapper = $element->applyDataSelector($element->settings['entity:select'])) {
    $type = $wrapper->type();
    $field_property = $element->settings['field'];
    // Get all possible properties and check whether we have one for the field.
    $properties = entity_get_all_property_info($type == 'entity' ? NULL : $type);

    if (isset($properties[$field_property])) {
      $assertion = array('property info' => array($field_property => $properties[$field_property]));
      return array($element->settings['entity:select'] => $assertion);
    }
  }
}

/**
 * Assert the selected entity type.
 */
function rules_condition_entity_is_of_type_assertions($element) {
  if ($type = $element->settings['type']) {
    return array('entity' => array('type' => $type));
  }
}

/**
 * Assert the selected entity type and bundle.
 */
function rules_condition_entity_is_of_bundle_assertions($element) {
  if ($bundle = $element->settings['bundle']) {
    $assertions = array();
    $assertions['entity']['type'] = $element->settings['type'];
    $assertions['entity']['bundle'] = $bundle;
    return $assertions;
  }
}

/**
 * Process callback for the condition entity_is_of_bundle.
 */
function rules_condition_entity_is_of_bundle_process(RulesAbstractPlugin $element) {
  // If we know the entity type, auto-populate it.
  if (($info = $element->getArgumentInfo('entity')) && $info['type'] != 'entity') {
    $element->settings['type'] = $info['type'];
  }
}

/**
 * Form alter callback for the condition entity_is_of_bundle.
 *
 * Use multiple steps to configure the condition as the needed bundle field list
 * depends on the selected entity type.
 */
function rules_condition_entity_is_of_bundle_form_alter(&$form, &$form_state, $options, RulesAbstractPlugin $element) {
  if (empty($element->settings['entity:select'])) {
    $step = 1;
  }
  elseif (empty($element->settings['type'])) {
    $step = 2;
  }
  else {
    $step = 3;
  }

  $form['reload'] = array(
    '#weight' => $form['submit']['#weight'] + 1,
    '#type' => 'submit',
    '#name' => 'reload',
    '#value' => $step != 3 ? t('Continue') : t('Reload form'),
    '#limit_validation_errors' => array(array('parameter', 'entity'), array('parameter', 'type')),
    '#submit' => array('rules_form_submit_rebuild'),
    '#ajax' => rules_ui_form_default_ajax('fade'),
    '#attributes' => array('class' => array('rules-hide-js')),
  );
  // Use ajax and trigger as the reload button.
  $form['parameter']['type']['settings']['type']['#ajax'] = $form['reload']['#ajax'] + array(
    'event' => 'change',
    'trigger_as' => array('name' => 'reload'),
  );

  switch ($step) {
    case 1:
      $form['reload']['#limit_validation_errors'] = array(array('parameter', 'entity'));
      unset($form['parameter']['type']);
      unset($form['reload']['#attributes']['class']);
      // NO break;
    case 2:
      $form['negate']['#access'] = FALSE;
      unset($form['parameter']['bundle']);
      unset($form['submit']);
      break;
    case 3:
      if (($info = $element->getArgumentInfo('entity')) && $info['type'] != 'entity') {
        // Hide the entity type parameter if not needed.
        unset($form['parameter']['type']);
      }
      break;
  }
}

/**
 * @}
 */
