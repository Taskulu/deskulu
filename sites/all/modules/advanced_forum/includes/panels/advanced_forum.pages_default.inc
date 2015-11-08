<?php

/**
 * @file
 * Default panel handlers for Advanced Forum.
 */

/**
 * Loads the included panels.
 *
 * This function is used instead of panels ability to autodiscover a panels
 * export .inc because this allows us to put each page in its own file.
 * Thanks to Moshe and OG for the code.
 */
function advanced_forum_default_page_manager_handlers() {
  if (variable_get('advanced_forum_autoload_page_handlers', TRUE)) {

    static $handlers = array();

    if (empty($handlers)) {
      $files = file_scan_directory(drupal_get_path('module', 'advanced_forum') . '/includes/panels', '/\.handler$/');
      foreach ($files as $absolute => $file) {
        $handler = NULL;
        require_once $absolute;
        if (isset($handler)) {
          $handlers[$file->name] = $handler;
        }
      }
    }

    return $handlers;
  }
}
