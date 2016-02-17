<?php

/**
 * @file
 * Editor integration functions for FCKeditor.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_fckeditor_editor() {
  $editor['fckeditor'] = array(
    'title' => 'FCKeditor',
    'vendor url' => 'http://www.fckeditor.net',
    'download url' => 'http://www.fckeditor.net/download',
    'libraries' => array(
      '' => array(
        'title' => 'Default',
        'files' => array('fckeditor.js'),
      ),
    ),
    'version callback' => 'wysiwyg_fckeditor_version',
    'themes callback' => 'wysiwyg_fckeditor_themes',
    'settings form callback' => 'wysiwyg_fckeditor_settings_form',
    'settings callback' => 'wysiwyg_fckeditor_settings',
    'plugin callback' => '_wysiwyg_fckeditor_plugins',
    'plugin meta callback' => '_wysiwyg_fckeditor_plugin_meta',
    'plugin settings callback' => '_wysiwyg_fckeditor_plugin_settings',
    'proxy plugin' => array(
      'drupal' => array(
        'load' => TRUE,
        'proxy' => TRUE,
      ),
    ),
    'proxy plugin settings callback' => '_wysiwyg_fckeditor_proxy_plugin_settings',
    'versions' => array(
      '2.6' => array(
        'js files' => array('fckeditor-2.6.js'),
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
function wysiwyg_fckeditor_version($editor) {
  $library = $editor['library path'] . '/fckeditor.js';
  if (!file_exists($library)) {
    return;
  }
  $library = fopen($library, 'r');
  $max_lines = 100;
  while ($max_lines && $line = fgets($library, 60)) {
    if (preg_match('@^FCKeditor.prototype.Version\s*= \'([\d\.]+)@', $line, $version)) {
      fclose($library);
      return $version[1];
    }
    $max_lines--;
  }
  fclose($library);
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
function wysiwyg_fckeditor_themes($editor, $profile) {
  return array('default', 'office2003', 'silver');
}

/**
 * Enhances the editor profile settings form for FCKeditor.
 *
 * @see http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options
 */
