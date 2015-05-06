<?php

/**
 * @file
 * Contains SearchApiViewsHandlerArgumentFulltext.
 */

/**
 * Views argument handler class for handling fulltext fields.
 */
class SearchApiViewsHandlerArgumentFulltext extends SearchApiViewsHandlerArgument {

  /**
   * Specify the options this filter uses.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['fields'] = array('default' => array());
    $options['conjunction'] = array('default' => 'AND');
    return $options;
  }

  /**
   * Extend the options form a bit.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['help']['#markup'] = t('Note: You can change how search keys are parsed under "Advanced" > "Query settings".');

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
      $form['conjunction'] = array(
        '#title' => t('Operator'),
        '#description' => t('Determines how multiple keywords entered for the search will be combined.'),
        '#type' => 'radios',
        '#options' => array(
          'AND' => t('Contains all of these words'),
          'OR' => t('Contains any of these words'),
        ),
        '#default_value' => $this->options['conjunction'],
      );

    }
    else {
      $form['fields'] = array(
        '#type' => 'value',
        '#value' => array(),
      );
    }
  }

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  public function query($group_by = FALSE) {
    if ($this->options['fields']) {
      try {
        $this->query->fields($this->options['fields']);
      }
      catch (SearchApiException $e) {
        $this->query->abort($e->getMessage());
        return;
      }
    }
    if ($this->options['conjunction'] != 'AND') {
      $this->query->setOption('conjunction', $this->options['conjunction']);
    }

    $old = $this->query->getOriginalKeys();
    $this->query->keys($this->argument);
    if ($old) {
      $keys = &$this->query->getKeys();
      if (is_array($keys)) {
        $keys[] = $old;
      }
      elseif (is_array($old)) {
        // We don't support such nonsense.
      }
      else {
        $keys = "($old) ($keys)";
      }
    }
  }

  /**
   * Helper method to get an option list of all available fulltext fields.
   */
  protected function getFulltextFields() {
    $ret = array();
    $index = search_api_index_load(substr($this->table, 17));
    if (!empty($index->options['fields'])) {
      $fields = $index->getFields();
      foreach ($index->getFulltextFields() as $field) {
        $ret[$field] = $fields[$field]['name'];
      }
    }
    return $ret;
  }

}
