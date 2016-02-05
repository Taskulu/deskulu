<?php

/**
 * @file VBO action to modify entity values (properties and fields).
 */

// Specifies that all available values should be shown to the user for editing.
define('VBO_MODIFY_ACTION_ALL', '_all_');

function views_bulk_operations_modify_action_info() {
  return array('views_bulk_operations_modify_action' => array(
    'type' => 'entity',
    'label' => t('Modify entity values'),
    'behavior' => array('changes_property'),
    // This action only works when invoked through VBO. That's why it's
    // declared as non-configurable to prevent it from being shown in the
    // "Create an advanced action" dropdown on admin/config/system/actions.
    'configurable' => FALSE,
    'vbo_configurable' => TRUE,
    'triggers' => array('any'),
  ));
}

/**
 * Action function.
 *
 * Goes through new values and uses them to modify the passed entity by either
 * replacing the existing values, or appending to them (based on user input).
 */
function views_bulk_operations_modify_action($entity, $context) {
  list(,,$bundle_name) = entity_extract_ids($context['entity_type'], $entity);
  // Handle Field API fields.
  if (!empty($context['selected']['bundle_' . $bundle_name])) {
    // The pseudo entity is cloned so that changes to it don't get carried
    // over to the next execution.
    $pseudo_entity = clone $context['entities'][$bundle_name];
    foreach ($context['selected']['bundle_' . $bundle_name] as $key) {
      // Get this field's language. We can just pull it from the pseudo entity
      // as it was created using field_attach_form and entity_language so it's
      // already been figured out if this field is translatable or not and
      // applied the appropriate language code to the field
      $language = key($pseudo_entity->{$key});
      // Replace any tokens that might exist in the field columns.
      foreach ($pseudo_entity->{$key}[$language] as $delta => &$item) {
        foreach ($item as $column => $value) {
          if (is_string($value)) {
            $item[$column] = token_replace($value, array($context['entity_type'] => $entity), array('sanitize' => FALSE));
          }
        }
      }

      if (in_array($key, $context['append']['bundle_' . $bundle_name]) && !empty($entity->$key)) {
        $entity->{$key}[$language] = array_merge($entity->{$key}[$language], $pseudo_entity->{$key}[$language]);

        // Check if we breached cardinality, and notify the user.
        $field_info = field_info_field($key);
        $field_count = count($entity->{$key}[$language]);
        if ($field_info['cardinality'] != FIELD_CARDINALITY_UNLIMITED && $field_count > $field_info['cardinality']) {
          $entity_label = entity_label($context['entity_type'], $entity);
          $warning = t('Tried to set !field_count values for field !field_name that supports a maximum of !cardinality.',
                       array('!field_count' => $field_count,
                             '!field_name' => $field_info['field_name'],
                             '!cardinality' => $field_info['cardinality']));
          drupal_set_message($warning, 'warning', FALSE);
        }

        // Prevent storing duplicate references.
        if (strpos($field_info['type'], 'reference') !== FALSE) {
          $entity->{$key}[$language] = array_unique($entity->{$key}[LANGUAGE_NONE], SORT_REGULAR);
        }
      }
      else {
        $entity->{$key}[$language] = $pseudo_entity->{$key}[$language];
      }
    }
  }

  // Handle properties.
  if (!empty($context['selected']['properties'])) {
    // Use the wrapper to set property values, since some properties need
    // additional massaging by their setter callbacks.
    // The wrapper will automatically modify $entity itself.
    $wrapper = entity_metadata_wrapper($context['entity_type'], $entity);
    foreach ($context['selected']['properties'] as $key) {
      if (!$wrapper->$key->access('update')) {
        // No access.
        continue;
      }

      if (in_array($key, $context['append']['properties'])) {
        $old_values = $wrapper->$key->value();
        $wrapper->$key->set($context['properties'][$key]);
        $new_values = $wrapper->{$key}->value();
        $all_values = array_merge($old_values, $new_values);
        $wrapper->$key->set($all_values);
      }
      else {
        $value = $context['properties'][$key];
        if (is_string($value)) {
          $value = token_replace($value, array($context['entity_type'] => $entity), array('sanitize' => FALSE));
        }
        $wrapper->$key->set($value);
      }
    }
  }
}

/**
 * Action form function.
 *
 * Displays form elements for properties acquired through Entity Metadata
 * (hook_entity_property_info()), as well as field widgets for each
 * entity bundle, as provided by field_attach_form().
 */
