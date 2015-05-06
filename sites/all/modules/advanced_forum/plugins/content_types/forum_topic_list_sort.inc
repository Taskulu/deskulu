<?php
/**
 * @file
 * Forum topic sort.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Forum topic list sort widget'),
  'icon' => 'icon_forum.png',
  'description' => t(
    'A small select widget to change the sort order of the forum topic list'
  ),
  'required context' => new ctools_context_required(t('Forum'), 'forum'),
  'category' => t('Forum'),
  'defaults' => array(),
);

/**
 * Render the content.
 */
function advanced_forum_forum_topic_list_sort_content_type_render(
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

  if (empty($tid)) {
    return;
  }

  $block = new stdClass();
  $block->module = 'forum-secondary-links';
  $block->delta = $tid;

  // By default this has no title.
  $block->title = '';
  _advanced_forum_add_files();

  $block->content = advanced_forum_forum_topic_list_sort();

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_topic_list_sort_content_type_edit_form(
  $form,
  &$form_state
) {
  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_topic_list_sort_content_type_edit_form_submit(
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
function advanced_forum_forum_topic_list_sort_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" topic list sort widget', array('@s' => $context->identifier));
}
