<?php

/**
 * @file
 * Date query type plugin for the Search API adapter.
 */

/**
 * Plugin for "date" query types.
 */
class SearchApiFacetapiDate extends SearchApiFacetapiTerm implements FacetapiQueryTypeInterface {

  /**
   * Loads the include file containing the date API functions.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    module_load_include('date.inc', 'facetapi');
    parent::__construct($adapter, $facet);
  }

  /**
   * Returns the query type associated with the plugin.
   *
   * @return string
   *   The query type.
   */
  static public function getType() {
    return 'date';
  }

  /**
   * Adds the filter to the query object.
   *
   * @param $query
   *   An object containing the query in the backend's native API.
   */
  public function execute($query) {
    // Return terms for this facet.
    $this->adapter->addFacet($this->facet, $query);

    $settings = $this->adapter->getFacet($this->facet)->getSettings()->settings;

    // First check if the facet is enabled for this search.
    $default_true = isset($settings['default_true']) ? $settings['default_true'] : TRUE;
    $facet_search_ids = isset($settings['facet_search_ids']) ? $settings['facet_search_ids'] : array();
    if ($default_true != empty($facet_search_ids[$query->getOption('search id')])) {
      // Facet is not enabled for this search ID.
      return;
    }

    // Change limit to "unlimited" (-1).
    $options = &$query->getOptions();
    if (!empty($options['search_api_facets'][$this->facet['name']])) {
      $options['search_api_facets'][$this->facet['name']]['limit'] = -1;
    }

    if ($active = $this->adapter->getActiveItems($this->facet)) {
      $item = end($active);
      $field = $this->facet['field'];
      $filter = $this->createRangeFilter($item['value']);
      $this->addFacetFilter($query, $field, $filter);
    }
  }

  /**
   * Rewrites the handler-specific date range syntax to the normal facet syntax.
   *
   * @param $value
   *   The user-facing facet value.
   *
   * @return string
   *   A facet to add as a filter, in the format used internally in this module.
   */
  protected function createRangeFilter($value) {
    // Gets the granularity. Ignore any filters passed directly from the server
    // (range or missing). We always create filters starting with a year.
    if (!$value || !ctype_digit($value[0])) {
      return $value;
    }

    $parts = explode('-', $value);
    $date = new DateTime();
    switch (count($parts)) {
      case 1:
        $date->setDate($parts[0], 1, 1);
        $date->setTime(0, 0, 0);
        $lower = $date->format('U');
        $date->setDate($parts[0] + 1, 1, 1);
        $date->setTime(0, 0, -1);
        $upper = $date->format('U');
        break;

      case 2:
        // Luckily, $month = 13 is treated as January of next year. (The same
        // goes for all other parameters.) We use the inverse trick for the
        // seconds of the upper bound, since that's inclusive and we want to
        // stop at a second before the next segment starts.
        $date->setDate($parts[0], $parts[1], 1);
        $date->setTime(0, 0, 0);
        $lower = $date->format('U');
        $date->setDate($parts[0], $parts[1] + 1, 1);
        $date->setTime(0, 0, -1);
        $upper = $date->format('U');
        break;

      case 3:
        $date->setDate($parts[0], $parts[1], $parts[2]);
        $date->setTime(0, 0, 0);
        $lower = $date->format('U');
        $date->setDate($parts[0], $parts[1], $parts[2] + 1);
        $date->setTime(0, 0, -1);
        $upper = $date->format('U');
        break;

      case 4:
        $date->setDate($parts[0], $parts[1], $parts[2]);
        $date->setTime($parts[3], 0, 0);
        $lower = $date->format('U');
        $date->setTime($parts[3] + 1, 0, -1);
        $upper = $date->format('U');
        break;

      case 5:
        $date->setDate($parts[0], $parts[1], $parts[2]);
        $date->setTime($parts[3], $parts[4], 0);
        $lower = $date->format('U');
        $date->setTime($parts[3], $parts[4] + 1, -1);
        $upper = $date->format('U');
        break;

      case 6:
        $date->setDate($parts[0], $parts[1], $parts[2]);
        $date->setTime($parts[3], $parts[4], $parts[5]);
        return $date->format('U');

      default:
        return $value;
    }

    return "[$lower TO $upper]";
  }

  /**
   * Replacement callback for replacing ISO dates with timestamps.
   *
   * Not used anymore, but kept for backwards compatibility with potential
   * subclasses.
   */
  public function replaceDateString($matches) {
    return strtotime($matches[0]);
  }

