<?php

/**
 * @file
 * Editor integration functions for CKEditor.
 */

/**
 * Plugin implementation of hook_editor().
 */
function wysiwyg_ckeditor_editor() {
  $editor['ckeditor'] = array(
    'title' => 'CKEditor',
    'vendor url' => 'http://ckeditor.com',
    'download url' => 'http://ckeditor.com/download',
    'libraries' => array(
      '' => array(
        'title' => 'Default',
        'files' => array(
          'ckeditor.js' => array('preprocess' => FALSE),
        ),
      ),
      'src' => array(
        'title' => 'Source',
        'files' => array(
          'ckeditor_source.js' => array('preprocess' => FALSE),
        ),
      ),
    ),
    'install note callback' => 'wysiwyg_ckeditor_install_note',
    'version callback' => 'wysiwyg_ckeditor_version',
    'themes callback' => 'wysiwyg_ckeditor_themes',
    'settings form callback' => 'wysiwyg_ckeditor_settings_form',
    'init callback' => 'wysiwyg_ckeditor_init',
    'settings callback' => 'wysiwyg_ckeditor_settings',
    'plugin callback' => 'wysiwyg_ckeditor_plugins',
    'plugin settings callback' => 'wysiwyg_ckeditor_plugin_settings',
    'proxy plugin' => array(
      'drupal' => array(
        'load' => TRUE,
        'proxy' => TRUE,
      ),
    ),
    'proxy plugin settings callback' => 'wysiwyg_ckeditor_proxy_plugin_settings',
    'versions' => array(
      '3.0.0.3665' => array(
        'js files' => array('ckeditor-3.0.js'),
      ),
    ),
  );
  return $editor;
}

/**
 * Return an install note.
 */
