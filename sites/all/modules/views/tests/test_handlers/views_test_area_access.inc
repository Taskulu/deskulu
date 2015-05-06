<?php

/**
 * @file
 * Contains views_test_area_access
 */

class views_test_area_access extends views_handler_area {

  /**
   * {@inheritdoc}
   */
  function access() {
    return $this->options['custom_access'];
 }

  /**
   * {@inheritdoc}
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['custom_access'] = array('default' => TRUE, 'bool' => TRUE);

    return $options;
  }

}
