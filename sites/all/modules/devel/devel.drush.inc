<?php

/**
 * @file
 * Drush integration for the devel module.
 */

/**
 * Implements hook_drush_command().
 */
function devel_drush_command() {
  $items['devel-download'] = array(
    'description' => dt('Downloads the FirePHP library from http://firephp.org/.'),
    'arguments' => array(
      'path' => dt('Path to the download folder. This path is relative to the Drupal root. If omitted Drush will use the default location (sites/all/libraries/FirePHPCore).'),
    ),
  );
  $items['devel-reinstall'] = array(
    'description' => dt('Disable, Uninstall, and Install a list of projects.'),
    'drush dependencies' => array('pm'),
    'arguments' => array(
      'projects' => dt('A space-separated list of project names.'),
    ),
    'allow-additional-options' => array('pm-disable', 'pm-uninstall', 'pm-enable'),
    'required-arguments' => 1,
    'aliases' => array('dre'),
  );
  $items['fn-hook'] = array(
    'description' => 'List implementations of a given hook and explore the source of the selected one.',
    'arguments' => array(
      'hook' => 'The name of the hook to explore (e.g. "menu" for hook_menu()).'
    ),
    'examples' => array(
      'fn-hook cron' => 'List implementations of hook_cron().',
    ),
    'allow-additional-options' => array('fn-view'),
    'required-arguments' => 1,
    'aliases' => array('fnh', 'hook'),
  );
  $items['fn-view'] = array(
    'description' => 'Show the source of specified function or method.',
    'arguments' => array(
      'function' => 'The name of the function or method to view.',
    ),
    'options' => array(
      'pipe' => 'Output just the filename of the function',
      'format' => 'Specify how the filename should be printed. Available placeholders are !startline, !endline and !file',
    ),
    'examples' => array(
      'fn-view drupal_set_breadcrumb' => 'View the source code for function "drupal_set_breadcrumb"',
      'vi `drush --pipe fn-view user_access --format=\'+!startline !file\'`' => 'Edit the file that contains the function "user_access"',
      'fn-view NodeController::load' => 'View the source code for method load in the class NodeController'
    ),
    'aliases' => array('fnv'),
    'required-arguments' => 1,
  );
  $items['devel-token'] = array(
    'description' => dt('List available tokens'),
    'aliases' => array('token'),
    'core' => array(7), // Remove once 3.0 is released.
    //@todo support --format option for json, csv, etc.
  );
  return $items;
}

/**
 * A command callback. This is faster than 3 separate bootstraps.
 */
function drush_devel_reinstall() {
  $projects = func_get_args();

  $args = array_merge(array('pm-disable'), $projects);
  call_user_func_array('drush_invoke', $args);

  $args = array_merge(array('pm-uninstall'), $projects);
  call_user_func_array('drush_invoke', $args);

  $args = array_merge(array('pm-enable'), $projects);
  call_user_func_array('drush_invoke', $args);
}

/**
 * A command callback.
 */
function drush_devel_download($path = NULL) {
  // If no path is provided by the user, set our default path.
  if (is_null($path)) {
    // We use devel folder for legacy reason.
    $path = drupal_get_path('module', 'devel') . '/FirePHPCore';
  }
  // If FirePHP is not installed and libraries module is enabled,
  // try to find FirePHP by its own means.
  if (!is_dir($path)) {
    if (module_exists('libraries')) {
      // Libraries 1.x will return a path even if it doesn't exist
      // while 2.x will return FALSE.
      $path = libraries_get_path('FirePHPCore');
      if (!$path) {
        $path = 'sites/all/libraries/FirePHPCore';
      }
    }
  }
  if (is_dir($path)) {
    drush_log(dt('FirePHP already present at @path. No download required.', array('@path' => $path)), 'ok');
  }
  elseif (drush_shell_exec('svn export http://firephp.googlecode.com/svn/branches/Library-FirePHPCore-0.3 %s', $path)) {
    drush_log(dt('FirePHP has been exported via svn to @path.', array('@path' => $path)), 'success');
  }
  else {
    drush_log(dt('Drush was unable to export FirePHP to @path.', array('@path' => $path)), 'warning');
  }
}

/**
 * Command handler. Show hook implementations.
 */
function drush_devel_fn_hook($hook) {
  // Get implementations in the .install files as well.
  include_once './includes/install.inc';
  drupal_load_updates();

  if ($hook_implementations = module_implements($hook)) {
    if ($choice = drush_choice(array_combine($hook_implementations, $hook_implementations), 'Enter the number of the hook implementation you wish to view.')) {
      return drush_devel_fn_view($choice . "_$hook");
    }
  }
  else {
    drush_log(dt('No implementations.'), 'ok');
  }
}

/**
 * Command handler.  Show source code of specified function or method.
 */
function drush_devel_fn_view($function_name) {
  // Get implementations in the .install files as well.
  include_once './includes/install.inc';
  drupal_load_updates();

  if (strpos($function_name, '::') === FALSE) {
    if (!function_exists($function_name)) {
      return drush_set_error(dt('Function not found'));
    }
    $reflect = new ReflectionFunction($function_name);
  }
  else {
    list($class, $method) = explode('::', $function_name);
    if (!method_exists($class, $method)) {
      return drush_set_error(dt('Method not found'));
    }
    $reflect = new ReflectionMethod($class, $method);
  }
  $func_info = array('!file' => $reflect->getFileName(), '!startline' => $reflect->getStartLine(), '!endline' => $reflect->getEndLine());
  $format = drush_get_option('format', '!file');
  drush_print_pipe(dt($format, $func_info));
  drush_print(dt("// file: !file, lines !startline-!endline", $func_info));

  _drush_devel_print_function($reflect->getFileName(), $reflect->getStartLine(), $reflect->getEndLine());
}

/**
 * Command callback. List available tokens.
 */
function drush_devel_token() {
  $rows[] = array(dt('Group'), dt('Token'), dt('Name'));
  $all = token_info();
  foreach ($all['tokens'] as $group => $tokens) {
    foreach ($tokens as $key => $token) {
      $rows[] = array($group, $key, $token['name']);
    }
  }
  drush_print_table($rows, TRUE);
}


/**
 * Print the specified function, including any
 * doxygen-style comments that come before it.
 */
function _drush_devel_print_function($file, $start_line, $end_line) {
  $line_num = 0;
  $doxygen = NULL;
  $fp = fopen( $file, 'r' );

  while (!feof($fp) && ($line_num < ($start_line - 1))) {
    $line = fgets($fp);
    ++$line_num;

    if (substr($line,0,3) == '/**') {
      $doxygen = $line;
    }
    elseif (isset($doxygen)) {
      $doxygen .= $line;
      if ($line_num + 1 == $start_line) {
        drush_print(rtrim($doxygen));
      }
      if (strstr($line, '*/') !== FALSE) {
        $doxygen = NULL;
      }
    }
  }
  while (!feof($fp) && ($line_num < $end_line)) {
    $line = fgets($fp);
    ++$line_num;
    drush_print(rtrim($line));
  }
}
