<?php
/**
 * @file
 * Implements a Migrate destination handler to automatically add translations
 * to entities with translatable fields.
 */

/**
 * Implements hook_migrate_api().
 */
function entity_translation_migrate_api() {
  $api = array(
    'api' => 2,
    'destination handlers' => array('MigrateTranslationEntityHandler'),
  );
  return $api;
}
