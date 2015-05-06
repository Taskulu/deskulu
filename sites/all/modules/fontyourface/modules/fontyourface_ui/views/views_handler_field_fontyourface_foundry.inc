<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler to provide simple renderer that allows linking to a foundry.
 */
class views_handler_field_fontyourface_foundry extends views_handler_field {

  /**
   * Override init function to provide generic option to link to foundry.
   */
  function init(&$view, &$data) {

    parent::init($view, $data);

    if (!empty($this->options['link_to_foundry'])) {
      $this->additional_fields['foundry_url'] = 'foundry_url';
    } // if

  } // init

  function option_definition() {

    $options = parent::option_definition();
    $options['link_to_foundry'] = array('default' => 0);

    return $options;

  } // option_definition

  /**
   * Provide link to font option
   */
  function options_form(&$form, &$form_state) {

    parent::options_form($form, $form_state);

    $form['link_to_foundry'] = array(
      '#title' => t('Link this field'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('No link'),
        'foundry_url' => t('To the foundry URL (if available)'),
      ),
      '#default_value' => $this->options['link_to_foundry'],
    );

  } // options_form

  function render($values) {

    if ($this->options['link_to_foundry'] == 'foundry_url') {

      $foundry_url = $values->{$this->aliases['foundry_url']};

      if ($foundry_url != '') {

        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = $foundry_url;

      } // if
      else {

        $this->options['alter']['make_link'] = FALSE;

      } // else

    } // if

    return check_plain($values->{$this->field_alias});

  } // render

} // views_handler_field_fontyourface_foundry
