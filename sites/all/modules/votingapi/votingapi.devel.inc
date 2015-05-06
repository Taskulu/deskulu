<?php

/**
 * @file
 * Generate fake votingapi votes for development testing.
 */

/**
 * Utility function to generate votes.
 */
function votingapi_generate_votes($entity_type = 'node', $vote_type = 'percent', $options = array()) {
  module_load_include('inc', 'devel_generate');
  $options += array(
    'age' => 36000,
    'types' => array(),
    'kill' => FALSE,
  );

  if (!empty($options['kill_votes'])) {
    db_truncate('votingapi_vote');
    db_truncate('votingapi_cache');
  }

  if (($schema = drupal_get_schema($entity_type)) && ($entity_id_column = array_shift($schema['primary key']))) {
    // oh look we found a schema yay
  }
  else {
    $entity_type = 'node';
    $entity_id_column = 'nid';
  }
  $uids = devel_get_users();

  $query = db_select($entity_type, 'e')->fields('e', array($entity_id_column));
  if ($entity_type == 'node' && !empty($options['types'])) {
    $query->condition('e.type', $options['types'], 'IN');
  }
  $results = $query->execute()->fetchAll(PDO::FETCH_ASSOC);

  foreach ($results as $entity) {
    _votingapi_cast_votes($entity_type, $entity[$entity_id_column], $options['age'], $uids, $vote_type);
  }
}

/**
 * Utility function to generate votes on a node by a set of users.
 */
function _votingapi_cast_votes($etype, $eid, $timestamp = 0, $uids = array(), $style = 'percent') {
  $votes = array();
  static $tags;
  if (!isset($tags)) {
    $tags = explode(' ', devel_create_greeking(30));
  }
  foreach ($uids as $uid) {
    switch ($style) {
      case 'percent':
        $votes[] = array(
          'uid' => $uid,
          'entity_type' => $etype,
          'entity_id' => $eid,
          'value_type' => 'percent',
          'timestamp' => REQUEST_TIME - mt_rand(0, $timestamp),
          'value' => mt_rand(1, 5) * 20,
          'tag' => $tags[mt_rand(0, 20)],
        );
        break;
      case 'points':
        if (rand(0, 3)) {
          $votes[] = array(
            'uid' => $uid,
            'entity_type' => $etype,
            'entity_id' => $eid,
            'value_type' => 'points',
            'timestamp' => REQUEST_TIME - mt_rand(0, $timestamp),
            'value' => rand(0, 1) ? 1 : -1,
          );
        }
        break;
    }
  }
  votingapi_set_votes($votes, array());
}

