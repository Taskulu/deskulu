<?php

/**
 * @file
 * CTools plugin. Provides support for rules components (rule, ruleset, action).
 */

$plugin = array(
  'title' => t('Rules component'),
  'list callback' => 'views_bulk_operations_operation_rules_component_list',
  'handler' => array(
    'file' => 'rules_component.class.php',
    'class' => 'ViewsBulkOperationsRulesComponent',
  ),
);

/**
 * Returns a prepared list of available rules components.
 *
 * @param $operation_id
 *   The full, prefixed operation_id of the operation (in this case, rules
 *   component) to return, or NULL to return an array with all operations.
 */
function views_bulk_operations_operation_rules_component_list($operation_id = NULL) {
  if (!module_exists('rules')) {
    return array();
  }

  $entity_info = entity_get_info();
  $entity_types = array_keys($entity_info);
  $supported_types = array('entity', 'list<entity>');
  $list_types = array('list<entity>');
  foreach ($entity_types as $type) {
    $supported_types[] = $type;
    $supported_types[] = "list<$type>";
    $list_types[] = "list<$type>";
  }

  $components = array();
  if (isset($operation_id)) {
    $id_fragments = explode('::', $operation_id);
    $components[$id_fragments[1]] = rules_config_load($id_fragments[1]);
    // No need to go any further if the component no longer exists.
    if (!$components[$id_fragments[1]]) {
      return FALSE;
    }
  }
  else {
    $components = rules_get_components(FALSE, 'action');
  }

  $operations = array();
  foreach ($components as $name => $component) {
    $parameter_info = $component->parameterInfo();
    $first_parameter = reset($parameter_info);
    $parameter_keys = array_keys($parameter_info);
    $entity_key = reset($parameter_keys);
    // If the first param is not an entity type, skip the component.
    if (!in_array($first_parameter['type'], $supported_types)) {
      continue;
    }

    // If the first parameter is a list type (list<node>, list<entity>, etc)
    // then turn aggregation on, and set the correct entity type.
    if (in_array($first_parameter['type'], $list_types)) {
      $type = str_replace(array('list<', '>'), '', $first_parameter['type']);
      $aggregate = TRUE;
    }
    else {
      $type = $first_parameter['type'];
      $aggregate = FALSE;
    }

    // All operations must be prefixed with the operation type.
    $new_operation_id = 'rules_component::' . $name;
    $operations[$new_operation_id] = array(
      'operation_type' => 'rules_component',
      // Keep the unprefixed key around, for internal use.
      'key' => $name,
      'label' => $component->label,
      'parameters' => array('component_key' => $name, 'entity_key' => $entity_key),
      'configurable' => count($parameter_info) > 1,
      'type' => $type,
      'aggregate' => $aggregate,
    );
  }

  if (isset($operation_id)) {
    return isset($operations[$operation_id]) ? $operations[$operation_id] : FALSE;
  }
  else {
    return $operations;
  }
}
