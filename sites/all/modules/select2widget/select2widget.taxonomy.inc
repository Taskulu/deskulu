<?php
/**
 * Process callback: 'select2widget' element type.
 */
function select2widget_taxonomy_process_callback($element, &$form_state, $form) {
  // Send Drupal.settings a reference to this form element.

  $settings = $element['#settings']['select2widgetajax'];
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
    'label' => $element['#label'],
    'allow_new' => $settings['allow_new'],
    'separator' => $settings['separator'],
    'cardinality' => $cardinality,
    'delay' => variable_get('select2_delay', '100'),
    'set_level' => $settings['set_level'],
    'min_char' => $settings['min_char'],
    'placeholder' => $settings['placeholder'],
    'type' => 'taxonomy_term_reference',
    'init' => $element['#init'],
  );

  // Attaching library, integration script, and settings array.
  $element['#attached']['library'][] = array('select2widget', 'select2');
  $element['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => $data,
  );
  return $element;
}


/**
 * Implements hook_field_widget_settings_form().
 */
function select2widget_taxonomy_term_reference_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];

  $settings = array_replace_recursive(field_info_widget_settings($widget['type']), $widget['settings']);

  $form = array();
  if ($widget['type'] == 'select2widgetajax') {

    $form['select2widgetajax'] = array(
      '#type' => 'fieldset',
      '#title' =>  t('Select2Widget settings'),
      '#tree' => TRUE,
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

    $form['select2widgetajax']['separator'] = array(
      '#type' => 'textfield',
      '#size' => 1,
      '#title' => t('Separator'),
      '#description' => t('Separator used'),
      '#default_value' => $settings['select2widgetajax']['separator'],
      '#maxlength' => 1,
    );

    $form['select2widgetajax']['allow_new'] = array(
      '#type' => 'radios',
      '#title' => 'Allow new terms',
      '#options' => array(
        1 => t('Allow and insert new terms'),
        2 => t('Allow new terms and insert them into a separate vocabulary'),
        0 => t('Deny any new terms'),
      ),
      '#default_value' => $settings['select2widgetajax']['allow_new'],
      '#description' => t('If option 2 is selected, re-save this settings form and afterwards select a second vocabulary for new terms in the field settings.'),
    );

    $form['select2widgetajax']['set_level'] = array(
      '#type' => 'checkbox',
      '#title' => t('Taxonomy term level'),
      '#description' => t('Show only terms from last level'),
      '#default_value' => $settings['select2widgetajax']['set_level'],
    );

    $form['select2widgetajax']['min_char'] = array(
      '#type' => 'select',
      '#title' => t('Number of minimum character'),
      '#description' => t('How many character must be typed before results is displayed'),
      '#options' => range(0, 5),
      '#default_value' => $settings['select2widgetajax']['min_char'],
    );
  }

  return $form;
}

