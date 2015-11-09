<?php

/**
 * @file
 * Support for the Pathauto module.
 */

/**
 * Field handler.
 */
class PathautoMigrationHandler extends MigrateDestinationHandler {
  public function __construct() {
    $this->registerTypes(array('entity'));
  }

  /**
   * Make the destination field visible.
   */
  public function fields() {
    return array(
      'pathauto' => t('Pathauto: Perform aliasing (set to 0 to prevent alias generation during migration'),
    );
  }

  public function prepare($entity, stdClass $row) {
    if (isset($entity->pathauto)) {
      if (!isset($entity->path)) {
        $entity->path = array();
      }
      elseif (is_string($entity->path)) {
        // If MigratePathEntityHandler->prepare() hasn't run yet, support
        // the alias (set as $entity->path as a string) being formatted properly
        // in the path alias array.
        $path = $entity->path;
        $entity->path = array();
        $entity->path['alias'] = $path;
      }
      $entity->path['pathauto'] = $entity->pathauto;
      if (!isset($entity->path['alias'])) {
        $entity->path['alias'] = '';
      }
      unset($entity->pathauto);
    }
  }
}

/*
 * Implementation of hook_migrate_api().
 */
function pathauto_migrate_api() {
  $api = array(
    'api' => 2,
    'destination handlers' => array('PathautoMigrationHandler'),
  );
  return $api;
}
