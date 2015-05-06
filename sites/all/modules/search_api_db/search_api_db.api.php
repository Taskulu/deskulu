<?php

/**
 * @file
 * Hooks provided by the Database Search module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Preprocesses a search's database query before it is executed.
 *
 * @param SelectQueryInterface $db_query
 *   The database query to be executed for the search. Will have "item_id" and
 *   "score" columns in its result.
 * @param SearchApiQueryInterface $query
 *   The search query that is being executed.
 *
 * @see SearchApiDbService::preQuery()
 */
function hook_search_api_db_query_alter(SelectQueryInterface &$db_query, SearchApiQueryInterface $query) {
  // If the option was set on the query, add additional SQL conditions.
  if ($custom = $query->getOption('custom_sql_conditions')) {
    foreach ($custom as $condition) {
      $db_query->condition($condition['field'], $condition['value'], $condition['operator']);
    }
  }
}

/**
 * @} End of "addtogroup hooks".
 */
