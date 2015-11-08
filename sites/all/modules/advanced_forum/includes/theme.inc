<?php

/**
 * @file
 * Holds theme functions and template preprocesses.
 * Other style related functions are in style.inc
 */

/**
 * Theme function to display a link, optionally buttonized.
 */
function theme_advanced_forum_l(&$variables) {
  $text = $variables['text'];
  $path = empty($variables['path']) ? NULL : $variables['path'];
  $options = empty($variables['options']) ? array() : $variables['options'];
  $button_class = empty($variables['button_class']) ? NULL : $variables['button_class'];

  $l = '';
  if (!isset($options['attributes'])) {
    $options['attributes'] = array();
  }
  if (!is_null($button_class)) {
    // Buttonized link: add our button class and the span.
    if (!isset($options['attributes']['class'])) {
      $options['attributes']['class'] = array("af-button-$button_class");
    }
    else {
      $options['attributes']['class'][] = "af-button-$button_class";
    }
    $options['html'] = TRUE;
    // @codingStandardsIgnoreStart
    $l = l('<span>' . $text . '</span>', $path, $options);
    // @codingStandardsIgnoreEnd
  }
  else {
    // Standard link: just send it through l().
    $l = l($text, $path, $options);
  }

  return $l;
}

/**
 * Theme function to show list of types that can be posted in forum.
 */
function theme_advanced_forum_node_type_create_list(&$variables) {
  $forum_id = $variables['forum_id'];

  // Get the list of node types to display links for.
  $type_list = advanced_forum_node_type_create_list($forum_id);

  $output = '';
  if (is_array($type_list)) {
    foreach ($type_list as $type => $item) {
      $output .= '<div class="forum-add-node forum-add-' . $type . '">';
      $output .= theme('advanced_forum_l', array(
        'text' => t('New @node_type', array('@node_type' => $item['name'])),
        'path' => $item['href'],
        'options' => NULL,
        'button_class' => 'large',
        ));
      $output .= '</div>';
    }
  }
  else {
    // User did not have access to create any node types in this fourm so
    // we just return the denial text / login prompt.
    $output = $type_list;
  }

  return $output;
}

/**
 * Theme function to show simple author pane when not using Author Pane.
 */
function theme_advanced_forum_simple_author_pane(&$variables) {
  $context = $variables['context'];
  // Sending the context rather than the account makes it work for anon comments.
  $name = theme('username', array('account' => $context));

  $account = user_load($context->uid);
  $picture = theme('user_picture', array('account' => $account));

  return '<div class="author-pane">' . $name . $picture . '</div>';
}

/**
 * Theme function to format the reply link at the top/bottom of topic.
 */
function theme_advanced_forum_reply_link(&$variables) {
  $node = $variables['node'];

  // Get the information about whether the user can reply and the link to do
  // so if the user is allowed to.
  $reply_link = advanced_forum_get_reply_link($node);

  if (is_array($reply_link)) {
    // Reply is allowed. Variable contains the link information.
    $output = '<div class="topic-reply-allowed">';
    $output .= theme('advanced_forum_l', array(
      'text' => $reply_link['title'],
      'path' => $reply_link['href'],
      'options' => $reply_link['options'],
      'button_class' => 'large',
        ));
    $output .= '</div>';
    return $output;
  }
  elseif ($reply_link == 'reply-locked') {
    // @TODO: The double span here is icky but I don't know how else to get
    // around the fact that there's no "a" to put the button class on.
    return '<div class="topic-reply-locked"><span class="af-button-large"><span>' . t('Topic locked') . '</span></span></div>';
  }
  elseif ($reply_link == 'reply-forbidden') {
    // User is not allowed to reply to this topic.
    return theme('comment_post_forbidden', array('node' => $node));
  }
}

/**
 * Theme function to a formatted list of subforums.
 *
 * @param array $variables
 *   Array of subforums.
 *
 * @return string
 *   Formatted list of subforums.
 */
