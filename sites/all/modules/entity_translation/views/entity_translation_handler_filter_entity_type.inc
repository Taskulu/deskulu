<?php
/**
 * @file
 * Contains an entity type filter handler.
 */

/**
 * This handler shows all available entity types that are enabled for entity
 * translation as options.
 */
class entity_translation_handler_filter_entity_type extends views_handler_filter_in_operator {

  /**
   * Show all entity types that are enabled for entity translation as options.
   */
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Entity type');
      $allowed_types_options = variable_get('entity_translation_entity_types');
      $allowed_types = array();
      $entity_info = entity_get_info();
      foreach ($allowed_types_options as $key => $allowed) {
        if ($allowed) {
          $allowed_types[$key] = $entity_info[$key]['label'];
        }
      }
      $this->value_options = $allowed_types;
    }
  }
}
