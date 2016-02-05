<?php
/**
 * Created by IntelliJ IDEA.
 * User: svip
 * Date: 19.07.13
 * Time: 17:09
 * To change this template use File | Settings | File Templates.
 */

/**
 * Implements hook_field_widget_settings_form().
 */
function select2widget_entityreference_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];

  $settings = array_replace_recursive(field_info_widget_settings($widget['type']), $widget['settings']);

  $form = array();
  if ($widget['type'] == 'select2widgetajax') {
    $view_mode_options = array(
      'select2widget_labels' => t('Entity labels (no render)'),
    );

    $entity_info = entity_get_info();
    $entity_type = (isset($field['settings']['target_type'])) ? $field['settings']['target_type'] : NULL;

    if (isset($entity_info[$entity_type]['view modes'])) {
      foreach ($entity_info[$entity_type]['view modes'] as $key => $value) {
        $view_mode_options[$key] = $value['label'];
      }
    }

    $form['select2widgetajax'] = array(
      '#type' => 'fieldset',
      '#title' =>  t('Select2Widget settings'),
      '#tree' => TRUE,
    );

    $form['select2widgetajax']['view_mode'] = array(
      '#type' => 'select',
      '#title' => t('View mode'),
      '#default_value' => (isset($settings['select2widgetajax']['view_mode'])) ? $settings['select2widgetajax']['view_mode'] : reset($view_mode_options),
      '#options' => $view_mode_options,
      '#description' => t('Install !entity_view_mode_link module to create own view modes.',
        array('!entity_view_mode_link' => l(t('"Entity view modes"'), "http://drupal.org/project/entity_view_mode",
        array('attributes' => array('target' => '_blank'))))),
    );

    $form['select2widgetajax']['placeholder'] = array(
      '#type' => 'textfield',
      '#title' => t('Placeholder text'),
      '#description' => t('Type a text for the placeholder'),
      '#default_value' => $settings['select2widgetajax']['placeholder'],
    );


    $form['select2widgetajax']['match_operator'] = array(
      '#type' => 'select',
      '#title' => t('Search matching'),
      '#default_value' => $settings['select2widgetajax']['match_operator'],
      '#options' => array(
        'STARTS_WITH' => t('Starts with'),
        'CONTAINS' => t('Contains'),
      ),
      '#description' => t('Select the method used to collect autocomplete suggestions. Note that <em>Contains</em> can cause performance issues on sites with thousands of nodes.'),
    );

    $form['select2widgetajax']['width'] = array(
      '#type' => 'textfield',
      '#size' => '5',
      '#title' => t('Width'),
      '#default_value' => isset($settings['select2widgetajax']['width']) ? $settings['select2widgetajax']['width'] : '',
      '#element_validate' => array('_element_width_validate'),
      '#description' => t('You may either leave Width empty or enter a string like "500px" or "50%"')
    );

    $form['select2widgetajax']['min_char'] = array(
      '#type' => 'select',
      '#title' => t('Number of minimum character'),
      '#description' => t('How many character must be typed before results is displayed'),
      '#options' => range(0, 5),
      '#default_value' => isset($settings['select2widgetajax']['min_char']) ? $settings['select2widgetajax']['min_char'] : 0,
    );


  }

  return $form;
}


function _element_width_validate($element, &$form_state) {
  $element['#value'] = trim($element['#value']);
  $input = preg_replace('/^[0-9]+\.?([0-9]+)?(px|%)$/', '', $element['#value']);

  if (!empty($element['#value']) && !empty($input)) {
    form_error($element, t('The "!name" option must contain a valid value. You may either leave the text field empty or enter a string like "500px" or "50%"', array('!name' => t($element['#title']))));
  }
}


/**
 * Process callback: 'select2widget' element type.
 */
function select2widget_entity_process_callback($element, &$form_state, $form) {
  // Send Drupal.settings a reference to this form element.
  $field_name = $element['#field_name'];
  $langcode = $element['#language'];

  if (!empty($form_state['field'][$field_name])) {
    $cardinality = $form_state['field'][$field_name][$langcode]['field']['cardinality'];
  }
  else {
    $parents = array_merge($element['#parents'], array($field_name));
    $field = drupal_array_get_nested_value($form_state, $parents, $langcode);
    $cardinality = $field['field']['cardinality'];
  }

  $data['select2widgetajax']['elements'][$element['#id']] = array(
    'id' => $element['#id'],
    'url' => $element['#autocomplete_path'],
    'cardinality' => $cardinality,
    'init' => $element['#init'],
    'placeholder' => isset($element['#settings']['select2widgetajax']['placeholder']) ? $element['#settings']['select2widgetajax']['placeholder'] : '',
    'min_char' => $element['#settings']['select2widgetajax']['min_char']
  );

  if(isset($element['#settings']['select2widgetajax']['width'])){
    $data['select2widgetajax']['elements'][$element['#id']]['width'] = $element['#settings']['select2widgetajax']['width'];
  }

  $data['select2widgetajax']['elements'][$element['#id']]['delay'] = variable_get('select2_delay', '100');

  // Attaching library, integration script, and settings array.
  $element['#attached']['library'][] = array('select2widget', 'select2');
  $element['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => $data,
  );

  return $element;
}


/**
 * Return JSON based on given field, instance and string.
 *
 * This function can be used by other modules that wish to pass a mocked
 * definition of the field on instance.
 *
 * @param $field
 *   The field array defintion.
 * @param $instance
 *   The instance array defintion.
 * @param $entity_type
 *   The entity type.
 * @param $entity_id
 *   Optional; The entity ID the entity-reference field is attached to.
 *   Defaults to ''.
 * @param $string
 *   The label of the entity to query by.
 */
