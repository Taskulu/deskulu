<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterTaxonomyTerm.
 */

/**
 * Views filter handler class for taxonomy term entities.
 *
 * Based on views_handler_filter_term_node_tid.
 */
class SearchApiViewsHandlerFilterTaxonomyTerm extends SearchApiViewsHandlerFilterEntity {

  /**
   * {@inheritdoc}
   */
  public function has_extra_options() {
    return !empty($this->definition['vocabulary']);
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['type'] = array('default' => !empty($this->definition['vocabulary']) ? 'textfield' : 'select');
    $options['hierarchy'] = array('default' => 0);
    $options['error_message'] = array('default' => TRUE, 'bool' => TRUE);

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function extra_options_form(&$form, &$form_state) {
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Selection type'),
      '#options' => array('select' => t('Dropdown'), 'textfield' => t('Autocomplete')),
      '#default_value' => $this->options['type'],
    );

    $form['hierarchy'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show hierarchy in dropdown'),
      '#default_value' => !empty($this->options['hierarchy']),
    );
    $form['hierarchy']['#states']['visible'][':input[name="options[type]"]']['value'] = 'select';
  }

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    if (!empty($this->definition['vocabulary'])) {
      $vocabulary = taxonomy_vocabulary_machine_name_load($this->definition['vocabulary']);
      $title = t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name));
    }
    else {
      $vocabulary = FALSE;
      $title = t('Select terms');
    }
    $form['value']['#title'] = $title;

    if ($vocabulary && $this->options['type'] == 'textfield') {
      $form['value']['#autocomplete_path'] = 'admin/views/ajax/autocomplete/taxonomy/' . $vocabulary->vid;
    }
    else {
      if ($vocabulary && !empty($this->options['hierarchy'])) {
        $tree = taxonomy_get_tree($vocabulary->vid);
        $options = array();

        if ($tree) {
          foreach ($tree as $term) {
            $choice = new stdClass();
            $choice->option = array($term->tid => str_repeat('-', $term->depth) . $term->name);
            $options[] = $choice;
          }
        }
      }
      else {
        $options = array();
        $query = db_select('taxonomy_term_data', 'td');
        $query->innerJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
        $query->fields('td');
        $query->orderby('tv.weight');
        $query->orderby('tv.name');
        $query->orderby('td.weight');
        $query->orderby('td.name');
        $query->addTag('term_access');
        if ($vocabulary) {
          $query->condition('tv.machine_name', $vocabulary->machine_name);
        }
        $result = $query->execute();
        foreach ($result as $term) {
          $options[$term->tid] = $term->name;
        }
      }

      $default_value = (array) $this->value;

      if (!empty($form_state['exposed'])) {
        $identifier = $this->options['expose']['identifier'];

        if (!empty($this->options['expose']['reduce'])) {
          $options = $this->reduce_value_options($options);

          if (!empty($this->options['expose']['multiple']) && empty($this->options['expose']['required'])) {
            $default_value = array();
          }
        }

        if (empty($this->options['expose']['multiple'])) {
          if (empty($this->options['expose']['required']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
            $default_value = 'All';
          }
          elseif (empty($default_value)) {
            $keys = array_keys($options);
            $default_value = array_shift($keys);
          }
          // Due to #1464174 there is a chance that array('') was saved in the
          // admin ui. Let's choose a safe default value.
          elseif ($default_value == array('')) {
            $default_value = 'All';
          }
          else {
            $copy = $default_value;
            $default_value = array_shift($copy);
          }
        }
      }
      $form['value']['#type'] = 'select';
      $form['value']['#multiple'] = TRUE;
      $form['value']['#options'] = $options;
      $form['value']['#size'] = min(9, count($options));
      $form['value']['#default_value'] = $default_value;

      if (!empty($form_state['exposed']) && isset($identifier) && !isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $default_value;
      }
    }
  }

  /**
   * Reduces the available exposed options according to the selection.
   */
  protected function reduce_value_options(array $options) {
    foreach ($options as $id => $option) {
      if (empty($this->options['value'][$id])) {
        unset($options[$id]);
      }
    }
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function value_validate($form, &$form_state) {
    // We only validate if they've chosen the text field style.
    if ($this->options['type'] != 'textfield') {
      return;
    }

    parent::value_validate($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    // We need to know the operator, which is normally set in
    // views_handler_filter::accept_exposed_input(), before we actually call
    // the parent version of ourselves.
    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator_id']) && isset($input[$this->options['expose']['operator_id']])) {
      $this->operator = $input[$this->options['expose']['operator_id']];
    }

    // If view is an attachment and is inheriting exposed filters, then assume
    // exposed input has already been validated.
    if (!empty($this->view->is_attachment) && $this->view->display_handler->uses_exposed()) {
      $this->validated_exposed_input = (array) $this->view->exposed_raw_input[$this->options['expose']['identifier']];
    }

    // If we're checking for EMPTY or NOT, we don't need any input, and we can
    // say that our input conditions are met by just having the right operator.
    if ($this->operator == 'empty' || $this->operator == 'not empty') {
      return TRUE;
    }

    // If it's non-required and there's no value don't bother filtering.
    if (!$this->options['expose']['required'] && empty($this->validated_exposed_input)) {
      return FALSE;
    }

    return parent::accept_exposed_input($input);
  }

  /**
   * {@inheritdoc}
   */
  public function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed']) || empty($this->options['expose']['identifier'])) {
      return;
    }

    // We only validate if they've chosen the text field style.
    if ($this->options['type'] != 'textfield') {
      $input = $form_state['values'][$this->options['expose']['identifier']];
      if ($this->options['is_grouped'] && isset($this->options['group_info']['group_items'][$input])) {
        $input = $this->options['group_info']['group_items'][$input]['value'];
      }

      if ($input != 'All')  {
        $this->validated_exposed_input = (array) $input;
      }
      return;
    }

    parent::exposed_validate($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  protected function validate_entity_strings(array &$form, array $values) {
    if (empty($values)) {
      return array();
    }

    $tids = array();
    $names = array();
    $missing = array();
    foreach ($values as $value) {
      $missing[strtolower($value)] = TRUE;
      $names[] = $value;
    }

    if (!$names) {
      return FALSE;
    }

    $query = db_select('taxonomy_term_data', 'td');
    $query->innerJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
    $query->fields('td');
    $query->condition('td.name', $names);
    if (!empty($this->definition['vocabulary'])) {
      $query->condition('tv.machine_name', $this->definition['vocabulary']);
    }
    $query->addTag('term_access');
    $result = $query->execute();
    foreach ($result as $term) {
      unset($missing[strtolower($term->name)]);
      $tids[] = $term->tid;
    }

    if ($missing) {
      if (!empty($this->options['error_message'])) {
        form_error($form, format_plural(count($missing), 'Unable to find term: @terms', 'Unable to find terms: @terms', array('@terms' => implode(', ', array_keys($missing)))));
      }
      else {
        // Add a bogus TID which will show an empty result for a positive filter
        // and be ignored for an excluding one.
        $tids[] = 0;
      }
    }

    return $tids;
  }

  /**
   * {@inheritdoc}
   */
  public function expose_form(&$form, &$form_state) {
    parent::expose_form($form, $form_state);
    if ($this->options['type'] != 'select') {
      unset($form['expose']['reduce']);
    }
    $form['error_message'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display error message'),
      '#description' => t('Display an error message if one of the entered terms could not be found.'),
      '#default_value' => !empty($this->options['error_message']),
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function ids_to_strings(array $ids) {
    return implode(', ', db_select('taxonomy_term_data', 'td')
      ->fields('td', array('name'))
      ->condition('td.tid', array_filter($ids))
      ->execute()
      ->fetchCol());
  }

}
