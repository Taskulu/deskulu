<?php

/**
 * @file
 * Hooks provided by the Search Views module.
 */

/**
 * Alter the query before executing the query.
 *
 * @param view $view
 *   The view object about to be processed.
 * @param SearchApiViewsQuery $query
 *   The Search API Views query to be altered.
 *
 * @see hook_views_query_alter()
 */
function hook_search_api_views_query_alter(view &$view, SearchApiViewsQuery &$query) {
  // (Example assuming a view with an exposed filter on node title.)
  // If the input for the title filter is a positive integer, filter against
  // node ID instead of node title.
  if ($view->name == 'my_view' && is_numeric($view->exposed_raw_input['title']) && $view->exposed_raw_input['title'] > 0) {
    // Traverse through the 'where' part of the query.
    foreach ($query->where as &$condition_group) {
      foreach ($condition_group['conditions'] as &$condition) {
        // If this is the part of the query filtering on title, chang the
        // condition to filter on node ID.
        if (reset($condition) == 'node.title') {
          $condition = array('node.nid', $view->exposed_raw_input['title'],'=');
        }
      }
    }
  }
}
