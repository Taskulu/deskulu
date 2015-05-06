<?php
/**
 * @file
 * Contains a language filter handler.
 */

/**
 * Extends the locale language filter in order for it to work with the entity
 * translation table structure.
 */
class entity_translation_handler_filter_language extends views_handler_filter_locale_language {

  /**
   * Override the default behaviour, insert an empty string instead of NULL.
   */
  function op_empty() {
    $this->ensure_my_table();
    if ($this->operator == 'empty') {
      $operator = "=";
    }
    else {
      $operator = "<>";
    }

    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field", '', $operator);
  }
}
