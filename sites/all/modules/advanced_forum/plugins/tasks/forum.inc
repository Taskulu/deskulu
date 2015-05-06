<?php
/**
 * @file
 * Plugin for tasks.
 */

/**
 * Page manager tasks.
 */
function advanced_forum_forum_page_manager_tasks() {
  return array(
    // This is a 'page' task and will fall under the page admin UI.
    'task type' => 'page',
    'title' => t('Forum page'),
    'admin title' => t('Forum page'),
    'admin description' => t('When enabled, this overrides the default Drupal behavior for displaying forum listings at <em>forum</em> and <em>forum/%forum</em>. If you add variants, you may use selection criteria such as roles or user access to provide different views of the forums. If no variant is selected, the normal advanced forum view will be selected.'),
    'admin path' => 'forum/%advanced_forum_forum',
    'hook menu' => 'advanced_forum_forum_menu',
    'hook menu alter' => 'advanced_forum_forum_menu_alter',
    // This is task uses 'context' handlers and must implement these to give the
    // handler data it needs.
    // Handler type -- misnamed.
    'handler type' => 'context',
    'get arguments' => 'advanced_forum_forum_get_arguments',
    'get context placeholders' => 'advanced_forum_forum_get_contexts',
    // Allow this to be enabled or disabled:
    'disabled' => variable_get('advanced_forum_forum_disabled', TRUE),
    'enable callback' => 'advanced_forum_forum_enable',
    'access callback' => 'advanced_forum_forum_access_check',
    // Allow additional operations.
    'operations' => array(
      'settings' => array(
        'title' => t('Settings'),
        'description' => t('Edit name, path and other basic settings for the page.'),
        'form' => 'advanced_forum_forum_page_settings',
      ),
    ),
    // Even though we don't have subtasks, this allows us to save our settings.
    'save subtask callback' => 'advanced_forum_forum_page_save',
  );
}

/**
 * Callback defined by advanced_forum_forum_page_manager_tasks().
 *
 * Alter the user view input so that user view comes to us rather than the
 * normal user view process.
 */
function advanced_forum_forum_menu_alter(&$items, $task) {
  if (variable_get('advanced_forum_forum_disabled')) {
    return;
  }

  $items['forum']['page callback'] = 'advanced_forum_forum_page';
  $items['forum']['file path'] = $task['path'];
  $items['forum']['file'] = $task['file'];

  // Take over forum/%advanced_forum_forum page.
  $items['forum/%advanced_forum_forum']['page callback'] = 'advanced_forum_forum_page';
  $items['forum/%advanced_forum_forum']['file path'] = $task['path'];
  $items['forum/%advanced_forum_forum']['file'] = $task['file'];
}

/**
 * Entry point for our overridden user view.
 *
 * This function asks its assigned handlers who, if anyone, would like
 * to run with it. If no one does, it passes through to advanced_forum_page().
 */
function advanced_forum_forum_page($forum_term = NULL) {
  if (!isset($forum_term)) {
    // On the main page, display all the top-level forums.
    $forum_term = advanced_forum_forum_load(0);
  }

  // Set tid for <root> container.
  if (!isset($forum_term->tid)) {
    $forum_term->tid = 0;
  }

  // Load my task plugin:
  $task = page_manager_get_task('forum');

  // Load the account into a context.
  ctools_include('context');
  ctools_include('context-task-handler');
  $contexts = ctools_context_handler_get_task_contexts($task, '', array($forum_term->tid));

  $output = ctools_context_handler_render($task, '', $contexts, array($forum_term->tid));
  if ($output === FALSE) {
    $output = advanced_forum_page($forum_term);
  }

  return $output;
}

/**
 * Callback to get arguments provided by this task handler.
 *
 * Since this is the node view and there is no UI on the arguments, we
 * create dummy arguments that contain the needed data.
 */
function advanced_forum_forum_get_arguments($task, $subtask_id) {
  return array(
    array(
      'keyword' => 'forum',
      'identifier' => t('Forum'),
      'id' => 1,
      'name' => 'forum_id',
      'settings' => array('breadcrumb' => variable_get('advanced_forum_forum_page_breadcrumb', FALSE)),
    ),
  );
}

/**
 * Callback to get context placeholders provided by this handler.
 */
function advanced_forum_forum_get_contexts($task, $subtask_id) {
  return ctools_context_get_placeholders_from_argument(advanced_forum_forum_get_arguments($task, $subtask_id));
}

/**
 * Callback to enable/disable the page from the UI.
 */
function advanced_forum_forum_enable($cache, $status) {
  if ($status == NULL) {
    $status = FALSE;
  }
  variable_set('advanced_forum_forum_disabled', $status);
}

/**
 * Settings page for this item.
 */
function advanced_forum_forum_page_settings($form, &$form_state) {
  if (empty($form_state['page']->update_values)) {
    $settings = array(
      'advanced_forum_forum_page_breadcrumb' => variable_get('advanced_forum_forum_page_breadcrumb', FALSE),
    );
  }
  else {
    $settings = $form_state['page']->update_values;
  }

  $form['advanced_forum_forum_page_breadcrumb'] = array(
    '#title' => t('Inject hierarchy of first term into breadcrumb trail'),
    '#type' => 'checkbox',
    '#default_value' => $settings['advanced_forum_forum_page_breadcrumb'],
    '#description' => t('If checked, taxonomy term parents will appear in the breadcrumb trail.'),
  );

  return $form;
}

/**
 * Copy form values into the page cache.
 */
function advanced_forum_forum_page_settings_submit($form, &$form_state) {
  $form_state['page']->update_values = $form_state['values'];
}

/**
 * Save when the page cache is saved.
 */
function advanced_forum_forum_page_save($subtask, $cache) {
  if (isset($cache->update_values)) {
    variable_set('advanced_forum_forum_page_breadcrumb', $cache->update_values['advanced_forum_forum_page_breadcrumb']);
  }
}

/**
 * Access check.
 */
function advanced_forum_forum_access_check($task, $subtask_id, $contexts) {
  $context = reset($contexts);
  return node_access('view', $context->data);
}
