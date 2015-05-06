<?php

/**
 * Field handler to translate a variable name into its readable form.
 */
class views_handler_field_variable_title extends views_handler_field {
  function render($values) {
    $name = $this->get_value($values);
    $variable = variable_build($name);
    return $variable['title'];
  }
}