function theme_advanced_forum_subforum_list(&$variables) {
  $subforums = array();
  foreach ($variables['subforum_list'] as $tid => $subforum) {
    // Note: $subforum->name has not been run through check_plain because
    // it ends up going through there when l() is called without the HTML
    // option. If you change this to set HTML to TRUE, you must sanitize it.
    $text = l($subforum->name, "forum/$tid");
    $text .= ' (' . $subforum->total_posts;

    if (empty($subforum->new_posts)) {
      $text .= ')';
    }
    else {
      $text .= ' - ' . l($subforum->new_posts_text, $subforum->new_posts_path, array('fragment' => 'new')) . ')';
    }

    $subforums[] = $text;
  }
  return implode(', ', $subforums);
}

/**
 * Theme function to a formatted list of subcontainers.
 *
 * @param array $variables
 *   Array of subcontainers.
 *
 * @return string
 *   Formatted list of subcontainers.
 */
function theme_advanced_forum_subcontainer_list(&$variables) {
  $subcontainers = array();
  foreach ($variables['subcontainer_list'] as $tid => $subcontainer) {
    // Note: $subcontainer->name has not been run through check_plain because
    // it ends up going through there when l() is called without the HTML
    // option. If you change this to set HTML to TRUE, you must sanitize it.
    $text = l($subcontainer->name, "forum/$tid");
    $text .= ' (' . $subcontainer->total_posts;

    if (empty($subcontainer->new_posts)) {
      $text .= ')';
    }
    else {
      $text .= ' - ' . l($subforum->new_posts_text, $subforum->new_posts_path, array('fragment' => 'new')) . ')';
    }

    $subcontainers[] = $text;
  }

  return implode(', ', $subcontainers);
}

// TEMPLATE PREPROCESS ******************************************************/

/* * * FORUM OVERVIEW & TOPIC LIST PAGES ************************************* */

/**
 * Preprocesses template variables for the forum template.
 */
function advanced_forum_preprocess_forums(&$variables) {
  include_once drupal_get_path('module', 'advanced_forum') . '/includes/advanced_forum_preprocess_forums.inc';
  _advanced_forum_preprocess_forums($variables);
}

/**
 * Preprocesses template variables for the forum search form template.
 */
function advanced_forum_preprocess_advanced_forum_search_forum(&$variables) {
  advanced_forum_add_template_suggestions("search_forum", $variables);

  $variables['forum'] = !empty($variables['tid']) ? $variables['tid'] : 'All';
  $variables['path'] = url('forum/search');
}

/**
 * Preprocesses template variables for the search results template.
 */
function advanced_forum_preprocess_views_view_fields__advanced_forum_search(&$variables) {
  _advanced_forum_add_files();
  advanced_forum_add_template_suggestions("search_result", $variables);
}

/**
 * Preprocesses template variables for the submitted by/in template.
 */
function advanced_forum_preprocess_forum_submitted(&$variables) {
  advanced_forum_add_template_suggestions("submitted", $variables);
  // Avoid E_ALL warning.
  $variables['topic_link'] = '';

  if (isset($variables['topic']->node_title)) {
    $nid = $variables['topic']->nid;

    // Make a fake node object to avoid the node load.
    $node = new stdClass();
    $node->nid = $nid;
    $node->type = $variables['topic']->type;

    // Find the page of the first unread comment, if any.
    $comment_count = db_query('SELECT COUNT(cid) FROM {comment} WHERE nid = :nid', array(':nid' => $nid))->fetchField();
    $new_replies = advanced_forum_reply_num_new($nid);
    $query = advanced_forum_page_first_new($comment_count, $new_replies, $node);

    // Format the node title with a link.
    $title_length = variable_get('advanced_forum_topic_title_length', 15);
    if ($title_length == 0) {
      $short_topic_title = $variables['topic']->node_title;
    }
    else {
      $short_topic_title = truncate_utf8($variables['topic']->node_title, $title_length, TRUE, TRUE);
    }

    $variables['short_topic_title'] = $short_topic_title;

    $fragment = ($new_replies) ? 'new' : NULL;

    $variables['topic_link_fragment'] = $fragment;
    $variables['topic_link_query'] = $query;
    $variables['topic_link'] = l($short_topic_title, "node/$nid", array('query' => $query, 'fragment' => $fragment));
  }

  if (isset($variables['topic']->created)) {
    $timestamp = $variables['topic']->created;
    $interval = REQUEST_TIME - $timestamp;
    $variables['time'] = format_interval($interval);

    // For items posted more than $cutoff hours ago, offer an actual date.
    $cutoff = variable_get('advanced_forum_time_ago_cutoff', 48) * 60 * 60;
    if ($interval > $cutoff) {
      $variables['date_posted'] = format_date($timestamp, 'short');
    }
    else {
      unset($variables['date_posted']);
    }
  }
}