function views_bulk_operations_modify_action_form($context, &$form_state) {
  // This action form uses admin-provided settings. If they were not set, pull the defaults now.
  if (!isset($context['settings'])) {
    $context['settings'] = views_bulk_operations_modify_action_views_bulk_operations_form_options();
  }

  $form_state['entity_type'] = $entity_type = $context['entity_type'];
  // For Field API integration to work, a pseudo-entity is constructed for each
  // bundle that has fields available for editing.
  // The entities then get passed to Field API functions
  // (field_attach_form(), field_attach_form_validate(), field_attach_submit()),
  // and filled with form data.
  // After submit, the pseudo-entities get passed to the actual action
  // (views_bulk_operations_modify_action()) which copies the data from the
  // relevant pseudo-entity constructed here to the actual entity being modified.
  $form_state['entities'] = array();

  $info = entity_get_info($entity_type);
  $properties = _views_bulk_operations_modify_action_get_properties($entity_type, $context['settings']['display_values']);
  $bundles = _views_bulk_operations_modify_action_get_bundles($entity_type, $context);

  $form['#attached']['css'][] = drupal_get_path('module', 'views_bulk_operations') . '/css/modify.action.css';
  $form['#tree'] = TRUE;

  if (!empty($properties)) {
    $form['properties'] = array(
      '#type' => 'fieldset',
      '#title' => t('Properties'),
    );
    $form['properties']['show_value'] = array(
      '#suffix' => '<div class="clearfix"></div>',
    );

    foreach ($properties as $key => $property) {
      $form['properties']['show_value'][$key] = array(
        '#type' => 'checkbox',
        '#title' => $property['label'],
      );

      $determined_type = ($property['type'] == 'boolean') ? 'checkbox' : 'textfield';
      $form['properties'][$key] = array(
        '#type' => $determined_type,
        '#title' => $property['label'],
        '#description' => $property['description'],
        '#states' => array(
          'visible' => array(
            '#edit-properties-show-value-' . str_replace('_', '-', $key) => array('checked' => TRUE),
          ),
        ),
      );
      // The default #maxlength for textfields is 128, while most varchar
      // columns hold 255 characters, which makes it a saner default here.
      if ($determined_type == 'textfield') {
        $form['properties'][$key]['#maxlength'] = 255;
      }

      if (!empty($property['options list'])) {
        $form['properties'][$key]['#type'] = 'select';
        $form['properties'][$key]['#options'] = $property['options list']($key, array());

        if ($property['type'] == 'list') {
          $form['properties'][$key]['#type'] = 'checkboxes';

          $form['properties']['_append::' . $key] = array(
            '#type' => 'checkbox',
            '#title' => t('Add new value(s) to %label, instead of overwriting the existing values.', array('%label' => $property['label'])),
            '#states' => array(
              'visible' => array(
                '#edit-properties-show-value-' . $key => array('checked' => TRUE),
              ),
            ),
          );
        }
      }
    }
  }

  // Going to need this for multilingual nodes
  global $language;
  foreach ($bundles as $bundle_name => $bundle) {
    $bundle_key = $info['entity keys']['bundle'];
    $default_values = array();
    // If the bundle key exists, it must always be set on an entity.
    if (!empty($bundle_key)) {
      $default_values[$bundle_key] = $bundle_name;
    }
    $default_values['language'] = $language->language;
    $entity = entity_create($context['entity_type'], $default_values);
    $form_state['entities'][$bundle_name] = $entity;

    // Show the more detailed label only if the entity type has multiple bundles.
    // Otherwise, it would just be confusing.
    if (count($info['bundles']) > 1) {
      $label = t('Fields for @bundle_key @label', array('@bundle_key' => $bundle_key, '@label' => $bundle['label']));
    }
    else {
      $label = t('Fields');
    }

    $form_key = 'bundle_' . $bundle_name;
    $form[$form_key] = array(
      '#type' => 'fieldset',
      '#title' => $label,
      '#parents' => array($form_key),
    );
    field_attach_form($context['entity_type'], $entity, $form[$form_key], $form_state, entity_language($context['entity_type'], $entity));
    // Now that all the widgets have been added, sort them by #weight.
    // This ensures that they will stay in the correct order when they get
    // assigned new weights.
    uasort($form[$form_key], 'element_sort');

    $display_values = $context['settings']['display_values'];
    $instances = field_info_instances($entity_type, $bundle_name);
    $weight = 0;
    foreach (element_get_visible_children($form[$form_key]) as $field_name) {
      // For our use case it makes no sense for any field widget to be required.
      if (isset($form[$form_key][$field_name]['#language'])) {
        $field_language = $form[$form_key][$field_name]['#language'];
        _views_bulk_operations_modify_action_unset_required($form[$form_key][$field_name][$field_language]);
      }

      // The admin has specified which fields to display, but this field didn't
      // make the cut. Hide it with #access => FALSE and move on.
      if (empty($display_values[VBO_MODIFY_ACTION_ALL]) && empty($display_values[$bundle_name . '::' . $field_name])) {
        $form[$form_key][$field_name]['#access'] = FALSE;
        continue;
      }

      if (isset($instances[$field_name])) {
        $field = $instances[$field_name];
        $form[$form_key]['show_value'][$field_name] = array(
          '#type' => 'checkbox',
          '#title' => $field['label'],
        );
        $form[$form_key][$field_name]['#states'] = array(
          'visible' => array(
            '#edit-bundle-' . str_replace('_', '-', $bundle_name) . '-show-value-' . str_replace('_', '-', $field_name) => array('checked' => TRUE),
          ),
        );
        // All field widgets get reassigned weights so that additional elements
        // added between them (such as "_append") can be properly ordered.
        $form[$form_key][$field_name]['#weight'] = $weight++;

        $field_info = field_info_field($field_name);
        if ($field_info['cardinality'] != 1) {
          $form[$form_key]['_append::' . $field_name] = array(
            '#type' => 'checkbox',
            '#title' => t('Add new value(s) to %label, instead of overwriting the existing values.', array('%label' => $field['label'])),
            '#states' => array(
              'visible' => array(
                '#edit-bundle-' . str_replace('_', '-', $bundle_name) . '-show-value-' . str_replace('_', '-', $field_name) => array('checked' => TRUE),
              ),
            ),
            '#weight' => $weight++,
          );
        }
      }
    }

    // Add a clearfix below the checkboxes so that the widgets are not floated.
    $form[$form_key]['show_value']['#suffix'] = '<div class="clearfix"></div>';
    $form[$form_key]['show_value']['#weight'] = -1;
  }

  // If the form has only one group (for example, "Properties"), remove the
  // title and the fieldset, since there's no need to visually group values.
  $form_elements = element_get_visible_children($form);
  if (count($form_elements) == 1) {
    $element_key = reset($form_elements);
    unset($form[$element_key]['#type']);
    unset($form[$element_key]['#title']);

    // Get a list of all elements in the group, and filter out the non-values.
    $values = element_get_visible_children($form[$element_key]);
    foreach ($values as $index => $key) {
      if ($key == 'show_value' || substr($key, 0, 1) == '_') {
        unset($values[$index]);
      }
    }
    // If the group has only one value, no need to hide it through #states.
    if (count($values) == 1) {
      $value_key = reset($values);
      $form[$element_key]['show_value'][$value_key]['#type'] = 'value';
      $form[$element_key]['show_value'][$value_key]['#value'] = TRUE;
    }
  }

  if (module_exists('token') && $context['settings']['show_all_tokens']) {
    $token_type = str_replace('_', '-', $entity_type);
    $form['tokens'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available tokens'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 998,
    );
    $form['tokens']['tree'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array($token_type, 'site'),
      '#global_types' => array(),
      '#dialog' => TRUE,
    );
  }

  return $form;
}

