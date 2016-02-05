<?php

/**
 * @file
 * Views Bulk Operations conditions and actions for Rules.
 */

/**
 * Implements hook_rules_condition_info().
 */
function views_bulk_operations_rules_condition_info() {
  $conditions = array();
  $conditions['views_bulk_operations_condition_result_count'] = array(
    'label' => t('Check number of results returned by a VBO View'),
    'parameter' => array(
      'view' => array(
        'type' => 'text',
        'label' => t('View and display'),
        'options list' => 'views_bulk_operations_views_list',
        'description' => t('Select the VBO view and display you want to check'),
        'restriction' => 'input',
      ),
      'args' => array(
        'type' => 'text',
        'label' => t('Arguments'),
        'description' => t('Any arguments to pass to the view, one per line.
          You may use token replacement patterns.'),
        'optional' => TRUE,
      ),
      'minimum' => array(
        'type' => 'integer',
        'label' => t('Minimum number of results'),
        'description' => t('This condition returns TRUE if the view has at
          least the given number of results.'),
      ),
    ),
    'group' => t('Views Bulk Operations'),
  );

  return $conditions;
}

/**
 * Implements hook_rules_action_info().
 */
function views_bulk_operations_rules_action_info() {
  $actions = array();
  $actions['views_bulk_operations_action_load_list'] = array(
    'label' => t('Load a list of entity objects from a VBO View.'),
    'parameter' => array(
      'view' => array(
        'type' => 'text',
        'label' => t('View and display'),
        'options list' => 'views_bulk_operations_views_list',
        'description' => t('Select the view and display you want to use to
          create a list.'),
        'restriction' => 'input',
      ),
      'args' => array(
        'type' => 'text',
        'label' => t('Arguments'),
        'description' => t('Any arguments to pass to the view, one per line.
          You may use token replacement patterns.'),
        'optional' => TRUE,
      ),
    ),
    'provides' => array(
      'entity_list' => array(
        'type' => 'list<entity>',
        'label' => t('A list of entities'),
      ),
    ),
    'group' => t('Views Bulk Operations'),
  );
  $actions['views_bulk_operations_action_load_id_list'] = array(
    'label' => t('Load a list of entity ids from a VBO View.'),
    'parameter' => array(
      'view' => array(
        'type' => 'text',
        'label' => t('View and display'),
        'options list' => 'views_bulk_operations_views_list',
        'description' => t('Select the view and display you want to use to
          create a list.'),
        'restriction' => 'input',
      ),
      'args' => array(
        'type' => 'text',
        'label' => t('Arguments'),
        'description' => t('Any arguments to pass to the view, one per line.
          You may use token replacement patterns.'),
        'optional' => TRUE,
      ),
    ),
    'provides' => array(
      'entity_id_list' => array(
        'type' => 'list<integer>',
        'label' => t('A list of entity ids'),
      ),
    ),
    'group' => t('Views Bulk Operations'),
  );

  return $actions;
}

/**
 * Lists all available VBO Views and their displays.
 * Naturally, only the displays that contain a VBO field are listed.
 *
 * @return array
 *   An array of all views and their displays on the form 'view|display',
 *   formatted to be used as an select list.
 */
function views_bulk_operations_views_list() {
  $selectable_displays = array();
  foreach (views_get_enabled_views() as $name => $base_view) {
    foreach ($base_view->display as $display_name => $display) {
      $view = $base_view->clone_view();
      $view->build($display_name);
      $vbo = _views_bulk_operations_get_field($view);
      if ($vbo) {
        $selectable_displays[$view->name . '|' . $display_name] = check_plain($view->human_name) . ' | ' . check_plain($display->display_title);
      }
    }
  }
  return $selectable_displays;
}

/**
 * The 'views_bulk_operations_condition_result_count' condition.
 *
 * @param $view
 *   A string in the format "$view_name|$display_name".
 * @param $args
 *   Arguments that should be passed to the View.
 * @param $minimum
 *   An integer representing the minimum number of results that satisfies the
 *   condition.
 *
 * @return
 *   TRUE if the view has more than $minimum results, FALSE otherwise.
 */
function views_bulk_operations_condition_result_count($view, $args, $minimum) {
  $vbo = _views_bulk_operations_rules_get_field($view, $args);
  return (count($vbo->view->result) >= $minimum);
}

/**
 * The 'views_bulk_operations_action_views_load_list' action.
 *
 * @param $view
 *   A string in the format "$view_name|$display_name".
 * @param $args
 *   Arguments that should be passed to the View.
 * @return array
 *   Array containing the entity_list, an array of entity objects.
 *   - array('entity_list' => array(...))
 */
function views_bulk_operations_action_load_list($view, $args) {
  $vbo = _views_bulk_operations_rules_get_field($view, $args);

  // Get all entities, pass ids to the wrapper for lazy loading.
  $entity_type = $vbo->get_entity_type();
  $entities = entity_metadata_wrapper("list<$entity_type>", array());
  foreach ($vbo->view->result as $row_index => $result) {
    $entities[] = entity_metadata_wrapper($entity_type, $vbo->get_value($result));
  }

  return array('entity_list' => $entities);
}

/**
 * The 'views_bulk_operations_action_views_load_id_list' action.
 *
 * @param $view
 *   A string in the format "$view_name|$display_name".
 * @param $args
 *   Arguments that should be passed to the View.
 * @return array
 *   Array containing the entity_id_list, an Array of entity ids as integer
 *   values.
 *   - array('entity_list' => array(...))
 */
function views_bulk_operations_action_load_id_list($view, $args) {
  $vbo = _views_bulk_operations_rules_get_field($view, $args);

  // Get all entity ids.
  $ids = array();
  foreach ($vbo->view->result as $row_index => $result) {
    $ids[] = $vbo->get_value($result);
  }

  return array('entity_id_list' => $ids);
}

/**
 * Info alteration callback for the 'views_bulk_operations_action_views_load_list' action.
 *
 * The info hook specifies that the action returns a generic list of entities
 * (list<entity>). All actions that require entities of specific type can't
 * use such entities, so this alter hook specifies the exact entity type
 * after the action has been configured, allowing the view to be loaded
 * and its entity type extracted.
 */
function views_bulk_operations_action_load_list_info_alter(&$element_info, RulesAbstractPlugin $element) {
  // The action hasn't been configured yet, hence no view. Abort.
  if (empty($element->settings['view'])) {
    return;
  }

  $entity_type = _views_bulk_operations_rules_get_entity_type($element->settings['view']);
  if ($entity_type) {
    $element_info['provides']['entity_list']['type'] = "list<$entity_type>";
  }
}

/**
 * Helper function that loads and builds (but doesn't execute) the specified view,
 * then determines the entity type on which the VBO field operates.
 *
 * @param $view_target
 *   A string in the format "$view_name|$display_name".
 *
 * @return
 *   The entity type on which the VBO field operates.
 */
function _views_bulk_operations_rules_get_entity_type($view_target) {
  $entity_types = &drupal_static(__FUNCTION__);

  if (!isset($entity_types[$view_target])) {
    $views_settings = explode('|', $view_target);
    if ($view = views_get_view($views_settings[0])) {
      $view->set_display($views_settings[1]);
      $view->build();

      $vbo = _views_bulk_operations_get_field($view);
    }
    $entity_type = !empty($vbo) ? $vbo->get_entity_type() : '';
    $entity_types[$view_target] = $entity_type;
  }

  return $entity_types[$view_target];
}

/**
 * Helper function that loads, builds and executes the specified view,
 * then returns its VBO field.
 *
 * @param $view_target
 *   A string in the format "$view_name|$display_name".
 * @param $args
 *   Arguments that should be passed to the View.
 *
 * @return
 *   The VBO field. Contains a reference to the View.
 */
function _views_bulk_operations_rules_get_field($view_target, $args) {
  $views = &drupal_static(__FUNCTION__);

  $views_settings = explode('|', $view_target);
  $view_name = $views_settings[0];
  $display_name = $views_settings[1];
  // Create an array of arguments.
  $view_arguments = explode("\n", $args);
  $view_arguments = array_map('trim', $view_arguments);
  $view_arguments = array_filter($view_arguments, 'strlen');
  // Append the filtered list of arguments to $views_target, so that the correct
  // View is fetched from cache.
  if (!empty($view_arguments)) {
    $view_target .= '|' . implode('&', $view_arguments);
  }

  // Don't execute the requested View more than once in a single page request.
  if (isset($views[$view_target])) {
    $vbo = _views_bulk_operations_get_field($views[$view_target]);
    return $vbo;
  }

  // Load the view and set the properties.
  $view = views_get_view($view_name);
  $view->set_display($display_name);
  $view->set_arguments($view_arguments);
  $view->build();
  $vbo = _views_bulk_operations_get_field($view);
  // Unset every field except the VBO one (which holds the entity id).
  // That way the performance hit becomes much smaller, because there is no
  // chance of views_handler_field_field::post_execute() firing entity_load().
  foreach ($view->field as $field_name => $field) {
    if ($field_name != $vbo->options['id']) {
      unset($view->field[$field_name]);
    }
  }
  $view->execute($view->current_display);
  // Save the view in the static cache.
  $views[$view_target] = $view;

  return $vbo;
}
