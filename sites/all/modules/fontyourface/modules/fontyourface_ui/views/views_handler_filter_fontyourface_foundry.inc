<?php

/**
 * @file
 * Views handler.
 */

/**
 * Filters by provider.
 */
class views_handler_filter_fontyourface_foundry extends views_handler_filter_in_operator {

  function get_value_options() {

    if (!isset($this->value_options)) {

      $this->value_title = t('Provider');
      $options = array();

      $foundries = db_select('fontyourface_font', 'f')
        ->fields('f', array('foundry'))
        ->distinct()
        ->execute();

      foreach ($foundries as $foundry) {
        $options[$foundry->foundry] = $foundry->foundry;
      } // foreach

      asort($options);
      $this->value_options = $options;

    } // if

  } // get_value_options

} // views_handler_filter_fontyourface_foundry
