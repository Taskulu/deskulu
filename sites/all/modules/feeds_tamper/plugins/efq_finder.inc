<?php

/**
 * @file
 * Provides a plugin that maps arbitrary values to entity ids.
 */

$plugin = array(
  'form' => 'feeds_tamper_efq_finder_form',
  'callback' => 'feeds_tamper_efq_finder_callback',
  'validate' => 'feeds_tamper_efq_finder_validate',
  'name' => 'Entity Field Query finder',
  'multi' => 'loop',
  'category' => 'Other',
);

/**
 * Configuration form.
 */
function feeds_tamper_efq_finder_form($importer, $element_key, array $settings, array &$form_state) {
  $form = array(
    '#prefix' => '<div id="feeds-tamper-efq-finder-wrapper">',
    '#suffix' => '</div>'
  );

  // Get some info on entities and bundles.
  $entity_types = array();
  foreach (entity_get_info() as $machine_name => $info) {
    $entity_types[$machine_name] = check_plain($info['label']);
  }

  $entity_type = _feeds_tamper_efq_finder_get_value('entity_type', $form_state, $settings);
  $form['update'] = array(
    '#type' => 'submit',
    '#submit' => array('feeds_tamper_ui_add_plugin_form_submit'),
    '#value' => t('Update'),
    '#attributes' => array('class' => array('no-js')),
  );

  $form['entity_type'] = array(
    '#type' => 'select',
    '#title' => t('Entity type'),
    '#options' => $entity_types,
    '#default_value' => $entity_type,
    '#ajax' => array(
      'callback' => 'feeds_tamper_efq_finder_ajax',
      'wrapper' => 'feeds-tamper-efq-finder-wrapper',
    ),
    '#required' => TRUE,
  );

  if (!$entity_type) {
    return $form;
  }

  // Gather bundle information.
  $entity_info = entity_get_info($entity_type);
  $bundles = array();
  foreach ($entity_info['bundles'] as $machine_name => $info) {
    $bundles[$machine_name] = check_plain($info['label']);
  }

  $bundle = _feeds_tamper_efq_finder_get_value('bundle', $form_state, $settings);
  $bundle = isset($bundles[$bundle]) ? $bundle : NULL;

  $form['bundle'] = array(
    '#type' => 'select',
    '#title' => t('Bundle'),
    '#options' => $bundles,
    '#default_value' => $bundle,
    '#ajax' => array(
      'callback' => 'feeds_tamper_efq_finder_ajax',
      'wrapper' => 'feeds-tamper-efq-finder-wrapper',
    ),
    '#empty_option' => t('-- Select --'),
  );

  // Gather field definitions.
  $fields = array();
  foreach ($entity_info['schema_fields_sql']['base table'] as $property) {
    $fields[$property] = $property;
  }
  if ($bundle) {
    foreach (field_info_instances($entity_type, $bundle) as $machine_name => $info) {
      $fields[$machine_name] = check_plain($info['label']);
    }
  }
  else {
    foreach (field_info_instances($entity_type) as $bundle_instances) {
      foreach ($bundle_instances as $machine_name => $info) {
        $fields[$machine_name] = check_plain($info['label']);
      }
    }
  }

  $field = _feeds_tamper_efq_finder_get_value('field', $form_state, $settings);
  $field = isset($fields[$field]) ? $field : NULL;

  $form['field'] = array(
    '#type' => 'select',
    '#title' => t('Field'),
    '#options' => $fields,
    '#default_value' => $field,
    '#ajax' => array(
      'callback' => 'feeds_tamper_efq_finder_ajax',
      'wrapper' => 'feeds-tamper-efq-finder-wrapper',
    ),
    '#required' => TRUE,
  );

  if (!$field || in_array($field, $entity_info['schema_fields_sql']['base table'])) {
    return $form;
  }

  $field_info = field_info_field($field);
  $columns = array();
  foreach (array_keys($field_info['columns']) as $column) {
    $columns[$column] = $column;
  }

  $column = _feeds_tamper_efq_finder_get_value('column', $form_state, $settings);
  $column = isset($columns[$column]) ? $column : key($columns);

  $form['column'] = array(
    '#type' => 'select',
    '#title' => t('Column'),
    '#options' => $columns,
    '#default_value' => $column,
    '#required' => TRUE,
  );

  return $form;
}

/**
 * Searches for the correct form value.
 */
function _feeds_tamper_efq_finder_get_value($key, array $form_state, array $settings) {
  if (!empty($form_state['values']['settings'][$key])) {
    return $form_state['values']['settings'][$key];
  }

  if (!empty($settings[$key])) {
    return $settings[$key];
  }
}

/**
 * Validation handler for feeds_tamper_efq_finder_form().
 */
function feeds_tamper_efq_finder_validate(array &$settings) {
  $entity_info = entity_get_info($settings['entity_type']);
  $settings['property'] = in_array($settings['field'], $entity_info['schema_fields_sql']['base table']);
}

/**
 * Ajax callback for form changes.
 */
function feeds_tamper_efq_finder_ajax(array $form, array &$form_state) {
  return isset($form['plugin']['settings']) ? $form['plugin']['settings'] : $form['settings'];
}

/**
 * Sets an entity id based on the value of a field.
 */
function feeds_tamper_efq_finder_callback($result, $item_key, $element_key, &$field, array $settings) {
  $query = new EntityFieldQuery();

  $query->entityCondition('entity_type', $settings['entity_type']);
  if ($settings['bundle']) {
    $query->entityCondition('bundle', $settings['bundle']);
  }

  if ($settings['property']) {
    $query->propertyCondition($settings['field'], $field);
  }
  else {
    $query->fieldCondition($settings['field'], $settings['column'], $field);
  }
  $query->range(0, 1);

  $query_result = $query->execute();

  // @todo: Implement choice of behavior when multiple entities are found.
  // Currently we only use the first.
  if (!empty($query_result)) {
    $result = reset($query_result);
    $field = key($result);
  }
  else {
    // We didn't find a value, remove the item.
    // @todo Fallback.
    unset($result->items[$item_key][$element_key]);
  }
}