function wysiwyg_ckeditor_install_note() {
  return '<p class="warning">' . t('Do NOT download the "CKEditor for Drupal" edition.') . '</p>';
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
function wysiwyg_ckeditor_version($editor) {
  $library = $editor['library path'] . '/ckeditor.js';
  if (!file_exists($library)) {
    return;
  }
  $library = fopen($library, 'r');
  $max_lines = 8;
  while ($max_lines && $line = fgets($library, 500)) {
    // version:'CKEditor 3.0 SVN',revision:'3665'
    // version:'3.0 RC',revision:'3753'
    // version:'3.0.1',revision:'4391'
    if (preg_match('@version:\'(?:CKEditor )?([\d\.]+)(?:.+revision:\'([\d]+))?@', $line, $version)) {
      fclose($library);
      // Version numbers need to have three parts since 3.0.1.
      $version[1] = preg_replace('/^(\d+)\.(\d+)$/', '${1}.${2}.0', $version[1]);
      return $version[1] . '.' . $version[2];
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
function wysiwyg_ckeditor_themes($editor, $profile) {
  // @todo Skins are not themes but this will do for now.
  $path = $editor['library path'] . '/skins/';
  if (file_exists($path) && ($dir_handle = opendir($path))) {
    $themes = array();
    while ($file = readdir($dir_handle)) {
      if (is_dir($path . $file) && substr($file, 0, 1) != '.' && $file != 'CVS') {
        $themes[] = $file;
      }
    }
    closedir($dir_handle);
    natcasesort($themes);
    $themes = array_values($themes);
    return !empty($themes) ? $themes : array('default');
  }
  else {
    return array('default');
  }
}

/**
 * Enhances the editor profile settings form for CKEditor.
 *
 * Adds support for CKEditor's advanced stylesSets, which are a more advanced
 * implementation and combination of block formats and font styles that allow
 * to adjust the HTML element, attributes, and CSS styles at once.
 *
 * @see http://docs.cksource.com/CKEditor_3.x/Developers_Guide/Styles
 * @see http://docs.cksource.com/ckeditor_api/symbols/CKEDITOR.config.html#.stylesSet
 */
function wysiwyg_ckeditor_settings_form(&$form, &$form_state) {
  if (version_compare($form_state['wysiwyg']['editor']['installed version'], '3.2.1', '>=')) {
    // Replace CSS classes element description to explain the advanced syntax.
    $form['css']['css_classes']['#description'] = t('Optionally define CSS classes for the "Font style" dropdown list.<br />Enter one class on each line in the format: !format. Example: !example<br />If left blank, CSS classes are automatically imported from loaded stylesheet(s).', array(
      '!format' => '<code>[label]=[element].[class]</code>',
      '!example' => '<code>Title=h1.title</code>',
    ));
    $form['css']['css_classes']['#element_validate'][] = 'wysiwyg_ckeditor_settings_form_validate_css_classes';
  }
  else {
    // Versions below 3.2.1 do not support Font styles at all.
    $form['css']['css_classes']['#access'] = FALSE;
  }
}

/**
 * #element_validate handler for CSS classes element altered by wysiwyg_ckeditor_settings_form().
 */
function wysiwyg_ckeditor_settings_form_validate_css_classes($element, &$form_state) {
  if (wysiwyg_ckeditor_settings_parse_styles($element['#value']) === FALSE) {
    form_error($element, t('The specified CSS classes are syntactically incorrect.'));
  }
}

/**
 * Returns an initialization JavaScript for this editor library.
 *
 * @param array $editor
 *   The editor library definition.
 * @param string $library
 *   The library variant key from $editor['libraries'].
 * @param object $profile
 *   The (first) wysiwyg editor profile.
 *
 * @return string
 *   A string containing inline JavaScript to execute before the editor library
 *   script is loaded.
 */
function wysiwyg_ckeditor_init($editor) {
  // CKEditor unconditionally searches for its library filename in SCRIPT tags
  // on the page upon loading the library in order to determine the base path to
  // itself. When JavaScript aggregation is enabled, this search fails and all
  // relative constructed paths within CKEditor are broken. The library has a
  // CKEditor.basePath property, but it is not publicly documented and thus not
  // reliable. The official documentation suggests to solve the issue through
  // the global window variable.
  // @see http://docs.cksource.com/CKEditor_3.x/Developers_Guide/Specifying_the_Editor_Path
  $library_path = base_path() . $editor['library path'] . '/';
  return <<<EOL
window.CKEDITOR_BASEPATH = '$library_path';
EOL;
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
function wysiwyg_ckeditor_settings($editor, $config, $theme) {
  $settings = array(
    'width' => 'auto',
    // For better compatibility with smaller textareas.
    'resize_minWidth' => 450,
    'height' => 420,
    // @todo Do not use skins as themes and add separate skin handling.
    'theme' => 'default',
    'skin' => !empty($theme) ? $theme : 'kama',
    // By default, CKEditor converts most characters into HTML entities. Since
    // it does not support a custom definition, but Drupal supports Unicode, we
    // disable at least the additional character sets. CKEditor always converts
    // XML default characters '&', '<', '>'.
    // @todo Check whether completely disabling ProcessHTMLEntities is an option.
    'entities_latin' => FALSE,
    'entities_greek' => FALSE,
  );

  // Add HTML block format settings; common block formats are already predefined
  // by CKEditor.
  if (isset($config['block_formats'])) {
    $block_formats = explode(',', drupal_strtolower($config['block_formats']));
    $predefined_formats = array('h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'p', 'pre', 'address', 'div');
    foreach (array_diff($block_formats, $predefined_formats) as $tag) {
      $tag = trim($tag);
      $settings["format_$tag"] = array('element' => $tag);
    }
    $settings['format_tags'] = implode(';', $block_formats);
  }

  if (isset($config['apply_source_formatting'])) {
    $settings['apply_source_formatting'] = $config['apply_source_formatting'];
  }

  if (isset($config['css_setting'])) {
    // Versions below 3.0.1 could only handle one stylesheet.
    if (version_compare($editor['installed version'], '3.0.1.4391', '<')) {
      if ($config['css_setting'] == 'theme') {
        $settings['contentsCss'] = reset(wysiwyg_get_css());
      }
      elseif ($config['css_setting'] == 'self' && isset($config['css_path'])) {
        $settings['contentsCss'] = strtr($config['css_path'], array('%b' => base_path(), '%t' => drupal_get_path('theme', variable_get('theme_default', NULL))));
      }
    }
    else {
      if ($config['css_setting'] == 'theme') {
        $settings['contentsCss'] = wysiwyg_get_css();
      }
      elseif ($config['css_setting'] == 'self' && isset($config['css_path'])) {
        $settings['contentsCss'] = explode(',', strtr($config['css_path'], array('%b' => base_path(), '%t' => drupal_get_path('theme', variable_get('theme_default', NULL)))));
      }
    }
  }

  // Parse and define the styles set for the Styles plugin (3.2.1+).
  // @todo This should be a plugin setting, but Wysiwyg does not support
  //   plugin-specific settings yet.
  if (!empty($config['buttons']['default']['Styles']) && version_compare($editor['installed version'], '3.2.1', '>=')) {
    if ($styles = wysiwyg_ckeditor_settings_parse_styles($config['css_classes'])) {
      $settings['stylesSet'] = $styles;
    }
  }

  if (isset($config['language'])) {
    $settings['language'] = $config['language'];
  }
  if (isset($config['resizing'])) {
    // CKEditor performs a type-agnostic comparison on this particular setting.
    $settings['resize_enabled'] = (bool) $config['resizing'];
  }
  if (isset($config['toolbar_loc'])) {
    $settings['toolbarLocation'] = $config['toolbar_loc'];
  }

  $settings['toolbar'] = array();
  if (!empty($config['buttons'])) {
    $extra_plugins = array();
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
            $settings['toolbar'][] = $button;
          }
          // Add external Drupal plugins to the list of extensions.
          if ($type == 'buttons' && !empty($plugins[$plugin]['proxy'])) {
            $extra_plugins[] = $button;
          }
          // Add external plugins to the list of extensions.
          elseif ($type == 'buttons' && empty($plugins[$plugin]['internal'])) {
            $extra_plugins[] = $plugin;
          }
          // Add internal buttons that also need to be loaded as extension.
          elseif ($type == 'buttons' && !empty($plugins[$plugin]['load'])) {
            $extra_plugins[] = $plugin;
          }
          // Add plain extensions.
          elseif ($type == 'extensions' && !empty($plugins[$plugin]['load'])) {
            $extra_plugins[] = $plugin;
          }
          // Allow plugins to add or override global configuration settings.
          if (!empty($plugins[$plugin]['options'])) {
            $settings = array_merge($settings, $plugins[$plugin]['options']);
          }
        }
      }
    }
    if (!empty($extra_plugins)) {
      $settings['extraPlugins'] = implode(',', $extra_plugins);
    }
  }
  // For now, all buttons are placed into one row.
  $settings['toolbar'] = array($settings['toolbar']);

  return $settings;
}

/**
 * Parses CSS classes settings string into a stylesSet JavaScript settings array.
 *
 * @param string $css_classes
 *   A string containing CSS class definitions to add to the Style dropdown
 *   list, separated by newlines.
 *
 * @return array|false
 *   An array containing the parsed stylesSet definition, or FALSE on parse
 *   error.
 *
 * @see wysiwyg_ckeditor_settings_form()
 * @see wysiwyg_ckeditor_settings_form_validate_css_classes()
 *
 * @todo This should be a plugin setting, but Wysiwyg does not support
 *   plugin-specific settings yet.
 */
function wysiwyg_ckeditor_settings_parse_styles($css_classes) {
  $set = array();
  $input = trim($css_classes);
  if (empty($input)) {
    return $set;
  }
  // Handle both Unix and Windows line-endings.
  foreach (explode("\n", str_replace("\r", '', $input)) as $line) {
    $line = trim($line);
    // [label]=[element].[class][.[class]][...] pattern expected.
    if (!preg_match('@^.+= *[a-zA-Z0-9]+(\.[a-zA-Z0-9_ -]+)*$@', $line)) {
      return FALSE;
    }
    list($label, $selector) = explode('=', $line, 2);
    $classes = explode('.', $selector);
    $element = array_shift($classes);

    $style = array();
    $style['name'] = trim($label);
    $style['element'] = trim($element);
    if (!empty($classes)) {
      $style['attributes']['class'] = implode(' ', array_map('trim', $classes));
    }
    $set[] = $style;
  }
  return $set;
}

/**
 * Build a JS settings array of native external plugins that need to be loaded separately.
 */
function wysiwyg_ckeditor_plugin_settings($editor, $profile, $plugins) {
  $settings = array();
  foreach ($plugins as $name => $plugin) {
    // Register all plugins that need to be loaded.
    if (!empty($plugin['load'])) {
      $settings[$name] = array();
      // Add path for native external plugins.
      if (empty($plugin['internal']) && isset($plugin['path'])) {
        $settings[$name]['path'] = base_path() . $plugin['path'] . '/';
      }
      // Force native internal plugins to use the standard path.
      else {
        $settings[$name]['path'] = base_path() . $editor['library path'] . '/plugins/' . $name . '/';
      }
      // CKEditor defaults to 'plugin.js' on its own when filename is not set.
      if (!empty($plugin['filename'])) {
        $settings[$name]['fileName'] = $plugin['filename'];
      }
    }
  }
  return $settings;
}

/**
 * Build a JS settings array for Drupal plugins loaded via the proxy plugin.
 */
function wysiwyg_ckeditor_proxy_plugin_settings($editor, $profile, $plugins) {
  $settings = array();
  foreach ($plugins as $name => $plugin) {
    // Populate required plugin settings.
    $settings[$name] = $plugin['dialog settings'] + array(
      'title' => $plugin['title'],
      'icon' => base_path() . $plugin['icon path'] . '/' . $plugin['icon file'],
      'iconTitle' => $plugin['icon title'],
      // @todo These should only be set if the plugin defined them.
      'css' => base_path() . $plugin['css path'] . '/' . $plugin['css file'],
    );
  }
  return $settings;
}

/**
 * Return internal plugins for this editor; semi-implementation of hook_wysiwyg_plugin().
 */
function wysiwyg_ckeditor_plugins($editor) {
  $plugins = array(
    'default' => array(
      'buttons' => array(
        'Bold' => t('Bold'), 'Italic' => t('Italic'), 'Underline' => t('Underline'),
        'Strike' => t('Strike-through'),
        'JustifyLeft' => t('Align left'), 'JustifyCenter' => t('Align center'), 'JustifyRight' => t('Align right'), 'JustifyBlock' => t('Justify'),
        'BidiLtr' => t('Left-to-right'), 'BidiRtl' => t('Right-to-left'),
        'BulletedList' => t('Bullet list'), 'NumberedList' => t('Numbered list'),
        'Outdent' => t('Outdent'), 'Indent' => t('Indent'),
        'Undo' => t('Undo'), 'Redo' => t('Redo'),
        'Link' => t('Link'), 'Unlink' => t('Unlink'), 'Anchor' => t('Anchor'),
        'Image' => t('Image'),
        'TextColor' => t('Forecolor'), 'BGColor' => t('Backcolor'),
        'Superscript' => t('Superscript'), 'Subscript' => t('Subscript'),
        'Blockquote' => t('Blockquote'), 'Source' => t('Source code'),
        'HorizontalRule' => t('Horizontal rule'),
        'Cut' => t('Cut'), 'Copy' => t('Copy'), 'Paste' => t('Paste'),
        'PasteText' => t('Paste Text'), 'PasteFromWord' => t('Paste from Word'),
        'ShowBlocks' => t('Show blocks'),
        'RemoveFormat' => t('Remove format'),
        'SpecialChar' => t('Character map'),
        'Format' => t('HTML block format'), 'Font' => t('Font'), 'FontSize' => t('Font size'), 'Styles' => t('Font style'),
        'Table' => t('Table'),
        'SelectAll' => t('Select all'), 'Find' => t('Search'), 'Replace' => t('Replace'),
        'Flash' => t('Flash'), 'Smiley' => t('Smiley'),
        'CreateDiv' => t('Div container'),
        'Iframe' => t('iFrame'),
        'Maximize' => t('Maximize'),
        'SpellChecker' => t('Check spelling'), 'Scayt' => t('Check spelling as you type'),
        'About' => t('About'),
      ),
      'internal' => TRUE,
    ),
  );

  if (version_compare($editor['installed version'], '3.1.0.4885', '<')) {
    unset($plugins['default']['buttons']['CreateDiv']);
  }
  if (version_compare($editor['installed version'], '3.4.0.5808', '<')) {
    unset($plugins['default']['buttons']['BidiLtr']);
    unset($plugins['default']['buttons']['BidiRtl']);
  }
  if (version_compare($editor['installed version'], '3.5.0.6260', '<')) {
    unset($plugins['default']['buttons']['Iframe']);
  }
  return $plugins;
}

