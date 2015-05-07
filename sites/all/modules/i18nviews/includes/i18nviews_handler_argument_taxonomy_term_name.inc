<?php

class i18nviews_handler_argument_taxonomy_term_name extends views_handler_argument_string {

  /**
   * Build the query based upon the formula.
   *
   * @todo:
   *   This function could be so much easier if this->value would be an array of the availible arguments.
   *   The only thing which would have to be done is to replace $argument.
   */
  function query($group_by = FALSE) {
    $argument = $this->argument;
    if (!empty($this->options['transform_dash'])) {
      $argument = strtr($argument, '-', ' ');
    }
    $this->placeholder_length = $this->placeholder();

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase_string($argument, $this);
    }
    else {
      $this->value = array($argument);
      $this->operator = 'or';
    }

    // Translate all availible terms.
    foreach ($this->value as &$value) {
      $source = i18nviews_locale_source($value, 'taxonomy');
      if ($source->source) {
        $value = $source->source;
      }
    }

    if (!empty($this->definition['many to one'])) {
      if (!empty($this->options['glossary'])) {
        $this->helper->formula = TRUE;
      }
      $this->helper->ensure_my_table();

      $this->helper->placeholders = array($this->placeholder_length => intval($this->options['limit']));
      $this->helper->add_filter();
      return;
    }

    $this->ensure_my_table();
    $formula = FALSE;
    if (empty($this->options['glossary'])) {
      $field = "$this->table_alias.$this->real_field";
    }
    else {
      $formula = TRUE;
      $field = $this->get_formula();
    }

    if (count($this->value) > 1) {
      $operator = 'IN';
      $argument = $this->value;
    }
    else {
      $operator = '=';
    }

    if ($formula) {
      $placeholder = $this->placeholder();
      $placeholder_length = $this->placeholder_length;
      if ($operator == 'IN') {
        $field .= " IN($placeholder)";
      }
      else {
        $field .= ' = ' . $placeholder;
      }
      $placeholders = array(
        $placeholder_length => intval($this->options['limit']),
        $placeholder => $argument,
      );
      $this->query->add_where_expression(0, $field, $placeholders);
    }
    else {
      $this->query->add_where(0, $field, $argument, $operator);
    }
  }
}