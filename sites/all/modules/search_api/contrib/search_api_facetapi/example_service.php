<?php

/**
 * @file
 * Example implementation for a service class which supports facets.
 */

/**
 * Example class explaining how facets can be supported by a service class.
 *
 * This class defines the "search_api_facets" and
 * "search_api_facets_operator_or" features. Read the method documentation and
 * inline comments in search() to learn how they can be supported by a service
 * class.
 */
abstract class SearchApiFacetapiExampleService extends SearchApiAbstractService {

  /**
   * Determines whether this service class implementation supports a given
   * feature. Features are optional extensions to Search API functionality and
   * usually defined and used by third-party modules.
   *
   * If the service class supports facets, it should return TRUE if called with
   * the feature name "search_api_facets". If it also supports "OR" facets, it
   * should also return TRUE if called with "search_api_facets_operator_or".
   *
   * @param string $feature
   *   The name of the optional feature.
   *
   * @return boolean
   *   TRUE if this service knows and supports the specified feature. FALSE
   *   otherwise.
   */
  public function supportsFeature($feature) {
    $supported = array(
      'search_api_facets' => TRUE,
      'search_api_facets_operator_or' => TRUE,
    );
    return isset($supported[$feature]);
  }

  /**
   * Executes a search on the server represented by this object.
   *
   * If the service class supports facets, it should check for an additional
   * option on the query object:
   * - search_api_facets: An array of facets to return along with the results
   *   for this query. The array is keyed by an arbitrary string which should
   *   serve as the facet's unique identifier for this search. The values are
   *   arrays with the following keys:
   *   - field: The field to construct facets for.
   *   - limit: The maximum number of facet terms to return. 0 or an empty
   *     value means no limit.
   *   - min_count: The minimum number of results a facet value has to have in
   *     order to be returned.
   *   - missing: If TRUE, a facet for all items with no value for this field
   *     should be returned (if it conforms to limit and min_count).
   *   - operator: (optional) If the service supports "OR" facets and this key
   *     contains the string "or", the returned facets should be "OR" facets. If
   *     the server doesn't support "OR" facets, this key can be ignored.
   *
   * The basic principle of facets is explained quite well in the
   * @link http://en.wikipedia.org/wiki/Faceted_search Wikipedia article on
   * "Faceted search" @endlink. Basically, you should return for each field
   * filter values which would yield some results when used with the search.
   * E.g., if you return for a field $field the term $term with $count results,
   * the given $query along with
   *   $query->condition($field, $term)
   * should yield exactly (or about) $count results.
   *
   * For "OR" facets, all existing filters on the facetted field should be
   * ignored for computing the facets.
   *
   * @param $query
   *   The SearchApiQueryInterface object to execute.
   *
   * @return array
   *   An associative array containing the search results, as required by
   *   SearchApiQueryInterface::execute().
   *   In addition, if the "search_api_facets" option is present on the query,
   *   the results should contain an array of facets in the "search_api_facets"
   *   key, as specified by the option. The facets array should be keyed by the
   *   facets' unique identifiers, and contain a numeric array of facet terms,
   *   sorted descending by result count. A term is represented by an array with
   *   the following keys:
   *   - count: Number of results for this term.
   *   - filter: The filter to apply when selecting this facet term. A filter is
   *     a string of one of the following forms:
   *     - "VALUE": Filter by the literal value VALUE (always include the
   *       quotes, not only for strings).
   *     - [VALUE1 VALUE2]: Filter for a value between VALUE1 and VALUE2. Use
   *       parantheses for excluding the border values and square brackets for
   *       including them. An asterisk (*) can be used as a wildcard. E.g.,
   *       (* 0) or [* 0) would be a filter for all negative values.
   *     - !: Filter for items without a value for this field (i.e., the
   *       "missing" facet).
   *
   * @throws SearchApiException
   *   If an error prevented the search from completing.
   */
  public function search(SearchApiQueryInterface $query) {
    // We assume here that we have an AI search which understands English
    // commands.

    // First, create the normal search query, without facets.
    $search = new SuperCoolAiSearch($query->getIndex());
    $search->cmd('create basic search for the following query', $query);
    $ret = $search->cmd('return search results in Search API format');

    // Then, let's see if we should return any facets.
    if ($facets = $query->getOption('search_api_facets')) {
      // For the facets, we need all results, not only those in the specified
      // range.
      $results = $search->cmd('return unlimited search results as a set');
      foreach ($facets as $id => $facet) {
        $field = $facet['field'];
        $limit = empty($facet['limit']) ? 'all' : $facet['limit'];
        $min_count = $facet['min_count'];
        $missing = $facet['missing'];
        $or = isset($facet['operator']) && $facet['operator'] == 'or';

        // If this is an "OR" facet, existing filters on the field should be
        // ignored for computing the facets.
        // You can ignore this if your service class doesn't support the
        // "search_api_facets_operator_or" feature.
        if ($or) {
          // We have to execute another query (in the case of this hypothetical
          // search backend, at least) to get the right result set to facet.
          $tmp_search = new SuperCoolAiSearch($query->getIndex());
          $tmp_search->cmd('create basic search for the following query', $query);
          $tmp_search->cmd("remove all conditions for field $field");
          $tmp_results = $tmp_search->cmd('return unlimited search results as a set');
        }
        else {
          // Otherwise, we can just use the normal results.
          $tmp_results = $results;
        }

        $filters = array();
        if ($search->cmd("$field is a date or numeric field")) {
          // For date, integer or float fields, range facets are more useful.
          $ranges = $search->cmd("list $limit ranges of field $field in the following set", $tmp_results);
          foreach ($ranges as $range) {
            if ($range->getCount() >= $min_count) {
              // Get the lower and upper bound of the range. * means unlimited.
              $lower = $range->getLowerBound();
              $lower = ($lower == SuperCoolAiSearch::RANGE_UNLIMITED) ? '*' : $lower;
              $upper = $range->getUpperBound();
              $upper = ($upper == SuperCoolAiSearch::RANGE_UNLIMITED) ? '*' : $upper;
              // Then, see whether the bounds are included in the range. These
              // can be specified independently for the lower and upper bound.
              // Parentheses are used for exclusive bounds, square brackets are
              // used for inclusive bounds.
              $lowChar = $range->isLowerBoundInclusive() ? '[' : '(';
              $upChar = $range->isUpperBoundInclusive() ? ']' : ')';
              // Create the filter, which separates the bounds with a single
              // space.
              $filter = "$lowChar$lower $upper$upChar";
              $filters[$filter] = $range->getCount();
            }
          }
        }
        else {
          // Otherwise, we use normal single-valued facets.
          $terms = $search->cmd("list $limit values of field $field in the following set", $tmp_results);
          foreach ($terms as $term) {
            if ($term->getCount() >= $min_count) {
              // For single-valued terms, we just need to wrap them in quotes.
              $filter = '"' . $term->getValue() . '"';
              $filters[$filter] = $term->getCount();
            }
          }
        }

        // If we should also return a "missing" facet, compute that as the
        // number of results without a value for the facet field.
        if ($missing) {
          $count = $search->cmd("return number of results without field $field in the following set", $tmp_results);
          if ($count >= $min_count) {
            $filters['!'] = $count;
          }
        }

        // Sort the facets descending by result count.
        arsort($filters);

        // With the "missing" facet, we might have too many facet terms (unless
        // $limit was empty and is therefore now set to "all"). If this is the
        // case, remove those with the lowest number of results.
        while (is_numeric($limit) && count($filters) > $limit) {
          array_pop($filters);
        }

        // Now add the facet terms to the return value, as specified in the doc
        // comment for this method.
        foreach ($filters as $filter => $count) {
          $ret['search_api_facets'][$id][] = array(
            'count' => $count,
            'filter' => $filter,
          );
        }
      }
    }

    // Return the results, which now also includes the facet information.
    return $ret;
  }

}