/**
 * Action form validate function.
 *
 * Checks that the user selected at least one value to modify, validates
 * properties and calls Field API to validate fields for each bundle.
 */
function views_bulk_operations_modify_action_validate($form, &$form_state) {
  // The form structure for "Show" checkboxes is a bit bumpy.
  $search = array('properties');
  foreach ($form_state['entities'] as $bundle => $entity) {
    $search[] = 'bundle_' . $bundle;
  }

  $has_selected = FALSE;
  foreach ($search as $group) {
    // Store names of selected and appended entity values in a nicer format.
    $form_state['selected'][$group] = array();
    $form_state['append'][$group] = array();

    // This group has no values, move on.
    if (!isset($form_state['values'][$group])) {
      continue;
    }

    foreach ($form_state['values'][$group]['show_value'] as $key => $value) {
      if ($value) {
        $has_selected = TRUE;
        $form_state['selected'][$group][] = $key;
      }
      if (!empty($form_state['values'][$group]['_append::' . $key])) {
        $form_state['append'][$group][] = $key;
        unset($form_state['values'][$group]['_append::' . $key]);
      }
    }
    unset($form_state['values'][$group]['show_value']);
  }

  if (!$has_selected) {
    form_set_error('', t('You must select at least one value to modify.'));
    return;
  }

  // Use the wrapper to validate property values.
  if (!empty($form_state['selected']['properties'])) {
    // The entity used is irrelevant, and we can't rely on
    // $form_state['entities'] being non-empty, so a new one is created.
    $info = entity_get_info($form_state['entity_type']);
    $bundle_key = $info['entity keys']['bundle'];
    $default_values = array();
    // If the bundle key exists, it must always be set on an entity.
    if (!empty($bundle_key)) {
      $bundle_names = array_keys($info['bundles']);
      $bundle_name = reset($bundle_names);
      $default_values[$bundle_key] = $bundle_name;
    }
    $entity = entity_create($form_state['entity_type'], $default_values);
    $wrapper = entity_metadata_wrapper($form_state['entity_type'], $entity);

    $properties = _views_bulk_operations_modify_action_get_properties($form_state['entity_type']);
    foreach ($form_state['selected']['properties'] as $key) {
      $value = $form_state['values']['properties'][$key];
      if (!$wrapper->$key->validate($value)) {
        $label = $properties[$key]['label'];
        form_set_error('properties][' . $key, t('%label contains an invalid value.', array('%label' => $label)));
      }
    }
  }

  foreach ($form_state['entities'] as $bundle_name => $entity) {
    field_attach_form_validate($form_state['entity_type'], $entity, $form['bundle_' . $bundle_name], $form_state);
  }
}

