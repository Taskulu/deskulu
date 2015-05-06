<?php

/**
 * CKEditor - The text editor for the Internet - http://ckeditor.com
 * Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
 *
 * == BEGIN LICENSE ==
 *
 * Licensed under the terms of any of the following licenses of your
 * choice:
 *
 *  - GNU General Public License Version 2 or later (the "GPL")
 *    http://www.gnu.org/licenses/gpl.html
 *
 *  - GNU Lesser General Public License Version 2.1 or later (the "LGPL")
 *    http://www.gnu.org/licenses/lgpl.html
 *
 *  - Mozilla Public License Version 1.1 or later (the "MPL")
 *    http://www.mozilla.org/MPL/MPL-1.1.html
 *
 * == END LICENSE ==
 *
 * @file
 * Drush integration for the CKEditor module.
 */

/**
 * Implements hook_drush_command().
 */
function ckeditor_drush_command() {
  $items['ckeditor-download'] = array(
    'callback' => 'ckeditor_drush_download',
    'description' => dt('Downloads the required CKEditor library from svn.ckeditor.com.'),
    'arguments' => array(
      'path' => dt('Optional. The path to the download folder. If omitted, Drush will use the default location (<code>sites/all/libraries/ckeditor</code>).'),
    ),
  );
  return $items;
}

/**
 * Downloads
 */
function ckeditor_drush_download() {
  $args = func_get_args();
  if ($args[0]) {
    $path = $args[0];
  }
  else {
    $path = drush_get_context('DRUSH_DRUPAL_ROOT') . '/sites/all/libraries/ckeditor';
  }
  $svn_cmd = 'svn checkout http://svn.ckeditor.com/CKEditor/releases/stable/ ' . $path;
  if (drush_shell_exec($svn_cmd)) {
    drush_log(dt('CKEditor was downloaded to !path.', array('!path' => '<code>' . $path . '</code>')), 'success');
  }
  else {
    drush_log(dt('Drush was unable to download CKEditor to !path.', array('!path' => '<code>' . $path . '</code>')) . '<br/>' . dt('Attempted command: !svn_cmd.', array('!svn_cmd' => '<code>' . $svn_cmd . '</code>')), 'error');
  }
}

/**
 * Implements drush_MODULE_post_COMMAND().
 */
function drush_ckeditor_post_enable() {
  $modules = func_get_args();
  if (in_array('ckeditor', $modules) && !drush_get_option('skip')) {
    ckeditor_drush_download();
  }
}
