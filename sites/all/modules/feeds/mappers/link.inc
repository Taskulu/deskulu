<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for link.module.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function link_feeds_processor_targets($entity_type, $bundle_name) {
  $targets = array();

  foreach (field_info_instances($entity_type, $bundle_name) as $name => $instance) {
    $info = field_info_field($name);
    if ($info['type'] == 'link_field') {
      if (array_key_exists('url', $info['columns'])) {
        $targets[$name . ':url'] = array(
          'name' => t('@name: URL', array('@name' => $instance['label'])),
          'callback' => 'link_feeds_set_target',
          'description' => t('The @label field of the entity.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
      }
      if (array_key_exists('title', $info['columns'])) {
        $targets[$name . ':title'] = array(
          'name' => t('@name: Title', array('@name' => $instance['label'])),
          'callback' => 'link_feeds_set_target',
          'description' => t('The @label field of the entity.', array('@label' => $instance['label'])),
          'real_target' => $name,
        );
      }
    }
  }

  return $targets;
}

/**
 * Callback for mapping link fields.
 */
function link_feeds_set_target(FeedsSource $source, $entity, $target, array $values, array $mapping) {
  $language = $mapping['language'];

  list($field_name, $column) = explode(':', $target);

  $field = isset($entity->$field_name) ? $entity->$field_name : array($language => array());
  $delta = 0;

  foreach ($values as $value) {
    if (is_object($value) && ($value instanceof FeedsElement)) {
      $value = $value->getValue();
    }

    if (is_scalar($value)) {
      $field[$language][$delta][$column] = (string) $value;
    }
    $delta++;
  }

  $entity->$field_name = $field;
}
