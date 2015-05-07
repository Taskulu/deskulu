<?php

/**
 * Filter by term id.
 */
class i18nviews_handler_filter_taxonomy_index_tid extends views_handler_filter_term_node_tid {

  function value_form(&$form, &$form_state) {
    $vocabulary = taxonomy_vocabulary_machine_name_load($this->options['vocabulary']);
    if (empty($vocabulary) && $this->options['limit']) {
      $form['markup'] = array(
        '#prefix' => '<div class="form-item">',
        '#suffix' => '</div>',
        '#value' => t('An invalid vocabulary is selected. Please change it in the options.'),
      );
      return;
    }

    if ($this->options['type'] == 'textfield') {
      $default = '';
      if ($this->value) {
        $terms = taxonomy_term_load_multiple($this->value);
        foreach ($terms as $term) {
          if ($default) {
            $default .= ', ';
          }
          $default .= check_plain(i18n_taxonomy_term_name($term));
        }
      }

      $form['value'] = array(
        '#title' => $this->options['limit'] ? t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)) : t('Select terms'),
        '#type' => 'textfield',
        '#default_value' => $default,
      );

      if ($this->options['limit']) {
        $form['value']['#autocomplete_path'] = 'i18n/taxonomy/autocomplete/language/' . $vocabulary->vid;
      }
    }
    else {
      if (!empty($this->options['hierarchy']) && $this->options['limit']) {
        global $language;
        $tree = taxonomy_get_tree($vocabulary->vid);
        $options = array();

        if ($tree) {
          foreach ($tree as $term) {
            $choice = new stdClass();
            $choice->option = array($term->tid => str_repeat('-', $term->depth) . check_plain(i18n_taxonomy_term_name($term)));
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
        if ($this->options['limit']) {
          $query->condition('tv.machine_name', $vocabulary->machine_name);
        }
        $result = $query->execute();
        foreach ($result as $term) {
          $options[$term->tid] = i18n_taxonomy_term_name($term);
        }
      }

      $default_value = (array) $this->value;

      if (!empty($form_state['exposed'])) {
        $identifier = $this->options['expose']['identifier'];

        if (!empty($this->options['expose']['reduce'])) {
          $options = $this->reduce_value_options($options);

          if (empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
            $default_value = array();
          }
        }

        if (!empty($this->options['expose']['single'])) {
          if (!empty($this->options['expose']['optional']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
            $default_value = 'All';
          }
          else if (empty($default_value)) {
            $keys = array_keys($options);
            $default_value = array_shift($keys);
          }
          else {
            $copy = $default_value;
            $default_value = array_shift($copy);
          }
        }
      }
      $form['value'] = array(
        '#type' => 'select',
        '#title' => $this->options['limit'] ? t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)) : t('Select terms'),
        '#multiple' => TRUE,
        '#options' => $options,
        '#size' => min(9, count($options)),
        '#default_value' => $default_value,
      );

      if (!empty($form_state['exposed']) && !isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $default_value;
      }
    }


    if (empty($form_state['exposed'])) {
      // Retain the helper option
      $this->helper->options_form($form, $form_state);
    }
  }

  /**
   * Validate the user string. Since this can come from either the form
   * or the exposed filter, this is abstracted out a bit so it can
   * handle the multiple input sources.
   */
  function validate_term_strings(&$form, $values) {
    if (empty($values)) {
      return array();
    }

    $tids = array();
    $placeholders = array();
    $args = array();
    $results = array();
    foreach ($values as $value) {
      $missing[strtolower($value)] = TRUE;
      $names[] = $value;
    }

    if (!$names) {
      return;
    }

    $query = db_select('taxonomy_term_data', 'td');
    $query->innerJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
    $query->fields('td');
    $query->condition('td.name', $names);
    $query->condition('tv.machine_name', $this->options['vocabulary']);
    $query->addTag('term_access');
    $result = $query->execute();
    foreach ($result as $term) {
      unset($missing[strtolower(i18n_taxonomy_term_name($term->name))]);
      $tids[] = $term->tid;
    }

    if ($missing && !empty($this->options['error_message'])) {
      form_error($form, format_plural(count($missing), 'Unable to find term: @terms', 'Unable to find terms: @terms', array('@terms' => implode(', ', array_keys($missing)))));
    }
    elseif ($missing && empty($this->options['error_message'])) {
      $tids = array(0);
    }

    return $tids;

  }


  function admin_summary() {
// set up $this->value_options for the parent summary
    $this->value_options = array();

    if ($this->value) {
      $result = db_select('taxonomy_term_data', 'td')
        ->fields('td')
        ->condition('td.tid', $this->value)
        ->execute();
      foreach ($result as $term) {
        $this->value_options[$term->tid] = i18n_taxonomy_term_name($term);
      }
    }
    return parent::admin_summary();
  }
}
