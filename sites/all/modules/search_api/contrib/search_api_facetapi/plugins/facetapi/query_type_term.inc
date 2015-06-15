<?php

/**
 * @file
 * Term query type plugin for the Apache Solr adapter.
 */

/**
 * Plugin for "term" query types.
 */
class SearchApiFacetapiTerm extends FacetapiQueryType implements FacetapiQueryTypeInterface {

  /**
   * Returns the query type associated with the plugin.
   *
   * @return string
   *   The query type.
   */
  static public function getType() {
    return 'term';
  }

  /**
   * Adds the filter to the query object.
   *
   * @param SearchApiQueryInterface $query
   *   An object containing the query in the backend's native API.
   */
  public function execute($query) {
    // Return terms for this facet.
    $this->adapter->addFacet($this->facet, $query);

    $settings = $this->getSettings()->settings;

    // First check if the facet is enabled for this search.
    $default_true = isset($settings['default_true']) ? $settings['default_true'] : TRUE;
    $facet_search_ids = isset($settings['facet_search_ids']) ? $settings['facet_search_ids'] : array();
    if ($default_true != empty($facet_search_ids[$query->getOption('search id')])) {
      // Facet is not enabled for this search ID.
      return;
    }

    // Retrieve the active facet filters.
    $active = $this->adapter->getActiveItems($this->facet);
    if (empty($active)) {
      return;
    }

    // Create the facet filter, and add a tag to it so that it can be easily
    // identified down the line by services when they need to exclude facets.
    $operator = $settings['operator'];
    if ($operator == FACETAPI_OPERATOR_AND) {
      $conjunction = 'AND';
    }
    elseif ($operator == FACETAPI_OPERATOR_OR) {
      $conjunction = 'OR';
      // When the operator is OR, remove parent terms from the active ones if
      // children are active. If we don't do this, sending a term and its
      // parent will produce the same results as just sending the parent.
      if ($settings['flatten'] == '0') {
        // Check the filters in reverse order, to avoid checking parents that
        // will afterwards be removed anyways.
        foreach (array_reverse(array_keys($active)) as $filter) {
          // Skip this filter if it was already removed, or if it is the
          // "missing value" filter ("!").
          if (!isset($active[$filter]) || !is_numeric($filter)) {
            continue;
          }
          $parents = taxonomy_get_parents_all($filter);
          // The return value of taxonomy_get_parents_all() includes the term
          // itself at index 0. Remove that to only get the term's ancestors.
          unset($parents[0]);
          foreach ($parents as $parent) {
            unset($active[$parent->tid]);
          }
        }
      }
    }
    else {
      $vars = array(
        '%operator' => $operator,
        '%facet' => !empty($this->facet['label']) ? $this->facet['label'] : $this->facet['name'],
      );
      watchdog('search_api_facetapi', 'Unknown facet operator %operator used for facet %facet.', $vars, WATCHDOG_WARNING);
      return;
    }
    $tags = array('facet:' . $this->facet['field']);
    $facet_filter = $query->createFilter($conjunction, $tags);

    foreach ($active as $filter => $filter_array) {
      $field = $this->facet['field'];
      $this->addFacetFilter($facet_filter, $field, $filter);
    }

    // Now add the filter to the query.
    $query->filter($facet_filter);
  }

  /**
   * Helper method for setting a facet filter on a query or query filter object.
   */
  protected function addFacetFilter($query_filter, $field, $filter) {
    // Test if this filter should be negated.
    $settings = $this->adapter->getFacet($this->facet)->getSettings();
    $exclude = !empty($settings->settings['exclude']);
    // Integer (or other non-string) filters might mess up some of the following
    // comparison expressions.
    $filter = (string) $filter;
    if ($filter == '!') {
      $query_filter->condition($field, NULL, $exclude ? '<>' : '=');
    }
    elseif ($filter[0] == '[' && $filter[strlen($filter) - 1] == ']' && ($pos = strpos($filter, ' TO '))) {
      $lower = trim(substr($filter, 1, $pos));
      $upper = trim(substr($filter, $pos + 4, -1));
      if ($lower == '*' && $upper == '*') {
        $query_filter->condition($field, NULL, $exclude ? '=' : '<>');
      }
      elseif (!$exclude) {
        if ($lower != '*') {
          // Iff we have a range with two finite boundaries, we set two
          // conditions (larger than the lower bound and less than the upper
          // bound) and therefore have to make sure that we have an AND
          // conjunction for those.
          if ($upper != '*' && !($query_filter instanceof SearchApiQueryInterface || $query_filter->getConjunction() === 'AND')) {
            $original_query_filter = $query_filter;
            $query_filter = new SearchApiQueryFilter('AND');
          }
          $query_filter->condition($field, $lower, '>=');
        }
        if ($upper != '*') {
          $query_filter->condition($field, $upper, '<=');
        }
      }
      else {
        // Same as above, but with inverted logic.
        if ($lower != '*') {
          if ($upper != '*' && ($query_filter instanceof SearchApiQueryInterface || $query_filter->getConjunction() === 'AND')) {
            $original_query_filter = $query_filter;
            $query_filter = new SearchApiQueryFilter('OR');
          }
          $query_filter->condition($field, $lower, '<');
        }
        if ($upper != '*') {
          $query_filter->condition($field, $upper, '>');
        }
      }
    }
    else {
      $query_filter->condition($field, $filter, $exclude ? '<>' : '=');
    }
    if (isset($original_query_filter)) {
      $original_query_filter->filter($query_filter);
    }
  }

  /**
   * Initializes the facet's build array.
   *
   * @return array
   *   The initialized render array.
   */
  public function build() {
    $facet = $this->adapter->getFacet($this->facet);
    // The current search per facet is stored in a static variable (during
    // initActiveFilters) so that we can retrieve it here and get the correct
    // current search for this facet.
    $search_ids = drupal_static('search_api_facetapi_active_facets', array());
    if (empty($search_ids[$facet['name']]) || !search_api_current_search($search_ids[$facet['name']])) {
      return array();
    }
    $search_id = $search_ids[$facet['name']];
    list(, $results) = search_api_current_search($search_id);
    $build = array();

    // Always include the active facet items.
    foreach ($this->adapter->getActiveItems($this->facet) as $filter)  {
      $build[$filter['value']]['#count'] = $results['result count'];
    }

    // Then, add the facets returned by the server.
    if (isset($results['search_api_facets']) && isset($results['search_api_facets'][$this->facet['name']])) {
      $values = $results['search_api_facets'][$this->facet['name']];
      foreach ($values as $value) {
        $filter = $value['filter'];
        // As Facet API isn't really suited for our native facet filter
        // representations, convert the format here. (The missing facet can
        // stay the same.)
        if ($filter[0] == '"') {
          $filter = substr($filter, 1, -1);
        }
        elseif ($filter != '!') {
          // This is a range filter.
          $filter = substr($filter, 1, -1);
          $pos = strpos($filter, ' ');
          if ($pos !== FALSE) {
            $filter = '[' . substr($filter, 0, $pos) . ' TO ' . substr($filter, $pos + 1) . ']';
          }
        }
        $build[$filter] = array(
          '#count' => $value['count'],
        );
      }
    }
    return $build;
  }

}
