<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for file.module and
 * image.module.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function file_feeds_processor_targets($entity_type, $bundle_name) {
  $targets = array();

  foreach (field_info_instances($entity_type, $bundle_name) as $name => $instance) {
    $info = field_info_field($name);

    if (in_array($info['type'], array('file', 'image'))) {
      $targets[$name . ':uri'] = array(
        'name' => t('@label: URI', array('@label' => $instance['label'])),
        'callback' => 'file_feeds_set_target',
        'description' => t('The URI of the @label field.', array('@label' => $instance['label'])),
        'real_target' => $name,
      );

      // Keep the old target name for backwards compatibility, but hide it from
      // the UI.
      $targets[$name] = $targets[$name . ':uri'];
      $targets[$name]['deprecated'] = TRUE;

      if ($info['type'] == 'image') {
        $targets[$name . ':alt'] = array(
          'name' => t('@label: Alt', array('@label' => $instance['label'])),
          'callback' => 'file_feeds_set_target',
          'description' => t('The alt tag of the @label field.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
        $targets[$name . ':title'] = array(
          'name' => t('@label: Title', array('@label' => $instance['label'])),
          'callback' => 'file_feeds_set_target',
          'description' => t('The title of the @label field.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
      }
      elseif ($info['type'] === 'file') {
        $targets[$name . ':description'] = array(
          'name' => t('@label: Description', array('@label' => $instance['label'])),
          'callback' => 'file_feeds_set_target',
          'description' => t('The description of the @label field.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
      }
    }
  }

  return $targets;
}

/**
 * Callback for mapping file fields.
 */
function file_feeds_set_target(FeedsSource $source, $entity, $target, array $values, array $mapping) {
  $language = $mapping['language'];

  // Add default of uri for backwards compatibility.
  list($field_name, $sub_field) = explode(':', $target . ':uri');
  $info = field_info_field($field_name);

  if ($sub_field == 'uri') {

    foreach ($values as $k => $v) {
      if (!($v instanceof FeedsEnclosure)) {
        if (is_string($v)) {
          $values[$k] = new FeedsEnclosure($v, file_get_mimetype($v));
        }
        else {
          // Set the value for FALSE rather than remove it to keep our deltas
          // correct.
          $values[$k] = FALSE;
        }
      }
    }

    if ($entity instanceof Entity) {
      $entity_type = $entity->entityType();
      $bundle = $entity->bundle();
    }
    else {
      $entity_type = $source->importer->processor->entityType();
      $bundle = $source->importer->processor->bundle();
    }
    $instance_info = field_info_instance($entity_type, $field_name, $bundle);

    // Determine file destination.
    // @todo This needs review and debugging.
    $data = array();
    if (!empty($entity->uid)) {
      $data[$entity_type] = $entity;
    }

    $destination = file_field_widget_uri($info, $instance_info, $data);
  }

  // Populate entity.
  $field = isset($entity->$field_name) ? $entity->$field_name : array($language => array());
  $delta = 0;
  foreach ($values as $v) {
    if ($info['cardinality'] == $delta) {
      break;
    }

    if (!isset($field[$language][$delta])) {
      $field[$language][$delta] = array();
    }

    switch ($sub_field) {
      case 'alt':
      case 'title':
      case 'description':
        $field[$language][$delta][$sub_field] = $v;
        break;

      case 'uri':
        if ($v) {
          try {
            $v->setAllowedExtensions($instance_info['settings']['file_extensions']);
            $field[$language][$delta] += (array) $v->getFile($destination);
            // @todo: Figure out how to properly populate this field.
            $field[$language][$delta]['display'] = 1;
          }
          catch (Exception $e) {
            watchdog('feeds', check_plain($e->getMessage()));
          }
        }
        break;
    }

    $delta++;
  }

  $entity->$field_name = $field;
}
