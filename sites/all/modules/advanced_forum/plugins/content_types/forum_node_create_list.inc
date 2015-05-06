<?php
/**
 * @file
 * Forum node create list.
 */

/**
 * Callback function to supply a list of content types.
 */
function advanced_forum_forum_node_create_list_ctools_content_types() {
  return array(
    'single' => TRUE,
    'title' => t('Forum node create list'),
    'icon' => 'icon_forum.png',
    'description' => t('List of links to create new forum content nodes.'),
    'required context' => new ctools_context_required(t('Forum'), 'forum'),
    'category' => t('Forum'),
    'defaults' => array(),
  );
}

/**
 * Render the content.
 */
function advanced_forum_forum_node_create_list_content_type_render(
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
  $block->module = 'forum-node-create-list';
  $block->delta = $tid;

  // By default this has no title.
  $block->title = '';
  _advanced_forum_add_files();

  $block->content = '<div class="forum-node-create-links">' . theme(
      'advanced_forum_node_type_create_list',
      array('forum_id' => $tid)
    ) . '</div>';

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_node_create_list_content_type_edit_form(
  $form,
  &$form_state
) {
  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_node_create_list_content_type_edit_form_submit(
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
function advanced_forum_forum_node_create_list_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" forum node create list', array('@s' => $context->identifier));
}