/**
 * Action form submit function.
 *
 * Fills each constructed entity with property and field values, then
 * passes them to views_bulk_operations_modify_action().
 */
function views_bulk_operations_modify_action_submit($form, $form_state) {
  foreach ($form_state['entities'] as $bundle_name => $entity) {
    field_attach_submit($form_state['entity_type'], $entity, $form['bundle_' . $bundle_name], $form_state);
  }

  return array(
    'append' => $form_state['append'],
    'selected' => $form_state['selected'],
    'entities' => $form_state['entities'],
    'properties' => isset($form_state['values']['properties']) ? $form_state['values']['properties'] : array(),
  );
}

/**
 * Returns all properties that can be modified.
 *
 * Properties that can't be changed are entity keys, timestamps, and the ones
 * without a setter callback.
 *
 * @param $entity_type
 *   The entity type whose properties will be fetched.
 * @param $display_values
 *   An optional, admin-provided list of properties and fields that should be
 *   displayed for editing, used to filter the returned list of properties.
 */
function _views_bulk_operations_modify_action_get_properties($entity_type, $display_values = NULL) {
  $properties = array();
  $info = entity_get_info($entity_type);

  // List of properties that can't be modified.
  $disabled_properties = array('created', 'changed');
  foreach (array('id', 'bundle', 'revision') as $key) {
    if (!empty($info['entity keys'][$key])) {
      $disabled_properties[] = $info['entity keys'][$key];
    }
  }
  // List of supported types.
  $supported_types = array('text', 'token', 'integer', 'decimal', 'date', 'duration',
                           'boolean', 'uri', 'list');
  $property_info = entity_get_property_info($entity_type);
  if (empty($property_info['properties'])) {
    // Stop here if no properties were found.
    return array();
  }

  foreach ($property_info['properties'] as $key => $property) {
    if (in_array($key, $disabled_properties)) {
      continue;
    }
    // Filter out properties that can't be set (they are usually generated by a
    // getter callback based on other properties, and not stored in the DB).
    if (empty($property['setter callback'])) {
      continue;
    }
    // Determine the property type. If it's empty (permitted), default to text.
    // If it's a list type such as list<boolean>, extract the "boolean" part.
    $property['type'] = empty($property['type']) ? 'text' : $property['type'];
    $type = $property['type'];
    if ($list_type = entity_property_list_extract_type($type)) {
      $type = $list_type;
      $property['type'] = 'list';
    }
    // Filter out non-supported types (such as the Field API fields that
    // Commerce adds to its entities so that they show up in tokens).
    if (!in_array($type, $supported_types)) {
      continue;
    }

    $properties[$key] = $property;
  }

  if (isset($display_values) && empty($display_values[VBO_MODIFY_ACTION_ALL])) {
    // Return only the properties that the admin specified.
    return array_intersect_key($properties, $display_values);
  }

  return $properties;
}

/**
 * Returns all bundles for which field widgets should be displayed.
 *
 * If the admin decided to limit the modify form to certain properties / fields
 * (through the action settings) then only bundles that have at least one field
 * selected are returned.
 *
 * @param $entity_type
 *   The entity type whose bundles will be fetched.
 * @param $context
 *   The VBO context variable.
 */
