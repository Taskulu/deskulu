<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for field.module.
 */

/**
 * Implements hook_feeds_presave().
 */
function field_feeds_presave(FeedsSource $source, $entity, $item, $entity_id) {
  $entity_type = $entity->feeds_item->entity_type;

  // Not a real entity.
  if (!entity_get_info($entity_type)) {
    return;
  }

  // Gather the fields that Feeds is mapping to.
  $feeds_fields = array();
  foreach ($source->importer()->processor->getMappings() as $mapping) {
    list($field) = explode(':', $mapping['target']);
    $feeds_fields[$field] = TRUE;
  }

  list(, , $bundle) = entity_extract_ids($entity_type, $entity);

  foreach (field_info_instances($entity_type, $bundle) as $instance) {
    $field_name = $instance['field_name'];

    // Skip fields that Feeds isn't mapping to, and empty fields.
    if (!isset($feeds_fields[$field_name]) || empty($entity->$field_name) || !is_array($entity->$field_name)) {
      continue;
    }

    $info = field_info_field($field_name);

    foreach ($entity->$field_name as $language => $values) {
      // Filter out empty values.
      $values = _field_filter_items($info, $values);

      // Check that the number of values doesn't exceed the field cardinality.
      if ($info['cardinality'] != FIELD_CARDINALITY_UNLIMITED && count($values) > $info['cardinality']) {
        $values = array_slice($values, 0, $info['cardinality']);
      }

      $entity->{$field_name}[$language] = $values;
    }
  }
}
