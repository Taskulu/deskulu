<?php

/**
 * @file
 * Contains SearchApiIgnoreCase.
 */

/**
 * Processor for making searches case-insensitive.
 */
class SearchApiIgnoreCase extends SearchApiAbstractProcessor {

  protected function process(&$value) {
    // We don't touch integers, NULL values or the like.
    if (is_string($value)) {
      $value = drupal_strtolower($value);
    }
  }

}