/**
 * Return JSON based on given field, instance and string.
 *
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

function select2widget_taxonomy_term_reference_get_matches($field, $instance, $entity_type, $string, $entity_id = '') {
  $voc_name = $field['settings']['allowed_values'][0]['vocabulary'];
  $settings = $instance['widget']['settings'];
  $voc = taxonomy_vocabulary_machine_name_load($voc_name);
  $vid = (int)$voc->vid;
  $set_level = isset($settings['set_level']) && $settings['set_level'];

  $query = db_select('taxonomy_term_data', 't')
    ->fields('t', array('tid', 'name'));

  if (module_exists('termstatus')) {
    $query->leftJoin('termstatus', 's', 't.tid = s.tid');
    $query->condition('s.status', 1);
  }

  if ($set_level) {
    $query->leftJoin('taxonomy_term_hierarchy', 'h', 'h.parent = t.tid');
    $query->condition('h.tid', NULL);
  }

  $query->condition('t.vid', (int)$vid)
        ->range(0, 10);

  if ($settings['match_operator'] === 'CONTAINS') {
    $query->condition('t.name', '%' . $string . '%', 'LIKE');
  }
  else {
    $query->condition('t.name', $string . '%', 'LIKE');
  }

  $results = $query->execute()->fetchAllAssoc('tid');

  $terms = array();

  if (!empty($results)) {
    $terms = taxonomy_term_load_multiple(array_keys($results));
  }

  $matches = array();

  $output = array();

  foreach ($terms as $term) {
    $parent = '';

    $output[$term->tid] = array(
      'name' => $term->name,
      'parent' => $parent,
    );

    $parents = taxonomy_get_parents_all($term->tid);
    $count = count($parents);
    for ($i=$count-1; $i>0; $i--) {
      $parent .= $parents[$i]->name;
      if ($i != 1) {
        $parent .= ' > ';
      }
    }

    if (!empty($parent)) {
      $output[$term->tid]['parent'] = ' (' . $parent . ')';
    }
  }

  foreach ($output as $tid => $term) {
    $matches[] = array(
      'id' => 'id' . chr(2) . $tid,
      'title' => $term['name'],
      'data' => '<div class="select2-taxonomy-term">' . $term['name'] . $term['parent'] . '</div>',
    );
  }

  drupal_json_output($matches);
}

/**
 * Sets form value
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function select2widget_validate_field(&$element, &$form_state, $form) {
  $field = field_widget_field($element, $form_state);
  $settings = $element['#settings']['select2widgetajax'];
  $vocabularies = array();
  $default = array();

  $vocabulary_name = $field['settings']['allowed_values'][0]['vocabulary'];
  $vocabularies[] = $vocabulary_name;

  if ($settings['allow_new'] == 2) {
    $vocabulary_name = $field['settings']['allowed_values'][1]['vocabulary'];
    $vocabularies[] = $vocabulary_name;
  }

  $vid = taxonomy_vocabulary_machine_name_load($vocabulary_name)->vid;

  $terms = $element['#value'];
  $terms = str_replace('id' . chr(2), '', $terms);
  $terms = explode($settings['separator'], $terms);
  $terms = array_filter($terms);

  $value = array();
  foreach ($terms as $term) {
    // In case we received a tid
    $taxonomy = taxonomy_term_load($term);

    if ($taxonomy !== FALSE && in_array($taxonomy->vocabulary_machine_name, $vocabularies)) {
      $value[] = array('tid' => $taxonomy->tid);
    }
    else { //We don't have the term in our taxonomy, so we have to create it
      $taxonomy = (object) array(
        'vid' => $vid,
        'name' => $term,
        'status' => 0,
        'vocabulary_machine_name' => $vocabulary_name,
      );
      taxonomy_term_save($taxonomy);
      $value[] = array('tid' => $taxonomy->tid);
    }

    $default['id' . chr(2) . $taxonomy->tid] = $taxonomy->name;
  }

  $element['#attached']['js'][0]['data']['select2widgetajax']['elements'][$element['#id']]['init'] = $default;
  form_set_value($element, $value, $form_state);
}

function select2widget_form_field_ui_field_edit_form_alter(&$form, &$form_state) {
  $field = $form['#field'];
  $instance = $form['#instance'];

  if (isset($instance['widget']['settings']['select2widgetajax']['allow_new'])
    && $instance['widget']['settings']['select2widgetajax']['allow_new'] == 2
    && $instance['widget']['type'] == 'select2widgetajax') {

    // Initialize settings, if not set.
    if (!isset($field['settings']['allowed_values'][1])) {
      $field['settings']['allowed_values'][1] = array(
        'vocabulary' => $field['settings']['allowed_values'][0]['vocabulary'],
        'parent' => 0,
      );
    }

    $vocabularies = taxonomy_get_vocabularies();
    $options = array();

    foreach ($vocabularies as $vocabulary) {
      $options[$vocabulary->machine_name] = $vocabulary->name;
    }

    $form['field']['settings']['allowed_values'][1]['vocabulary'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary for new terms'),
      '#default_value' => isset($field['settings']['allowed_values'][1]['vocabulary']) ? $field['settings']['allowed_values'][1]['vocabulary'] : '',
      '#options' => $options,
      '#description' => t('New terms form autocompletes will be inserted in this vocabulary.'),
      //'#disabled' => $has_data, //todo
    );
    $form['field']['settings']['allowed_values'][1]['parent'] = array(
      '#type' => 'value',
      '#value' => $field['settings']['allowed_values'][1]['parent'],
    );
  }
  else {
    if (isset($field['settings']['allowed_values'][1])) {
      unset($form['field']['settings']['allowed_values'][1]);
    }
  }
}

