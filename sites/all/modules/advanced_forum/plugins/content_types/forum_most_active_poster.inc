<?php
/**
 * @file
 * Most active poster.
 */

/**
 * Callback function to supply a list of content types.
 */
function advanced_forum_forum_most_active_poster_ctools_content_types() {
  if (module_exists('nodecomment')) {
    return array(
      'single' => TRUE,
      'title' => t('Most active forum poster'),
      'icon' => 'icon_forum.png',
      'description' => t(
        'A block with information about the most active forum poster.'
      ),
      'required context' => new ctools_context_required(t('Forum'), 'forum'),
      'category' => t('Forum'),
      'defaults' => array('period' => 'ever', 'exclude' => 0, 'uidone' => TRUE),
    );
  }
}

/**
 * Render the content.
 */
function advanced_forum_forum_most_active_poster_content_type_render(
  $subtype,
  $conf,
  $panel_args,
  $context
) {
  if (!empty($context) && empty($context->data)) {
    return;
  }

  $tid = 0;
  if (!empty($context)) {
    $tid = $context->data->tid;
  }

  $period = empty($conf['period']) ? 'ever' : $conf['period'];

  switch ($period) {
    case 'day':
      $period_arg = 60 * 60 * 24;
      break;

    case 'week':
      $period_arg = 60 * 60 * 24 * 7;
      break;

    case 'month':
      $period_arg = 60 * 60 * 24 * 30;
      break;

    case 'quarter':
      $period_arg = 60 * 60 * 24 * 91;
      break;

    case 'year':
      $period_arg = 60 * 60 * 24 * 365;
      break;
  }

  $block = new stdClass();
  $block->module = 'forum-active-poster';
  $block->delta = $tid;
  _advanced_forum_add_files();

  $args = array();
  $query = "SELECT n.uid, COUNT(DISTINCT n.nid) AS posts FROM {node} n ";
  $query .= "LEFT JOIN {node_comments} nc ON n.nid = nc.cid ";
  $query .= "INNER JOIN {node} n2 ON COALESCE(nc.nid, n.nid) = n2.nid ";
  $query .= "INNER JOIN {forum} f ON f.vid = n2.vid ";

  // Add a join to exclude our roles if needed.
  if (!empty($conf['exclude'])) {
    $query .= "LEFT JOIN {users_roles} ur ON n.uid = ur.uid AND ur.rid = %d ";
    $args[] = $conf['exclude'];
  }

  $query .= "WHERE n2.status <> 0 AND n.uid <> 0 ";

  if ($tid) {
    $forum = taxonomy_term_load($tid);
    $block->title = t(
      'Most active poster in @forum',
      array('@forum' => $forum->name)
    );
    $query .= "AND f.tid = %d ";
    $args = array($tid);
  }
  else {
    $block->title = t('Most active poster');
  }

  // Add a WHERE to make sure our excluded role is not present.
  if (!empty($conf['exclude'])) {
    $query .= "AND ur.rid IS NULL ";
  }

  // Exclude the superuser if configged to do so.
  if (!empty($conf['uidone'])) {
    $query .= "AND n.uid <> 1 ";
  }

  if (!empty($period_arg)) {
    $query .= "AND n2.created >= %d ";
    $args[] = REQUEST_TIME - $period_arg;
  }

  $query .= "GROUP BY n.uid ";
  $query .= "ORDER BY COUNT(DISTINCT n.nid) DESC LIMIT 1";

  $info = db_fetch_object(db_query($query, $args));

  // This can happen if there are no posts.
  if (!$info) {
    return;
  }

  $account = user_load($info->uid);

  // Now that we have a user and that user's # of posts, get # of topics for
  // this forum.
  $args = array($info->uid);
  $query = "SELECT COUNT(n.nid) AS posts FROM {node} n ";
  $query .= "INNER JOIN {forum} f ON f.vid = n.vid ";
  $query .= "WHERE n.status <> 0 AND n.uid = %d";
  if ($tid) {
    $query .= " AND f.tid = %d";
    $args[] = $tid;
  }

  if (!empty($period_arg)) {
    $query .= " AND n.created >= %d";
    $args[] = REQUEST_TIME - $period_arg;
  }
  $topics = db_result(db_query($query, $args));

  // And get their last post.
  $args = array($info->uid);
  $query = "SELECT n.nid FROM {node} n ";
  $query .= "LEFT JOIN {node_comments} nc ON n.nid = nc.cid ";
  $query .= "INNER JOIN {node} n2 ON COALESCE(nc.nid, n.nid) = n2.nid ";
  $query .= "INNER JOIN {forum} f ON f.vid = n2.vid ";
  $query .= "WHERE n.status <> 0 AND n.uid = %d ";
  if ($tid) {
    $query .= " AND f.tid = %d ";
    $args[] = $tid;
  }

  $query .= "ORDER BY n.created DESC LIMIT 1";

  $last_post = node_load(db_result(db_query($query, $args)));
  $block->content = theme(
    'advanced_forum_active_poster',
    array(
      'forum' => $tid,
      'account' => $account,
      'posts' => $info->posts,
      'topics' => $topics,
      'last_post' => $last_post,
    )
  );
  $block->more = array(
    'title' => t('More posts'),
    'href' => 'forum/user/' . $info->uid,
  );

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_most_active_poster_content_type_edit_form(
  $form,
  &$form_state
) {
  // Make sure conf contains newer defaults. CTools should do this for us.
  $conf = $form_state['conf'];
  $conf += $form_state['plugin']['defaults'];

  $form['period'] = array(
    '#type' => 'select',
    '#title' => t('Period'),
    '#options' => array(
      'day' => t('In the last day'),
      'week' => t('In the last week'),
      'month' => t('In the last month'),
      'quarter' => t('In the last quarter'),
      'year' => t('In the last year'),
      'ever' => t('Ever'),
    ),
    '#description' => t(
      'Select what period you want to search for the most active poster.'
    ),
    '#default_value' => $conf['period'],
  );

  $roles = array('0' => t('Allow all roles')) + user_roles(TRUE);
  unset($roles[DRUPAL_AUTHENTICATED_RID]);

  $form['exclude'] = array(
    '#type' => 'select',
    '#title' => t('Exclude role'),
    '#options' => $roles,
    '#description' => t(
      'Choose a role that will be excluded from being an active poster.'
    ),
    '#default_value' => $conf['exclude'],
  );

  $form['uidone'] = array(
    '#type' => 'checkbox',
    '#title' => t('Exclude UID 1'),
    '#description' => t(
      'If checked the super user will not be a possible most active poster.'
    ),
    '#default_value' => $conf['uidone'],
  );

  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_most_active_poster_content_type_edit_form_submit(
  $form,
  &$form_state
) {
  // Copy everything from our defaults.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Callback for admin title.
 */
function advanced_forum_forum_most_active_poster_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" most active poster', array('@s' => $context->identifier));
}
