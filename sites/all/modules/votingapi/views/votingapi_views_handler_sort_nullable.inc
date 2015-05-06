<?php

/**
 * @file
 * Provide a views handlers for votingapi data fields.
 */

class votingapi_views_handler_sort_nullable extends views_handler_sort {
  function option_definition() {
    $options = parent::option_definition();
    $options['coalesce'] = array('default' => FALSE);
    $options['null_value'] = array('default' => 0);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['coalesce'] = array(
      '#type' => 'checkbox',
      '#title' => t('Treat missing votes as zeros'),
      '#default_value' => $this->options['coalesce'],
    );
  }

  /**
   * Called to add the sort to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field.

    if ($this->options['coalesce']) {
      $this->query->add_orderby(NULL, "COALESCE($this->table_alias.$this->field, 0)", $this->options['order'], $this->table_alias . '_' . $this->field . '_coalesced');
    }
    else {
      $this->query->add_orderby($this->table_alias, $this->real_field, $this->options['order']);
    }
  }
}
