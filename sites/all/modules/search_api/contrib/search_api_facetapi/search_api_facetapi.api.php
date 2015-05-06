<?php

/**
 * @file
 * Hooks provided by the Search facets module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Lets modules alter the search keys that are returned to FacetAPI and used
 * in the current search block and breadcrumb trail.
 *
 * @param string $keys
 *   The string representing the user's current search query.
 * @param SearchApiQuery $query
 *   The SearchApiQuery object for the current search.
 */
function hook_search_api_facetapi_keys_alter(&$keys, $query) {
  if ($keys == '[' . t('all items') . ']') {
    // Change $keys to something else, perhaps based on filters in the query
    // object.
  }
}

/**
 * @} End of "addtogroup hooks".
 */