function wysiwyg_fckeditor_settings_form(&$form, &$form_state) {
  $profile = $form_state['wysiwyg_profile'];
  $settings = $profile->settings;
  $settings += array(
    'AutoDetectPasteFromWord' => TRUE,
    'ForcePasteAsPlainText' => FALSE,
    'FontFormats' => 'p;address;pre;h2;h3;h4;h5;h6;div',
    'FormatOutput' => TRUE,
    'FormatSource' => TRUE,
  );
  $form['output']['FormatSource'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply source formatting'),
    '#default_value' => $settings['FormatSource'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will re-format the HTML source code when switching to Source View.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'FormatSource', '@url' => url('http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options/FormatSource'))),
  );
  $form['output']['FormatOutput'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply output formatting'),
    '#default_value' => $settings['FormatOutput'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will re-format the HTML source code output. Disabling this option could avoid conflicts with other input filters.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'FormatOutput', '@url' => url('http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options/FormatOutput'))),
  );
  $form['css']['FontFormats'] = array(
    '#type' => 'textfield',
    '#title' => t('Block formats'),
    '#default_value' => $settings['FontFormats'],
    '#size' => 40,
    '#maxlength' => 250,
    '#description' => t('Semicolon separated list of HTML block formats. Possible values: <code>@format-list</code>.', array('@format-list' => 'p;h1;h2;h3;h4;h5;h6;div;address;pre')) . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'FontFormats', '@url' => url('http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options/FontFormats'))),
  );
  $form['paste'] = array(
    '#type' => 'fieldset',
    '#title' => t('Paste plugin'),
    '#description' => t('Settings for the paste plugin.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );
  $form['paste']['AutoDetectPasteFromWord'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto detect paste from Word'),
    '#default_value' => $settings['AutoDetectPasteFromWord'],
    '#return_value' => 1,
    '#description' => t('If enabled, FCKeditor checks if pasted text comes from MS Word. If so the editor will launch the "Paste from Word" window. <strong>Only works in Internet Explorer.</strong>') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'AutoDetectPasteFromWord', '@url' => url('http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options/AutoDetectPasteFromWord'))),
  );
  $form['paste']['ForcePasteAsPlainText'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force paste as plain text'),
    '#default_value' => $settings['ForcePasteAsPlainText'],
    '#return_value' => 1,
    '#description' => t('If enabled, forces the editor to discard all formatting when pasting text. It will also disable the <strong>Paste from Word</strong> operation.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'ForcePasteAsPlainText', '@url' => url('http://docs.cksource.com/FCKeditor_2.x/Developers_Guide/Configuration/Configuration_Options/ForcePasteAsPlainText'))),
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
function wysiwyg_fckeditor_settings($editor, $config, $theme) {
  $settings = array(
    'EditorPath' => base_path() . $editor['library path'] . '/',
    'SkinPath' => base_path() . $editor['library path'] . '/editor/skins/' . $theme . '/',
    'CustomConfigurationsPath' => base_path() . drupal_get_path('module', 'wysiwyg') . '/editors/js/fckeditor.config.js',
    'Width' => '100%',
    'LinkBrowser' => FALSE,
    'LinkUpload' => FALSE,
    'ImageBrowser' => FALSE,
    'ImageUpload' => FALSE,
    'FlashBrowser' => FALSE,
    'FlashUpload' => FALSE,
    // By default, FCKeditor converts most characters into HTML entities. Since
    // it does not support a custom definition, but Drupal supports Unicode, we
    // disable at least the additional character sets. FCKeditor always converts
    // XML default characters '&', '<', '>'.
    // @todo Check whether completely disabling ProcessHTMLEntities is an option.
    'IncludeLatinEntities' => FALSE,
    'IncludeGreekEntities' => FALSE,
  );
  if (isset($config['FontFormats'])) {
    $settings['FontFormats'] = preg_replace('@\s+@', '', $config['FontFormats']);
  }
  if (isset($config['FormatOutput'])) {
    $settings['FormatOutput'] = $config['FormatOutput'];
  }
  if (isset($config['FormatSource'])) {
    $settings['FormatSource'] = $config['FormatSource'];
  }
  if (isset($config['ForcePasteAsPlainText'])) {
    $settings['ForcePasteAsPlainText'] = $config['ForcePasteAsPlainText'];
  }
  if (isset($config['AutoDetectPasteFromWord'])) {
    $settings['AutoDetectPasteFromWord'] = $config['AutoDetectPasteFromWord'];
  }

  if (isset($config['css_setting'])) {
    if ($config['css_setting'] == 'theme') {
      $settings['EditorAreaCSS'] = implode(',', wysiwyg_get_css());
    }
    elseif ($config['css_setting'] == 'self' && isset($config['css_path'])) {
      $settings['EditorAreaCSS'] = strtr($config['css_path'], array('%b' => base_path(), '%t' => drupal_get_path('theme', variable_get('theme_default', NULL))));
    }
  }

  // Use our custom toolbar set.
  $settings['ToolbarSet'] = 'Wysiwyg';
  // Populate our custom toolbar set for fckeditor.config.js.
  $settings['buttons'] = array();
  if (!empty($config['buttons'])) {
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
            $settings['buttons'][] = $button;
          }
          // Allow plugins to add or override global configuration settings.
          if (!empty($plugins[$plugin]['options'])) {
            $settings = array_merge($settings, $plugins[$plugin]['options']);
          }
        }
      }
    }
  }
  // For now, all buttons are placed into one row.
  $settings['buttons'] = array($settings['buttons']);

  return $settings;
}

/**
 * Build a JS settings array with global metadata for native external plugins.
 */
function _wysiwyg_fckeditor_plugin_meta($editor, $plugin) {
  $meta = array();
  // Add path for native external plugins; internal ones do not need a path.
  if (empty($plugin['internal']) && isset($plugin['path'])) {
    // All native FCKeditor plugins use the filename fckplugin.js.
    $meta['path'] = base_path() . $plugin['path'] . '/';
  }
  if (!empty($plugin['languages'])) {
    $meta['languages'] = $plugin['languages'];
  }
  return $meta;
}

/**
 * Build a JS settings array for native external plugins.
 */
