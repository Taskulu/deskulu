<?php

/**
 * @file
 * Contains feeds_views_handler_field_log_message.
 */

/**
 * Views handler for displaying a log message.
 */
class feeds_views_handler_field_log_message extends views_handler_field {

  /**
   * Override parent::render().
   */
  public function render($values) {
    $message = $values->{$this->field_alias};
    $variables = unserialize($values->{$this->aliases['variables']});

    // Messages that are too long may generate a PDO exception when calling t()
    // if Drupal tries to add them in {locales_source} table.
    return t(truncate_utf8($message, 65000, TRUE, TRUE), $variables);
  }

  /**
   * Disallow advanced rendering.
   */
  public function allow_advanced_render() {
    return FALSE;
  }

}
