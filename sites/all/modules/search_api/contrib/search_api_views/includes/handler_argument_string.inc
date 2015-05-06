<?php

/**
 * @file
 * Contains SearchApiViewsHandlerArgumentString.
 */

/**
 * Views argument handler class for handling string fields.
 */
class SearchApiViewsHandlerArgumentString extends SearchApiViewsHandlerArgument {

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  public function query($group_by = FALSE) {
    if (empty($this->value)) {
      if (!empty($this->options['break_phrase'])) {
        views_break_phrase_string($this->argument, $this);
      }
      else {
        $this->value = array($this->argument);
      }
    }

    parent::query($group_by);
  }

}
