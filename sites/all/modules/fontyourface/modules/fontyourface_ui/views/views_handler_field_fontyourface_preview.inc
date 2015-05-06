<?php

/**
 * @file
 * Views handler.
 */


/**
 * Field handler to present font preview.
 */
class views_handler_field_fontyourface_preview extends views_handler_field {

  function construct() {

    parent::construct();
    $this->additional_fields['fid'] = 'fid';

  } // construct

  function query() {

    $this->ensure_my_table();
    $this->add_additional_fields();

  } // query

  function render($values) {

    $fid = $this->get_value($values, 'fid');

    $font = fontyourface_get_font($fid);
    return fontyourface_preview($font, fontyourface_short_preview_text($font), 40);

  } // render

} // views_handler_field_fontyourface_preview
