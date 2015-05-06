<?php

/**
 * @file
 * Definition of views_handler_field_counter.
 */

/**
 * Field handler to show a counter of the current row.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_counter extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();
    $options['counter_start'] = array('default' => 1);
    $options['reverse'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['counter_start'] = array(
      '#type' => 'textfield',
      '#title' => t('Starting value'),
      '#default_value' => $this->options['counter_start'],
      '#description' => t('Specify the number the counter should start at.'),
      '#size' => 2,
    );

    $form['reverse'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse'),
      '#default_value' => $this->options['reverse'],
      '#description' => t('Reverse the counter.'),
    );

    parent::options_form($form, $form_state);
  }

  function query() {
    // do nothing -- to override the parent query.
  }

  function render($values) {
    $reverse = empty($this->options['reverse']) ? 1 : -1;

    // Note:  1 is subtracted from the counter start value below because the
    // counter value is incremented by 1 at the end of this function.
    $counter_start = is_numeric($this->options['counter_start']) ? $this->options['counter_start'] : 0;
    $count = ($reverse == -1) ? count($this->view->result) + $counter_start : $counter_start -1;
    $pager = $this->view->query->pager;

    // Get the base count of the pager.
    if ($pager->use_pager()) {
      if ($reverse == -1) {
        $count = ($pager->total_items + $counter_start - ($pager->get_current_page() * $pager->get_items_per_page()) + $pager->get_offset());
      } else {
        $count += (($pager->get_items_per_page() * $pager->get_current_page() + $pager->get_offset())) * $reverse;
      }
    }
    // Add the counter for the current site.
    $count += ($this->view->row_index + 1) * $reverse;

    return $count;
  }
}