/**
 * Preprocess forum topic pager.
 */
function advanced_forum_preprocess_advanced_forum_topic_pager(&$variables) {
  $pagecount = $variables['pagecount'];
  $topic = $variables['topic'];

  // Get the information to assemble the pager. This returns an object with
  // the following properties:
  // - initial_pages: Array of linked numbers for first set of pages
  // - last_page_text: Linked text "Last page" (translatable)
  // - last_page_number: Linked number pointing to the last page.
  $topic_pager = advanced_forum_create_topic_pager($pagecount, $topic);

  $variables['last_page_text'] = '';

  if (!empty($topic_pager->initial_pages)) {
    $variables['pages'] = $topic_pager->initial_pages;
    if (!empty($topic_pager->last_page_text)) {
      $variables['last_page'] = $topic_pager->last_page;
      $variables['last_page_text'] = $topic_pager->last_page_text;
      $variables['last_page_number'] = $topic_pager->last_page_number;

      // If you prefer to end with the number, replace
      // $topic_pager->last_page_text with $topic_pager->last_page_number
      $variables['last_page_text'] = ' &hellip; ' . $topic_pager->last_page_text;
    }
  }
}

/**
 * Preprocess variables for advanced-forum.naked.post-edited.tpl.php.
 */
function advanced_forum_preprocess_advanced_forum_post_edited(&$variables) {
  $editor = user_load($variables['who']);

  $variables['edited_name'] = theme('username', array('account' => $editor));
  $variables['edited_datetime'] = format_date($variables['when'], 'custom', variable_get('date_format_short', 'm/d/Y - H:i'));
  $variables['edited_reason'] = empty($variables['why']) ? '' : $variables['why'];
}

/**
 * Preprocess forum shadow topic.
 */
function advanced_forum_preprocess_advanced_forum_shadow_topic(&$variables) {
  $nid = $variables['nid'];

  $variables['new_forum_url'] = url("node/$nid");
  // Compatibility variable.
  $variables['new_forum_link'] = l(t('View topic'), "node/$nid");
}

/* * * JUST FORUM OVERVIEW PAGE ********************************************** */

/**
 * Preprocesses template variables for the forum legend template.
 */
function advanced_forum_preprocess_advanced_forum_forum_legend(&$variables) {
  advanced_forum_add_template_suggestions("forum_legend", $variables);
}

/**
 * Preprocesses template variables for the forum statistics template.
 */
function advanced_forum_preprocess_advanced_forum_statistics(&$variables) {
  advanced_forum_add_template_suggestions("statistics", $variables);

  $variables['topics'] = advanced_forum_statistics_topics();
  $variables['posts'] = advanced_forum_statistics_replies() + $variables['topics'];
  $variables['users'] = advanced_forum_statistics_users();

  $authenticated_users = advanced_forum_statistics_online_users();
  $variables['online_users'] = implode(', ', $authenticated_users);
  $variables['current_users'] = advanced_forum_session_count(FALSE);
  // For backwards compatibility.
  $variables['current_guests'] = 0;
  $variables['current_total'] = $variables['current_users'] + $variables['current_guests'];

  $latest_users = advanced_forum_statistics_latest_users();
  $variables['latest_users'] = implode(', ', $latest_users);
}

