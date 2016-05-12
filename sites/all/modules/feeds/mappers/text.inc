<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for text.module.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function text_feeds_processor_targets($entity_type, $bundle_name) {
  $targets = array();

  $text_types = array(
    'text',
    'text_long',
    'text_with_summary',
  );
  foreach (field_info_instances($entity_type, $bundle_name) as $name => $instance) {
    $info = field_info_field($name);

    if (in_array($info['type'], $text_types)) {
      $targets[$name] = array(
        'name' => check_plain($instance['label']),
        'callback' => 'text_feeds_set_target',
        'description' => t('The @label field of the entity.', array('@label' => $instance['label'])),
      );
      if ($info['type'] == 'text_with_summary') {
        // Allow mapping to summary.
        $targets[$name . ':summary'] = array(
          'name' => t('@name: Summary', array('@name' => $instance['label'])),
          'callback' => 'text_feeds_set_target',
          'description' => t('The @label field of the entity.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
      }
    }

    if (!empty($instance['settings']['text_processing'])) {
      $targets[$name]['summary_callbacks'] = array('text_feeds_summary_callback');
      $targets[$name]['form_callbacks'] = array('text_feeds_form_callback');
    }
  }

  return $targets;
}

/**
 * Callback for mapping text fields.
 */
function text_feeds_set_target(FeedsSource $source, $entity, $target, array $values, array $mapping) {
  $language = $mapping['language'];

  list($field_name, $column) = explode(':', $target . ':value');

  if ($column === 'value' && isset($source->importer->processor->config['input_format'])) {
    $format = $source->importer->processor->config['input_format'];
    // Add in default values.
    $mapping += array(
      'format' => $format,
    );
  }

  $field = isset($entity->$field_name) ? $entity->$field_name : array($language => array());

  // Iterate over all values.
  $delta = 0;
  foreach ($values as $value) {

    if (is_object($value) && $value instanceof FeedsElement) {
      $value = $value->getValue();
    }

    if (is_scalar($value) && strlen($value)) {

      $field[$language][$delta][$column] = (string) $value;

      if (isset($mapping['format'])) {
        $field[$language][$delta]['format'] = $mapping['format'];
      }
    }

    $delta++;
  }

  $entity->$field_name = $field;
}

/**
 * Summary callback for text field targets.
 *
 * Displays which text format will be used for the text field target.
 *
 * @see text_feeds_processor_targets()
 * @see text_feeds_form_callback()
 */
function text_feeds_summary_callback(array $mapping, $target, array $form, array $form_state) {
  global $user;
  $formats = filter_formats($user);

  // Processor-wide input format setting.
  $importer = feeds_importer($form['#importer']);
  $default_format = !empty($importer->processor->config['input_format']) ? $importer->processor->config['input_format'] : filter_fallback_format();
  $mapping += array(
    'format' => $default_format,
  );

  return t('Text format: %format', array('%format' => $formats[$mapping['format']]->name));
}

/**
 * Form callback for text field targets.
 *
 * Allows to select a text format for the text field target.
 *
 * @see text_feeds_processor_targets()
 * @see text_feeds_summary_callback()
 */
function text_feeds_form_callback(array $mapping, $target, array $form, array $form_state) {
  global $user;
  $formats_options = array();
  $formats = filter_formats($user);
  foreach ($formats as $id => $format) {
    $formats_options[$id] = $format->name;
  }

  // Processor-wide text format setting.
  $importer = feeds_importer($form['#importer']);
  $default_format = !empty($importer->processor->config['input_format']) ? $importer->processor->config['input_format'] : filter_fallback_format();
  $mapping += array(
    'format' => $default_format,
  );

  return array(
    'format' => array(
      '#type' => 'select',
      '#title' => t('Text format'),
      '#options' => $formats_options,
      '#default_value' => $mapping['format'],
    ),
  );
}
