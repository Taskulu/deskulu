<?php

/**
 * @file
 * Views integration for advanced_forum.
 */

/**
 * Loads the included views.
 *
 * This function is used instead of views ability to autodiscover a views
 * export .inc because this allows us to put each view in its own file.
 * Thanks to Moshe and OG for the code.
 */
function advanced_forum_views_default_views() {
  if (variable_get('advanced_forum_autoload_views', TRUE)) {
    global $theme, $theme_path;
    $files = file_scan_directory(drupal_get_path('module', 'advanced_forum') . '/includes/views', '/\.view$/');
    $files += file_scan_directory(drupal_get_path('theme', variable_get('theme_default', 'garland')) . '/advanced_forum/views', '/\.view$/');

    foreach ($files as $absolute => $file) {
      $view = NULL;
      require $absolute;
      if (isset($view)) {
        $views[$view->name] = $view;
      }
    }

    return $views;
  }
}

/**
 * Use views_data_alter to add items to the node table that are relevant to topic icons.
 */
function advanced_forum_views_data_alter(&$data) {
  // Topic icon.
  $data['node']['topic_icon'] = array(
    'title' => t('Topic Icon'),
    'help' => t('Icon that shows new posts, hot, sticky, locked, etc.'),
    'field' => array(
      'handler' => 'advanced_forum_handler_field_node_topic_icon',
    ),
  );

  $data['node']['topic_pager'] = array(
    'title' => t('Topic Pager'),
    'help' => t('Small pager for individual topics.'),
    'field' => array(
      'handler' => 'advanced_forum_handler_field_node_topic_pager',
    ),
  );
}

/**
 * Implements hook_views_plugins().
 */
function advanced_forum_views_plugins() {
  $path = drupal_get_path('module', 'advanced_forum') . '/includes/views';
  return array(
    'style' => array(
      'forum_topic_list' => array(
        'parent' => 'table',
        'path' => $path,
        'title' => t('Forum topic list'),
        'help' => t('Displays the forum topic list as a view.'),
        'handler' => 'advanced_forum_plugin_style_forum_topic_list',
        'theme path' => drupal_get_path('module', 'advanced_forum') . '/includes',
        'theme file' => 'theme.inc',
        'theme' => 'advanced_forum_topic_list_view',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
    ),
  );
}

/**
 * Implements hook_views_data().
 */
function advanced_forum_views_data() {
  $data = array();
  // Forum table.
  // Have not defined $data['forum']['table']['group'] since
  // no fields are defined here yet.
  $data['forum']['table']['join'] = array(
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  return $data;
}
