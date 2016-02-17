<?php

/**
 * @file
 * Editor integration functions for CKEditor.
 */

define('WYSIWYG_CKEDITOR_ACF_DISABLED', 0);
define('WYSIWYG_CKEDITOR_ACF_AUTOMATIC', 1);
define('WYSIWYG_CKEDITOR_ACF_CUSTOM', 2);

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
    'plugin callback' => '_wysiwyg_ckeditor_plugins',
    'plugin meta callback' => '_wysiwyg_ckeditor_plugin_meta',
    'proxy plugin' => array(
      'drupal' => array(
        'load' => TRUE,
        'proxy' => TRUE,
      ),
    ),
    'proxy plugin settings callback' => '_wysiwyg_ckeditor_proxy_plugin_settings',
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
  $output = '<p class="warning">' . t('Do NOT download the "CKEditor for Drupal" edition.') . '</br>';
  $output .= t('Make sure you install the full package as not all plugins work with the standard package.') . '</p>';
  return $output;
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
    // version:"4.0",revision:"769d96134b"
    if (preg_match('@version:[\'"](?:CKEditor )?([\d\.]+)(?:.+revision:[\'"]([[:xdigit:]]+))?@', $line, $version)) {
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
 * @see http://docs.cksource.com/ckeditor_api/symbols/CKEDITOR.config.html
 */
function wysiwyg_ckeditor_settings_form(&$form, &$form_state) {
  $profile = $form_state['wysiwyg_profile'];
  $settings = $profile->settings;
  $installed_version = $form_state['wysiwyg']['editor']['installed version'];

  $ckeditor_defaults = array(
    'block_formats' => 'p,address,pre,h2,h3,h4,h5,h6,div',
    // Custom setting.
    'default_toolbar_grouping' => FALSE,
    'forcePasteAsPlainText' => FALSE,
    'resize_enabled' => TRUE,
    'simple_source_formatting' => FALSE,
    'toolbarLocation' => 'top',
    'allowedContent' => TRUE,
  );

  if (version_compare($installed_version, '3.2.1', '>=')) {
    $ckeditor_defaults['stylesSet'] = '';
  }

  $settings += $ckeditor_defaults;

  $form['appearance']['toolbarLocation'] = array(
    '#type' => 'select',
    '#title' => t('Toolbar location'),
    '#default_value' => $settings['toolbarLocation'],
    '#options' => array('bottom' => t('Bottom'), 'top' => t('Top')),
    '#description' => t('This option controls whether the editor toolbar is displayed above or below the editing area.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'toolbarLocation', '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-toolbarLocation'))),
  );

  $form['appearance']['resize_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable resizing button'),
    '#default_value' => $settings['resize_enabled'],
    '#return_value' => 1,
    '#description' => t('This option gives you the ability to enable/disable the editor resizing feature.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'resize_enabled', '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-resize_enabled'))),
  );

  $form['output']['simple_source_formatting'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply simple source formatting'),
    '#default_value' => $settings['simple_source_formatting'],
    '#return_value' => 1,
    '#description' => t('If enabled, the editor will re-format the HTML source code using a simple set of predefined rules. Disabling this option could avoid conflicts with other input filters.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'dataProcessor.write.setRules()', '@url' => url('http://docs.cksource.com/ckeditor_api/symbols/src/plugins_htmlwriter_plugin.js.html'))),
  );

  $form['paste'] = array(
    '#type' => 'fieldset',
    '#title' => t('Paste plugin'),
    '#description' => t('Settings for the paste plugin.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );

  $form['paste']['forcePasteAsPlainText'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force paste as plain text'),
    '#default_value' => !empty($settings['forcePasteAsPlainText']),
    '#return_value' => 1,
    '#description' => t('If enabled, all pasting operations insert plain text into the editor, losing any formatting information possibly available in the source text. Note: Paste from Word is not affected by this setting.') . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'forcePasteAsPlainText', '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-forcePasteAsPlainText'))),
  );

  if (version_compare($installed_version, '4.1.0', '>=')) {
    $form['output']['acf'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced Content Filter'),
      '#description' => t('ACF limits and adapts input data (HTML code added in source mode or by the editor.setData method, pasted HTML code, etc.) so it matches the editor configuration in the best possible way. It may also deactivate features which generate HTML code that is not allowed by the configuration. See <a href="@url">@url</a> for details.', array('@url' => url('http://docs.ckeditor.com/#!/guide/dev_advanced_content_filter'))),
    );
    $form['output']['acf']['acf_mode'] = array(
      '#type' => 'select',
      '#title' => t('Mode'),
      '#options' => array(
        WYSIWYG_CKEDITOR_ACF_AUTOMATIC => t('Automatic'),
        WYSIWYG_CKEDITOR_ACF_CUSTOM => t('Custom'),
        WYSIWYG_CKEDITOR_ACF_DISABLED => t('Disabled'),
      ),
      '#default_value' => isset($profile->settings['acf_mode']) ? $profile->settings['acf_mode'] : WYSIWYG_CKEDITOR_ACF_DISABLED,
      '#description' => t('If set to <em>Automatic</em> or <em>Custom</em>, the editor will strip out any content not explicitely allowed <strong>when the editor loads</strong>.'),
    );

    $form['output']['acf']['acf_allowed_content'] = array(
      '#type' => 'textarea',
      '#title' => t('Content Rules'),
      '#default_value' => isset($profile->settings['acf_allowed_content']) ? $profile->settings['acf_allowed_content'] : '',
      '#description' => t('Rules for whitelisting content for the advanced content filter. Both string and object formats accepted. Uses the <a href="@allowed_url">allowedContent</a> setting in <em>Custom</em> mode <strong>or</strong> the <a href="@extra_allwed_url">extraAllowedContent</a> settings in <em>Automatic</em> mode internally. See <a href="@info_url">@info_url</a> for details.', array('@info_url' => url('http://docs.ckeditor.com/#!/guide/dev_allowed_content_rules'), '@allowed_url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-allowedContent'), '@allowed_extra_url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-extraAllowedContent'))),
      '#states' => array(
        'visible' => array(
          ':input[name="acf_mode"]' => array(
            array('value' => WYSIWYG_CKEDITOR_ACF_AUTOMATIC),
            array('value' => WYSIWYG_CKEDITOR_ACF_CUSTOM),
          ),
        ),
      ),
      '#element_validate' => array('wysiwyg_ckeditor_settings_form_validate_allowed_content'),
    );
  }

  if (version_compare($installed_version, '3.6.0', '>=')) {
    $form['appearance']['default_toolbar_grouping'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use default toolbar button grouping'),
      '#default_value' => !empty($settings['default_toolbar_grouping']),
      '#return_value' => 1,
      '#description' => t('This option gives you the ability to enable/disable the usage of default groupings for toolbar buttons. If enabled, toolbar buttons will be placed into predetermined groups instead of all in a single group.'),
    );
  }

  if (version_compare($installed_version, '3.2.1', '>=')) {
    // Versions below 3.2.1 do not support Font styles at all.
    $form['css']['stylesSet'] = array(
      '#type' => 'textarea',
      '#title' => t('CSS classes'),
      '#description' => t('Optionally define CSS classes for the "Font style" dropdown list.<br />Enter one class on each line in the format: !format. Example: !example<br />If left blank, CSS classes are automatically imported from loaded stylesheet(s).', array(
        '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.stylesSet'),
        '!format' => '<code>[label]=[element].[class]</code>',
        '!example' => '<code>Title=h1.title</code>',
      )) . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'stylesSet', '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-stylesSet'))),
      '#default_value' => $settings['stylesSet'],
      '#element_validate' => array('wysiwyg_ckeditor_settings_form_validate_stylessets'),
    );

  }

  $form['css']['block_formats'] = array(
    '#type' => 'textfield',
    '#title' => t('Block formats'),
    '#default_value' => $settings['block_formats'],
    '#size' => 40,
    '#maxlength' => 250,
    '#description' => t('Comma separated list of HTML block formats. Possible values: <code>@format-list</code>.', array('@format-list' => 'p,h1,h2,h3,h4,h5,h6,div,blockquote,address,pre,code,dt,dd and other block elements')) . ' ' . t('Uses the <a href="@url">@setting</a> setting internally.', array('@setting' => 'block_formats', '@url' => url('http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-format_tags'))),
  );
}

