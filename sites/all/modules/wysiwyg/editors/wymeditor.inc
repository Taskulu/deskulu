<?php

/**
 * @file
 * Editor integration functions for WYMeditor.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_wymeditor_editor() {
  $editor['wymeditor'] = array(
    'title' => 'WYMeditor',
    'vendor url' => 'http://www.wymeditor.org/',
    'download url' => 'http://www.wymeditor.org/download/',
    'library path' => wysiwyg_get_path('wymeditor') . '/wymeditor',
    'libraries' => array(
      'min' => array(
        'title' => 'Minified',
        'files' => array('jquery.wymeditor.min.js'),
      ),
      'pack' => array(
        'title' => 'Packed',
        'files' => array('jquery.wymeditor.pack.js'),
      ),
      'src' => array(
        'title' => 'Source',
        'files' => array('jquery.wymeditor.js'),
      ),
    ),
    'version callback' => 'wysiwyg_wymeditor_version',
    'themes callback' => 'wysiwyg_wymeditor_themes',
    'settings form callback' => 'wysiwyg_wymeditor_settings_form',
    'settings callback' => 'wysiwyg_wymeditor_settings',
    'plugin callback' => '_wysiwyg_wymeditor_plugins',
    'versions' => array(
      '0.5-rc1' => array(
        'js files' => array('wymeditor.js'),
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
function wysiwyg_wymeditor_version($editor) {
  $script = $editor['library path'] . '/jquery.wymeditor.js';
  if (!file_exists($script)) {
    return;
  }
  $script = fopen($script, 'r');
  fgets($script);
  $line = fgets($script);
  if (preg_match('@version\s+([0-9a-z\.-]+)@', $line, $version)) {
    fclose($script);
    return $version[1];
  }
  fclose($script);
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
function wysiwyg_wymeditor_themes($editor, $profile) {
  return array('compact', 'default', 'minimal', 'silver', 'twopanels');
}

/**
 * Enhances the editor profile settings form for WYMeditor.
 *
 * @see http://wymeditor.readthedocs.org/en/latest/version_1.0_and_0.5/getting_started/customize.html
 */
