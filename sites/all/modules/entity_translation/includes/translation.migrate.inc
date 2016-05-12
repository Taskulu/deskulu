<?php
/**
 * @file
 * MigrateTranslationEntityHandler class for entity_translation.
 */

/**
 * Destination handler implementing translatable fields.
 */
class MigrateTranslationEntityHandler extends MigrateDestinationHandler {
  /**
   * Registers all entites as handled by this class.
   */
  public function __construct() {
    $this->registerTypes(array('entity'));
  }

  /**
   * Handles Entity Translations.
   *
   * @param stdClass $entity
   * @param stdClass $sourceRow
   */
  public function prepare($entity, stdClass $row) {
    $migration = Migration::currentMigration();
    $entity_type = $migration->getDestination()->getEntityType();

    // Only continue if the entity type + bundle combination is enabled for
    // entity_translation.
    if (entity_translation_enabled($entity_type, $entity)) {
      // Get the entity_translation handler and load any existing translations.
      $handler = entity_translation_get_handler($entity_type, $entity);
      $handler->loadTranslations();
      $original_language = $handler->getLanguage();

      // Get the bundle of the entity.
      list(, , $bundle) = entity_extract_ids($entity_type, $entity);

      // We need to get all of the possible translations to create. So we look
      // for any translatable fields.
      $translatable_fields = array();
      foreach (field_info_instances($entity_type, $bundle) as $instance) {
        $field_name = $instance['field_name'];
        $field = field_info_field($field_name);

        if ($field['translatable']) {
          $translatable_fields[] = $field_name;
        }
      }

      // In those fields we look for translated values to build out an array
      // of languages we need to set translations for.
      $translate_languages = array();
      foreach ($translatable_fields as $translatable_field) {
        if (!empty($entity->{$translatable_field})) {
          $field_languages = array_keys($entity->{$translatable_field});
          foreach ($field_languages as $field_language) {
            $translate_languages[$field_language] = $field_language;
          }
        }
      }

      // Remove the LANGUAGE_NONE results.
      unset($translate_languages[LANGUAGE_NONE]);

      // Remove the original language.
      unset($translate_languages[$original_language]);

      // Anything we're left with is a translation that should be set.
      foreach ($translate_languages as $translate_language) {
        if (!isset($entity->translations->data[$translate_language])) {
          // Add the new translation and store it.
          $handler->setTranslation(array(
            'translate' => 0,
            'status' => 1,
            'language' => $translate_language,
            'source' => $original_language,
            'uid' => (empty($entity->uid)) ? 0 : $entity->uid,
            'changed' => (empty($entity->changed)) ? REQUEST_TIME : $entity->changed,
            'created' => (empty($entity->created)) ? REQUEST_TIME : $entity->created,
          ));
        }
      }
    }
  }
}
