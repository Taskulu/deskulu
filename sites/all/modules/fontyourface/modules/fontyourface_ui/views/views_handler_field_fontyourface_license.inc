<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler to provide simple renderer that allows linking to a license.
 */
class views_handler_field_fontyourface_license extends views_handler_field {

  /**
   * Override init function to provide generic option to link to license.
   */
  function init(&$view, &$data) {

    parent::init($view, $data);

    if (!empty($this->options['link_to_license'])) {
      $this->additional_fields['license_url'] = 'license_url';
    } // if

  } // init

  function option_definition() {

    $options = parent::option_definition();
    $options['link_to_license'] = array('default' => 0);

    return $options;

  } // option_definition

  /**
   * Provide link to license option
   */
  function options_form(&$form, &$form_state) {

    parent::options_form($form, $form_state);

    $form['link_to_license'] = array(
      '#title' => t('Link this field'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('No link'),
        'license_url' => t('To the license URL (if available)'),
      ),
      '#default_value' => $this->options['link_to_license'],
    );

  } // options_form

  function render($values) {

    if ($this->options['link_to_license'] == 'license_url') {

      $license_url = $values->{$this->aliases['license_url']};

      if ($license_url != '') {

        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = $license_url;

      } // if

    } // if

    return check_plain($values->{$this->field_alias});

  } // render

} // views_handler_field_fontyourface_license