function wysiwyg_wymeditor_settings_form(&$form, &$form_state) {
  $profile = $form_state['wysiwyg_profile'];
  $settings = $profile->settings;
  $settings += array(
    'block_formats' => 'p,blockquote,pre,h2,h3,h4,h5,h6,div',
  );

  $form['css']['#description'] = t('Note: WYMeditor can only load a single stylesheet into the editor.');

  $form['css']['block_formats'] = array(
    '#type' => 'textfield',
    '#title' => t('Block formats'),
    '#default_value' => $settings['block_formats'],
    '#size' => 40,
    '#maxlength' => 250,
    '#description' => t('Comma separated list of HTML block formats. Possible values: <code>@format-list</code>.', array('@format-list' => 'p,h1,h2,h3,h4,h5,h6,blockquote,pre,th')),
  );
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
function wysiwyg_wymeditor_settings($editor, $config, $theme) {
  // @todo Setup $library in wysiwyg_load_editor() already.
  $library = (isset($editor['library']) ? $editor['library'] : key($editor['libraries']));
  $settings = array(
    'basePath' => base_path() . $editor['library path'] . '/',
    'wymPath' => $editor['libraries'][$library]['files'][0],
    // @todo Does not work in Drupal; jQuery can live anywhere.
    'jQueryPath' => base_path() . 'misc/jquery.js',
    // WYMeditor's update event handler will revert the field contents if
    // changes were made after it was detached. Wysiwyg takes care of submit
    // events anyway so make sure WYMeditor does not bind it anywhere.
    'updateSelector' => '#wysiwyg-no-element',
    'updateEvent' => 'wysiwyg-no-event',
    'skin' => $theme,
  );

  if (isset($config['language'])) {
    $settings['lang'] = $config['language'];
  }

  // Add configured buttons.
  $settings['toolsItems'] = array();
  if (!empty($config['buttons'])) {
    $buttoninfo = _wysiwyg_wymeditor_button_info();
    $plugins = wysiwyg_get_plugins($editor['name']);
    foreach ($config['buttons'] as $plugin => $buttons) {
      foreach ($buttons as $button => $enabled) {
        // Iterate separately over buttons and extensions properties.
        foreach (array('buttons', 'extensions') as $type) {
          // Skip unavailable plugins.
          if (!isset($plugins[$plugin][$type][$button])) {
            continue;
          }
          // Add buttons.
          if ($type == 'buttons') {
            // Merge meta-data for internal default buttons.
            if (isset($buttoninfo[$button])) {
              $buttoninfo[$button] += array('name' => $button);
              $settings['toolsItems'][] = $buttoninfo[$button];
            }
            // For custom buttons, try to provide a valid button definition.
            else {
              $settings['toolsItems'][] = array(
                'name' => $button,
                'title' => $plugins[$plugin][$type][$button],
                'css' => 'wym_tools_' . $button,
              );
            }
          }
        }
      }
    }
  }

  if (!empty($config['block_formats'])) {
    $containers = array(
      'p' => 'Paragraph',
      'h1' => 'Heading_1',
      'h2' => 'Heading_2',
      'h3' => 'Heading_3',
      'h4' => 'Heading_4',
      'h5' => 'Heading_5',
      'h6' => 'Heading_6',
      'pre' => 'Preformatted',
      'blockquote' => 'Blockquote',
      'th' => 'Table_Header',
    );
    foreach (explode(',', preg_replace('@\s+@', '', $config['block_formats'])) as $tag) {
      if (isset($containers[$tag])) {
        $settings['containersItems'][] = array(
          'name' => strtoupper($tag),
          'title' => $containers[$tag],
          'css' => 'wym_containers_' . $tag,
        );
      }
    }
  }

  // Add editor content stylesheet.
  if (isset($config['css_setting'])) {
    if ($config['css_setting'] == 'theme') {
      // WYMeditor only supports one CSS file currently.
      $css = wysiwyg_get_css();
      $settings['stylesheet'] = reset($css);
    }
    elseif ($config['css_setting'] == 'self' && isset($config['css_path'])) {
      $settings['stylesheet'] = strtr($config['css_path'], array('%b' => base_path(), '%t' => drupal_get_path('theme', variable_get('theme_default', NULL))));
    }
  }

  return $settings;
}

/**
 * Return internal plugins for this editor; semi-implementation of hook_wysiwyg_plugin().
 */
function _wysiwyg_wymeditor_plugins($editor) {
  $plugins = array(
    'default' => array(
      'buttons' => array(
        'Bold' => t('Bold'), 'Italic' => t('Italic'),
        'InsertOrderedList' => t('Numbered list'), 'InsertUnorderedList' => t('Bullet list'),
        'Outdent' => t('Outdent'), 'Indent' => t('Indent'),
        'Undo' => t('Undo'), 'Redo' => t('Redo'),
        'CreateLink' => t('Link'), 'Unlink' => t('Unlink'),
        'InsertImage' => t('Image'),
        'Superscript' => t('Superscript'), 'Subscript' => t('Subscript'),
        'ToggleHtml' => t('Source code'),
        'Paste' => t('Paste'),
        'InsertTable' => t('Table'),
        'Preview' => t('Preview'),
      ),
      'internal' => TRUE,
    ),
  );
  return $plugins;
}

/**
 * Helper function to provide additional meta-data for internal default buttons.
 */
function _wysiwyg_wymeditor_button_info() {
  return array(
    'Bold' => array('title' => 'Strong', 'css' => 'wym_tools_strong'),
    'Italic' => array('title' => 'Emphasis', 'css' => 'wym_tools_emphasis'),
    'Superscript' => array('title' => 'Superscript', 'css' => 'wym_tools_superscript'),
    'Subscript' => array('title' => 'Subscript', 'css' => 'wym_tools_subscript'),
    'InsertOrderedList' => array('title' => 'Ordered_List', 'css' => 'wym_tools_ordered_list'),
    'InsertUnorderedList' => array('title' => 'Unordered_List', 'css' => 'wym_tools_unordered_list'),
    'Indent' => array('title' => 'Indent', 'css' => 'wym_tools_indent'),
    'Outdent' => array('title' => 'Outdent', 'css' => 'wym_tools_outdent'),
    'Undo' => array('title' => 'Undo', 'css' => 'wym_tools_undo'),
    'Redo' => array('title' => 'Redo', 'css' => 'wym_tools_redo'),
    'CreateLink' => array('title' => 'Link', 'css' => 'wym_tools_link'),
    'Unlink' => array('title' => 'Unlink', 'css' => 'wym_tools_unlink'),
    'InsertImage' => array('title' => 'Image', 'css' => 'wym_tools_image'),
    'InsertTable' => array('title' => 'Table', 'css' => 'wym_tools_table'),
    'Paste' => array('title' => 'Paste_From_Word', 'css' => 'wym_tools_paste'),
    'ToggleHtml' => array('title' => 'HTML', 'css' => 'wym_tools_html'),
    'Preview' => array('title' => 'Preview', 'css' => 'wym_tools_preview'),
  );
}
