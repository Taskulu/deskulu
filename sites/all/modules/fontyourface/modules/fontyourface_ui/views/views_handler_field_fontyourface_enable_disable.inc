<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler to present font enable/disable link.
 */
class views_handler_field_fontyourface_enable_disable extends views_handler_field {

  function construct() {

    parent::construct();
    $this->additional_fields['enabled'] = 'enabled';
    $this->additional_fields['fid'] = 'fid';

  } // construct

  function query() {

    $this->ensure_my_table();
    $this->add_additional_fields();

  } // query

  function render($values) {

    $enabled = $this->get_value($values, 'enabled');
    $fid = $this->get_value($values, 'fid');
    $destination = drupal_get_destination();

    if ($enabled) {

      return l(t('Disable'), 'admin/appearance/fontyourface/disable/' . $fid . '?' . drupal_http_build_query($destination), array('attributes' => array('class' => 'disable-link')));

    } // if
    else {

      return l(t('Enable'), 'admin/appearance/fontyourface/enable/' . $fid . '?' . drupal_http_build_query($destination), array('attributes' => array('class' => 'enable-link')));

    } // else

  } // render

} // views_handler_field_fontyourface_preview
