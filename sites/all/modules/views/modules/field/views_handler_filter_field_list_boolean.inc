<?php

/**
 * @file
 * Definition of views_handler_filter_field_list_boolean.
 */

/**
 * Filter handler for boolean fields.
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_field_list_boolean extends views_handler_filter_field_list {

  function get_value_options() {
    $field = field_info_field($this->definition['field_name']);
    $value_options = list_allowed_values($field);

    // Boolean fields have an option for using the label as the 'on' value. This
    // results in there being no label values in the allows values array.
    // If this is the case, we need to provide the labels.
    $filtered = array_filter($value_options);
    if (empty($filtered)) {
      // We can't provide the label in the same way the FieldAPI formatter does,
      // as these are different on each instance, and we may be operating on
      // more than one bundle.
      $value_options[0] = t('Off');
      $value_options[1] = t('On');
    }

    $this->value_options = $value_options;
  }
}
