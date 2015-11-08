<?php

/**
 * @file
 * User page callbacks for the forum module.
 */

/**
 * Menu callback; prints a forum listing.
 */
function advanced_forum_page($forum_term = NULL) {
  if (!isset($forum_term)) {
    // On the main page, display all the top-level forums.
    $forum_term = advanced_forum_forum_load(0);
  }

  // Set tid for <root> container.
  if (!isset($forum_term->tid)) {
    $forum_term->tid = 0;
  }
  _advanced_forum_add_files();

  $forum_per_page = variable_get('forum_per_page', 25);
  $sortby = variable_get('forum_order', 1);

  if (empty($forum_term->container)) {
    $topics = advanced_forum_get_topics($forum_term->tid, $sortby, $forum_per_page);
  }
  else {
    $topics = '';
  }

  $vid = variable_get('forum_nav_vocabulary', 0);
  $vocabulary = taxonomy_vocabulary_load($vid);

  // Breadcrumb navigation:
  $breadcrumb[] = l(t('Home'), NULL);
  if ($forum_term->tid) {
    $breadcrumb[] = l($vocabulary->name, 'forum');
  }

  if ($forum_term->parents) {
    foreach (array_reverse($forum_term->parents) as $p) {
      if ($p->tid != $forum_term->tid) {
        $breadcrumb[] = l($p->name, 'forum/' . $p->tid);
      }
      else {
        $title = $p->name;
      }
    }
  }

  if (empty($title)) {
    $title = $vocabulary->name;
  }

  if (!variable_get('advanced_forum_disable_breadcrumbs', FALSE)) {
    drupal_set_breadcrumb($breadcrumb);
  }

  drupal_set_title($title);

  return theme('forums', array(
    'forums' => $forum_term->forums,
    'topics' => $topics,
    'parents' => $forum_term->parents,
    'tid' => $forum_term->tid,
    'sortby' => $sortby,
    'forums_per_page' => $forum_per_page,
  ));
}

/**
 * Returns a tree of all forums for a given taxonomy term ID.
 *
 * This is copied from the forum module and adapted.
 *
 * @param int|null $tid
 *   (optional) Taxonomy ID of the forum, if not givin all forums will be returned.
 *
 * @return object
 *   A tree of taxonomy objects, with the following additional properties:
 *    - 'num_topics': Number of topics in the forum
 *    - 'num_posts': Total number of posts in all topics
 *    - 'last_post': Most recent post for the forum
 *    - 'forums': An array of child forums
 */
