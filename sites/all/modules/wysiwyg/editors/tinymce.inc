<?php

/**
 * @file
 * Editor integration functions for TinyMCE.
 */

/**
 * Plugin implementation of hook_editor().
 *
 * @todo wysiwyg_<editor>_alter() to add/inject optional libraries like gzip.
 */
function wysiwyg_tinymce_editor() {
  $editor['tinymce'] = array(
    'title' => 'TinyMCE',
    'vendor url' => 'http://www.tinymce.com',
    'download url' => 'http://www.tinymce.com/download/download.php',
    'library path' => wysiwyg_get_path('tinymce') . '/jscripts/tiny_mce',
    'libraries' => array(
      '' => array(
        'title' => 'Minified',
        'files' => array('tiny_mce.js'),
      ),
      'src' => array(
        'title' => 'Source',
        'files' => array('tiny_mce_src.js'),
      ),
    ),
    'version callback' => 'wysiwyg_tinymce_version',
    'themes callback' => 'wysiwyg_tinymce_themes',
    'settings form callback' => 'wysiwyg_tinymce_settings_form',
    'init callback' => 'wysiwyg_tinymce_init',
    'settings callback' => 'wysiwyg_tinymce_settings',
    'plugin callback' => '_wysiwyg_tinymce_plugins',
    'plugin meta callback' => '_wysiwyg_tinymce_plugin_meta',
    'proxy plugin' => array(
      'drupal' => array(
        'load' => TRUE,
        'proxy' => TRUE,
      ),
    ),
    'proxy plugin settings callback' => '_wysiwyg_tinymce_proxy_plugin_settings',
    'versions' => array(
      '2.1' => array(
        'js files' => array('tinymce-2.js'),
        'css files' => array('tinymce-2.css'),
        'download url' => 'http://sourceforge.net/project/showfiles.php?group_id=103281&package_id=111430&release_id=557383',
      ),
      // @todo Starting from 3.3, tiny_mce.js may support JS aggregation.
      '3.1' => array(
        'js files' => array('tinymce-3.js'),
        'css files' => array('tinymce-3.css'),
        'libraries' => array(
          '' => array(
            'title' => 'Minified',
            'files' => array(
              'tiny_mce.js' => array('preprocess' => FALSE),
            ),
          ),
          'jquery' => array(
            'title' => 'jQuery',
            'files' => array('tiny_mce_jquery.js'),
          ),
          'src' => array(
            'title' => 'Source',
            'files' => array('tiny_mce_src.js'),
          ),
        ),
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
function wysiwyg_tinymce_version($editor) {
  $script = $editor['library path'] . '/tiny_mce.js';
  if (!file_exists($script)) {
    return;
  }
  $script = fopen($script, 'r');
  // Version is contained in the first 200 chars.
  $line = fgets($script, 200);
  fclose($script);
  // 2.x: this.majorVersion="2";this.minorVersion="1.3"
  // 3.x: majorVersion:'3',minorVersion:'2.0.1'
  if (preg_match('@majorVersion[=:]["\'](\d).+?minorVersion[=:]["\']([\d\.]+)@', $line, $version)) {
    return $version[1] . '.' . $version[2];
  }
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
function wysiwyg_tinymce_themes($editor, $profile) {
  /*
  $themes = array();
  $dir = $editor['library path'] . '/themes/';
  if (is_dir($dir) && $dh = opendir($dir)) {
    while (($file = readdir($dh)) !== FALSE) {
      if (!in_array($file, array('.', '..', 'CVS', '.svn')) && is_dir($dir . $file)) {
        $themes[$file] = $file;
      }
    }
    closedir($dh);
    asort($themes);
  }
  return $themes;
  */
  return array('advanced', 'simple');
}

/**
 * Enhances the editor profile settings form for TinyMCE.
 *
 * @see http://www.tinymce.com/wiki.php/Configuration
 */
function wysiwyg_tinymce_settings_form(&$form, &$form_state) {
  $profile = $form_state['wysiwyg_profile'];
  $settings = $profile->settings;
  $settings += array(
    'apply_source_formatting' => FALSE,
    // Also available, but buggy in TinyMCE 2.x: blockquote,code,dt,dd,samp.
    'theme_advanced_blockformats' => 'p,address,pre,h2,h3,h4,h5,h6,div',
    'convert_fonts_to_spans' => TRUE,
    'paste_auto_cleanup_on_paste' => TRUE,
    'theme_advanced_styles' => '',
    'theme_advanced_statusbar_location' => 'bottom',
    'preformatted' => FALSE,
    'remove_linebreaks' => TRUE,
    'theme_advanced_resizing' => TRUE,
    'theme_advanced_toolbar_align' => 'left',
    'theme_advanced_toolbar_location' => 'top',
    'verify_html' => TRUE,
  );

  $form['appearance']['theme_advanced_toolbar_location'] = array(
    '#type' => 'select',
    '#title' => t('Toolbar location'),
    '#default_value' => $settings['theme_advanced_toolbar_location'],
    '#options' => array('bottom' => t('Bottom'), 'top' => t('Top')),
    '#description' => t('This option controls whether the editor toolbar is displayed above or below the editing area.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'theme_advanced_toolbar_location', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_toolbar_location'))),
  );

  $form['appearance']['theme_advanced_toolbar_align'] = array(
    '#type' => 'select',
    '#title' => t('Button alignment'),
    '#default_value' => $settings['theme_advanced_toolbar_align'],
    '#options' => array('center' => t('Center'), 'left' => t('Left'), 'right' => t('Right')),
    '#description' => t('This option controls the alignment of icons in the editor toolbar.'),
  );

  $form['appearance']['theme_advanced_statusbar_location'] = array(
    '#type' => 'select',
    '#title' => t('Path location'),
    '#default_value' => $settings['theme_advanced_statusbar_location'],
    '#options' => array('none' => t('Hide'), 'top' => t('Top'), 'bottom' => t('Bottom')),
    '#description' => t('Where to display the path to HTML elements (i.e. <code>body > table > tr > td</code>).') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'theme_advanced_statusbar_location', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_statusbar_location'))),
  );

  $form['appearance']['theme_advanced_resizing'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable resizing button'),
    '#default_value' => $settings['theme_advanced_resizing'],
    '#return_value' => 1,
    '#description' => t('This option gives you the ability to enable/disable the resizing button. If enabled, the Path location toolbar must be set to "Top" or "Bottom" in order to display the resize icon.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'theme_advanced_resizing', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_resizing'))),
  );

  $form['output']['verify_html'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verify HTML'),
    '#default_value' => $settings['verify_html'],
    '#return_value' => 1,
    '#description' => t('If enabled, potentially malicious code like <code>&lt;HEAD&gt;</code> tags will be removed from HTML contents.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'verify_html', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:preformatted'))),
  );

  $form['output']['preformatted'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preformatted'),
    '#default_value' => $settings['preformatted'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will insert TAB characters on tab and preserve other whitespace characters just like a PRE element in HTML does.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'preformatted', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:preformatted'))),
  );

  $form['output']['convert_fonts_to_spans'] = array(
    '#type' => 'checkbox',
    '#title' => t('Convert &lt;font&gt; tags to styles'),
    '#default_value' => $settings['convert_fonts_to_spans'],
    '#return_value' => 1,
    '#description' => t('If enabled, HTML tags declaring the font size, font family, font color and font background color will be replaced by inline CSS styles.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array(
      '@setting' => 'convert_fonts_to_spans', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:convert_fonts_to_spans'))),
  );

  $form['output']['remove_linebreaks'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove linebreaks'),
    '#default_value' => $settings['remove_linebreaks'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will remove most linebreaks from contents. Disabling this option could avoid conflicts with other input filters.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'remove_linebreaks', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:remove_linebreaks'))),
  );

  $form['output']['apply_source_formatting'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply source formatting'),
    '#default_value' => $settings['apply_source_formatting'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will re-format the HTML source code. Disabling this option could avoid conflicts with other input filters.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'apply_source_formatting', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:apply_source_formatting'))),
  );

  $form['css']['theme_advanced_styles'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS classes'),
    '#default_value' => $settings['theme_advanced_styles'],
    '#description' => t('Optionally define CSS classes for the "Font style" dropdown list.<br />Enter one class on each line in the format: !format. Example: !example<br />If left blank, CSS classes are automatically imported from all loaded stylesheet(s).',
      array(
        '!format' => '<code>[title]=[class]</code>',
        '!example' => 'My heading=header1',
      )
    ) . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'theme_advanced_styles', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_styles'))),
  );


  $form['css']['theme_advanced_blockformats'] = array(
    '#type' => 'textfield',
    '#title' => t('Block formats'),
    '#default_value' => $settings['theme_advanced_blockformats'],
    '#size' => 40,
    '#maxlength' => 250,
    '#description' => t('Comma separated list of HTML block formats. Possible values: <code>@format-list</code>.', array('@format-list' => 'p,h1,h2,h3,h4,h5,h6,div,blockquote,address,pre,code,dt,dd')) . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'theme_advanced_blockformats', '@url' => url('http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_blockformats'))),
    '#element_validate' => array('wysiwyg_tinymce_settings_form_validate_blockformats'),
  );

  $form['paste'] = array(
    '#type' => 'fieldset',
    '#title' => t('Paste plugin'),
    '#description' => t('Settings for the paste plugin.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );
  $form['paste']['paste_auto_cleanup_on_paste'] = array(
    '#type' => 'checkbox',
    '#title' => t('Process contents on paste'),
    '#default_value' => !empty($settings['paste_auto_cleanup_on_paste']),
    '#return_value' => 1,
    '#description' => t('If enabled, contents will be automatically processed when you paste using Ctrl+V or similar methods. Cleaning up contents from MS Word or pasting as plain text will <strong>not</strong> work without this.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'paste_auto_cleanup_on_paste', '@url' => url('http://www.tinymce.com/wiki.php/Plugin3x:paste'))),
  );
}

/**
 * #element_validate handler for theme_advanced_blockformats element added by wysiwyg_tinymce_settings_form().
 */
function wysiwyg_tinymce_settings_form_validate_blockformats($element, &$form_state) {
  // Remove any white-space from 'theme_advanced_blockformats' setting, since
  // the editor relies on a comma-separated list to explode().
  form_set_value($element, preg_replace('@\s+@', '', $element['#value']), $form_state);
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
function wysiwyg_tinymce_init($editor, $library) {
  // TinyMCE unconditionally searches for its library filename in SCRIPT tags on
  // on the page upon loading the library in order to determine the base path to
  // itself. When JavaScript aggregation is enabled, this search fails and all
  // relative constructed paths within TinyMCE are broken. The library has a
  // tinyMCE.baseURL property, but it is not publicly documented and thus not
  // reliable. The official support forum suggests to solve the issue through
  // the global window.tinyMCEPreInit variable also used by various serverside
  // compressor scrips available from the official website.
  // @see http://www.tinymce.com/forum/viewtopic.php?id=23286
  $settings = drupal_json_encode(array(
    'base' => base_path() . $editor['library path'],
    'suffix' => (strpos($library, 'src') !== FALSE || strpos($library, 'dev') !== FALSE ? '_src' : ''),
    'query' => '',
  ));
  return <<<EOL
window.tinyMCEPreInit = $settings;
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
function wysiwyg_tinymce_settings($editor, $config, $theme) {
  $settings = array(
    'button_tile_map' => TRUE, // @todo Add a setting for this.
    'document_base_url' => base_path(),
    'mode' => 'none',
    'plugins' => array(),
    'theme' => $theme,
    'width' => '100%',
    // Strict loading mode must be enabled; otherwise TinyMCE would use
    // document.write() in IE and Chrome.
    'strict_loading_mode' => TRUE,
    // TinyMCE's URL conversion magic breaks Drupal modules that use a special
    // syntax for paths. This makes 'relative_urls' obsolete.
    'convert_urls' => FALSE,
    // The default entity_encoding ('named') converts too many characters in
    // languages (like Greek). Since Drupal supports Unicode, we only convert
    // HTML control characters and invisible characters. TinyMCE always converts
    // XML default characters '&', '<', '>'.
    'entities' => '160,nbsp,173,shy,8194,ensp,8195,emsp,8201,thinsp,8204,zwnj,8205,zwj,8206,lrm,8207,rlm',
  );
  $check_if_set = array(
    'apply_source_formatting',
    'convert_fonts_to_spans',
    'language',
    'paste_auto_cleanup_on_paste',
    'preformatted',
    'remove_linebreaks',
    'theme_advanced_blockformats',
  );
  foreach ($check_if_set as $setting_name) {
    if (isset($config[$setting_name])) {
      $settings[$setting_name] = $config[$setting_name];
    }
  }
  if (isset($config['verify_html'])) {
    // TinyMCE performs a type-agnostic comparison on this particular setting.
    $settings['verify_html'] = (bool) $config['verify_html'];
  }

  if (!empty($config['theme_advanced_styles'])) {
    $settings['theme_advanced_styles'] = implode(';', array_filter(explode("\n", str_replace("\r", '', $config['theme_advanced_styles']))));
  }

  if (isset($config['css_setting'])) {
    if ($config['css_setting'] == 'theme') {
      $settings['content_css'] = implode(',', wysiwyg_get_css());
    }
    elseif ($config['css_setting'] == 'self' && isset($config['css_path'])) {
      $settings['content_css'] = strtr($config['css_path'], array('%b' => base_path(), '%t' => drupal_get_path('theme', variable_get('theme_default', NULL))));
    }
  }

  // Find the enabled buttons and the button row they belong on.
  // Also map the plugin metadata for each button.
  // @todo What follows is a pain; needs a rewrite.
  // $settings['buttons'] are stacked into $settings['theme_advanced_buttons1']
  // later.
  $settings['buttons'] = array();
  if (!empty($config['buttons']) && is_array($config['buttons'])) {
    // Only array keys in $settings['extensions'] matter; added to
    // $settings['plugins'] later.
    $settings['extensions'] = array();
    // $settings['extended_valid_elements'] are just stacked, unique'd later,
    // and transformed into a comma-separated string in
    // wysiwyg_add_editor_settings().
    // @todo Needs a complete plugin API redesign using arrays for
    //   tag => attributes definitions and array_merge_recursive().
    $settings['extended_valid_elements'] = array();

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
          // Add external Drupal plugins to the list of extensions.
          if ($type == 'buttons' && !empty($plugins[$plugin]['proxy'])) {
            $settings['extensions'][_wysiwyg_tinymce_plugin_name('add', $button)] = 1;
          }
          // Add external plugins to the list of extensions.
          elseif ($type == 'buttons' && empty($plugins[$plugin]['internal'])) {
            $settings['extensions'][_wysiwyg_tinymce_plugin_name('add', $plugin)] = 1;
          }
          // Add internal buttons that also need to be loaded as extension.
          elseif ($type == 'buttons' && !empty($plugins[$plugin]['load'])) {
            $settings['extensions'][$plugin] = 1;
          }
          // Add plain extensions.
          elseif ($type == 'extensions' && !empty($plugins[$plugin]['load'])) {
            $settings['extensions'][$plugin] = 1;
          }
          // Allow plugins to add valid HTML elements.
          if (!empty($plugins[$plugin]['extended_valid_elements'])) {
            $settings['extended_valid_elements'] = array_merge($settings['extended_valid_elements'], $plugins[$plugin]['extended_valid_elements']);
          }
          // Allow plugins to add or override global configuration settings.
          if (!empty($plugins[$plugin]['options'])) {
            $settings = array_merge($settings, $plugins[$plugin]['options']);
          }
        }
      }
    }
    // Clean-up.
    $settings['extended_valid_elements'] = array_unique($settings['extended_valid_elements']);
    if ($settings['extensions']) {
      $settings['plugins'] = array_keys($settings['extensions']);
    }
    unset($settings['extensions']);
  }

  // Add theme-specific settings.
  switch ($theme) {
    case 'advanced':
      $settings += array(
        'theme_advanced_resize_horizontal' => FALSE,
        'theme_advanced_resizing_use_cookie' => FALSE,
        'theme_advanced_statusbar_location' => isset($config['theme_advanced_statusbar_location']) ? $config['theme_advanced_statusbar_location'] : 'bottom',
        'theme_advanced_resizing' => isset($config['theme_advanced_resizing']) ? $config['theme_advanced_resizing'] : 1,
        'theme_advanced_toolbar_location' => isset($config['theme_advanced_toolbar_location']) ? $config['theme_advanced_toolbar_location'] : 'top',
        'theme_advanced_toolbar_align' => isset($config['theme_advanced_toolbar_align']) ? $config['theme_advanced_toolbar_align'] : 'left',
      );
      if (isset($settings['buttons'])) {
        // These rows explicitly need to be set to be empty, otherwise TinyMCE
        // loads its default buttons of the advanced theme for each row.
        $settings += array(
          'theme_advanced_buttons1' => array(),
          'theme_advanced_buttons2' => array(),
          'theme_advanced_buttons3' => array(),
        );
        // @todo Allow to sort/arrange editor buttons.
        for ($i = 0; $i < count($settings['buttons']); $i++) {
          $settings['theme_advanced_buttons1'][] = $settings['buttons'][$i];
        }
      }
      break;
  }
  unset($settings['buttons']);

  // Convert the config values into the form expected by TinyMCE.
  $csv_settings = array('plugins', 'extended_valid_elements', 'theme_advanced_buttons1', 'theme_advanced_buttons2', 'theme_advanced_buttons3');
  foreach ($csv_settings as $key) {
    if (isset($settings[$key]) && is_array($settings[$key])) {
      $settings[$key] = implode(',', $settings[$key]);
    }
  }

  return $settings;
}

/**
 * Build a JS settings array with global metadata for native external plugins.
 */
function _wysiwyg_tinymce_plugin_meta($editor, $plugin) {
  $meta = NULL;
  if (!empty($plugin['load'])) {
    // Add path for native external plugins; internal ones are loaded
    // automatically.
    if (empty($plugin['internal']) && isset($plugin['filename'])) {
      $meta = base_path() . $plugin['path'] . '/' . $plugin['filename'];
    }
  }
  return $meta;
}

/**
 * Add or remove leading hiven to/of external plugin names.
 *
 * TinyMCE requires that external plugins, which should not be loaded from
 * its own plugin repository are prefixed with a hiven in the name.
 *
 * @param string $op
 *   Operation to perform, 'add' or 'remove' (hiven).
 * @param string $name
 *   A plugin name.
 */
function _wysiwyg_tinymce_plugin_name($op, $name) {
  if ($op == 'add') {
    if (strpos($name, '-') !== 0) {
      return '-' . $name;
    }
    return $name;
  }
  elseif ($op == 'remove') {
    if (strpos($name, '-') === 0) {
      return substr($name, 1);
    }
    return $name;
  }
}

/**
 * Build a JS settings array for Drupal plugins loaded via the proxy plugin.
 */
function _wysiwyg_tinymce_proxy_plugin_settings($editor, $profile, $plugins) {
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
function _wysiwyg_tinymce_plugins($editor) {
  $plugins = array(
    'default' => array(
      'path' => $editor['library path'] . '/themes/advanced',
      'buttons' => array(
        'bold' => t('Bold'), 'italic' => t('Italic'), 'underline' => t('Underline'),
        'strikethrough' => t('Strikethrough'),
        'justifyleft' => t('Align left'), 'justifycenter' => t('Align center'), 'justifyright' => t('Align right'), 'justifyfull' => t('Align full'),
        'bullist' => t('Unordered list'), 'numlist' => t('Ordered list'),
        'outdent' => t('Outdent'), 'indent' => t('Indent'),
        'undo' => t('Undo'), 'redo' => t('Redo'),
        'link' => t('Link'), 'unlink' => t('Unlink'), 'anchor' => t('Anchor'),
        'image' => t('Image'),
        'cleanup' => t('Cleanup messy code'),
        'forecolor' => t('Select text color'), 'backcolor' => t('Select background color'),
        'formatselect' => t('Block format'), 'styleselect' => t('Styles'),
        'fontselect' => t('Font'), 'fontsizeselect' => t('Font size'),
        'sup' => t('Superscript'), 'sub' => t('Subscript'),
        'blockquote' => t('Blockquote'), 'code' => t('Edit HTML Source'),
        'hr' => t('Insert horizontal ruler'),
        'cut' => t('Cut'), 'copy' => t('Copy'), 'paste' => t('Paste'),
        'visualaid' => t('Toggle guidelines/invisible elements'),
        'removeformat' => t('Remove formatting'),
        'charmap' => t('Insert custom character'),
        'help' => t('Help'),
      ),
      'internal' => TRUE,
    ),
    'advhr' => array(
      'path' => $editor['library path'] . '/plugins/advhr',
      'buttons' => array('advhr' => t('Advanced horizontal rule')),
      'extended_valid_elements' => array('hr[class|width|size|noshade]'),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:advhr',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'advimage' => array(
      'path' => $editor['library path'] . '/plugins/advimage',
      'extensions' => array('advimage' => t('Advanced image')),
      'extended_valid_elements' => array('img[src|alt|title|align|width|height|usemap|hspace|vspace|border|style|class|onmouseover|onmouseout|id|name|longdesc]'),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:advimage',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'advlink' => array(
      'path' => $editor['library path'] . '/plugins/advlink',
      'extensions' => array('advlink' => t('Advanced link')),
      'extended_valid_elements' => array('a[name|href|target|title|class|onfocus|onblur|onclick|ondlbclick|onmousedown|onmouseup|onmouseover|onmouseout|onkeypress|onkeydown|onkeyup|id|style|rel]'),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:advlink',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'autosave' => array(
      'path' => $editor['library path'] . '/plugins/autosave',
      'extensions' => array('autosave' => t('Auto save')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:autosave',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'contextmenu' => array(
      'path' => $editor['library path'] . '/plugins/contextmenu',
      'extensions' => array('contextmenu' => t('Context menu')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:contextmenu',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'directionality' => array(
      'path' => $editor['library path'] . '/plugins/directionality',
      'buttons' => array('ltr' => t('Left-to-right'), 'rtl' => t('Right-to-left')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:directionality',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'emotions' => array(
      'path' => $editor['library path'] . '/plugins/emotions',
      'buttons' => array('emotions' => t('Emotions')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:emotions',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'fullscreen' => array(
      'path' => $editor['library path'] . '/plugins/fullscreen',
      'buttons' => array('fullscreen' => t('Fullscreen')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:fullscreen',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'inlinepopups' => array(
      'path' => $editor['library path'] . '/plugins/inlinepopups',
      'extensions' => array('inlinepopups' => t('Inline popups')),
      'options' => array(
        'dialog_type' => array('modal'),
      ),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:inlinepopups',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'insertdatetime' => array(
      'path' => $editor['library path'] . '/plugins/insertdatetime',
      'buttons' => array('insertdate' => t('Insert date'), 'inserttime' => t('Insert time')),
      'options' => array(
        'plugin_insertdate_dateFormat' => '%Y-%m-%d',
        'plugin_insertdate_timeFormat' => '%H:%M:%S',
      ),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:insertdatetime',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'layer' => array(
      'path' => $editor['library path'] . '/plugins/layer',
      'buttons' => array('insertlayer' => t('Insert layer'), 'moveforward' => t('Move forward'), 'movebackward' => t('Move backward'), 'absolute' => t('Absolute')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:layer',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'paste' => array(
      'path' => $editor['library path'] . '/plugins/paste',
      'buttons' => array('pastetext' => t('Paste text'), 'pasteword' => t('Paste from Word'), 'selectall' => t('Select all')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:paste',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'preview' => array(
      'path' => $editor['library path'] . '/plugins/preview',
      'buttons' => array('preview' => t('Preview')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:preview',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'print' => array(
      'path' => $editor['library path'] . '/plugins/print',
      'buttons' => array('print' => t('Print')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:print',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'searchreplace' => array(
      'path' => $editor['library path'] . '/plugins/searchreplace',
      'buttons' => array('search' => t('Search'), 'replace' => t('Replace')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:searchreplace',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'style' => array(
      'path' => $editor['library path'] . '/plugins/style',
      'buttons' => array('styleprops' => t('Advanced CSS styles')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:style',
      'internal' => TRUE,
      'load' => TRUE,
    ),
    'table' => array(
      'path' => $editor['library path'] . '/plugins/table',
      'buttons' => array('tablecontrols' => t('Table')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:table',
      'internal' => TRUE,
      'load' => TRUE,
    ),
  );
  if (version_compare($editor['installed version'], '3', '<')) {
    $plugins['flash'] = array(
      'path' => $editor['library path'] . '/plugins/flash',
      'buttons' => array('flash' => t('Flash')),
      'extended_valid_elements' => array('img[class|src|alt|title|hspace|vspace|width|height|align|onmouseover|onmouseout|name|obj|param|embed]'),
      'internal' => TRUE,
      'load' => TRUE,
    );
  }
  if (version_compare($editor['installed version'], '2.0.6', '>')) {
    $plugins['media'] = array(
      'path' => $editor['library path'] . '/plugins/media',
      'buttons' => array('media' => t('Media')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:media',
      'internal' => TRUE,
      'load' => TRUE,
    );
    $plugins['xhtmlxtras'] = array(
      'path' => $editor['library path'] . '/plugins/xhtmlxtras',
      'buttons' => array('cite' => t('Citation'), 'del' => t('Deleted'), 'abbr' => t('Abbreviation'), 'acronym' => t('Acronym'), 'ins' => t('Inserted'), 'attribs' => t('HTML attributes')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:xhtmlxtras',
      'internal' => TRUE,
      'load' => TRUE,
    );
  }
  if (version_compare($editor['installed version'], '3', '>')) {
    $plugins['bbcode'] = array(
      'path' => $editor['library path'] . '/plugins/bbcode',
      'extensions' => array('bbcode' => t('BBCode')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:bbcode',
      'internal' => TRUE,
      'load' => TRUE,
    );
    if (version_compare($editor['installed version'], '3.3', '<')) {
      $plugins['safari'] = array(
        'path' => $editor['library path'] . '/plugins/safari',
        'extensions' => array('safari' => t('Safari compatibility')),
        'internal' => TRUE,
        'load' => TRUE,
      );
    }
  }
  if (version_compare($editor['installed version'], '3.2.5', '>=')) {
    $plugins['autoresize'] = array(
      'path' => $editor['library path'] . '/plugins/autoresize',
      'extensions' => array('autoresize' => t('Auto resize')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:autoresize',
      'internal' => TRUE,
      'load' => TRUE,
    );
  }
  if (version_compare($editor['installed version'], '3.3', '>=')) {
    $plugins['advlist'] = array(
      'path' => $editor['library path'] . '/plugins/advlist',
      'extensions' => array('advlist' => t('Advanced list')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:advlist',
      'internal' => TRUE,
      'load' => TRUE,
    );
  }
  if (version_compare($editor['installed version'], '3.2.6', '>=')) {
    $plugins['wordcount'] = array(
      'path' => $editor['library path'] . '/plugins/wordcount',
      'extensions' => array('wordcount' => t('Word count')),
      'internal' => TRUE,
      'load' => TRUE,
    );
  }
  if (version_compare($editor['installed version'], '3.4.1', '>=')) {
    $plugins['lists'] = array(
      'path' => $editor['library path'] . 'plugins/lists',
      'extensions' => array('lists' => t('List normalizer')),
      'url' => 'http://www.tinymce.com/wiki.php/Plugin:lists',
      'internal' => TRUE,
      'load' => TRUE,
      'extended_valid_elements' => array(
        'li[class|dir|id|lang|onclick|ondblclick|onkeydown|onkeypress|onkeyup|onmousedown|onmousemove|onmouseout|onmouseover|onmouseup|style|title|type|value]',
        'ol[class|compact|dir|id|lang|onclick|ondblclick|onkeydown|onkeypress|onkeyup|onmousedown|onmousemove|onmouseout|onmouseover|onmouseup|start|style|title|type]',
        'ul[class|compact|dir|id|lang|onclick|ondblclick|onkeydown|onkeypress|onkeyup|onmousedown|onmousemove|onmouseout|onmouseover|onmouseup|style|title|type]',
      ),
    );
  }
  return $plugins;
}