  /**
   * Initializes the facet's build array.
   *
   * @return array
   *   The initialized render array.
   */
  public function build() {
    $facet = $this->adapter->getFacet($this->facet);
    $search_ids = drupal_static('search_api_facetapi_active_facets', array());
    if (empty($search_ids[$facet['name']]) || !search_api_current_search($search_ids[$facet['name']])) {
      return array();
    }
    $search_id = $search_ids[$facet['name']];
    $build = array();
    $search = search_api_current_search($search_id);
    $results = $search[1];
    // Gets total number of documents matched in search.
    $total = $results['result count'];

    // Executes query, iterates over results.
    if (isset($results['search_api_facets']) && isset($results['search_api_facets'][$this->facet['name']])) {
      $values = $results['search_api_facets'][$this->facet['name']];
      foreach ($values as $value) {
        if ($value['count']) {
          $filter = $value['filter'];
          // We only process single values further. The "missing" filter and
          // range filters will be passed on unchanged.
          if ($filter == '!') {
            $build[$filter]['#count'] = $value['count'];
          }
          elseif ($filter[0] == '"') {
            $filter = substr($value['filter'], 1, -1);
            if ($filter) {
              $raw_values[$filter] = $value['count'];
            }
          }
          else {
            $build[$filter]['#count'] = $value['count'];
          }
        }
      }
    }

    // Get the finest level of detail we're allowed to drill down to.
    $settings = $facet->getSettings()->settings;
    $max_granularity = isset($settings['date_granularity']) ? $settings['date_granularity'] : FACETAPI_DATE_MINUTE;

    // Gets active facets, starts building hierarchy.
    $parent = $granularity = NULL;
    $active_items = $this->adapter->getActiveItems($this->facet);
    foreach ($active_items as $value => $item) {
      // If the item is active, the count is the result set count.
      $build[$value] = array('#count' => $total);

      // Gets next "gap" increment. Ignore any filters passed directly from the
      // server (range or missing). We always create filters starting with a
      // year.
      $value = "$value";
      if (!$value || !ctype_digit($value[0])) {
        continue;
      }

      $granularity = search_api_facetapi_date_get_granularity($value);
      if (!$granularity) {
        continue;
      }
      $granularity = facetapi_get_next_date_gap($granularity, $max_granularity);

      // If there is a previous item, there is a parent, uses a reference so the
      // arrays are populated when they are updated.
      if (NULL !== $parent) {
        $build[$parent]['#item_children'][$value] = &$build[$value];
        $build[$value]['#item_parents'][$parent] = $parent;
      }

      // Stores the last value iterated over.
      $parent = $value;
    }

    if (empty($raw_values)) {
      return $build;
    }
    ksort($raw_values);

    // Mind the gap! Calculates gap from min and max timestamps.
    $timestamps = array_keys($raw_values);
    if (NULL === $parent) {
      if (count($raw_values) > 1) {
        $granularity = facetapi_get_timestamp_gap(min($timestamps), max($timestamps), $max_granularity);
        // Array of numbers used to determine whether the next gap is smaller than
        // the minimum gap allowed in the drilldown.
        $gap_numbers = array(
          FACETAPI_DATE_YEAR => 6,
          FACETAPI_DATE_MONTH => 5,
          FACETAPI_DATE_DAY => 4,
          FACETAPI_DATE_HOUR => 3,
          FACETAPI_DATE_MINUTE => 2,
          FACETAPI_DATE_SECOND => 1,
        );
        // Gets gap numbers for both the gap and minimum gap, checks if the gap
        // is within the limit set by the $granularity parameter.
        if ($gap_numbers[$granularity] < $gap_numbers[$max_granularity]) {
          $granularity = $max_granularity;
        }
      }
      else {
        $granularity = $max_granularity;
      }
    }

    // Groups dates by the range they belong to, builds the $build array with
    // the facet counts and formatted range values.
    $format = search_api_facetapi_date_get_granularity_format($granularity);
    foreach ($raw_values as $value => $count) {
      $new_value = date($format, $value);
      if (!isset($build[$new_value])) {
        $build[$new_value] = array('#count' => $count);
      }
      // Active items already have their value set because it's the current
      // result count.
      elseif (!isset($active_items[$new_value])) {
        $build[$new_value]['#count'] += $count;
      }

      // Adds parent information if not already set.
      if (NULL !== $parent && $parent != $new_value) {
        $build[$parent]['#item_children'][$new_value] = &$build[$new_value];
        $build[$new_value]['#item_parents'][$parent] = $parent;
      }
    }

    return $build;
  }

}
