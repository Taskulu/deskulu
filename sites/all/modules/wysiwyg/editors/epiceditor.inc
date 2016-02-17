<?php

/**
 * @file
 * Editor integration functions for EpicEditor.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_epiceditor_editor() {
  $editor['epiceditor'] = array(
    'title' => 'EpicEditor',
    'vendor url' => 'http://epiceditor.com',
    'download url' => 'http://epiceditor.com',
    'libraries' => array(
      '' => array(
        'title' => 'Minified',
        'files' => array('js/epiceditor.min.js'),
      ),
      'src' => array(
        'title' => 'Source',
        'files' => array('js/epiceditor.js'),
      ),
    ),
    'version callback' => 'wysiwyg_epiceditor_version',
    'themes callback' => 'wysiwyg_epiceditor_themes',
    'settings form callback' => 'wysiwyg_epiceditor_settings_form',
    'settings callback' => 'wysiwyg_epiceditor_settings',
    'versions' => array(
      '0.1.1' => array(
        'js files' => array('epiceditor.js'),
      ),
    ),
  );
  return $editor;
}

/**
 * Detect editor version.
 *
 * @param $editor
 *   An array containing editor properties as returned from hook_editor().
 *
 * @return
 *   The installed editor version.
 */
function wysiwyg_epiceditor_version($editor) {
  $library = $editor['library path'] . '/js/epiceditor.js';
  if (!file_exists($library)) {
    return;
  }
  // @todo Do not load the entire file; use fgets() instead.
  $library = file_get_contents($library, 'r');
  $version = preg_match('%EpicEditor\.version = \'(.*)\'\;%', $library, $matches);
  if (!isset($matches[1])) {
    return;
  }
  return $matches[1];
}

/**
 * Determine available editor themes or check/reset a given one.
 *
 * @param $editor
 *   A processed hook_editor() array of editor properties.
 * @param $profile
 *   A wysiwyg editor profile.
 *
 * @return
 *   An array of theme names. The first returned name should be the default
 *   theme name.
 */
function wysiwyg_epiceditor_themes($editor, $profile) {
  return array('epic-dark', 'epic-light');
  // @todo Use the preview themes somewhere.
  //return array('preview-dark', 'github');
}

/**
 * Enhances the editor profile settings form for EpicEditor.
 *
 */
function wysiwyg_epiceditor_settings_form(&$form, &$form_state) {
  $form['buttons']['#access'] = FALSE;
  $form['basic']['language']['#access'] = FALSE;
  $form['css']['#access'] = FALSE;
}

/**
 * Return runtime editor settings for a given wysiwyg profile.
 *
 * @param $editor
 *   A processed hook_editor() array of editor properties.
 * @param $config
 *   An array containing wysiwyg editor profile settings.
 * @param $theme
 *   The name of a theme/GUI/skin to use.
 *
 * @return
 *   A settings array to be populated in
 *   Drupal.settings.wysiwyg.configs.{editor}
 */
function wysiwyg_epiceditor_settings($editor, $config, $theme) {
  $settings = array(
    'basePath' => base_path() . $editor['library path'],
    'clientSideStorage' => FALSE,
    'theme' => $theme,
    //'preview_theme' => '',
  );
  return $settings;
}

