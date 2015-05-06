<?php

/**
 * @file
 * Contains SearchApiViewsHandlerArgument.
 */

/**
 * Views argument handler class for handling all non-fulltext types.
 */
class SearchApiViewsHandlerArgument extends views_handler_argument {

  /**
   * The associated views query object.
   *
   * @var SearchApiViewsQuery
   */
  public $query;

  /**
   * The operator to use for multiple arguments.
   *
   * Either "and" or "or".
   *
   * @var string
   *
   * @see views_break_phrase
   */
  public $operator;

  /**
   * Determine if the argument can generate a breadcrumb
   *
   * @return boolean
   */
  // @todo Change and implement set_breadcrumb()?
  public function uses_breadcrumb() {
    return FALSE;
  }

  /**
   * Provide a list of default behaviors for this argument if the argument
   * is not present.
   *
   * Override this method to provide additional (or fewer) default behaviors.
   */
  public function default_actions($which = NULL) {
    $defaults = array(
      'ignore' => array(
        'title' => t('Display all values'),
        'method' => 'default_ignore',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'not found' => array(
        'title' => t('Hide view / Page not found (404)'),
        'method' => 'default_not_found',
        'hard fail' => TRUE, // This is a hard fail condition
      ),
      'empty' => array(
        'title' => t('Display empty text'),
        'method' => 'default_empty',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'default' => array(
        'title' => t('Provide default argument'),
        'method' => 'default_default',
        'form method' => 'default_argument_form',
        'has default argument' => TRUE,
        'default only' => TRUE, // this can only be used for missing argument, not validation failure
      ),
    );

    if ($which) {
      return isset($defaults[$which]) ? $defaults[$which] : NULL;
    }
    return $defaults;
  }

  public function option_definition() {
    $options = parent::option_definition();

    $options['break_phrase'] = array('default' => FALSE);
    $options['not'] = array('default' => FALSE);

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Allow passing multiple values.
    $form['break_phrase'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple values'),
      '#description' => t('If selected, users can enter multiple values in the form of 1+2+3 (for OR) or 1,2,3 (for AND).'),
      '#default_value' => $this->options['break_phrase'],
      '#fieldset' => 'more',
    );

    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude'),
      '#description' => t('If selected, the numbers entered for the filter will be excluded rather than limiting the view.'),
      '#default_value' => !empty($this->options['not']),
      '#fieldset' => 'more',
    );
  }

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  public function query($group_by = FALSE) {
    if (empty($this->value)) {
      if (!empty($this->options['break_phrase'])) {
        views_break_phrase($this->argument, $this);
      }
      else {
        $this->value = array($this->argument);
      }
    }

    $operator = empty($this->options['not']) ? '=' : '<>';

    if (count($this->value) > 1) {
      $filter = $this->query->createFilter(drupal_strtoupper($this->operator));
      // $filter will be NULL if there were errors in the query.
      if ($filter) {
        foreach ($this->value as $value) {
          $filter->condition($this->real_field, $value, $operator);
        }
        $this->query->filter($filter);
      }
    }
    else {
      $this->query->condition($this->real_field, reset($this->value), $operator);
    }
  }

}
