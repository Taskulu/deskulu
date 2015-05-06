<?php

/**
 * @file
 * Holds the contents of a preprocess function moved into its own file
 * to ease memory requirements and having too much code in one file.
 */

/**
 * Preprocess comment.
 */
function _advanced_forum_preprocess_comment(&$variables) {
  /* Easy links to the comment and parent node */
  $comment = $variables['comment'];
  $node = node_load($comment->nid);
  $variables['first_post'] = $node;

  // This is a comment, not the node.
  $variables['top_post'] = FALSE;

  /* Determine the template file to use for the comment. */
  if (arg(1) == 'reply' || arg(2) == 'edit') {
    // Use the preview version.
    advanced_forum_add_template_suggestions("post-preview", $variables);
  }
  else {
    // Use our combined node/comment template file.
    advanced_forum_add_template_suggestions("post", $variables);
  }

  if (!variable_get('advanced_forum_keep_classes', FALSE) || !is_array($variables['classes_array'])) {
    // Add in our classes overwriting existing.
    $variables['classes_array'] = array();
  }
  $variables['classes_array'][] = 'forum-post clearfix';

  // Add the current language to the classes for image handling.
  global $language;
  if (!empty($language->language)) {
    $variables['classes_array'][] = $language->language;
  }

  // Add the poster's UID.
  $variables['classes_array'][] = "posted-by-$comment->uid";

  // Add class if the poster is the viewer.
  global $user;
  if ($user->uid > 0 && $user->uid == $comment->uid) {
    $variables['classes_array'][] = "post-by-viewer";
  }

  // Add class if the poster is the topic starter.
  if ($node->uid > 0 && $node->uid == $comment->uid) {
    $variables['classes_array'][] = "post-by-starter";
  }

  // Set the post ID for theming / targetting.
  $variables['post_id'] = "post-$comment->cid";
  $num_comments = $node->comment_count;
  $posts_per_page = variable_get('comment_default_per_page_' . $node->type, 50);

  $page_number = !empty($_GET['page']) ? $_GET['page'] : 0;

  // Page_number sanity check.
  if ($page_number > floor($num_comments / $posts_per_page)) {
    $page_number = floor($num_comments / $posts_per_page);
  }

  if (!$page_number) {
    $page_number = 0;
  }
  /* Linked post number */
  if (!isset($post_number)) {
    static $post_number = 1;
  }

  // Check if node type is forum.
  $topic_id = advanced_forum_node_is_styled($variables['node']);
  $post_number_delta = ($topic_id) ? 1 : 0;
  $linktext = '#' . (($page_number * $posts_per_page) + $post_number + $post_number_delta);
  $post_number++;

  // Permalink.
  // You can erase next 3 lines if you wish to use built-in Permalink.
  // Template adjusted: $post_link -> $permalink.
  $uri = entity_uri('comment', $comment);
  $uri['options'] += array('attributes' => array('class' => array('permalink'), 'rel' => 'bookmark'));
  $variables['permalink'] = l($linktext, $uri['path'], $uri['options']);

  /* In reply to */
  $variables['in_reply_to'] = "";
  if ($comment->pid > 0) {
    // Find the display position of the parent post;.
    $post_position = advanced_forum_post_position($node, $comment);

    // This extra if test is a sanity check in case the comment being replied
    // to no longer exists.
    if ($post_position > 0) {
      // Find the page the post is on. We need to compensate for the topic node
      // being #1 which puts an extra post on the first page but not on the rest.
      $page_number = floor(($post_position - 1) / $posts_per_page);

      // Assemble the link.
      $fragment = 'comment-' . $comment->pid;

      if ($page_number) {
        $query = array('page' => $page_number);
      }

      $linktext = t("(Reply to #!post_position)", array('!post_position' => $post_position));
      $linkpath = "node/$node->nid";
      $variables['in_reply_to'] = l($linktext, $linkpath, array('query' => empty($query) ? array() : $query, 'fragment' => $fragment));
    }
  }

  /* Title */
  if (variable_get('comment_subject_field_' . $node->type, 1) == 0) {
    // If comment titles are disabled, don't display it.
    $variables['title'] = '';
  }
  else {
    // Assign the subject to the title variable for consistancy with nodes.
    $variables['title'] = check_plain($comment->subject);
  }

  $variables['account'] = new stdClass();
  /* User information / author pane */
  if ($comment->uid == 0) {
    // Anonymous user. Make a fake user object for theme_username
    $variables['account']->name = empty($comment->name) ? "" : $comment->name;
    $variables['account']->homepage = empty($comment->homepage) ? "" : $comment->homepage;
    $variables['account']->email = empty($comment->email) ? "" : $comment->email;
  }
  else {
    // Load up the real user object.
    $variables['account'] = user_load($comment->uid);
  }

  // Create the author pane.
  if (module_exists('author_pane')) {
    $variables['author_pane'] = theme('author_pane', array(
      'account' => $variables['account'],
      'caller' => 'advanced_forum',
      'picture_preset' => variable_get('advanced_forum_user_picture_preset', ''),
      'context' => $comment,
      'disable_css' => TRUE,
      'join_date_type' => variable_get('advanced_forum_author_pane_join_date_type', 'short'),
        ));
  }
  else {
    $variables['author_pane'] = theme('advanced_forum_simple_author_pane', array('context' => $comment));
  }
  /* Content */
  // Helpful $content variable for templates.
  foreach (element_children($variables['elements']) as $key) {
    // Adjust content vriable to suit post template.
    if ($key == 'comment_body') {
      $variables['content']['body'] = $variables['elements'][$key];
    }
    else {
      $variables['content'][$key] = $variables['elements'][$key];
    }
  }

  // Adjust comment timestamp to match node template.
  $variables['date'] = $variables['created'];

  /* Post edited */
  $variables['post_edited'] = (isset($variables['comment_edited'])) ? $variables['comment_edited'] : "";
}
