<?php

/**
 * @file
 * Definition of views_handler_argument_node_type.
 */

/**
 * Argument handler to accept a node type.
 */
class i18nviews_handler_argument_node_type extends views_handler_argument_node_type {
  function node_type($type) {
    $output = node_type_get_name($type);
    if (empty($output)) {
      return t('Unknown content type');
    }
    return i18n_node_type_name($type, $output);
  }
}
