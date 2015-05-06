<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilter.
 */

/**
 * Views filter handler base class for handling all "normal" cases.
 */
class SearchApiViewsHandlerFilter extends views_handler_filter {

  /**
   * The value to filter for.
   *
   * @var mixed
   */
  public $value;

  /**
   * The operator used for filtering.
   *
   * @var string
   */
  public $operator;

  /**
   * The associated views query object.
   *
   * @var SearchApiViewsQuery
   */
  public $query;

  /**
   * Provide a list of options for the operator form.
   */
  public function operator_options() {
    return array(
      '<' => t('Is less than'),
      '<=' => t('Is less than or equal to'),
      '=' => t('Is equal to'),
      '<>' => t('Is not equal to'),
      '>=' => t('Is greater than or equal to'),
      '>' => t('Is greater than'),
      'empty' => t('Is empty'),
      'not empty' => t('Is not empty'),
    );
  }

  /**
   * Provide a form for setting the filter value.
   */
  public function value_form(&$form, &$form_state) {
    while (is_array($this->value) && count($this->value) < 2) {
      $this->value = $this->value ? reset($this->value) : NULL;
    }
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => empty($form_state['exposed']) ? t('Value') : '',
      '#size' => 30,
      '#default_value' => isset($this->value) ? $this->value : '',
    );

    // Hide the value box if the operator is 'empty' or 'not empty'.
    // Radios share the same selector so we have to add some dummy selector.
    if (empty($form_state['exposed'])) {
      $form['value']['#states']['visible'] = array(
        ':input[name="options[operator]"],dummy-empty' => array('!value' => 'empty'),
        ':input[name="options[operator]"],dummy-not-empty' => array('!value' => 'not empty'),
      );
    }
    elseif (!empty($this->options['expose']['use_operator'])) {
      $name = $this->options['expose']['operator_id'];
      $form['value']['#states']['visible'] = array(
        ':input[name="' . $name . '"],dummy-empty' => array('!value' => 'empty'),
        ':input[name="' . $name . '"],dummy-not-empty' => array('!value' => 'not empty'),
      );
    }
  }

  /**
   * Display the filter on the administrative summary
   */
  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    if ($this->operator === 'empty') {
      return t('is empty');
    }
    if ($this->operator === 'not empty') {
      return t('is not empty');
    }

    return check_plain((string) $this->operator) . ' ' . check_plain((string) $this->value);
  }

  /**
   * Add this filter to the query.
   */
  public function query() {
    if ($this->operator === 'empty') {
      $this->query->condition($this->real_field, NULL, '=', $this->options['group']);
    }
    elseif ($this->operator === 'not empty') {
      $this->query->condition($this->real_field, NULL, '<>', $this->options['group']);
    }
    else {
      while (is_array($this->value)) {
        $this->value = $this->value ? reset($this->value) : NULL;
      }
      if (strlen($this->value) > 0) {
        $this->query->condition($this->real_field, $this->value, $this->operator, $this->options['group']);
      }
    }
  }

}
