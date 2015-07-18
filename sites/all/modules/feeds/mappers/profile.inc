<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for profile.module.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function profile_feeds_processor_targets($entity_type, $bundle_name) {
  $targets = array();

  if ($entity_type != 'user') {
    return $targets;
  }

  $categories = profile_user_categories();

  foreach ($categories as $category) {
    foreach (_profile_get_fields($category['name']) as $record) {
      $targets[$record->name] = array(
        'name' => t('Profile: @name', array('@name' => $record->title)),
        'description' => t('Profile: @name', array('@name' => $record->title)),
        'callback' => 'profile_feeds_set_target',
      );
    }
  }

  return $targets;
}

/**
 * Set the user profile target after import.
 */
function profile_feeds_set_target(FeedsSource $source, $entity, $target, array $values) {
  $entity->$target = reset($values);
}