function advanced_forum_forum_load($tid = NULL) {
  $cache = &drupal_static(__FUNCTION__, array());

  global $language;

  // Return a cached forum tree if available.
  if (empty($tid)) {
    $tid = 0;
  }
  if (isset($cache[$tid])) {
    return $cache[$tid];
  }

  // Find out from the style's .info how many posts per forum to collect.
  $info = advanced_forum_style_info();
  $post_count = isset($info['forum list post count']) ? intval($info['forum list post count']) : 1;

  $vid = variable_get('forum_nav_vocabulary', 0);

  // Load and validate the parent term.
  if ($tid) {
    $forum_term = taxonomy_term_load($tid);

    if (module_exists('i18n_taxonomy')) {
      $forum_term = i18n_taxonomy_localize_terms($forum_term);
    }
    if (!$forum_term || ($forum_term->vid != $vid)) {
      return $cache[$tid] = FALSE;
    }
  }
  // If $tid is 0, create an empty object to hold the child terms.
  elseif ($tid === 0) {
    $forum_term = (object) array(
      'tid' => 0,
    );
  }

  // Determine if the requested term is a container.
  if (!$forum_term->tid || in_array($forum_term->tid, variable_get('forum_containers', array()))) {
    $forum_term->container = 1;
  }

  // Load parent terms.
  $forum_term->parents = taxonomy_get_parents_all($forum_term->tid);

  // Load the tree below.
  $forums = array();
  $_forums = taxonomy_get_tree($vid, $tid);

  if (module_exists('i18n_taxonomy')) {
    $_forums = i18n_taxonomy_localize_terms($_forums);
  }

  if ($cached = cache_get('adv_forum_counts' . $tid . ':' . $language->language, 'cache')) {
    if ($cached->expire > time()) {
      $counts = $cached->data;
    }
  }
  if (count($_forums) && empty($counts)) {
    $query = db_select('node', 'n');
    $query->join('node_comment_statistics', 'ncs', 'n.nid = ncs.nid');
    $query->join('forum_index', 'f', 'n.nid = f.nid');
    $query->addExpression('COUNT(DISTINCT(f.nid))', 'topic_count');
    $query->addExpression('SUM(f.comment_count)', 'comment_count');
    $counts = $query
      ->fields('f', array('tid'))
      ->condition('status', 1)
      ->groupBy('tid')
      ->addTag('node_access')
      ->execute()
      ->fetchAllAssoc('tid');

    // Limit the query to only node types that can actually be associated with
    // the forum vocabulary.
    if ($vid) {
      $vocabulary = taxonomy_vocabulary_load($vid);
      $vocabulary_fields = field_read_fields(array('module' => 'taxonomy'));
      $node_types = array();
      foreach ($vocabulary_fields as $field_name => $vocabulary_field) {
        foreach ($vocabulary_field['settings']['allowed_values'] as $key => $allowed_value) {
          if ($allowed_value['vocabulary'] == $vocabulary->machine_name) {
            $field_info = field_info_field($field_name);
            if (isset($field_info['bundles']['node'])) {
              $node_types += $field_info['bundles']['node'];
            }
          }
        }
      }

      if (!empty($node_types)) {
        $query->condition('type', $node_types, 'IN');
      }
    }


    cache_set('adv_forum_counts' . $tid, $counts, 'cache', time() + 60*60);
  }

  // Get last nodes with comments for subquery SELECT optimization.
  $subquery = db_select('node_comment_statistics', 'lnsc')
    ->fields('lnsc', array('nid'))
    ->distinct()
    ->orderBy('last_comment_timestamp', 'DESC')
    ->range(0, variable_get('advanced_forum_last_post_query', 10000));
  // Should be moved to options or additionally calculated elsewhere.
  foreach ($_forums as $forum) {
    // Determine if the child term is a container.
    if (in_array($forum->tid, variable_get('forum_containers', array()))) {
      $forum->container = 1;
    }

    // Merge in the topic and post counters.
    if (!empty($counts[$forum->tid])) {
      $forum->num_topics = $counts[$forum->tid]->topic_count;
      $forum->num_posts = $counts[$forum->tid]->topic_count + $counts[$forum->tid]->comment_count;
    }
    else {
      $forum->num_topics = 0;
      $forum->num_posts = 0;
    }

    // Query "Last Post" information for this forum.
    $query = db_select('node', 'n');
    $query->join($subquery, 'lastnodes', 'n.nid = lastnodes.nid');
    $query->join('forum_index', 'f', 'n.nid = f.nid AND f.tid = :tid', array(':tid' => $forum->tid));
    $query->join('node_comment_statistics', 'ncs', 'n.nid = ncs.nid');
    $query->join('users', 'u', 'ncs.last_comment_uid = u.uid');
    $query->addExpression('CASE ncs.last_comment_uid WHEN 0 THEN ncs.last_comment_name ELSE u.name END', 'last_comment_name');

    $topics = $query
        ->fields('ncs', array('last_comment_timestamp', 'last_comment_uid'))
        ->fields('n', array('nid', 'title', 'type'))
        ->condition('n.status', 1)
        ->orderBy('last_comment_timestamp', 'DESC')
        ->range(0, $post_count)
        ->addTag('node_access')
        ->execute();

    while ($topic = $topics->fetchObject()) {
      // Merge in the "Last Post" information.
      $last_post = new stdClass();
      if (!empty($topic->last_comment_timestamp)) {
        $last_post->nid = $topic->nid;
        $last_post->node_title = $topic->title;
        $last_post->type = $topic->type;
        $last_post->created = $topic->last_comment_timestamp;
        $last_post->name = $topic->last_comment_name;
        $last_post->uid = $topic->last_comment_uid;
      }
      if ($post_count > 1) {
        $forum->last_post[] = $last_post;
      }
      else {
        $forum->last_post = $last_post;
      }
    }

    $forums[$forum->tid] = $forum;
  }

  // Cache the result, and return the tree.
  $forum_term->forums = $forums;
  $cache[$tid] = $forum_term;
  return $forum_term;
}

/**
 * Update parent post count.
 */
function _advanced_forum_update_parent_post_count(&$forums, $forum) {
  foreach ($forum->parents as $parent_tid) {
    if (!empty($forums[$parent_tid])) {
      $forums[$parent_tid]->num_topics += $forum->num_topics;
      $forums[$parent_tid]->num_posts += $forum->num_posts;

      // Recursive loop to update all parents/
      if (!empty($forums[$parent_tid]->parents)) {
        _advanced_forum_update_parent_post_count($forums, $forums[$parent_tid]);
      }
    }
  }
}

/**
 * This is copied from the forum module and adapted.
 */
function advanced_forum_get_topics($tid, $sortby, $forum_per_page, $sort_form = TRUE) {
  $term = taxonomy_term_load($tid);
  drupal_add_feed('taxonomy/term/' . $tid . '/feed', 'RSS - ' . check_plain($term->name));

  // Views handles this page.
  $view = views_get_view('advanced_forum_topic_list');
  $view->set_items_per_page($forum_per_page);
  $view->sort_form = $sort_form;

  return $view->preview('default', array($tid));
}
