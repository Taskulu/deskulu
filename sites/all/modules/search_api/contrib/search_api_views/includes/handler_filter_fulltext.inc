<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterFulltext.
 */

/**
 * Views filter handler class for handling fulltext fields.
 */
class SearchApiViewsHandlerFilterFulltext extends SearchApiViewsHandlerFilterText {

  /**
   * Displays the operator form, adding a description.
   */
  public function show_operator_form(&$form, &$form_state) {
    $this->operator_form($form, $form_state);
    $form['operator']['#description'] = t('This operator is only useful when using \'Search keys\'.');
  }

  /**
   * Provide a list of options for the operator form.
   */
  public function operator_options() {
    return array(
      'AND' => t('Contains all of these words'),
      'OR' => t('Contains any of these words'),
      'NOT' => t('Contains none of these words'),
    );
  }

  /**
   * Specify the options this filter uses.
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'AND';

    $options['mode'] = array('default' => 'keys');
    $options['min_length'] = array('default' => '');
    $options['fields'] = array('default' => array());

    return $options;
  }

  /**
   * Extend the options form a bit.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['mode'] = array(
      '#title' => t('Use as'),
      '#type' => 'radios',
      '#options' => array(
        'keys' => t('Search keys – multiple words will be split and the filter will influence relevance. You can change how search keys are parsed under "Advanced" > "Query settings".'),
        'filter' => t("Search filter – use as a single phrase that restricts the result set but doesn't influence relevance."),
      ),
      '#default_value' => $this->options['mode'],
    );

    $fields = $this->getFulltextFields();
    if (!empty($fields)) {
      $form['fields'] = array(
        '#type' => 'select',
        '#title' => t('Searched fields'),
        '#description' => t('Select the fields that will be searched. If no fields are selected, all available fulltext fields will be searched.'),
        '#options' => $fields,
        '#size' => min(4, count($fields)),
        '#multiple' => TRUE,
        '#default_value' => $this->options['fields'],
      );
    }
    else {
      $form['fields'] = array(
        '#type' => 'value',
        '#value' => array(),
      );
    }
    if (isset($form['expose'])) {
      $form['expose']['#weight'] = -5;
    }

    $form['min_length'] = array(
      '#title' => t('Minimum keyword length'),
      '#description' => t('Minimum length of each word in the search keys. Leave empty to allow all words.'),
      '#type' => 'textfield',
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $this->options['min_length'],
    );
  }

  /**
   * {@inheritdoc}
   */
  public function exposed_validate(&$form, &$form_state) {
    // Only validate exposed input.
    if (empty($this->options['exposed']) || empty($this->options['expose']['identifier'])) {
      return;
    }

    // We only need to validate if there is a minimum word length set.
    if ($this->options['min_length'] < 2) {
      return;
    }

    $identifier = $this->options['expose']['identifier'];
    $input = &$form_state['values'][$identifier];

    if ($this->options['is_grouped'] && isset($this->options['group_info']['group_items'][$input])) {
      $this->operator = $this->options['group_info']['group_items'][$input]['operator'];
      $input = &$this->options['group_info']['group_items'][$input]['value'];
    }

    // If there is no input, we're fine.
    if (!trim($input)) {
      return;
    }

    $words = preg_split('/\s+/', $input);
    foreach ($words as $i => $word) {
      if (drupal_strlen($word) < $this->options['min_length']) {
        unset($words[$i]);
      }
    }
    if (!$words) {
      $vars['@count'] = $this->options['min_length'];
      $msg = t('You must include at least one positive keyword with @count characters or more.', $vars);
      form_error($form[$identifier], $msg);
    }
    $input = implode(' ', $words);
  }

  /**
   * Add this filter to the query.
   */
  public function query() {
    while (is_array($this->value)) {
      $this->value = $this->value ? reset($this->value) : '';
    }
    // Catch empty strings entered by the user, but not "0".
    if ($this->value === '') {
      return;
    }
    $fields = $this->options['fields'];
    $available_fields = array_keys($this->getFulltextFields());
    $fields = $fields ? array_intersect($fields, $available_fields) : $available_fields;

    // If something already specifically set different fields, we silently fall
    // back to mere filtering.
    $filter = $this->options['mode'] == 'filter';
    if (!$filter) {
      $old = $this->query->getFields();
      $filter = $old && (array_diff($old, $fields) || array_diff($fields, $old));
    }

    if ($filter) {
      $filter = $this->query->createFilter('OR');
      $op = $this->operator === 'NOT' ? '<>' : '=';
      foreach ($fields as $field) {
        $filter->condition($field, $this->value, $op);
      }
      $this->query->filter($filter);
      return;
    }

    // If the operator was set to OR or NOT, set OR as the conjunction. (It is
    // also set for NOT since otherwise it would be "not all of these words".)
    if ($this->operator != 'AND') {
      $this->query->setOption('conjunction', 'OR');
    }

    try {
      $this->query->fields($fields);
    }
    catch (SearchApiException $e) {
      $this->query->abort($e->getMessage());
      return;
    }
    $old = $this->query->getKeys();
    $old_original = $this->query->getOriginalKeys();
    $this->query->keys($this->value);
    if ($this->operator == 'NOT') {
      $keys = &$this->query->getKeys();
      if (is_array($keys)) {
        $keys['#negation'] = TRUE;
      }
      else {
        // We can't know how negation is expressed in the server's syntax.
      }
    }

    // If there were fulltext keys set, we take care to combine them in a
    // meaningful way (especially with negated keys).
    if ($old) {
      $keys = &$this->query->getKeys();
      // Array-valued keys are combined.
      if (is_array($keys)) {
        // If the old keys weren't parsed into an array, we instead have to
        // combine the original keys.
        if (is_scalar($old)) {
          $keys = "($old) ({$this->value})";
        }
        else {
          // If the conjunction or negation settings aren't the same, we have to
          // nest both old and new keys array.
          if (!empty($keys['#negation']) != !empty($old['#negation']) || $keys['#conjunction'] != $old['#conjunction']) {
            $keys = array(
              '#conjunction' => 'AND',
              $old,
              $keys,
            );
          }
          // Otherwise, just add all individual words from the old keys to the
          // new ones.
          else {
            foreach (element_children($old) as $i) {
              $keys[] = $old[$i];
            }
          }
        }
      }
      // If the parse mode was "direct" for both old and new keys, we
      // concatenate them and set them both via method and reference (to also
      // update the originalKeys property.
      elseif (is_scalar($old_original)) {
        $combined_keys = "($old_original) ($keys)";
        $this->query->keys($combined_keys);
        $keys = $combined_keys;
      }
    }
  }

  /**
   * Helper method to get an option list of all available fulltext fields.
   */
  protected function getFulltextFields() {
    $fields = array();
    $index = search_api_index_load(substr($this->table, 17));
    if (!empty($index->options['fields'])) {
      $f = $index->getFields();
      foreach ($index->getFulltextFields() as $name) {
        $fields[$name] = $f[$name]['name'];
      }
    }
    return $fields;
  }

}
