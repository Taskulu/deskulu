<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterEntity.
 */

/**
 * Views filter handler class for entities.
 *
 * Should be extended for specific entity types, such as
 * SearchApiViewsHandlerFilterUser and SearchApiViewsHandlerFilterTaxonomyTerm.
 *
 * Based on views_handler_filter_term_node_tid.
 */
abstract class SearchApiViewsHandlerFilterEntity extends SearchApiViewsHandlerFilter {

  /**
   * If exposed form input was successfully validated, the entered entity IDs.
   *
   * @var array
   */
  protected $validated_exposed_input;

  /**
   * Validates entered entity labels and converts them to entity IDs.
   *
   * Since this can come from either the form or the exposed filter, this is
   * abstracted out a bit so it can handle the multiple input sources.
   *
   * @param array $form
   *   The form or form element for which any errors should be set.
   * @param array $values
   *   The entered user names to validate.
   *
   * @return array
   *   The entity IDs corresponding to all entities that could be found.
   */
  abstract protected function validate_entity_strings(array &$form, array $values);

  /**
   * Transforms an array of entity IDs into a comma-separated list of labels.
   *
   * @param array $ids
   *   The entity IDs to transform.
   *
   * @return string
   *   A string containing the labels corresponding to the IDs, separated by
   *   commas.
   */
  abstract protected function ids_to_strings(array $ids);

  /**
   * {@inheritdoc}
   */
  public function operator_options() {
    $operators = array(
      '=' => $this->isMultiValued() ? t('Is one of') : t('Is'),
      'all of' => t('Is all of'),
      '<>' => $this->isMultiValued() ? t('Is not one of') : t('Is not'),
      'empty' => t('Is empty'),
      'not empty' => t('Is not empty'),
    );
    if (!$this->isMultiValued()) {
      unset($operators['all of']);
    }
    return $operators;
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['expose']['multiple']['default'] = TRUE;

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    if (!is_array($this->value)) {
      $this->value = $this->value ? array($this->value) : array();
    }

    // Set the correct default value in case the admin-set value is used (and a
    // value is present). The value is used if the form is either not exposed,
    // or the exposed form wasn't submitted yet. (There doesn't seem to be an
    // easier way to check for that.)
    if ($this->value && (empty($form_state['input']) || !empty($form_state['input']['live_preview']))) {
      $form['value']['#default_value'] = $this->ids_to_strings($this->value);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function value_validate($form, &$form_state) {
    if (!empty($form['value'])) {
      $value = &$form_state['values']['options']['value'];
      if (strlen($value)) {
        $values = $this->isMultiValued($form_state['values']['options']) ? drupal_explode_tags($value) : array($value);
        $ids = $this->validate_entity_strings($form['value'], $values);

        if ($ids) {
          $value = $ids;
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function accept_exposed_input($input) {
    $rc = parent::accept_exposed_input($input);

    if ($rc) {
      // If we have previously validated input, override.
      if ($this->validated_exposed_input) {
        $this->value = $this->validated_exposed_input;
      }
    }

    return $rc;
  }

  /**
   * {@inheritdoc}
   */
  public function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed']) || empty($this->options['expose']['identifier'])) {
      return;
    }

    $this->validated_exposed_input = FALSE;
    $identifier = $this->options['expose']['identifier'];
    $input = $form_state['values'][$identifier];

    if ($this->options['is_grouped'] && isset($this->options['group_info']['group_items'][$input])) {
      $this->operator = $this->options['group_info']['group_items'][$input]['operator'];
      $input = $this->options['group_info']['group_items'][$input]['value'];
    }

    if (!strlen($input)) {
      return;
    }
    $values = $this->isMultiValued() ? drupal_explode_tags($input) : array($input);

    if (!$this->options['is_grouped'] || ($this->options['is_grouped'] && ($input != 'All'))) {
      $this->validated_exposed_input = $this->validate_entity_strings($form[$identifier], $values);
    }
  }

  /**
   * Determines whether multiple user names can be entered into this filter.
   *
   * This is either the case if the form isn't exposed, or if the " Allow
   * multiple selections" option is enabled.
   *
   * @param array $options
   *   (optional) The options array to use. If not supplied, the options set on
   *   this filter will be used.
   *
   * @return bool
   *   TRUE if multiple values can be entered for this filter, FALSE otherwise.
   */
  protected function isMultiValued(array $options = array()) {
    $options = $options ? $options : $this->options;
    return empty($options['exposed']) || !empty($options['expose']['multiple']);
  }

  /**
   * {@inheritdoc}
   */
  public function admin_summary() {
    if (!is_array($this->value)) {
      $this->value = $this->value ? array($this->value) : array();
    }
    $value = $this->value;
    $this->value = empty($value) ? '' : $this->ids_to_strings($value);
    $ret = parent::admin_summary();
    $this->value = $value;
    return $ret;
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    if ($this->operator === 'empty') {
      $this->query->condition($this->real_field, NULL, '=', $this->options['group']);
    }
    elseif ($this->operator === 'not empty') {
      $this->query->condition($this->real_field, NULL, '<>', $this->options['group']);
    }
    elseif (is_array($this->value)) {
      $all_of = $this->operator === 'all of';
      $operator = $all_of ? '=' : $this->operator;
      if (count($this->value) == 1) {
        $this->query->condition($this->real_field, reset($this->value), $operator, $this->options['group']);
      }
      else {
        $filter = $this->query->createFilter($operator === '<>' || $all_of ? 'AND' : 'OR');
        foreach ($this->value as $value) {
          $filter->condition($this->real_field, $value, $operator);
        }
        $this->query->filter($filter, $this->options['group']);
      }
    }
  }

}
