<?php

/**
 * @file
 *  Drush commands to generate votingapi votes, recalculate results
 *  for existing votes, or flush VotingAPI data entirely.
 */

/**
 * Implements of hook_drush_help().
 */
function votingapi_drush_help($section) {
  switch ($section) {
    case 'drush:generate-votes':
      return dt('Usage: drush generate-votes <entity_type> <vote_type>.');
    case 'drush:votingapi-recalculate':
      return dt('Usage: drush votingapi-recalculate <entity_type>.');
    case 'drush:votingapi-flush':
      return dt('Usage: drush votingapi-flush <entity_type>.');
  }
}

/**
 * Implements of hook_drush_command().
 */
function votingapi_drush_command() {
  $items['generate-votes'] = array(
    'description' => 'Creates dummy voting data.',
    'arguments' => array(
      'entity_type' => 'The type of entity to generate votes for.',
      'vote_type' => 'The type of votes to generate, defaults to \'percent\'.',
    ),
    'drupal dependencies' => array('devel_generate'),
    'options' => array(
      'kill' => 'Specify \'kill\' to delete all existing votes before generating new ones.',
      'age' => 'The maximum age, in seconds, of each vote.',
      'node_types' => 'A comma delimited list of node types to generate votes for, if the entity type is \'node\'.',
    ),
    'aliases' => array('genv'),
  );
  $items['votingapi-recalculate'] = array(
    'description' => 'Regenerates voting results from raw vote data.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // Various D7 code assumes we have a uid.
    'arguments' => array(
      'entity_type' => 'The type of entity to recalculate vote results for.',
    ),
    'aliases' => array('vcalc'),
  );
  $items['votingapi-flush'] = array(
    'description' => 'Deletes all existing voting data.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // Various D7 code assumes we have a uid.
    'arguments' => array(
      'entity_type' => 'The type of entity whose voting data should be flushed.',
    ),
    'aliases' => array('vflush'),
  );
  return $items;
}

/**
 * Command callback. Generate a number of votes.
 */
function drush_votingapi_generate_votes($entity_type = 'node', $vote_type = 'percent') {
  $options = array(
    'kill' => drush_get_option('kill'),
    'age' => drush_get_option('age'),
    'types' => drush_get_option('types'),
  );

  require_once(drupal_get_path('module', 'votingapi') . '/votingapi.devel.inc');
  votingapi_generate_votes($entity_type, $vote_type, $options);

  drush_log(t('Generating @vtype votes for @etype entities.', array('@vtype' => $vote_type, '@etype' => $entity_type)), 'success');
}

/**
 * Command callback. Recalculate voting results.
 */
function drush_votingapi_recalculate($entity_type = 'node', $entity_id = NULL) {
  // Prep some starter query objects.
  $data = array();
  if (empty($entity_id)) {
    $votes = db_select('votingapi_vote', 'vv')
      ->fields('vv', array('entity_type', 'entity_id'))
      ->condition('entity_type', $entity_type, '=')->distinct(TRUE)
      ->execute()->fetchAll(PDO::FETCH_ASSOC);
      $message = t('Rebuilt voting results for @type votes.', array('@type' => $entity_type));
  }
  else {
    $votes[] = array('entity_type' => $entity_type, 'entity_id' => $entity_id);
    $message = t('Rebuilt all voting results.');
  }

  foreach ($votes as $vote) {
    votingapi_recalculate_results($vote['entity_type'], $vote['entity_id'], TRUE);
  }

  drush_log($message, 'success');
}

/**
 * Command callback. Flush votes and results.
 */
function drush_votingapi_flush($entity_type = NULL, $entity_id = NULL) {
  if (drush_confirm(dt("Delete @type voting data?", array('@type' => empty($entity_type) ? dt('all') : $entity_type)))) {
    $cache = db_delete('votingapi_cache');
    $votes = db_delete('votingapi_vote');

    if (!empty($entity_type)) {
      $cache->condition('entity_type', $entity_type);
      $votes->condition('entity_type', $entity_type);
    }
    if (!empty($entity_id)) {
      $cache->condition('entity_id', $entity_id);
      $votes->condition('entity_id', $entity_id);
    }

    $cache->execute();
    $votes->execute();

    drush_log(t('Flushed vote data for @type entities.', array('@type' => empty($entity_type) ? t('all') : $entity_type)), 'success');
  }
}
