<?php

/**
 * @file
 * Editor integration functions for jWYSIWYG.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_jwysiwyg_editor() {
  $editor['jwysiwyg'] = array(
    'title' => 'jWYSIWYG',
    'vendor url' => 'http://github.com/akzhan/jwysiwyg',
    'download url' => 'http://github.com/akzhan/jwysiwyg/tags',
    'libraries' => array(
      '' => array(
        'title' => 'Source',
        'files' => array('jquery.wysiwyg.js'),
      ),
    ),
    'version callback' => 'wysiwyg_jwysiwyg_version',
    'settings form callback' => 'wysiwyg_jwysiwyg_settings_form',
    // @todo Wrong property; add separate properties for editor requisites.
    'css path' => wysiwyg_get_path('jwysiwyg'),
    'versions' => array(
      '0.97' => array(
        'js files' => array('jwysiwyg.js'),
        'css files' => array('jquery.wysiwyg.css'),
      ),
    ),
  );
  return $editor;
}

/**
 * Enhances the editor profile settings form for jWYSIWYG.
 */
function wysiwyg_jwysiwyg_settings_form(&$form, &$form_state) {
  $form['buttons']['#access'] = FALSE;
  $form['basic']['language']['#access'] = FALSE;
  $form['css']['#access'] = FALSE;
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
function wysiwyg_jwysiwyg_version($editor) {
  $script = $editor['library path'] . '/jquery.wysiwyg.js';
  if (!file_exists($script)) {
    return;
  }
  $script = fopen($script, 'r');
  fgets($script);
  $line = fgets($script);
  if (preg_match('@([0-9\.]+)$@', $line, $version)) {
    fclose($script);
    return $version[1];
  }
  fclose($script);
}

