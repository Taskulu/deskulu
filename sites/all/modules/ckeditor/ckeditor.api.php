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
 * Hook to register the CKEditor plugin - it would appear in the plugins list on the profile setting page.
 */
function hook_ckeditor_plugin() {
  return array(
    'plugin_name' => array(
      // Name of the plugin used to write it.
      'name' => 'plugin_name',
      // Description of the plugin - it would be displayed in the plugins management section of profile settings.
      'desc' => t('Plugin description'),
      // The full URL to the CKEditor plugins directory, with the trailing slash.
      'path' => base_path() . drupal_get_path('module', 'my_module') . '/plugin_dir/',
      'buttons' => array(
        'button_name' => array(
          // Path to an icon relative to the plugins folder.
          'icon' => 'icon/plugin_name.png',
          'label' => 'Button Label',
        )
      )
    )
  );
}

/**
 * Hook to extend/change CKEditor plugins.
 *
 * @param $plugins
 *   An associative array of plugins.
 */
function hook_ckeditor_plugin_alter(&$plugins) {
  // Remove a plugin button.
  unset($plugins['plugin_name']['buttons']['button']);
}

/**
 * Hook to register the CKEditor security filter - it would appear in the security filters list on the profile setting page.
 */
function hook_ckeditor_security_filter() {
  return array(
    'security_filter_name' => array(
      // Title of the security filter - it would be displayed in the security filters section of profile settings.
      'title' => t('Security filter title'),
      // Description of the security filter - it would be displayed in the security filters section of profile settings.
      'description' => t('Security filter description'),
    )
  );
}

/**
 * Hook to alter CKEditor security filters.
 */
function hook_ckeditor_security_filter_alter(&$security_filters) {
  // Modify a $security_filter.
}

/**
 * Hook to extend/change the ckeditor settings.
 *
 * This hook is invoked from ckeditor_profile_settings_compile(). The settings
 * may be customized or enhanced; typically with options that cannot be
 * controlled though the administrative UI from the ckeditor module.
 *
 * @param $settings
 *   An associative array of settings.
 * @param $conf
 *   An associative array with access to raw profile settings that might be helpful to alter the real $settings.
 */
function hook_ckeditor_settings_alter(&$settings, $conf) {
  // Change the ckeditor config path.
  $settings['customConfig'] = drupal_get_path('module', 'ckeditor') . '/ckeditor.config.js';
}

/**
 * Hook that allows to alter the user default settings.
 *
 * @param $settings
 *   An associative array of settings.
 */
function hook_ckeditor_default_settings_alter(&$settings) {
  $settings['show_toggle'] = 'f';
}

/**
 * Hook to extend CKEditor security allowed tags list.
 *
 * This hook is invoked from ckeditor_filter_xss() where text is filtered from potentially insecure tags.
 */
function hook_ckeditor_filter_xss_allowed_tags() {
  // Return an array of additional allowed tags
}