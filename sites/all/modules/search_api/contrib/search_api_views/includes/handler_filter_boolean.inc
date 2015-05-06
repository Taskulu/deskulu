<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterBoolean.
 */

/**
 * Views filter handler class for handling fulltext fields.
 */
class SearchApiViewsHandlerFilterBoolean extends SearchApiViewsHandlerFilter {

  /**
   * Provide a list of options for the operator form.
   */
  public function operator_options() {
    return array();
  }

  /**
   * Provide a form for setting the filter value.
   */
  public function value_form(&$form, &$form_state) {
    while (is_array($this->value)) {
      $this->value = $this->value ? array_shift($this->value) : NULL;
    }
    $form['value'] = array(
      '#type' => 'select',
      '#title' => empty($form_state['exposed']) ? t('Value') : '',
      '#options' => array(1 => t('True'), 0 => t('False')),
      '#default_value' => isset($this->value) ? $this->value : '',
    );
  }

}
