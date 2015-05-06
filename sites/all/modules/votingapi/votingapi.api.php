<?php

/**
 * @file
 * Provides hook documentation for the VotingAPI module.
 */


/**
 * Adds to or changes the calculated vote results for a piece of content.
 *
 * VotingAPI calculates a number of common aggregate functions automatically,
 * including the average vote and total number of votes cast. Results are grouped
 * by 'tag', 'value_type', and then 'function' in the following format:
 *
 *   $results[$tag][$value_type][$aggregate_function] = $value;
 *
 * If no custom tag is being used for votes, the catch-all "vote" tag should be
 * used. In cases where custom tags are used to vote on different aspects of a
 * piece of content, a catch-all "vote" value should still be calculated for use
 * on summary screens, etc.
 *
 * @param $vote_results
 *   An alterable array of aggregate vote results.
 * @param $content_type
 *   A string identifying the type of content being rated. Node, comment,
 *   aggregator item, etc.
 * @param $content_id
 *   The key ID of the content being rated.
 *
 * @see votingapi_recalculate_results()
 */
function hook_votingapi_results_alter(&$vote_results, $content_type, $content_id) {
  // We're using a MySQLism (STDDEV isn't ANSI SQL), but it's OK because this is
  // an example. And no one would ever base real code on sample code. Ever. Never.

  $sql  = "SELECT v.tag, STDDEV(v.value) as standard_deviation ";
  $sql .= "FROM {votingapi_vote} v ";
  $sql .= "WHERE v.content_type = '%s' AND v.content_id = %d AND v.value_type = 'percent' ";
  $sql .= "GROUP BY v.tag";

  $aggregates = db_query($sql, $content_type, $content_id);

  // VotingAPI wants the data in the following format:
  // $vote_results[$tag][$value_type][$aggregate_function] = $value;

  while ($aggregate = db_fetch_array($aggregates)) {
    $vote_results[$result['tag']]['percent']['standard_deviation'] = $result['standard_deviation'];
  }
}


/**
 * Adds to or alters metadata describing Voting tags, value_types, and functions.
 *
 * If your module uses custom tags or value_types, or calculates custom aggregate
 * functions, please implement this hook so other modules can properly interperet
 * and display your data.
 *
 * Three major bins of data are stored: tags, value_types, and aggregate result
 * functions. Each entry in these bins is keyed by the value stored in the actual
 * VotingAPI tables, and contains an array with (minimally) 'name' and
 * 'description' keys. Modules can add extra keys to their entries if desired.
 *
 * @param $data
 *   An alterable array of aggregate vote results.
 *
 * @see votingapi_metadata()
 */
function hook_votingapi_metadata_alter(&$data) {
  // Document several custom tags for rating restaurants and meals.
  $data['tags']['bread'] = array(
    'name' => t('Bread'),
    'description' => t('The quality of the food at a restaurant.'),
    'module' => 'mymodule', // This is optional; we can add it for our own purposes.
  );
  $data['tags']['circuses'] = array(
    'name' => t('Circuses'),
    'description' => t('The quality of the presentation and atmosphere at a restaurant.'),
    'module' => 'mymodule',
  );

  // Document two custom aggregate function.
  $data['functions']['standard_deviation'] = array(
    'name' => t('Standard deviation'),
    'description' => t('The standard deviation of all votes cast on a given piece of content. Use this to find controversial content.'),
    'module' => 'mymodule',
  );
  $data['functions']['median'] = array(
    'name' => t('Median vote'),
    'description' => t('The median vote value cast on a given piece of content. More accurate than a pure average when there are a few outlying votes.'),
    'module' => 'mymodule',
  );
}

