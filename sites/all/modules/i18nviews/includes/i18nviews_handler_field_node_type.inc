<?php

/**
 * @file
 * Definition of i18nviews_handler_field_node_type.
 */

/**
 * Field handler to translate a node type into its readable form.
 *
 * @ingroup views_field_handlers
 */
class i18nviews_handler_field_node_type extends views_handler_field_node_type {
  /**
   * Render node type as human readable name, unless using machine_name option.
   */
  function render_name($data, $values) {
    if ($this->options['machine_name'] != 1 && $data !== NULL && $data !== '') {
      return $this->sanitize_value(i18n_node_type_name($data, node_type_get_name($data)));
    }
    return $this->sanitize_value($data);
  }
}
