<?php

/**
 * @file
 *   Contains the SearchApiViewsHandlerArgumentTaxonomyTerm class.
 */

/**
 * Defines a contextual filter searching through all indexed taxonomy fields.
 */
class SearchApiViewsHandlerArgumentTaxonomyTerm extends SearchApiViewsHandlerArgument {

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  public function query($group_by = FALSE) {
    if (empty($this->value)) {
      $this->fillValue();
    }

    $outer_conjunction = strtoupper($this->operator);

    if (empty($this->options['not'])) {
      $operator = '=';
      $inner_conjunction = 'OR';
    }
    else {
      $operator = '<>';
      $inner_conjunction = 'AND';
    }

    if (!empty($this->value)) {
      $terms = entity_load('taxonomy_term', $this->value);

      if (!empty($terms)) {
        $filter = $this->query->createFilter($outer_conjunction);
        $vocabulary_fields = $this->definition['vocabulary_fields'];
        $vocabulary_fields += array('' => array());
        foreach ($terms as $term) {
          $inner_filter = $filter;
          if ($outer_conjunction != $inner_conjunction) {
            $inner_filter = $this->query->createFilter($inner_conjunction);
          }
          // Set filters for all term reference fields which don't specify a
          // vocabulary, as well as for all fields specifying the term's
          // vocabulary.
          if (!empty($this->definition['vocabulary_fields'][$term->vocabulary_machine_name])) {
            foreach ($this->definition['vocabulary_fields'][$term->vocabulary_machine_name] as $field) {
              $inner_filter->condition($field, $term->tid, $operator);
            }
          }
          foreach ($vocabulary_fields[''] as $field) {
            $inner_filter->condition($field, $term->tid, $operator);
          }
          if ($outer_conjunction != $inner_conjunction) {
            $filter->filter($inner_filter);
          }
        }

        $this->query->filter($filter);
      }
    }
  }

  /**
   * Get the title this argument will assign the view, given the argument.
   */
  public function title() {
    if (!empty($this->argument)) {
      if (empty($this->value)) {
        $this->fillValue();
      }
      $terms = array();
      foreach ($this->value as $tid) {
        $taxonomy_term = taxonomy_term_load($tid);
        if ($taxonomy_term) {
          $terms[] = check_plain($taxonomy_term->name);
        }
      }

      return $terms ? implode(', ', $terms) : check_plain($this->argument);
    }
    else {
      return check_plain($this->argument);
    }
  }

  /**
   * Fill $this->value with data from the argument.
   *
   * Uses views_break_phrase(), if appropriate.
   */
  protected function fillValue() {
    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }
  }

}
