<?php

/**
 * @file
 * Holds functions relating to the Mark Forum/All Read functionality.
 */

/**
 * Either fill a $links array or return a string version of the link to mark read.
 */
function advanced_forum_get_mark_read_link($tid = 0, &$links = array()) {
  if (advanced_forum_markasread_access() && !in_array($tid, variable_get('forum_containers', array()))) {
    if ($tid) {
      $links['mark-read']['title'] = t('Mark all topics read');
      $links['mark-read']['href'] = "forum/markasread/$tid";

      return l(t('Mark all topics read') . '<span class="image-replace"></span>', "forum/markasread/$tid", array('html' => TRUE));
    }
    else {
      $links['mark-read']['title'] = t('Mark all forums read');
      $links['mark-read']['href'] = "forum/markasread";

      return l(t('Mark all forums read') . '<span class="image-replace"></span>', "forum/markasread", array('html' => TRUE));
    }
  }
}

/**
 * Marks all posts in forums or in a given forum as read by the current user.
 */
function advanced_forum_markasread($tid = NULL) {
  global $user;
  $vid = variable_get('forum_nav_vocabulary', 0);

  // Don't bother trying to mark things read for anonymous users.
  if (empty($user->uid)) {
    return;
  }
  // Delete all entries in the history table for the current $uid and
  // optionally a forum term id.

  // Subquery to find nids to delete from history table.
  $query_node = db_select('node', 'n');
  $query_node->join('forum', 'f', 'n.vid = f.vid');
  $query_node->join('taxonomy_term_data', 't', 'f.tid = t.tid');
  $query_node->join('node_comment_statistics', 'ncs', 'n.nid = ncs.nid');

  if (isset($tid)) {
    $query_node->condition('f.tid', $tid);
  }

  $query_node->condition('t.vid', $vid);
  $query_node->addField('n', 'nid');
  // Select query objects are one-shot, so clone for INSERT below.
  $query_history_insert = clone($query_node);
  // Delete values based upon sub-query.
  $query = db_delete('history')
    ->condition('uid', $user->uid)
    ->condition('nid', $query_node, 'IN')
    ->execute();

  // Now insert the nids into the history table.
  $query_history_insert->addExpression(':uid', 'uid', array(':uid' => $user->uid));
  $query_history_insert->addExpression(':time', 'timestamp', array(':time' => REQUEST_TIME));

  db_insert('history')
    ->fields(array('nid', 'uid', 'timestamp'))
    ->from($query_history_insert)
    ->execute();

  drupal_goto(empty($tid) ? 'forum' : 'forum/' . $tid);
}

/**
 * Access callback for menus and link display.
 *
 * @TODO: D7 check if needed
 *
 * This separate function is needed because the Drupal 6 menu system doesn't
 * run hook_menu() every time and the logged-in status of the user can get
 * cached and re-used for other users.
 */
function advanced_forum_markasread_access() {
  global $user;
  return user_access('access content') && $user->uid;
}
