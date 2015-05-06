<?php

/**
 * @file
 * Provide field replacement information for core entities and type specific
 * callbacks.
 */

/**
 * Implements hook_entity_info().
 */
function title_entity_info() {
  $info = array();

  $field = array(
    'type' => 'text',
    'cardinality' => 1,
    'translatable' => TRUE,
  );

  $instance = array(
    'required' => TRUE,
    'settings' => array(
      'text_processing' => 0,
    ),
    'widget' => array(
      'weight' => -5,
    ),
    'display' => array(
      'default' => array(
        'type' => 'hidden',
      ),
    ),
  );

  $info['node'] = array(
    'field replacement' => array(
      'title' => array(
        'field' => $field,
        'instance' => array(
          'label' => t('Title'),
          'description' => '',
        ) + $instance,
      ),
    ),
    'efq bundle conditions' => TRUE,
  );

  if (module_exists('taxonomy')) {
    $info['taxonomy_term'] = array(
      'field replacement' => array(
        'name' => array(
          'field' => $field,
          'instance' => array(
            'label' => t('Name'),
            'description' => '',
          ) + $instance,
          'preprocess_key' => 'term_name',
        ),
        'description' => array(
          'field' => array(
            'type' => 'text_with_summary',
          ) + $field,
          'instance' => array(
            'required' => FALSE,
            'label' => t('Description'),
            'description' => '',
            'settings' => array(
              'text_processing' => 1,
            ),
          ) + $instance,
          'callbacks' => array(
            'submit' => 'title_field_term_description_submit',
          ),
          'additional keys' => array(
            'format' => 'format',
          ),
        ),
      ),
    );
  }

  if (module_exists('comment')) {
    $info['comment'] = array(
      'field replacement' => array(
        'subject' => array(
          'field' => $field,
          'instance' => array(
            'label' => t('Subject'),
            'description' => '',
          ) + $instance,
          'preprocess_key' => 'title',
        ),
      ),
    );
  }

  return $info;
}

/**
 * Submit callback for the taxonomy term description.
 */
function title_field_term_description_submit($entity_type, $entity, $legacy_field, $info, $langcode, &$values) {
  if (!isset($values['description'])) {
    $values['description'] = array();
  }
  foreach (array('value', 'format') as $key) {
    if (isset($entity->{$info['field']['field_name']}[$langcode][0][$key])) {
      $values['description'][$key] = $entity->{$info['field']['field_name']}[$langcode][0][$key];
    }
    // If the keys are not defined an empty value has been submitted, hence we
    // need to update the term description accordingly.
    else {
      $values['description'][$key] = ($key == 'value') ?  '' : filter_default_format();
    }
  }
}

/**
 * Sync callback for the text field type.
 */
function title_field_text_sync_get($entity_type, $entity, $legacy_field, $info, $langcode) {
  $value = NULL;
  $field_name = $info['field']['field_name'];
  if (!empty($entity->{$field_name}[$langcode]) && is_array($entity->{$field_name}[$langcode])) {
    $items = $entity->{$field_name}[$langcode];
    $value = !empty($items[0]['value']) ? $items[0]['value'] : NULL;
  }
  return $value;
}

/**
 * Sync back callback for the text field type.
 */
function title_field_text_sync_set($entity_type, $entity, $legacy_field, $info, $langcode) {
  $entity->{$info['field']['field_name']}[$langcode][0]['value'] = $entity->{$legacy_field};
}

/**
 * Sync callback for the text with summary field type.
 */
function title_field_text_with_summary_sync_get($entity_type, $entity, $legacy_field, $info, $langcode) {
  $value = NULL;
  $format_key = $info['additional keys']['format'];
  $field_name = $info['field']['field_name'];
  // Return values only if there is any available to process for the current
  // language.
  if (!empty($entity->{$field_name}[$langcode]) && is_array($entity->{$field_name}[$langcode])) {
    $entity->{$format_key} = $entity->{$field_name}[$langcode][0]['format'];
    $items = $entity->{$field_name}[$langcode];
    $value = !empty($items[0]['value']) ? $items[0]['value'] : NULL;
  }
  return $value;
}

/**
 * Sync back callback for the text with summary field type.
 */
function title_field_text_with_summary_sync_set($entity_type, $entity, $legacy_field, $info, $langcode) {
  $format_key = $info['additional keys']['format'];
  $entity->{$info['field']['field_name']}[$langcode][0]['value'] = $entity->{$legacy_field};
  $entity->{$info['field']['field_name']}[$langcode][0]['format'] = $entity->{$format_key};
}

/**
 * Process variables for page.tpl.php.
 */
function title_process_page(&$variables) {
  // Ugly but necessary: there is no standardized way to tell if the current
  // page is an entity view page. This information should be injected here in
  // some form by entity-defining modules.
  $entity_types = array(
    'comment' => 1,
    'node' => 1,
    'taxonomy_term' => 2,
  );

  foreach ($entity_types as $entity_type => $position) {
    if ($entity = menu_get_object($entity_type, $position)) {
      break;
    }
  }

  if ($entity) {
    title_field_replacement_hide_label($entity_type, $entity, $variables, TRUE);
  }
}

/**
 * Process variables for node.tpl.php.
 */
function title_process_node(&$variables) {
  title_field_replacement_hide_label('node', $variables['node'], $variables);
}

/**
 * Process variables for taxonomy-term.tpl.php.
 */
function title_process_taxonomy_term(&$variables) {
  title_field_replacement_hide_label('taxonomy_term', $variables['term'], $variables);
}

/**
 * Process variables for comment.tpl.php.
 */
function title_process_comment(&$variables) {
  title_field_replacement_hide_label('comment', $variables['comment'], $variables);
}
