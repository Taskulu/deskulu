<?php

/**
 * @file
 * Views handler.
 */

/**
 * Filters by provider.
 */

class views_handler_filter_fontyourface_provider extends views_handler_filter_in_operator {

  function get_value_options() {

    if (!isset($this->value_options)) {

      $this->value_title = t('Provider');
      $options = array();

      foreach (module_implements('fontyourface_info') as $module) {

        $info_function = $module . '_fontyourface_info';
        $provider = $info_function();
        $options[$module] = $provider['name'];

      } // foreach

      asort($options);
      $this->value_options = $options;

    } // if

  } // get_value_options

} // views_handler_filter_fontyourface_provider
