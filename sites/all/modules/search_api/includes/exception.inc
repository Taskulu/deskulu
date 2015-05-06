<?php

/**
 * @file
 * Contains SearchApiException.
 */

/**
 * Represents an exception or error that occurred in some part of the Search API
 * framework.
 */
class SearchApiException extends Exception {

  /**
   * Creates a new SearchApiException.
   *
   * @param $message
   *   A string describing the cause of the exception.
   */
  public function __construct($message = NULL) {
    if (!$message) {
      $message = t('An error occcurred in the Search API framework.');
    }
    parent::__construct($message);
  }

}

/**
 * Represents an exception that occurred in a data source controller.
 */
class SearchApiDataSourceException extends SearchApiException {

}
