<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler to present font enable/disable link.
 */
class views_handler_field_fontyourface_enabled_yes_no extends views_handler_field {

  function construct() {

    parent::construct();
    $this->additional_fields['enabled'] = 'enabled';

  } // construct

  function query() {

    $this->ensure_my_table();
    $this->add_additional_fields();

  } // query

  function render($values) {

    $enabled = $this->get_value($values, 'enabled');

    // This intentionall not translated, so it can be used in class names.

    if ($enabled) {

      return 'yes';

    } // if
    else {

      return 'no';

    } // else

  } // render

} // views_handler_field_fontyourface_enabled_yes_no