function _wysiwyg_fckeditor_plugin_settings($editor, $profile, $plugins) {
  $settings = array();
  foreach ($plugins as $name => $plugin) {
    if (!empty($plugin['load'])) {
      // Just need a list of all enabled plugins for each instance.
      $settings[$name] = TRUE;
    }
  }
  return $settings;
}

/**
 * Build a JS settings array for Drupal plugins loaded via the proxy plugin.
 */
function _wysiwyg_fckeditor_proxy_plugin_settings($editor, $profile, $plugins) {
  $settings = array();
  foreach ($plugins as $name => $plugin) {
    // Just need a list of all enabled plugins for each instance.
    $settings[$name] = TRUE;
  }
  return $settings;
}


/**
 * Return internal plugins for this editor; semi-implementation of hook_wysiwyg_plugin().
 */
function _wysiwyg_fckeditor_plugins($editor) {
  $plugins = array(
    'default' => array(
      'buttons' => array(
        'Bold' => t('Bold'), 'Italic' => t('Italic'), 'Underline' => t('Underline'),
        'StrikeThrough' => t('Strike-through'),
        'JustifyLeft' => t('Align left'), 'JustifyCenter' => t('Align center'), 'JustifyRight' => t('Align right'), 'JustifyFull' => t('Justify'),
        'UnorderedList' => t('Bullet list'), 'OrderedList' => t('Numbered list'),
        'Outdent' => t('Outdent'), 'Indent' => t('Indent'),
        'Undo' => t('Undo'), 'Redo' => t('Redo'),
        'Link' => t('Link'), 'Unlink' => t('Unlink'), 'Anchor' => t('Anchor'),
        'Image' => t('Image'),
        'TextColor' => t('Forecolor'), 'BGColor' => t('Backcolor'),
        'Superscript' => t('Superscript'), 'Subscript' => t('Subscript'),
        'Blockquote' => t('Blockquote'), 'Source' => t('Source code'),
        'Rule' => t('Horizontal rule'),
        'Cut' => t('Cut'), 'Copy' => t('Copy'), 'Paste' => t('Paste'),
        'PasteText' => t('Paste Text'), 'PasteWord' => t('Paste from Word'),
        'ShowBlocks' => t('Show blocks'),
        'RemoveFormat' => t('Remove format'),
        'SpecialChar' => t('Character map'),
        'About' => t('About'),
        'FontFormat' => t('HTML block format'), 'FontName' => t('Font'), 'FontSize' => t('Font size'), 'Style' => t('Font style'),
        'Table' => t('Table'),
        'Find' => t('Search'), 'Replace' => t('Replace'), 'SelectAll' => t('Select all'),
        'CreateDiv' => t('Create DIV container'),
        'Flash' => t('Flash'), 'Smiley' => t('Smiley'),
        'FitWindow' => t('FitWindow'),
        'SpellCheck' => t('Check spelling'),
      ),
      'internal' => TRUE,
    ),
    'autogrow' => array(
      'path' => $editor['library path'] . '/editor/plugins',
      'extensions' => array(
        'autogrow' => t('Autogrow'),
      ),
      'options' => array(
        'AutoGrowMax' => 800,
      ),
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'bbcode' => array(
      'path' => $editor['library path'] . '/editor/plugins',
      'extensions' => array(
        'bbcode' => t('BBCode'),
      ),
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'dragresizetable' => array(
      'path' => $editor['library path'] . '/editor/plugins',
      'extensions' => array(
        'dragresizetable' => t('Table drag/resize'),
      ),
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'tablecommands' => array(
      'path' => $editor['library path'] . '/editor/plugins',
      'buttons' => array(
        'TableCellProp' => t('Table: Cell properties'),
        'TableInsertRowAfter' => t('Table: Insert row after'),
        'TableInsertColumnAfter' => t('Table: Insert column after'),
        'TableInsertCellAfter' => t('Table: Insert cell after'),
        'TableDeleteRows' => t('Table: Delete rows'),
        'TableDeleteColumns' => t('Table: Delete columns'),
        'TableDeleteCells' => t('Table: Delete cells'),
        'TableMergeCells' => t('Table: Merge cells'),
        'TableHorizontalSplitCell' => t('Table: Horizontal split cell'),
      ),
      'internal' => TRUE,
      'load' => TRUE,
    ),
  );
  return $plugins;
}