function select2widget_entityreference_get_matches($field, $instance, $entity_type, $string, $entity_id = '') {
  $matches = array();
  $options = array();

  $entity = NULL;
  if ($entity_id !== 'NULL') {
    $entity = entity_load_single($entity_type, $entity_id);
    if (!$entity || !entity_access('view', $entity_type, $entity)) {
      return MENU_ACCESS_DENIED;
    }
  }
  if ($instance['widget']['type'] == 'og_complex') {
    $instance['field_mode'] = 'default';
    // Set the current field mode if passed in.
    if (!empty($_GET['field_mode']) && in_array($_GET['field_mode'], array('admin', 'default'))) {
      $instance['field_mode'] = $_GET['field_mode'];
    }
    $settings = isset($instance['settings']['behaviors']['og_widget'][$instance['field_mode']]['widget_settings']['select2widgetajax']) ? $instance['settings']['behaviors']['og_widget'][$instance['field_mode']]['widget_settings']['select2widgetajax'] : array();
  }
  else {
    $settings = $instance['widget']['settings']['select2widgetajax'];
  }

  $handler = entityreference_get_selection_handler($field, $instance, $entity_type, $entity);
  $entity_labels = $handler->getReferencableEntities($string, $settings['match_operator'], 20);
  $matches = select2widget_render_modes($entity_labels, $settings['view_mode'], $field['settings']['target_type']);

  drupal_json_output($matches);
}


/**
 * Render entities using display mode
 *
 * @param $entity_labels
 * @param string $view_mode
 * @return array
 */
function select2widget_render_modes($entity_labels, $view_mode = "full", $entity_type = "node") {
  $matches = array();

  foreach ($entity_labels as $values) {
    $ids = array_keys($values);
    $entities = entity_load_multiple_by_name($entity_type, $ids);
    foreach ($entities as $entity) {
      $entity_array = select2widget_render_entity($entity_type, $entity, $view_mode);
      $matches[] = array(
        'id' => $entity_array['id'],
        'title' => $entity_array['title'],
        'data' => '<div class="reference-select2widget">' . render($entity_array['data']) . '</div>',
      );
    }
  }
  return $matches;
}


/**
 * Render single entity
 *
 * @param $entity_type
 * @param $entity
 * @param $view_mode
 * @return array
 */
function select2widget_render_entity($entity_type, $entity, $view_mode = 'full') {
  $entity->select2widget = TRUE;
  $entity_array = array(
    'id' => NULL,
    'title' => NULL,
    'data' => NULL,
  );

  // Allow just the entity label to be returned.
  if ($view_mode == 'select2widget_labels') {
    $entity_id = entity_id($entity_type, $entity);
    $entity_label = entity_label($entity_type, $entity);
    $entity_array['id'] = $entity_id;
    $entity_array['title'] = $entity_label . " - " . $entity_id;
    $entity_array['data'] = array('#markup' => $entity_label);
    return $entity_array;
  }

  switch ($entity_type) {
    case "node":
        $entity_array['id'] = $entity->nid;
        $entity_array['title'] = $entity->title . " - " . $entity->nid;
        $entity_array['data'] = node_view($entity, $view_mode);
      break;
    case "user":
        $entity_array['id'] = $entity->uid;
        $entity_array['title'] = $entity->name . " - " . $entity->uid;
        $entity_array['data'] = user_view($entity, $view_mode);
      break;
    case "taxonomy_term":
        $entity_array['id'] = $entity->tid;
        $entity_array['title'] = $entity->name . " - " . $entity->tid;
        $entity_array['data'] = taxonomy_term_view($entity, $view_mode);
      break;
    default:
        $entity_id = entity_id($entity_type, $entity);
        $entity_label = entity_label($entity_type, $entity);
        $entity_array['id'] = $entity_id;
        $entity_array['title'] =  $entity_label . " - " . $entity_id;
        $entity_array['data'] = entity_view($entity_type, array($entity), $view_mode);
  }

  return $entity_array;
}

/**
 * Sets form value
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function select2widget_entity_validate_field(&$element, &$form_state, $form) {
  $value = array();
  $entity_labels = array();
  $settings = $element['#settings']['select2widgetajax'];

  $field = field_widget_field($element, $form_state);
  $instance = field_info_instance($element['#entity_type'], $element['#field_name'], $element['#bundle']);

  $target_entities = explode(',', $element['#value']);

  foreach ($target_entities as &$target_entity) {
    if (is_numeric($target_entity)) {
      $value[] = array('target_id' => $target_entity);
      $entity = entity_load_single($field['settings']['target_type'], $target_entity);

      if($entity !== FALSE) {
        $label = entity_label($field['settings']['target_type'], $entity);
        $key = "$label ($target_entity)";
      }
      else {
        $key = "Anonymous ($target_entity)";
      }
      // Labels containing commas or quotes must be wrapped in quotes.
      if (strpos($key, ',') !== FALSE || strpos($key, '"') !== FALSE) {
        $key = '"' . str_replace('"', '""', $key) . '"';
      }

      $entity_labels[$target_entity] = $key;
    }

    elseif (preg_match("/.+\((\d+)\)/", $target_entity, $matches)) {
      $entity_labels[$matches[1]] = check_plain($target_entity);
      $value[] = array('target_id' => $matches[1]);
    }

  }

  $element['#attached']['js'][0]['data']['select2widgetajax']['elements'][$element['#id']]['init'] = $entity_labels; //Update default values

  form_set_value($element, $value, $form_state);
}
