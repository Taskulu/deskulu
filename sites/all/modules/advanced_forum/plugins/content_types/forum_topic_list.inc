<?php
/**
 * @file
 * Forum topic list.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Forum topic list'),
  'icon' => 'icon_forum.png',
  'description' => t('The list of all topics for a forum.'),
  'required context' => new ctools_context_required(t('Forum'), 'forum'),
  'category' => t('Forum'),
  'defaults' => array('show_sort' => TRUE),
);

/**
 * Render the content.
 */
function advanced_forum_forum_topic_list_content_type_render(
  $subtype,
  $conf,
  $panel_args,
  $context
) {
  if (!empty($context) && empty($context->data)) {
    return;
  }

  if (!empty($context->data->container)) {
    return;
  }

  $tid = 0;
  if (!empty($context)) {
    $tid = $context->data->tid;
  }

  $block = new stdClass();
  $block->module = 'forum-secondary-links';
  $block->delta = $tid;

  // By default this has no title.
  $block->title = '';
  _advanced_forum_add_files();

  // TODO: We could make these settings, but they're ignored if it isusing
  // the view, so we have to be careful about that.
  $forum_per_page = variable_get('forum_per_page', 25);
  $sortby = variable_get('forum_order', 1);

  $block->content = advanced_forum_get_topics(
    $tid,
    $sortby,
    $forum_per_page,
    !empty($conf['show_sort'])
  );

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_topic_list_content_type_edit_form(
  $form,
  &$form_state
) {
  $conf = $form_state['conf'];
  $form['show_sort'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show sort form'),
    '#description' => t(
      'If checked the sort form will appear at the top of the table. Uncheck this if you wish it to appear in a separate pane.'
    ),
    '#default_value' => !empty($conf['show_sort']),
  );

  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_topic_list_content_type_edit_form_submit(
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
function advanced_forum_forum_topic_list_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" topic list', array('@s' => $context->identifier));
}