/**
 * #element_validate handler for ACF Allowed Content element altered by wysiwyg_ckeditor_settings_form().
 */
function wysiwyg_ckeditor_settings_form_validate_allowed_content($element, &$form_state) {
  if (_wysiwyg_ckeditor_settings_acf_is_obj($element['#value']) && json_decode($element['#value']) === NULL) {
    form_error($element, t('Allowed content is not valid JSON.'));
  }
}

/**
 * #element_validate handler for CSS classes element altered by wysiwyg_ckeditor_settings_form().
 */
function wysiwyg_ckeditor_settings_form_validate_stylessets($element, &$form_state) {
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
  $default_skin = (version_compare($editor['installed version'], '4.0.0', '<') ? 'kama' : 'moono');
  $settings = array(
    // Needed to make relative paths work in the editor.
    'baseHref' => $GLOBALS['base_url'] . '/',
    'width' => 'auto',
    // For better compatibility with smaller textareas.
    'resize_minWidth' => 450,
    // @todo Do not use skins as themes and add separate skin handling.
    'theme' => 'default',
    'skin' => !empty($theme) ? $theme : $default_skin,
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
    $block_formats = explode(',', drupal_strtolower(preg_replace('@\s+@', '', $config['block_formats'])));
    $predefined_formats = array('h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'p', 'pre', 'address', 'div');
    foreach (array_diff($block_formats, $predefined_formats) as $tag) {
      $tag = trim($tag);
      $settings["format_$tag"] = array('element' => $tag, 'name' => strtoupper(substr($tag, 0, 1)) . substr($tag, 1));
    }
    $settings['format_tags'] = implode(';', $block_formats);
  }

  // Advanced Content Filter
  // @see http://docs.ckeditor.com/#!/guide/dev_advanced_content_filter
  if (!isset($config['acf_mode']) || WYSIWYG_CKEDITOR_ACF_DISABLED == $config['acf_mode']) {
    $settings['allowedContent'] = TRUE;
  }
  else {
    if (_wysiwyg_ckeditor_settings_acf_is_obj($config['acf_allowed_content'])) {
      $acf_content = json_decode($config['acf_allowed_content']);
    }
    else {
      $acf_content = $config['acf_allowed_content'];
    }

    if (WYSIWYG_CKEDITOR_ACF_CUSTOM == $config['acf_mode']) {
      $settings['allowedContent'] = $acf_content;
    }
    elseif (WYSIWYG_CKEDITOR_ACF_AUTOMATIC == $config['acf_mode']) {
      $settings['extraAllowedContent'] = $acf_content;
    }
  }

  if (isset($config['css_setting'])) {
    // Versions below 3.0.1 could only handle one stylesheet.
    if (version_compare($editor['installed version'], '3.0.1.4391', '<')) {
      if ($config['css_setting'] == 'theme') {
        $css = wysiwyg_get_css();
        $settings['contentsCss'] = reset($css);
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
  if (!empty($config['buttons']['default']['Styles']) && version_compare($editor['installed version'], '3.2.1', '>=') && !empty($config['stylesSet'])) {
    if ($styles = wysiwyg_ckeditor_settings_parse_styles($config['stylesSet'])) {
      $settings['stylesSet'] = $styles;
    }
  }

  $check_if_set = array(
    'forcePasteAsPlainText',
    'language',
    'simple_source_formatting',
    'toolbarLocation',
  );
  foreach ($check_if_set as $setting_name) {
    if (isset($config[$setting_name])) {
      $settings[$setting_name] = $config[$setting_name];
    }
  }

  if (isset($config['resize_enabled'])) {
    // CKEditor performs a type-agnostic comparison on this particular setting.
    $settings['resize_enabled'] = (bool) $config['resize_enabled'];
  }

  $settings['toolbar'] = array();
  $supports_groups = version_compare($editor['installed version'], '3.6.0', '>=');
  $use_default_groups = $supports_groups && !empty($config['default_toolbar_grouping']);
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
            if ($use_default_groups) {
              $settings['toolbar'][_wysiwyg_ckeditor_group($button)][] = $button;
            }
            else {
              // Use one button row for backwards compatibility.
              $settings['toolbar'][] = $button;
            }
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

  if ($use_default_groups) {
    // Organize groups to use lables to improves accessibility.
    // http://docs.ckeditor.com/#!/guide/dev_toolbar-section-3.
    $groups_toolbar = array();
    foreach ($settings['toolbar'] as $group => $items) {
      $groups_toolbar[] = array(
        'name' => $group,
        'items' => $items,
      );
      $settings['toolbar'] = $groups_toolbar;
    }
  }
  else {
    // For now, all buttons are placed into one row.
    $settings['toolbar'] = array($settings['toolbar']);
  }

  return $settings;
}

/**
 * Parses stylesSet settings string into a stylesSet JavaScript settings array.
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
 * Build a JS settings array with global metadata for native external plugins.
 */
function _wysiwyg_ckeditor_plugin_meta($editor, $plugin) {
  $meta = array();
  $name = $plugin['name'];
  // Register all plugins that need to be loaded.
  if (!empty($plugin['load'])) {
    // Add path for native external plugins.
    if (empty($plugin['internal']) && isset($plugin['path'])) {
      $meta['path'] = base_path() . $plugin['path'] . '/';
    }
    // Force native internal plugins to use the standard path.
    else {
      $meta['path'] = base_path() . $editor['library path'] . '/plugins/' . $name . '/';
    }
    // CKEditor defaults to 'plugin.js' on its own when filename is not set.
    if (!empty($plugin['filename'])) {
      $meta['fileName'] = $plugin['filename'];
    }
  }
  return $meta;
}

/**
 * Build a JS settings array for Drupal plugins loaded via the proxy plugin.
 */
function _wysiwyg_ckeditor_proxy_plugin_settings($editor, $profile, $plugins) {
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
function _wysiwyg_ckeditor_plugins($editor) {
  $plugins = array(
    'default' => array(
      'buttons' => array(
        'Bold' => t('Bold'), 'Italic' => t('Italic'), 'Underline' => t('Underline'),
        'Strike' => t('Strike through'),
        'JustifyLeft' => t('Align left'), 'JustifyCenter' => t('Center'), 'JustifyRight' => t('Align right'), 'JustifyBlock' => t('Justify'),
        'BulletedList' => t('Insert/Remove Bullet list'), 'NumberedList' => t('Insert/Remove Numbered list'),
        'BidiLtr' => t('Left-to-right'), 'BidiRtl' => t('Right-to-left'),
        'Outdent' => t('Outdent'), 'Indent' => t('Indent'),
        'Undo' => t('Undo'), 'Redo' => t('Redo'),
        'Link' => t('Link'), 'Unlink' => t('Unlink'), 'Anchor' => t('Anchor'),
        'Image' => t('Image'),
        'TextColor' => t('Text color'), 'BGColor' => t('Background color'),
        'Superscript' => t('Superscript'), 'Subscript' => t('Subscript'),
        'Blockquote' => t('Block quote'), 'Source' => t('Source code'),
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
        'Iframe' => t('IFrame'),
        'Maximize' => t('Maximize'),
        'SpellChecker' => t('Check spelling'), 'Scayt' => t('Spell check as you type'),
        'About' => t('About'),
        'Templates' => t('Templates'),
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

/**
 * Define grouping for ckEditor buttons.
 */
function _wysiwyg_ckeditor_group($button) {
  switch ($button) {
    case 'Source':
      $group = 'document';
      break;

    case 'Cut':
    case 'Copy':
    case 'Paste':
    case 'PasteText':
    case 'PasteFromWord':
    case 'Undo':
    case 'Redo':
      $group = 'clipboard';
      break;

    case 'Find':
    case 'Replace':
    case 'SelectAll':
    case 'SpellChecker':
    case 'Scayt':
      $group = 'editing';
      break;

    case 'Bold':
    case 'Italic':
    case 'Underline':
    case 'Strike':
    case 'Subscript':
    case 'Superscript':
    case 'RemoveFormat':
      $group = 'basicstyles';
      break;

    case 'NumberedList':
    case 'BulletedList':
    case 'Outdent':
    case 'Indent':
    case 'Blockquote':
    case 'CreateDiv':
    case 'JustifyLeft':
    case 'JustifyCenter':
    case 'JustifyRight':
    case 'JustifyBlock':
    case 'BidiLtr':
    case 'BidiRtl':
      $group = 'paragraph';
      break;

    case 'Link':
    case 'Unlink':
    case 'Anchor':
      $group = 'links';
      break;

    case 'Image':
    case 'Flash':
    case 'Table':
    case 'HorizontalRule':
    case 'Smiley':
    case 'SpecialChar':
    case 'Iframe':
    case 'Templates':
      $group = 'insert';
      break;

    case 'Styles':
    case 'Format':
    case 'Font':
    case 'FontSize':
      $group = 'styles';
      break;

    case 'TextColor':
    case 'BGColor':
      $group = 'colors';
      break;

    case 'Maximize':
    case 'ShowBlocks':
    case 'About':
      $group = 'tools';
      break;

    default:
      $group = 'other';
  }
  return $group;
}

/**
 * Determine if string is supposed to be ACF obj format.
 *
 * @see http://docs.ckeditor.com/#!/guide/dev_allowed_content_rules
 */
function _wysiwyg_ckeditor_settings_acf_is_obj($string) {
  if (strstr($string, ':') === FALSE) {
    return FALSE;
  }

  return TRUE;
}
