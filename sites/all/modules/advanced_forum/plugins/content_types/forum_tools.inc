<?php
/**
 * @file
 * Forum tools.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Forum tools'),
  'icon' => 'icon_forum.png',
  'description' => t('Jump list with various forum destinations.'),
  'required context' => new ctools_context_required(t('Forum'), 'forum'),
  'category' => t('Forum'),
  'defaults' => array(),
);

/**
 * Render the content.
 */
function advanced_forum_forum_tools_content_type_render(
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
  $block->module = 'forum-tools';
  $block->delta = $tid;

  // By default this has no title.
  $block->title = '';
  _advanced_forum_add_files();

  $block->content = advanced_forum_forum_tools($tid);

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function advanced_forum_forum_tools_content_type_edit_form(
  $form,
  &$form_state
) {
  return $form;
}

/**
 * Submit callback.
 */
function advanced_forum_forum_tools_content_type_edit_form_submit(
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
function advanced_forum_forum_tools_content_type_admin_title(
  $subtype,
  $conf,
  $context
) {
  return t('"@s" forum tools', array('@s' => $context->identifier));
}