/**
 * Preprocesses template variables for the forum list template.
 */
function advanced_forum_preprocess_forum_list(&$variables) {
  include_once drupal_get_path('module', 'advanced_forum') . '/includes/advanced_forum_preprocess_forum_list.inc';
  _advanced_forum_preprocess_forum_list($variables);
}

/* * * JUST TOPIC LIST PAGES ************************************************* */

/**
 * Preprocess views forum topic list.
 */
function advanced_forum_preprocess_views_view__advanced_forum_topic_list(&$variables) {
  _advanced_forum_add_files();
  advanced_forum_add_template_suggestions("topic_list_outer_view", $variables);

  $variables['node_create_list'] = '';
  $variables['forum_tools'] = '';
  $variables['forum_jump'] = '';

  $menu_item = menu_get_item();

  if (!empty($menu_item) && $menu_item['access']) {
    if (($menu_item['map'][0] == 'forum') && (!empty($menu_item['map'][1]))) {
      $forum = $menu_item['map'][1];
      if ($forum->vid == variable_get('forum_nav_vocabulary') && $forum->tid > 0) {
        $variables['node_create_list'] = theme('advanced_forum_node_type_create_list', array('forum_id' => $forum->tid));
        $variables['forum_description'] = $forum->description;
        $variables['forum_tools'] = advanced_forum_forum_tools($forum->tid);
        $variables['forum_jump'] = advanced_forum_forum_jump($forum->tid);
      }
    }
  }
}

/**
 * Display a view as a forum topic list style.
 */
function template_preprocess_advanced_forum_topic_list_view(&$variables) {
  include_once drupal_get_path('module', 'advanced_forum') . '/includes/template_preprocess_advanced_forum_topic_list_view.inc';
  _template_preprocess_advanced_forum_topic_list_view($variables);
}

/**
 * Preprocesses template variables for the topic list template.
 */
function advanced_forum_preprocess_forum_topic_list(&$variables) {
  advanced_forum_add_template_suggestions("topic_list", $variables);
}

/**
 * Preprocesses template variables for the forum icon template.
 */
function advanced_forum_preprocess_forum_icon(&$variables) {
  advanced_forum_add_template_suggestions("topic_icon", $variables);
}

/**
 * Preprocesses template variables for the topic legend template.
 */
function advanced_forum_preprocess_advanced_forum_topic_legend(&$variables) {
  advanced_forum_add_template_suggestions("topic_legend", $variables);
}

/* * * TOPIC PAGES *********************************************************** */
/**
 * Preprocess forum search topic.
 */
function advanced_forum_preprocess_advanced_forum_search_topic(&$variables) {
  advanced_forum_add_template_suggestions("search_topic", $variables);

  $variables['path'] = url('node/' . $variables['node']->nid . '/search');
}

/**
 * Preprocess views forum search topic.
 */
function advanced_forum_preprocess_views_view_fields__advanced_forum_search_topic(&$variables) {
  _advanced_forum_add_files();
  advanced_forum_add_template_suggestions("search_result", $variables);
}

/**
 * Preprocesses template variables for the topic header template.
 */
function advanced_forum_preprocess_advanced_forum_topic_header(&$variables) {
  advanced_forum_add_template_suggestions("topic_header", $variables);

  $node = $variables['node'];

  // Reply link/button.
  $variables['reply_link'] = theme('advanced_forum_reply_link', array('node' => $node));

  // Total posts, including first post.
  $posts = (empty($variables['comment_count'])) ? 1 : $variables['comment_count'] + 1;
  $variables['total_posts_count'] = format_plural($posts, '1 post', '@count posts');

  // Number of new posts on topic.
  $variables['new_posts_count'] = advanced_forum_reply_num_new($node->nid);

  // Link to first new post.
  $variables['first_new_post_link'] = '';
  if ($posts > 1) {
    $variables['first_new_post_link'] = advanced_forum_first_new_post_link($variables['node'], $variables['comment_count']);
  }

  // Link to last post in topic.
  $variables['last_post_link'] = advanced_forum_last_post_link($node);

  // Pager.
  if (isset($variables['node']->comment_count)) {
    pager_default_initialize($variables['node']->comment_count, variable_get('comment_default_per_page_' . $variables['node']->type, 50));
  }

  $variables['pager'] = theme('pager');
}

