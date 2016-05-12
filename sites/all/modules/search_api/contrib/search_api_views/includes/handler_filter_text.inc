<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterText.
 */

/**
 * Views filter handler class for handling fulltext fields.
 */
class SearchApiViewsHandlerFilterText extends SearchApiViewsHandlerFilter {

  /**
   * Provide a list of options for the operator form.
   */
  public function operator_options() {
    return array('=' => t('contains'), '<>' => t("doesn't contain"));
  }

  /**
   * Determines whether input from the exposed filters affects this filter.
   *
   * Overridden to not treat "All" differently.
   *
   * @param array $input
   *   The user input from the exposed filters.
   *
   * @return bool
   *   TRUE if the input should change the behavior of this filter.
   */
  public function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator_id']) && isset($input[$this->options['expose']['operator_id']])) {
      $this->operator = $input[$this->options['expose']['operator_id']];
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = $input[$this->options['expose']['identifier']];

      // Various ways to check for the absence of non-required input.
      if (empty($this->options['expose']['required'])) {
        if (($this->operator == 'empty' || $this->operator == 'not empty') && $value === '') {
          $value = ' ';
        }

        if (!empty($this->always_multiple) && $value === '') {
          return FALSE;
        }
      }

      if (isset($value)) {
        $this->value = $value;
        if (empty($this->always_multiple) && empty($this->options['expose']['multiple'])) {
          $this->value = array($value);
        }
      }
      else {
        return FALSE;
      }
    }

    return TRUE;
  }

}
