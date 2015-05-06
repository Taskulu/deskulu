<?php
/**
 * A handler to display data from variable_values
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_variable_value extends views_handler_field {
  function construct() {
    parent::construct();
    // Be explicit about the table we are using.
    $this->additional_fields['name'] = array('table' => 'variable', 'field' => 'name');
  }

  /**
   * Render value using variable name for formatting.
   */
  function render($values) {
    $name = $this->get_value($values, 'name');
    $variable = variable_build($name);
    $variable['value'] = unserialize($this->get_value($values));
    return variable_format_value($variable);
  }
}
