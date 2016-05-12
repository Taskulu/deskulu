<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for entity_translation.module.
 */

/**
 * Implements hook_feeds_presave().
 */
function entity_translation_feeds_presave(FeedsSource $source, $entity, $item, $entity_id) {
  $entity_type = $entity->feeds_item->entity_type;

  // Check that it's a real entity type, and translation is enabled.
  if (!entity_get_info($entity_type) || !entity_translation_enabled($entity_type, $entity)) {
    return;
  }

  if (!$handler = entity_translation_get_handler($entity_type, $entity)) {
    return;
  }

  list(, , $bundle) = entity_extract_ids($entity_type, $entity);

  $languages_seen = array();

  foreach (field_info_instances($entity_type, $bundle) as $instance) {
    $field_name = $instance['field_name'];

    // No values in this field, skip it.
    if (empty($entity->$field_name) || !is_array($entity->$field_name)) {
      continue;
    }

    // Not translatable.
    $info = field_info_field($field_name);
    if (!$info || !$info['translatable']) {
      continue;
    }

    // Init the translation handler.
    if (empty($handler->getTranslations()->original)) {
      $handler->initTranslations();
    }

    // Avoid invalid user configuration. Entity translation does this when
    // loading the translation overview page.
    if (count($entity->$field_name) === 1 && key($entity->$field_name) === LANGUAGE_NONE && $handler->getLanguage() !== LANGUAGE_NONE) {
      $entity->{$field_name}[$handler->getLanguage()] = $entity->{$field_name}[LANGUAGE_NONE];
      $entity->{$field_name}[LANGUAGE_NONE] = array();
    }

    // Look for languages we haven't created a translation for yet.
    foreach (array_diff_key($entity->$field_name, $languages_seen) as $language => $v) {
      if ($language === LANGUAGE_NONE) {
        continue;
      }

      $languages_seen[$language] = TRUE;

      if ($language === $handler->getLanguage()) {
        continue;
      }

      $translation = array(
        'translate' => 0,
        'status' => 1,
        'language' => $language,
        'source' => $handler->getLanguage(),
      );

      $handler->setTranslation($translation, $entity);
    }
  }

  // Loop through every language for the site, and remove translations for the
  // ones that don't have any values.
  foreach (language_list() as $language) {
    if (!isset($languages_seen[$language->language])) {
      $handler->removeTranslation($language->language);
    }
  }
}