/**
 * Returns callback functions and descriptions to format a VotingAPI Views field.
 *
 * Loads all votes for a given piece of content, then calculates and caches the
 * aggregate vote results. This is only intended for modules that have assumed
 * responsibility for the full voting cycle: the votingapi_set_vote() function
 * recalculates automatically.
 *
 * @param $field
 *   A Views field object. This can be used to expose formatters only for tags,
 *   vote values, aggregate functions, etc.
 * @return
 *   An array of key-value pairs, in which each key is a callback function and
 *   each value is a human-readable description of the formatter.
 *
 * @see votingapi_set_votes()
 */
function hook_votingapi_views_formatters($field) {
  if ($field->field == 'value') {
    return array('mymodule_funky_formatter' => t('MyModule value formatter'));
  }
  if ($field->field == 'tag') {
    return array('mymodule_funky_tags' => t('MyModule tag formatter'));
  }
}

/**
 * Save a vote in the database.
 *
 * @param $vote
 *   See votingapi_add_votes() for the structure of this array, with the
 *   defaults loaded from votingapi_prep_vote().
 */
function hook_votingapi_storage_add_vote(&$vote) {
  _mongodb_votingapi_prepare_vote($criteria);
  mongodb_collection('votingapi_vote')->insert($vote);
}

/**
 * Delete votes from the database.
 *
 * @param $votes
 *   An array of votes to delete. Minimally, each vote must have the 'vote_id'
 *   key set.
 * @param $vids
 *   A list of the 'vote_id' values from $voes.
 */
function hook_votingapi_storage_delete_votes($votes, $vids) {
  mongodb_collection('votingapi_vote')->delete(array('vote_id' => array('$in' => array_map('intval', $vids))));
}

/**
 * Select invidual votes from the database
 *
 * @param $criteria
 *   A keyed array used to build the select query. Keys can contain
 *   a single value or an array of values to be matched.
 *   $criteria['vote_id']       (If this is set, all other keys are skipped)
 *   $criteria['entity_id']
 *   $criteria['entity_type']
 *   $criteria['value_type']
 *   $criteria['tag']
 *   $criteria['uid']
 *   $criteria['vote_source']
 *   $criteria['timestamp']   If this is set, records with timestamps
 *      GREATER THAN the set value will be selected. Defaults to
 *      REQUEST_TIME - variable_get('votingapi_anonymous_window', 3600); if
 *      the anonymous window is above zero.
 * @param $limit
 *   An integer specifying the maximum number of votes to return. 0 means
 *   unlimited and is the default.
 * @return
 *   An array of votes matching the criteria.
 */
function hook_votingapi_storage_select_votes($criteria, $limit) {
  _mongodb_votingapi_prepare_vote($criteria);
  $find = array();
  foreach ($criteria as $key => $value) {
    $find[$key] = is_array($value) ? array('$in' => $value) : $value;
  }
  $cursor = mongodb_collection('votingapi_vote')->find($find);
  if (!empty($limit)) {
    $cursor->limit($limit);
  }
  $votes = array();
  foreach ($cursor as $vote) {
    $votes[] = $vote;
  }
  return $votes;
}

/**
 * Allows to act on votes before being inserted.
 *
 * @param $votes
 *  An array of votes, each with the following structure:
 *  $vote['entity_type']  (Optional, defaults to 'node')
 *  $vote['entity_id']    (Required)
 *  $vote['value_type']    (Optional, defaults to 'percent')
 *  $vote['value']         (Required)
 *  $vote['tag']           (Optional, defaults to 'vote')
 *  $vote['uid']           (Optional, defaults to current user)
 *  $vote['vote_source']   (Optional, defaults to current IP)
 *  $vote['timestamp']     (Optional, defaults to REQUEST_TIME)
 */
function hook_votingapi_preset_votes(&$votes) {
  foreach ($votes as $vote) {
    if ($vote['tag'] == 'recommend') {
      // Do something if the 'recommend' vote is being inserted.
    }
  }
}

/**
 * TODO
 *
 */
function hook_votingapi_storage_standard_results($entity_id, $entity) {
  // TODO
}
