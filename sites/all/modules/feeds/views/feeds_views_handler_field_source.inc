<?php

/**
 * @file
 * Views handler for showing a feeds source config field.
 */

class feeds_views_handler_field_source extends views_handler_field {

  /**
   * Override parent::render().
   */
  function render($values) {
    $value = unserialize($values->{$this->field_alias});
    if (isset($value['FeedsHTTPFetcher']['source'])) {
      return check_url($value['FeedsHTTPFetcher']['source']);
    }
    elseif (isset($value['FeedsFileFetcher']['feeds_source'])) {
      // @todo This is untested.
      return $GLOBALS['base_url'] . '/' . check_plain($value['FeedsFileFetcher']['feeds_source']);
    }
    return '';
  }

  /**
   * Disallow advanced rendering.
   */
  function allow_advanced_render() {
    return FALSE;
  }
}
