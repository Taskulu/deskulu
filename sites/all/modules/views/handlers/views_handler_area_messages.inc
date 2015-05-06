<?php

/**
 * @file
 * Contains views_handler_area_messages.
 */

/**
 * Provides an area for messages.
 */
class views_handler_area_messages extends views_handler_area {

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    // Set the default to TRUE so it shows on empty pages by default.
    $options['empty']['default'] = TRUE;
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function render($empty = FALSE) {
    $return = '';
    if (!$empty || !empty($this->options['empty'])) {
      $return = theme('status_messages');
    }
    return $return;
  }

}
