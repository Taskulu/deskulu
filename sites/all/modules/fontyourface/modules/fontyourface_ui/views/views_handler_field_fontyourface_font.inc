<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler to provide simple renderer that allows linking to a font.
 */
class views_handler_field_fontyourface_font extends views_handler_field {

  /**
   * Override init function to provide generic option to link to font.
   */
  function init(&$view, &$data) {

    parent::init($view, $data);

    if (!empty($this->options['link_to_font'])) {
      $this->additional_fields['fid'] = 'fid';
    } // if

  } // init

  function option_definition() {

    $options = parent::option_definition();
    $options['link_to_font'] = array('default' => 0);
    $options['display_as'] = array('default' => 0);

    return $options;

  } // option_definition

  /**
   * Provide link to font option
   */
  function options_form(&$form, &$form_state) {

    parent::options_form($form, $form_state);

    $form['link_to_font'] = array(
      '#title' => t('Link this field'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('No link'),
        'edit' => t('To the font edit page'),
      ),
      '#default_value' => $this->options['link_to_font'],
    );

    $form['display_as'] = array(
      '#title' => t('Display as'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('Plain text'),
        'preview' => t('Font preview'),
      ),
      '#default_value' => $this->options['display_as'],
    );

  } // options_form

  function render($values) {

    if ($this->options['link_to_font'] == 'edit') {

      $destination = drupal_get_destination();

      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = 'admin/appearance/fontyourface/edit/' . $values->{$this->aliases['fid']} . '?' . drupal_http_build_query($destination);

    } // if

    if ($this->options['display_as'] == 'preview') {

      $font = fontyourface_get_font($values->fid);
      return fontyourface_preview($font, $values->{$this->field_alias}, 16);

    } // if

    return check_plain($values->{$this->field_alias});

  } // render

} // views_handler_field_fontyourface_font
