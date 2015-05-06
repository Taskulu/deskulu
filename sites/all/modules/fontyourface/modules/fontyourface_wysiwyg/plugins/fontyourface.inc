<?php
// $Id$

/**
 * @file
 * Wysiwyg API integration for @font-your-face module.
 */

/**
 * Implementation of hook_INCLUDE_plugin().
 */
function fontyourface_wysiwyg_fontyourface_plugin() {

  $plugins = array();

  $fonts = fontyourface_get_fonts();
  
  $fonts_setting = array();
  
  foreach ($fonts as $font) {

    $fonts_setting[] = array(
      'fid' => $font->fid,
      'name' => $font->name,
    );

  } // foreach

  $plugins['fontyourface'] = array(
    'title' => t('@font-your-face'),
    'vendor url' => 'http://drupal.org/project/fontyourface',
    'icon file' => 'fontyourface.jpg',
    'icon title' => t('Set font'),
    'css path' => 'sites/default/files/fontyourface',
    'css file' => 'wysiwyg.css',
    'settings' => array(
      'fonts' => $fonts_setting,
    ),
  );

  return $plugins;

} // fontyourface_wysiwyg_fontyourface_plugin

