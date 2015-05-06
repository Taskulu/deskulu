<?php

/**
 * @file
 * Holds the contents of a preprocess function moved into its own file
 * to ease memory requirements and having too much code in one file.
 */

/**
 * Preprocess node.
 */
function _advanced_forum_preprocess_node(&$variables) {
  /* Shortcut variables */
  $node = $variables['node'];

  // Determine the template file to use for the node.
  if (isset($_GET['page']) && $_GET['page'] > 0) {
    // This is the repeated node on the top of subsequent pages.
    // We send this to a special .tpl so people can adjust it to their needs.
    advanced_forum_add_template_suggestions("post-repeated", $variables);
  }
  elseif (arg(1) == 'reply' || (arg(0) == 'node' && arg(1) == 'add') || !empty($node->in_preview)) {
    // 'reply' deals with the first post being shown when adding a comment.
    // 'node/add' is when previewing a new forum post.
    advanced_forum_add_template_suggestions("post-preview", $variables);
  }
  else {
    // Use our combined node/comment template file.
    advanced_forum_add_template_suggestions("post", $variables);
  }

  /* Topic header */
  $variables['top_post'] = TRUE;

  // Build the topic header.
  $variables['topic_header'] = theme('advanced_forum_topic_header', array(
    'node' => $node,
    'comment_count' => isset($variables['comment_count']) ? $variables['comment_count'] : 0,
    ));

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
  $variables['classes_array'][] = "posted-by-$node->uid";

  /* Post ID */
  // Set the post ID for theming/targetting.
  $variables['post_id'] = "post-$node->nid";

  /* Linked post number */
  if (!isset($post_number)) {
    static $post_number = 1;
  }

  $page_number = !empty($_GET['page']) ? $_GET['page'] : 0;

  // If this is the topic starting node, we need to start off the numbering.
  // Variable changed from post_link to permalink to match core variable.
  // @codingStandardsIgnoreStart
  $variables['permalink'] = l('#1', "node/$node->nid");
  // @codingStandardsIgnoreEnd

  /* In reply to */
  $variables['in_reply_to'] = "";

  /* User information / author pane */
  $variables['account'] = user_load($node->uid);

  if (!module_exists('author_pane')) {
    // If not using the Author Pane module, create a simple version with just
    // name and photo. If using AP, that's handled down in the
    // "uncached variables" section.
    $variables['author_pane'] = theme('advanced_forum_simple_author_pane', array('context' => $node));
  }

  /* Reply link */
  // Build the reply link / button. This isn't used in the default template
  // but is provided as a convenience to people who want to customize theirs.
  $variables['reply_link'] = theme('advanced_forum_reply_link', array('node' => $node));

  /* Uncached variables */
  if (!$variables['top_post'] && method_exists('views_plugin_cache', 'post_render') && !$variables['node']->in_preview) {
    /* Author Pane */
    if (module_exists('author_pane')) {
      $variables['author_pane'] = '<!--post:author-pane-' . $node->uid . '-->';
    }

    /* Revisions */
    if (!empty($variables['revision_timestamp']) && user_access('view last edited notice') && $variables['created'] != $variables['revision_timestamp']) {
      $variables['post_edited'] = '<!--post:post-edited-' . $node->nid . '-->';
    }

    /* Signatures */
    if (variable_get('user_signatures', 0)) {
      $variables['signature'] = '<!--post:signature-core-' . $node->uid . '-->';
    }

    /* User specific node theming class */
    // Add class if the poster is the viewer.
    $variables['classes_array'][] = "<!--post:poster-id-' . $node->uid . '-->";
  }
  else {
    // Create variables normally.
    /* Author Pane */
    if (module_exists('author_pane')) {
      $variables['author_pane'] = theme('author_pane', array(
            'account' => $variables['account'],
            'caller' => 'advanced_forum',
            'picture_preset' => variable_get('advanced_forum_user_picture_preset', ''),
            'context' => $node,
            'disable_css' => TRUE,
            'join_date_type' => variable_get('advanced_forum_author_pane_join_date_type', 'short'),
          ));
    }

    /* Revisions */
    if (isset($variables['revision_timestamp']) && user_access('view last edited notice') && $variables['created'] != $variables['revision_timestamp']) {
      $variables['post_edited'] = theme('advanced_forum_post_edited', array(
        'who' => $variables['revision_uid'],
        'when' => $variables['revision_timestamp'],
        'why' => $variables['log'],
      ));
    }

    /* Viewer is poster class */
    global $user;
    if ($user->uid > 0 && $user->uid == $node->uid) {
      $variables['classes_array'][] = " post-by-viewer";
    }
  }
}
