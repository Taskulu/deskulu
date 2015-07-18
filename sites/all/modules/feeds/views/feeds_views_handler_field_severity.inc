<?php

/**
 * @file
 * Views handler for displaying the logs severity field.
 */

class feeds_views_handler_field_severity extends views_handler_field {

  /**
   * Override parent::render().
   */
  function render($values) {
    $value = $values->{$this->field_alias};
    $levels = watchdog_severity_levels();
    return $levels[$value];
  }

  /**
   * Disallow advanced rendering.
   */
  function allow_advanced_render() {
    return FALSE;
  }
}
