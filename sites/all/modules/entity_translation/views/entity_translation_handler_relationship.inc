<?php
/**
 * @file
 * Contains the relationship plugin for relating entities to translation metadata.
 */

/**
 * Add a relationship to the entity translation table.
 */
class entity_translation_handler_relationship extends views_handler_relationship {

  /**
   * Add a relationship to the entity_translation table.
   */
  function query() {
    $this->ensure_my_table();
    $def = $this->definition;
    $def['table'] = 'entity_translation';
    $def['field'] = 'entity_id';
    $def['left_table'] = $this->table_alias;
    $def['type'] = empty($this->options['required']) ? 'LEFT' : 'INNER';
    $join = new views_join();
    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;
    // Use a short alias for the table.
    $alias = $def['table'] . '_' . $this->table;
    // We need to add a condition on entity type to the join to avoid getting
    // relationships to entities with other types.
    $join->extra = "$alias.entity_type = '{$def['entity type']}'";
    $this->alias = $this->query->add_relationship($alias, $join, 'entity_translation', $this->relationship);
  }
}