/**
 * Preprocesses template variables for the active poster template.
 */
function advanced_forum_preprocess_advanced_forum_active_poster(&$variables) {
  advanced_forum_add_template_suggestions("active_poster", $variables);

  $variables['account_name'] = theme('username', array('account' => $variables['account']));
  $variables['picture'] = theme('advanced_forum_user_picture', array('account' => $variables['account']));

  $node = $variables['last_post'];
  $variables['last_post_title'] = l($node->title, "node/$node->nid");
  $variables['last_post_date'] = format_date($node->created);
}

/**
 * Preprocesses template variables for the author pane.
 */
function advanced_forum_preprocess_author_pane(&$variables) {
  // Author pane is used in various places. Check the caller to make sure
  // we are the one that called it.
  if (!empty($variables['caller']) && $variables['caller'] == 'advanced_forum') {
    advanced_forum_add_template_suggestions("author_pane", $variables);
  }
}

/**
 * Preprocesses template variables for the page template.
 */
function advanced_forum_preprocess_page(&$variables) {
  if (arg(0) == 'forum') {
    $variables['forum_page'] = TRUE;
  }
  elseif (arg(0) == 'node' && !empty($variables['node']) && advanced_forum_type_is_in_forum($variables['node']->type)) {
    $variables['forum_page'] = TRUE;
  }
}

/**
 * Preprocesses template variables for the node template.
 */
function advanced_forum_preprocess_node(&$variables) {
  if (advanced_forum_is_styled($variables['node'], $variables['teaser'], 'node')) {
    include_once drupal_get_path('module', 'advanced_forum') . '/includes/advanced_forum_preprocess_node.inc';
    _advanced_forum_preprocess_node($variables);
  }
}

/**
 * Preprocess comment wrapper.
 */
function advanced_forum_preprocess_comment_wrapper(&$variables) {
  $variables['reply_link'] = '';
  if (advanced_forum_is_styled($variables['node'], FALSE, 'comment-wrapper')) {
    advanced_forum_add_template_suggestions("advanced_forum_comment_wrapper", $variables);

    $form_on_seperate_page = variable_get('comment_form_location_' . $variables['node']->type, COMMENT_FORM_SEPARATE_PAGE) == COMMENT_FORM_SEPARATE_PAGE;

    $comments_locked = $variables['node']->comment != COMMENT_NODE_OPEN;

    if ($form_on_seperate_page || $comments_locked) {
      // If the post is locked or the comment form is on a seperate page,
      // build the reply/locked link / button.
      $variables['reply_link'] = theme('advanced_forum_reply_link', array('node' => $variables['node']));
    }
  }
}

/**
 * Preprocesses template variables for the comment template.
 */
function advanced_forum_preprocess_comment(&$variables) {
  if (advanced_forum_is_styled($variables['comment'], FALSE, 'comment')) {
    include_once drupal_get_path('module', 'advanced_forum') . '/includes/advanced_forum_preprocess_comment.inc';
    _advanced_forum_preprocess_comment($variables);
  }
}

/* * * ORGANIC GROUPS ******************************************************** */
/**
 * Preprocess view forum group list.
 */
function advanced_forum_preprocess_views_view__advanced_forum_group_topic_list(&$variables) {
  _advanced_forum_add_files();
  advanced_forum_add_template_suggestions("group_topic_list_outer_view", $variables);

  // Set a variable for displaying the topic legend.
  $variables['topic_legend'] = theme('advanced_forum_topic_legend');
}
