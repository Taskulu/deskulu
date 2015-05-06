<?php

/**
 * @file
 * Provide a views handlers for votingapi data fields.
 */

class votingapi_views_handler_field_value extends views_handler_field_numeric {
  function construct() {
    parent::construct();
    $this->definition['float'] = TRUE;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['appearance'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $appearances = module_invoke_all('votingapi_views_formatters', $this);
    $options = array('' => t('Default appearance'));
    $options += $appearances;

    if (count($options) > 1) {
      $form['appearance'] = array(
        '#type' => 'select',
        '#title' => t('Appearance'),
        '#options' => $options,
        '#default_value' => $this->options['appearance'],
        '#weight' => -5,
      );
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $this->query->add_orderby(NULL, "COALESCE($this->table_alias.$this->field, 0)", $order, $this->table_alias . '_' . $this->field . '_coalesced');
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $function = $this->options['appearance'];
    if (!empty($function) && function_exists($function)) {
      return $function($value, $this, $values);
    }
    else {
      return parent::render($values);
    }
  }
}
