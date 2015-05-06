<?php
/**
 * @file
 * Forum list.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Forum list'),
  'icon' => 'icon_forum.png',
  'description' => t('A list of forums for the forum.'),
  'required context' => new ctools_context_required(t('Forum'), 'forum'),
  'category' => t('Forum'),
  'defaults' => array(),
);

/**
 * Render the content.
 */
function advanced_forum_forum_list_content_type_render(
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

  $block = new stdClass();
  $block->module = 'forum-list';
  $block->delta = $tid;

  _advanced_forum_add_files();

  $forum_term = advanced_forum_forum_load($tid);
  $forums = isset($forum_term->forums) ? $forum_term->forums : array();

  if (empty($forums)) {
    return $block;
  }
  // No parents on the main forum page.
  $block->content = theme(
    'forum_list',
    array(
      'forums' => $forums,
      'parents' => empty($context->parents) ? array() : $context->parents,
      'tid' => $tid,
    )
  );

  if (user_access('administer forums')) {
    $block->admin_links['administer_forums'] = array(
      'title' => t('Administer forums'),
      'alt' => t("Add, delete or modify forums"),
      'href' => "admin/structure/forum",
    );

    if ($tid) {
      $block->admin_links['edit_forum'] = array(
        'title' => t('Edit forum'),
        'alt' => t("Modify this forum"),
        'href' => "admin/structure/forum/edit/" . ($context->data->container ? 'container/' : 'forum/') . $tid,
        'query' => drupal_get_destination(),
      );
    }
  }

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_list_content_type_edit_form($form, &$form_state) {
  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_list_content_type_edit_form_submit(
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
function advanced_forum_forum_list_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" forum list', array('@s' => $context->identifier));
}