function _views_bulk_operations_modify_action_get_bundles($entity_type, $context) {
  $bundles = array();

  $view = $context['view'];
  $vbo = _views_bulk_operations_get_field($view);
  $display_values = $context['settings']['display_values'];
  $info = entity_get_info($entity_type);
  $bundle_key = $info['entity keys']['bundle'];

  // Check if this View has a filter on the bundle key and assemble a list
  // of allowed bundles according to the filter.
  $filtered_bundles = array_keys($info['bundles']);

  // Go over all the filters and find any relevant ones.
  foreach ($view->filter as $key => $filter) {
    // Check it's the right field on the right table.
    if ($filter->table == $vbo->table && $filter->field == $bundle_key) {
      // Exposed filters may have no bundles, so check that there is a value.
      if (empty($filter->value)) {
        continue;
      }

      $operator = $filter->operator;
      if ($operator == 'in') {
        $filtered_bundles = array_intersect($filtered_bundles, $filter->value);
      }
      elseif ($operator == 'not in') {
        $filtered_bundles = array_diff($filtered_bundles, $filter->value);
      }
    }
  }

  foreach ($info['bundles'] as $bundle_name => $bundle) {
    // The view is limited to specific bundles, but this bundle isn't one of
    // them. Ignore it.
    if (!in_array($bundle_name, $filtered_bundles)) {
      continue;
    }

    $instances = field_info_instances($entity_type, $bundle_name);
    // Ignore bundles that don't have any field instances attached.
    if (empty($instances)) {
      continue;
    }

    $has_enabled_fields = FALSE;
    foreach ($display_values as $key) {
      if (strpos($key, $bundle_name . '::') !== FALSE) {
        $has_enabled_fields = TRUE;
      }
    }
    // The admin has either specified that all values should be modifiable, or
    // selected at least one field belonging to this bundle.
    if (!empty($display_values[VBO_MODIFY_ACTION_ALL]) || $has_enabled_fields) {
      $bundles[$bundle_name] = $bundle;
    }
  }

  return $bundles;
}

/**
 * Helper function that recursively strips #required from field widgets.
 */
function _views_bulk_operations_modify_action_unset_required(&$element) {
  unset($element['#required']);
  foreach (element_children($element) as $key) {
    _views_bulk_operations_modify_action_unset_required($element[$key]);
  }
}

/**
 * VBO settings form function.
 */
function views_bulk_operations_modify_action_views_bulk_operations_form_options() {
  $options['show_all_tokens'] = TRUE;
  $options['display_values'] = array(VBO_MODIFY_ACTION_ALL);
  return $options;
}

/**
 * The settings form for this action.
 */
function views_bulk_operations_modify_action_views_bulk_operations_form($options, $entity_type, $dom_id) {
  // Initialize default values.
  if (empty($options)) {
    $options = views_bulk_operations_modify_action_views_bulk_operations_form_options();
  }

  $form['show_all_tokens'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show available tokens'),
    '#description' => t('Check this to show a list of all available tokens in the bottom of the form. Requires the token module.'),
    '#default_value' => $options['show_all_tokens'],
  );

  $info = entity_get_info($entity_type);
  $properties = _views_bulk_operations_modify_action_get_properties($entity_type);
  $values = array(VBO_MODIFY_ACTION_ALL => t('- All -'));
  foreach ($properties as $key => $property) {
    $label = t('Properties');
    $values[$label][$key] = $property['label'];
  }
  foreach ($info['bundles'] as $bundle_name => $bundle) {
    $bundle_key = $info['entity keys']['bundle'];
    // Show the more detailed label only if the entity type has multiple bundles.
    // Otherwise, it would just be confusing.
    if (count($info['bundles']) > 1) {
      $label = t('Fields for @bundle_key @label', array('@bundle_key' => $bundle_key, '@label' => $bundle['label']));
    }
    else {
      $label = t('Fields');
    }

    $instances = field_info_instances($entity_type, $bundle_name);
    foreach ($instances as $field_name => $field) {
      $values[$label][$bundle_name . '::' . $field_name] = $field['label'];
    }
  }

  $form['display_values'] = array(
    '#type' => 'select',
    '#title' => t('Display values'),
    '#options' => $values,
    '#multiple' => TRUE,
    '#description' => t('Select which values the action form should present to the user.'),
    '#default_value' => $options['display_values'],
    '#size' => 10,
  );
  return $form;
}
