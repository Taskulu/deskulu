<?php

/**
 * @file
 * Definition of i18views_handler_filter_node_type.
 *
 * This is a copy of views_handler_filter_node_type but replaces a call to t()
 * with the proper i18n_node_type_name().
 */

/**
 * Filter by node type.
 *
 * @ingroup views_filter_handlers
 */
class i18nviews_handler_filter_node_type extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Content types');
      $types = node_type_get_types();
      $options = array();
      foreach ($types as $type => $info) {
        $options[$type] = i18n_node_type_name($type, $info->name);
      }
      asort($options);
      $this->value_options = $options;
    }
  }
}
