<?php

/**
 * List field filter handler.
 *
 * Translate allowed values.
 */
class i18nviews_handler_filter_field_list extends views_handler_filter_field_list {
  function get_value_options() {
    $field = field_info_field($this->definition['field_name']);
    $this->value_options = list_allowed_values($field);

    // Translate list options
    if (($translate = i18n_field_type_info($field['type'], 'translate_options')) && !empty($this->value_options)) {
      $this->value_options = $translate($field);
    }
  }
}
