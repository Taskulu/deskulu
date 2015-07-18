<?php

/**
 * @file
 * Argument handler for importer ids.
 */
class feeds_views_handler_argument_importer_id extends views_handler_argument_string {

  /**
   * Argument must be a valid importer id.
   */
  function validate_arg($arg) {
    // By using % in URLs, arguments could be validated twice; this eases
    // that pain.
    if (isset($this->argument_validated)) {
      return $this->argument_validated;
    }
    $this->argument_validated = FALSE;
    if (in_array($arg, feeds_enabled_importers())) {
      $this->argument_validated = TRUE;
    }
    return $this->argument_validated;
  }
}
