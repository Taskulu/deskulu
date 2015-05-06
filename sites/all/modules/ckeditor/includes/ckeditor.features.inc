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
 * CKEditor Module for Drupal 7.x
 *
 * This module allows Drupal to replace textarea fields with CKEditor.
 *
 * CKEditor is an online rich text editor that can be embedded inside web pages.
 * It is a WYSIWYG (What You See Is What You Get) editor which means that the
 * text edited in it looks as similar as possible to the results end users will
 * see after the document gets published. It brings to the Web popular editing
 * features found in desktop word processors such as Microsoft Word and
 * OpenOffice.org Writer. CKEditor is truly lightweight and does not require any
 * kind of installation on the client computer.
 */

/**
 * Implementation of hook_features_export_options()
 */
function ckeditor_profile_features_export_options() {
  $options = array();
  $profiles = (array) ckeditor_profile_load();
  foreach ($profiles as $name => $profile) {
    $options[$name] = $profile->name;
  }
  return $options;
}

/**
 * Implementation of hook_features_export()
 */
function ckeditor_profile_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ((array) $data as $name) {
    $profile = ckeditor_profile_load($name);
    if ($profile) {
      $export['features']['ckeditor_profile'][$name] = $name;

      // Write dependencies on all the roles referenced by this profile
      foreach ((array) $profile->input_formats as $input_format => $input_format_name) {
        $pipe['input_formats'][] = $input_format;
      }
    }
  }
  $export['dependencies'][] = 'ckeditor';
  return $pipe;
}

/**
 * Implementation of hook_features_export_render()
 */
function ckeditor_profile_features_export_render($module_name, $data) {
  $profiles = array();
  $roles = user_roles();
  foreach ($data as $name) {
    $profile = (array) ckeditor_profile_load($name, TRUE, FALSE);

    $profiles[$name] = $profile;
  }
  $code = '  $data = ' . features_var_export($profiles, '  ') . ';' . PHP_EOL;
  $code .= '  return $data;';

  return array('ckeditor_profile_defaults' => $code);
}

/**
 * Implementation of hook_features_rebuild()
 */
function ckeditor_profile_features_rebuild($module) {
  ckeditor_profile_features_revert($module);
}

/**
 * Implementation of hook_features_revert()
 */
function ckeditor_profile_features_revert($module) {
  if ($data = features_get_default('ckeditor_profile', $module)) {
    $input_formats = filter_formats();
    foreach ($data as $name => $profile) {
      // Restore the profile settings
      db_query("DELETE FROM {ckeditor_settings} WHERE name = :name", array(':name' => $name));
      db_query("INSERT INTO {ckeditor_settings} (name, settings) VALUES(:name, :settings)", array(':name' => $name, ':settings' => serialize($profile['settings'])));

      if (empty($profile["input_formats"])) {
        // Remove input format if none is specified
        db_query("DELETE FROM {ckeditor_input_format} WHERE name = :name", array(':name' => $name));
      }
      else {
        // Restore the profile roles
        foreach ($profile["input_formats"] as $input_format => $input_format_name) {
          if (!db_query("SELECT name FROM {ckeditor_input_format} WHERE format = :format AND name = :name", array(':name' => $name, ':format' => $input_format))->fetchField()) {
            db_query("INSERT INTO {ckeditor_input_format} (name, format) VALUES(:name, :format)", array(':name' => $name, ':format' => $input_format));
          }
        }
      }
    }
  }
}
