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
 * Guess the absolute server path to the document root
 * Usually it should return $_SERVER['DOCUMENT_ROOT']
 *
 * @todo Improve me!!
 * Returns absolute path or false on failure
 *
 * @return string|boolean
 */
function ckeditor_get_document_root_full_path() {
  $found = 0;
  $index_dir = realpath(dirname($_SERVER['SCRIPT_FILENAME'])); // {/dir1/dir2/home/drupal}/index.php
  if (getcwd() == $index_dir) {
    $found++;
  }
  $drupal_dir = base_path();
  $index_dir = str_replace('\\', "/", $index_dir);
  $drupal_dir = str_replace('\\', "/", $drupal_dir);
  $document_root_dir = $index_dir . '/' . str_repeat('../', substr_count($drupal_dir, '/') - 1);
  $document_root_dir = realpath($document_root_dir);
  $document_root_dir = rtrim($document_root_dir, '/\\');
  if ($document_root_dir == $_SERVER['DOCUMENT_ROOT']) {
    $found++;
  }
  $document_root_dir = str_replace('\\', '/', $document_root_dir);

  if ($document_root_dir != '') {
    $found++;
  }
  if (file_exists($document_root_dir)) {
    $found++;
  }
  if (file_exists($document_root_dir . base_path() . 'includes/bootstrap.inc')) {
    $found++;
  }

  if ($found >= 3) {
    return $document_root_dir;
  }
  else {
    return FALSE;
  }
}

/**
 * Emulates the asp Server.mapPath function.
 * Given an url path return the physical directory that it corresponds to.
 *
 * Returns absolute path or false on failure
 *
 * @param string $path
 * @return string|boolean
 */
function ckeditor_resolve_url($path) {
  if (function_exists('apache_lookup_uri')) {
    $info = @apache_lookup_uri($path);
    if (!$info) {
      return FALSE;
    }
    return $info->filename . $info->path_info;
  }

  $document_root = ckeditor_get_document_root_full_path();
  if ($document_root !== FALSE) {
    return $document_root . $path;
  }

  return FALSE;
}

/**
 * List of configured CKEditor toolbars
 *
 * @return array
 */
function ckeditor_load_toolbar_options() {
  $arr = array('Basic' => 'Basic', 'Full' => 'Full');
  $module_drupal_path = drupal_get_path('module', 'ckeditor');
  $editor_local_path = ckeditor_path('local');
  $ckconfig_js = $editor_local_path . '/config.js';
  $ckeditor_config_js = $module_drupal_path . '/ckeditor.config.js';
  if ($editor_local_path != '<URL>' && file_exists($ckconfig_js) && is_readable($ckconfig_js)) {
    $fp = @fopen($ckconfig_js, "r");
    if ($fp) {
      while (!feof($fp)) {
        $line = fgets($fp, 1024);
        $matches = array();
        if (preg_match('/config.toolbar_([a-z0-9_]+)/i', $line, $matches)) {
          $arr[$matches[1]] = drupal_ucfirst($matches[1]);
        }
      }
      fclose($fp);
    }
  }
  if (file_exists($ckeditor_config_js) && is_readable($ckeditor_config_js)) {
    $fp = @fopen($ckeditor_config_js, "r");
    if ($fp) {
      while (!feof($fp)) {
        $line = fgets($fp, 1024);
        $matches = array();
        if (preg_match('/config.toolbar_([a-z0-9_]+)/i', $line, $matches)) {
          $arr[$matches[1]] = drupal_ucfirst($matches[1]);
        }
      }
      fclose($fp);
    }
  }

  //oops, we have no information about toolbars, let's use hardcoded array
  if (empty($arr)) {
    $arr = array(
      'Basic' => 'Basic',
      'Default' => 'Default',
    );
  }
  asort($arr);

  return $arr;
}

/**
 * List of installed CKEditor skins
 *
 * @return array
 */
function ckeditor_load_skin_options() {
  $arr = array();
  $editor_local_path = ckeditor_path('local');
  $skin_dir = $editor_local_path . '/skins';
  if ($editor_local_path != '<URL>' && is_dir($skin_dir)) {
    $dh = @opendir($skin_dir);
    if (FALSE !== $dh) {
      while (($file = readdir($dh)) !== FALSE) {
        if (in_array($file, array(".", "..", "CVS", ".svn"))) {
          continue;
        }
        if (is_dir($skin_dir . DIRECTORY_SEPARATOR . $file)) {
          $arr[$file] = drupal_ucfirst($file);
        }
      }
      closedir($dh);
    }
  }
  //oops, we have no information about skins, let's use only default
  if (empty($arr)) {
    $arr = array(
      'moono' => 'Moono',
      'kama' => 'Kama'
    );
  }
  asort($arr);

  return $arr;
}

/**
 * Return default skin for CKEditor
 *
 * @return string
 */
function ckeditor_default_skin() {
  $skin_options = ckeditor_load_skin_options();
  if (array_key_exists('moono', $skin_options)) {
    return 'moono';
  }
  if (array_key_exists('kama', $skin_options)) {
    return 'kama';
  }
  reset($skin_options);
  //if any default theme not exists select first from the list
  return key($skin_options);
}

/**
 * List of installed CKEditor languages
 *
 * @return array
 */
function ckeditor_load_lang_options() {
  $arr = array();
  $editor_local_path = ckeditor_path('local');
  $lang_file = $editor_local_path . '/lang/_languages.js';
  if ($editor_local_path != '<URL>' && file_exists($lang_file)) {
    $f = fopen($lang_file, 'r');
    $file = fread($f, filesize($lang_file));
    $tmp = explode('{', $file);
    if (isset($tmp[2])) {
      $tmp = explode('}', $tmp[2]);
    }
    $langs = explode(',', $tmp[0]);
    foreach ($langs AS $key => $lang) {
      preg_match("/'?(\w+-?\w+)'?:'([\w\s\(\)]+)'/i", $lang, $matches);
      if (isset($matches[1]) && isset($matches[2]))
        $arr[$matches[1]] = $matches[2];
    }
  }
  //oops, we have no information about languages, let's use those available in CKEditor 2.4.3
  if (empty($arr)) {
    $arr = array(
      'af' => 'Afrikaans',
      'ar' => 'Arabic',
      'bg' => 'Bulgarian',
      'bn' => 'Bengali/Bangla',
      'bs' => 'Bosnian',
      'ca' => 'Catalan',
      'cs' => 'Czech',
      'da' => 'Danish',
      'de' => 'German',
      'el' => 'Greek',
      'en' => 'English',
      'en-au' => 'English (Australia)',
      'en-ca' => 'English (Canadian)',
      'en-uk' => 'English (United Kingdom)',
      'eo' => 'Esperanto',
      'es' => 'Spanish',
      'et' => 'Estonian',
      'eu' => 'Basque',
      'fa' => 'Persian',
      'fi' => 'Finnish',
      'fo' => 'Faroese',
      'fr' => 'French',
      'gl' => 'Galician',
      'he' => 'Hebrew',
      'hi' => 'Hindi',
      'hr' => 'Croatian',
      'hu' => 'Hungarian',
      'it' => 'Italian',
      'ja' => 'Japanese',
      'km' => 'Khmer',
      'ko' => 'Korean',
      'lt' => 'Lithuanian',
      'lv' => 'Latvian',
      'mn' => 'Mongolian',
      'ms' => 'Malay',
      'nb' => 'Norwegian Bokmal',
      'nl' => 'Dutch',
      'no' => 'Norwegian',
      'pl' => 'Polish',
      'pt' => 'Portuguese (Portugal)',
      'pt-br' => 'Portuguese (Brazil)',
      'ro' => 'Romanian',
      'ru' => 'Russian',
      'sk' => 'Slovak',
      'sl' => 'Slovenian',
      'sr' => 'Serbian (Cyrillic)',
      'sr-latn' => 'Serbian (Latin)',
      'sv' => 'Swedish',
      'th' => 'Thai',
      'tr' => 'Turkish',
      'uk' => 'Ukrainian',
      'vi' => 'Vietnamese',
      'zh' => 'Chinese Traditional',
      'zh-cn' => 'Chinese Simplified',
    );
  }
  asort($arr);
  return $arr;
}

/**
 * Get the language locale code supported by Scayt for the specified language
 */
function ckeditor_scayt_langcode($lang) {
  $scayt_langs = array(
    'en' => 'en_US',
    'es' => 'es_ES',
    'fr' => 'fr_FR',
    'de' => 'de_DE',
    'it' => 'it_IT',
    'el' => 'el_EL',
    'pt' => 'pt_PT',
    'da' => 'da_DA',
    'sv' => 'sv_SE',
    'nl' => 'nl_NL',
    'nb' => 'no_NO',
    'fi' => 'fi_FI',
  );
  if (array_key_exists($lang, $scayt_langs)) {
    return $scayt_langs[$lang];
  }

  $default = language_default();
  $default = $default->language;
  if (array_key_exists($default, $scayt_langs)) {
    return $scayt_langs[$default];
  }

  return 'en_US';
}

/**
 * List of CKEditor plugins.
 *
 * @param boolean $render
 *   Whether to generate plugin paths. Default to FALSE.
 * @return array
 */
function ckeditor_load_plugins($render = FALSE) {
  $plugins = module_invoke_all('ckeditor_plugin');

  // [#2159403] introduced a slight change in CKEditor API, expecting the path
  // to a plugin returned by hook_ckeditor_plugin() to be a full URL. This
  // breaks backward compatibility for external plugins, so we're fixing it
  // below.
  foreach ($plugins as $i => $plugin) {
    if (strpos($plugin['path'], '%') !== 0 && !preg_match('|^(http(s)?:)?\/?\/|i', $plugin['path'])) {
      $plugins[$i]['path'] = '%base_path%' . $plugin['path'];
    }
  }

  drupal_alter('ckeditor_plugin', $plugins);

  ksort($plugins);

  if ($render) {
    $plugins = ckeditor_plugins_render($plugins);
  }

  return $plugins;
}

/**
 * Render CKEditor plugins path
 */
function ckeditor_plugins_render($plugins) {
  $render = array();
  $render["%base_path%"] = ckeditor_base_path('relative') . '/';
  $render["%editor_path%"] = ckeditor_path('relative') . '/';
  $render["%module_path%"] = ckeditor_module_path('relative') . '/';
  $render["%plugin_dir%"] = $render["%module_path%"] . 'plugins/';
  $render["%plugin_dir_extra%"] = ckeditor_plugins_path('relative') . '/';

  foreach ((array) $plugins as $i => $plugin) {
    $plugins[$i]['path'] = str_replace(array_keys($render), array_values($render), $plugin['path']);
  }

  return $plugins;
}

/**
 * Get default ckeditor settings
 *
 * @return array
 */
function ckeditor_user_get_setting_default() {
  $default = array(
    'default' => 't',
    'show_toggle' => 't',
    'width' => '100%',
    'lang' => 'en',
    'auto_lang' => 't',
  );

  // Allow other modules to alter the default settings.
  drupal_alter('ckeditor_default_settings', $default);

  return $default;
}

/**
 * Return CKEditor settings
 *
 * @param object $user
 * @param object $profile
 * @param string $setting
 * @return array
 */
function ckeditor_user_get_setting($user, $profile, $setting) {
  $default = ckeditor_user_get_setting_default();

  if (user_access('customize ckeditor')) {
    $status = isset($user->data['ckeditor_' . $setting]) ? $user->data['ckeditor_' . $setting] : (isset($profile->settings[$setting]) ? $profile->settings[$setting] : $default[$setting]);
  }
  else {
    $status = isset($profile->settings[$setting]) ? $profile->settings[$setting] : $default[$setting];
  }

  return $status;
}

/**
 * Return CKEditor profile by input format
 *
 * @param string $input_format
 * @return object|boolean
 */
function ckeditor_get_profile($input_format) {
  $select = db_select('ckeditor_settings', 's');
  $select->join('ckeditor_input_format', 'f', 'f.name = s.name');
  $result = $select->fields('s', array("name"))->condition('f.format', $input_format)->condition('f.name', 'CKEditor Global Profile', '<>')->range(0, 1)->execute()->fetchAssoc();

  if ($result && $profile = ckeditor_profile_load($result['name'])) {
    return $profile;
  }

  return FALSE;
}

/**
 * Return CKEditor profile list
 */
function ckeditor_profile_input_formats() {
  $select = db_select('ckeditor_settings', 's');
  $select->join('ckeditor_input_format', 'f', 'f.name = s.name');
  $result = $select->fields('s', array("name"))->fields('f', array("format"))->execute();

  $list = array();
  while ($row = $result->fetchAssoc()) {
    if (!isset($row['name'])) {
      $list[$row['name']] = array();
    }
    $list[$row['name']][] = $row['format'];
  }

  return $list;
}

/**
 * Search and return CKEditor plugin path
 *
 * @return string
 */
function _ckeditor_script_path() {
  $jspath = '';
  $module_path = drupal_get_path('module', 'ckeditor');

  if (file_exists($module_path . '/ckeditor/ckeditor.js')) {
    $jspath = '%m/ckeditor';
  }
  elseif (file_exists($module_path . '/ckeditor/ckeditor/ckeditor.js')) {
    $jspath = '%m/ckeditor/ckeditor';
  }
  elseif (file_exists(ckeditor_library_path('url') . '/ckeditor/ckeditor.js')) {
    $jspath = '%l/ckeditor';
  }
  return $jspath;
}

/**
 * Determines whether the CKEditor sources are present
 *
 * It checks if ckeditor.js is present.
 *
 * This function is used by ckeditor_requirements()
 *
 * @return boolean True if CKEditor is installed
 */
function _ckeditor_requirements_isinstalled() {
  $editor_path = ckeditor_path('local');
  if ($editor_path == '<URL>')
    return TRUE;
  $jspath = $editor_path . '/ckeditor.js';

  $jsp = file_exists($jspath);
  if (!$jsp && ($editor_path = _ckeditor_script_path())) {
    $result = db_select('ckeditor_settings', 's')->fields('s')->condition('name', 'CKEditor Global Profile')->execute()->fetchAssoc();
    if ($result) {
      $result['settings'] = unserialize($result['settings']);
      $result['settings']['ckeditor_path'] = $editor_path;
      $result['settings'] = serialize($result['settings']);
      db_update('ckeditor_settings')->fields(array("settings" => $result['settings']))->condition('name', 'CKEditor Global Profile')->execute();

      $jsp = TRUE;
      ckeditor_path('local', TRUE);
    }
  }
  return $jsp;
}

/**
 * Compile settings of all profiles at returns is as array
 *
 * @param string $input_format
 * @param boolean $clear
 * @return array
 */
function ckeditor_profiles_compile($input_format = FALSE, $clear = FALSE) {
  static $compiled = FALSE;
  static $_ckeditor_compiled = array();

  if ($clear !== FALSE && $compiled !== FALSE) {
    $compiled = FALSE;
  }

  if ($compiled === TRUE) {
    return ( $input_format === FALSE ) ? $_ckeditor_compiled : ( isset($_ckeditor_compiled[$input_format]) ? $_ckeditor_compiled[$input_format] : array() );
  }

  $global_profile = ckeditor_profile_load('CKEditor Global Profile');

  $profiles_list = ckeditor_profile_input_formats();

  foreach ($profiles_list AS $_profile => $_inputs) {
    $profile = ckeditor_profile_load($_profile);
    $setting = ckeditor_profile_settings_compile($global_profile, $profile);

    foreach ($_inputs AS $_input) {
      $_ckeditor_compiled[$_input] = $setting;
    }
  }

  $compiled = TRUE;

  return ( $input_format === FALSE ) ? $_ckeditor_compiled : $_ckeditor_compiled[$input_format];
}

/**
 * Compile settings of profile
 *
 * @param object $global_profile
 * @param object $profile
 * @return array
 */
function ckeditor_profile_settings_compile($global_profile, $profile) {
  global $user, $language, $theme;

  $current_theme = variable_get('theme_default', $theme);

  $settings = array();
  $conf = $profile->settings;
  $profile_name = $profile->name;

  if (user_access('customize ckeditor')) {
    foreach (array('default', 'show_toggle', 'width', 'lang', 'auto_lang') as $setting) {
      $conf[$setting] = ckeditor_user_get_setting($user, $profile, $setting);
    }
  }

  if (!isset($conf['ss'])) {
    $conf['ss'] = 2;
  }

  $themepath = drupal_get_path('theme', $current_theme) . '/';
  $host = base_path();

  // setting some variables
  $module_drupal_path = ckeditor_module_path('relative');
  $module_drupal_local_path = ckeditor_module_path('local');
  $editor_path = ckeditor_path('relative');
  $editor_local_path = ckeditor_path('local');

  $toolbar = $conf['toolbar'];

  if (!empty($conf['theme_config_js']) && $conf['theme_config_js'] == 't' && file_exists($themepath . 'ckeditor.config.js')) {
    $ckeditor_config_path = $host . $themepath . 'ckeditor.config.js?' . @filemtime($themepath . 'ckeditor.config.js');
  }
  else {
    $ckeditor_config_path = $module_drupal_path . "/ckeditor.config.js?" . @filemtime($module_drupal_path . "/ckeditor.config.js");
  }

  $settings['customConfig'] = $ckeditor_config_path;
  $settings['defaultLanguage'] = $conf['lang'];
  $settings['toolbar'] = $toolbar;
  $settings['enterMode'] = constant("CKEDITOR_ENTERMODE_" . strtoupper($conf['enter_mode']));
  $settings['shiftEnterMode'] = constant("CKEDITOR_ENTERMODE_" . strtoupper($conf['shift_enter_mode']));
  $settings['toolbarStartupExpanded'] = ( $conf['expand'] == 't' );
  if ($conf['expand'] == 'f') {
    $settings['toolbarCanCollapse'] = true;
  }
  $settings['width'] = $conf['width'];
  //check if skin exists, if not select default one
  if (isset($global_profile->settings['skin']) && file_exists($editor_local_path . '/skins/' . $global_profile->settings['skin'])) {
    $settings['skin'] = $global_profile->settings['skin'];
  }
  else {
    $settings['skin'] = ckeditor_default_skin();
  }
  $settings['format_tags'] = $conf['font_format'];
  $settings['show_toggle'] = $conf['show_toggle'];
  $settings['default'] = $conf['default'];
  if (!empty($conf['allowed_content']) && $conf['allowed_content'] === 'f') {
    $settings['allowedContent'] = true;
  }
  elseif (!empty($conf['extraAllowedContent'])) {
    $settings['extraAllowedContent'] = $conf['extraAllowedContent'];
  }
  $settings['ss'] = $conf['ss'];

  if (isset($conf['language_direction'])) {
    switch ($conf['language_direction']) {
      case 'default':
        if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL) {
          $settings['contentsLangDirection'] = 'rtl';
        }
        break;
      case 'ltr':
        $settings['contentsLangDirection'] = 'ltr';
        break;
      case 'rtl':
        $settings['contentsLangDirection'] = 'rtl';
        break;
    }
  }

  if (isset($conf['loadPlugins'])) {
    $settings['loadPlugins'] = ckeditor_plugins_render($conf['loadPlugins']);
  }
  else {
    $settings['loadPlugins'] = array();
  }

  //add support for divarea plugin from CKE4
  if (isset($conf['use_divarea']) && $conf['use_divarea'] == 't' && $editor_local_path != '<URL>' && file_exists($editor_local_path . '/plugins/divarea/plugin.js')) {
    $settings['loadPlugins']['divarea'] = array('name' => 'divarea', 'path' => $editor_path . '/plugins/divarea/', 'buttons' => FALSE, 'default' => 'f');
  }

  if (isset($conf['html_entities']) && $conf['html_entities'] == 'f') {
    $settings['entities'] = FALSE;
    $settings['entities_greek'] = FALSE;
    $settings['entities_latin'] = FALSE;
  }
  if (isset($conf['scayt_autoStartup']) && $conf['scayt_autoStartup'] == 't') {
    $settings['scayt_autoStartup'] = TRUE;
  }
  else {
    $settings['scayt_autoStartup'] = FALSE;
  }

  if ($conf['auto_lang'] == "f") {
    $settings['language'] = $conf['lang'];
    //[#1473010]
    $settings['scayt_sLang'] = ckeditor_scayt_langcode($conf['lang']);
  }

  if (isset($conf['forcePasteAsPlainText']) && $conf['forcePasteAsPlainText'] == 't') {
    $settings['forcePasteAsPlainText'] = TRUE;
  }

  if (isset($conf['custom_formatting']) && $conf['custom_formatting'] == 't') {
    foreach ($conf['formatting']['custom_formatting_options'] as $k => $v) {
      if ($v === 0) {
        $conf['formatting']['custom_formatting_options'][$k] = FALSE;
      }
      else {
        $conf['formatting']['custom_formatting_options'][$k] = TRUE;
      }
    }
    $settings['output_pre_indent'] = $conf['formatting']['custom_formatting_options']['pre_indent'];
    unset($conf['formatting']['custom_formatting_options']['pre_indent']);
    $settings['custom_formatting'] = $conf['formatting']['custom_formatting_options'];
  }

  // add code for filebrowser for users that have access
  $filebrowser = !empty($conf['filebrowser']) ? $conf['filebrowser'] : 'none';
  $filebrowser_image = !empty($conf['filebrowser_image']) ? $conf['filebrowser_image'] : $filebrowser;
  $filebrowser_flash = !empty($conf['filebrowser_flash']) ? $conf['filebrowser_flash'] : $filebrowser;

  if ($filebrowser == 'imce' && !module_exists('imce')) {
    $filebrowser = 'none';
  }

  if ($filebrowser == 'elfinder' && !module_exists('elfinder')) {
    $filebrowser = 'none';
  }

  /* MODULES NOT PORTED TO D7
    if ($filebrowser == 'tinybrowser' && !module_exists('tinybrowser')) {
    $filebrowser = 'none';
    }

    if ($filebrowser == 'ib' && !module_exists('imagebrowser')) {
    $filebrowser = 'none';
    }
    if ($filebrowser == 'webfm' && !module_exists('webfm_popup')) {
    $filebrowser = 'none';
    }
   */
  if ($filebrowser_image != $filebrowser) {
    if ($filebrowser_image == 'imce' && !module_exists('imce')) {
      $filebrowser_image = $filebrowser;
    }

    if ($filebrowser_image == 'elfinder' && !module_exists('elfinder')) {
      $filebrowser_image = $filebrowser;
    }
    /* MODULES NOT PORTED TO D7
      if ($filebrowser_image == 'tinybrowser' && !module_exists('tinybrowser')) {
      $filebrowser_image = $filebrowser;
      }
      if ($filebrowser_image == 'ib' && !module_exists('imagebrowser')) {
      $filebrowser_image = $filebrowser;
      }
      if ($filebrowser_image == 'webfm' && !module_exists('webfm_popup')) {
      $filebrowser_image = $filebrowser;
      }
     */
  }

  if ($filebrowser_flash != $filebrowser) {
    if ($filebrowser_flash == 'imce' && !module_exists('imce')) {
      $filebrowser_flash = $filebrowser;
    }

    if ($filebrowser_image == 'elfinder' && !module_exists('elfinder')) {
      $filebrowser_flash = $filebrowser;
    }
    /* MODULES NOT PORTED TO D7
      if ($filebrowser_image == 'tinybrowser' && !module_exists('tinybrowser')) {
      $filebrowser_flash = $filebrowser;
      }
      if ($filebrowser_flash == 'ib' && !module_exists('imagebrowser')) {
      $filebrowser_flash = $filebrowser;
      }
      if ($filebrowser_flash == 'webfm' && !module_exists('webfm_popup')) {
      $filebrowser_flash = $filebrowser;
      }
     */
  }

  if ($filebrowser == 'ckfinder' || $filebrowser_image == 'ckfinder' || $filebrowser_flash == 'ckfinder') {
    if (user_access('allow CKFinder file uploads')) {
      if (!empty($profile->settings['UserFilesPath'])) {
        $_SESSION['ckeditor'][$profile_name]['UserFilesPath'] = strtr($profile->settings['UserFilesPath'], array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => $user->uid, "%b" => $host, "%n" => $user->name));
      }
      else {
        $_SESSION['ckeditor'][$profile_name]['UserFilesPath'] = strtr('%b%f/', array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => $user->uid, "%b" => $host, "%n" => $user->name));
      }
      if (!empty($profile->settings['UserFilesAbsolutePath'])) {
        $_SESSION['ckeditor'][$profile_name]['UserFilesAbsolutePath'] = strtr($profile->settings['UserFilesAbsolutePath'], array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => $user->uid, "%b" => base_path(), "%d" => ckeditor_get_document_root_full_path(), "%n" => $user->name));
      }
      else {
        $_SESSION['ckeditor'][$profile_name]['UserFilesAbsolutePath'] = strtr('%d%b%f/', array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => $user->uid, "%b" => base_path(), "%d" => ckeditor_get_document_root_full_path(), "%n" => $user->name));
      }
      if (variable_get('file_default_scheme', '') == 'private') {
        $private_dir = isset($global_profile->settings['private_dir']) ? trim($global_profile->settings['private_dir'], '/') : '';
        if (!empty($private_dir)) {
          $private_dir = strtr($private_dir, array('%u' => $user->uid, '%n' => $user->name));
          $_SESSION['ckeditor'][$profile_name]['UserFilesPath'] = url('system/files') . '/' . $private_dir . '/';
          $private_upload_path = file_uri_target('private://' . variable_get('file_private_path', '')) . '/' . $private_dir;
        }
        else {
          $_SESSION['ckeditor'][$profile_name]['UserFilesPath'] = url('system/files') . '/';
          $private_upload_path =  file_uri_target('private://' . variable_get('file_private_path', ''));
        }
        //add '/' to beginning of path if necessary
        if (strpos(variable_get('file_private_path', ''), '/') === 0 && $private_upload_path[0] != '/') {
          $private_upload_path = '/' . $private_upload_path;
        }
        //check if CKEditor private dir exists and create it if not
        if ($private_dir && !is_dir($private_upload_path)) {
          mkdir($private_upload_path, 0755, TRUE);
        }
        $_SESSION['ckeditor'][$profile_name]['UserFilesAbsolutePath'] = drupal_realpath($private_upload_path) . '/';
      }
    }
  }
  /* MODULES NOT PORTED TO D7
    if (in_array('tinybrowser', array($filebrowser, $filebrowser_image, $filebrowser_flash))) {
    $popup_win_size = variable_get('tinybrowser_popup_window_size', '770x480');
    if (!preg_match('#\d+x\d+#is', $popup_win_size)) {
    $popup_win_size = '770x480';
    }
    $popup_win_size = trim($popup_win_size);
    $popup_win_size = strtolower($popup_win_size);
    $win_size = split('x', $popup_win_size);
    }
   */
  switch ($filebrowser) {
    case 'ckfinder':
      if (user_access('allow CKFinder file uploads')) {
        $ckfinder_full_path = ckfinder_path('relative');
        $settings['filebrowserBrowseUrl'] = $ckfinder_full_path . '/ckfinder.html?id=' . $profile_name;
        $settings['filebrowserImageBrowseUrl'] = $ckfinder_full_path . '/ckfinder.html?Type=Images&id=' . $profile_name;
        $settings['filebrowserFlashBrowseUrl'] = $ckfinder_full_path . '/ckfinder.html?Type=Flash&id=' . $profile_name;
        $settings['filebrowserUploadUrl'] = $ckfinder_full_path . '/core/connector/php/connector.php?command=QuickUpload&type=Files&id=' . $profile_name;
        $settings['filebrowserImageUploadUrl'] = $ckfinder_full_path . '/core/connector/php/connector.php?command=QuickUpload&type=Images&id=' . $profile_name;
        $settings['filebrowserFlashUploadUrl'] = $ckfinder_full_path . '/core/connector/php/connector.php?command=QuickUpload&type=Flash&id=' . $profile_name;
      }
      break;
    case 'imce':
      $settings['filebrowserBrowseUrl'] = url('imce', array('query' => array('app' => 'ckeditor|sendto@ckeditor_imceSendTo|')));
      break;
    case 'elfinder':
      $settings['filebrowserBrowseUrl'] = $host . "index.php?q=elfinder&app=ckeditor";
      break;
    /* MODULES NOT PORTED TO D7
      case 'webfm':
      if (user_access('access webfm')) {
      $settings['filebrowserBrowseUrl'] = $host . "index.php?q=webfm_popup";
      }
      break;
      case 'ib':
      if (user_access('browse own images')) {
      $settings['filebrowserBrowseUrl'] = $host . "index.php?q=imagebrowser/view/browser&app=ckeditor";
      $settings['filebrowserWindowWidth'] = 700;
      $settings['filebrowserWindowHeight'] = 520;
      }
      break;
      case 'tinybrowser':
      $settings['filebrowserBrowseUrl'] = $host . drupal_get_path('module', 'tinybrowser') . "/tinybrowser/tinybrowser.php?type=file";
      $settings['filebrowserWindowWidth'] = (int) $win_size[0] + 15;
      $settings['filebrowserWindowHeight'] = (int) $win_size[1] + 15;
      break;
     */
  }

  if ($filebrowser_image != $filebrowser) {
    switch ($filebrowser_image) {
      case 'ckfinder':
        if (user_access('allow CKFinder file uploads')) {
          $ckfinder_full_path = ckfinder_path('relative');
          $settings['filebrowserImageBrowseUrl'] = $ckfinder_full_path . '/ckfinder.html?Type=Images&id=' . $profile_name;
          $settings['filebrowserImageUploadUrl'] = $ckfinder_full_path . '/core/connector/php/connector.php?command=QuickUpload&type=Images&id=' . $profile_name;
        }
        break;
      case 'imce':
        $settings['filebrowserImageBrowseUrl'] = url('imce', array('query' => array('app' => 'ckeditor|sendto@ckeditor_imceSendTo|')));
        break;
      case 'elfinder':
        $settings['filebrowserImageBrowseUrl'] = $host . "index.php?q=elfinder&app=ckeditor";
        break;
      /* MODULES NOT PORTED TO D7
        case 'webfm':
        if (user_access('access webfm')) {
        $settings['filebrowserImageBrowseUrl'] = $host . "index.php?q=webfm_popup";
        }
        break;
        case 'ib':
        if (user_access('browse own images')) {
        $settings['filebrowserImageBrowseUrl'] = $host . "index.php?q=imagebrowser/view/browser&app=ckeditor";
        $settings['filebrowserImageWindowWidth'] = 680;
        $settings['filebrowserImageWindowHeight'] = 439;
        }
        break;
        case 'tinybrowser':
        $settings['filebrowserImageBrowseUrl'] = $host . drupal_get_path('module', 'tinybrowser') . "/tinybrowser/tinybrowser.php?type=image";
        $settings['filebrowserImageWindowWidth'] = (int) $win_size[0] + 15;
        $settings['filebrowserImageWindowHeight'] = (int) $win_size[1] + 15;
        break;
       */
    }
  }

  if ($filebrowser_flash != $filebrowser) {
    switch ($filebrowser_flash) {
      case 'ckfinder':
        if (user_access('allow CKFinder file uploads')) {
          $ckfinder_full_path = ckfinder_path('relative');
          $settings['filebrowserFlashBrowseUrl'] = $ckfinder_full_path . '/ckfinder.html?Type=Images&id=' . $profile_name;
          $settings['filebrowserFlashUploadUrl'] = $ckfinder_full_path . '/core/connector/php/connector.php?command=QuickUpload&type=Images&id=' . $profile_name;
        }
        break;
      case 'imce':
        $settings['filebrowserFlashBrowseUrl'] = url('imce', array('query' => array('app' => 'ckeditor|sendto@ckeditor_imceSendTo|')));
        break;
      case 'elfinder':
        $settings['filebrowserFlashBrowseUrl'] = $host . "index.php?q=elfinder&app=ckeditor";
        break;
      /* MODULES NOT PORTED TO D7
        case 'webfm':
        if (user_access('access webfm')) {
        $settings['filebrowserFlashBrowseUrl'] = $host . "index.php?q=webfm_popup";
        }
        break;
        case 'ib':
        if (user_access('browse own images')) {
        $settings['filebrowserFlashBrowseUrl'] = $host . "index.php?q=imagebrowser/view/browser&app=ckeditor";
        $settings['filebrowserFlashWindowWidth'] = 680;
        $settings['filebrowserFlashWindowHeight'] = 439;
        }
        break;
        case 'tinybrowser':
        $settings['filebrowserFlashBrowseUrl'] = $host . drupal_get_path('module', 'tinybrowser') . "/tinybrowser/tinybrowser.php?type=media";
        $settings['filebrowserFlashWindowWidth'] = (int) $win_size[0] + 15;
        $settings['filebrowserFlashWindowHeight'] = (int) $win_size[1] + 15;
        break;
       */
    }
  }

  if (!empty($conf['js_conf'])) {
    preg_match_all('#config\.(\w+)[\s]*=[\s]*(.+?);[\s]*(?=config\.|$)#is', preg_replace("/[\n\r]+/", "", $conf['js_conf']), $matches);
    foreach ($matches[2] as $i => $match) {
      if (!empty($match)) {
        $value = trim($match, " ;\n\r\t\0\x0B");
        if (strcasecmp($value, 'true') == 0) {
          $value = TRUE;
        }
        if (strcasecmp($value, 'false') == 0) {
          $value = FALSE;
        }
        $settings["js_conf"][$matches[1][$i]] = $value;
      }
    }
  }

  $settings['stylesCombo_stylesSet'] = "drupal:" . $module_drupal_path . '/ckeditor.styles.js';
  if (!empty($conf['css_style'])) {
    if ($conf['css_style'] == 'theme' && file_exists($themepath . 'ckeditor.styles.js')) {
      $settings['stylesCombo_stylesSet'] = "drupal:" . $host . $themepath . 'ckeditor.styles.js';
    }
    elseif (!empty($conf['css_style']) && $conf['css_style'] == 'self') {
      $conf['styles_path'] = str_replace("%h%t", "%t", $conf['styles_path']);
      $settings['stylesCombo_stylesSet'] = "drupal:" . str_replace(array('%h', '%t', '%m'), array($host, $host . $themepath, $module_drupal_path), $conf['styles_path']);
    }
  }

  // add custom stylesheet if configured
  // lets hope it exists but we'll leave that to the site admin
  $query_string = '?' . variable_get('css_js_query_string', '0');
  $css_files = array();
  switch ($conf['css_mode']) {
    case 'theme':
      global $language, $base_theme_info;
      $themes = list_themes();
      $theme_info = $themes[$current_theme];
      if (!empty($theme_info->stylesheets)) {
        $editorcss = "\"";
        foreach ($base_theme_info as $base) { // Grab stylesheets from base theme
          if (!empty($base->stylesheets)) { // may be empty when the base theme reference in the info file is invalid
            foreach ($base->stylesheets as $type => $stylesheets) {
              if ($type != "print") {
                foreach ($stylesheets as $name => $path) {
                  if (file_exists($path)) {
                    $css_files[$name] = $host . $path . $query_string;
                    // Grab rtl stylesheets ( will get rtl css files when thay are named with suffix "-rtl.css" (ex: fusion baased themes) )
                    if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL && substr($path, 0, -8) != "-rtl.css") {
                      $rtl_path = substr($path, 0, -4) . "-rtl.css";
                      if (file_exists($rtl_path)) {
                        $css_files[$name . "-rtl"] = $host . $rtl_path . $query_string;
                      }
                    }
                  }
                }
              }
            }
          }
        }
        if (!empty($theme_info->stylesheets)) { // Grab stylesheets from current theme
          foreach ($theme_info->stylesheets as $type => $stylesheets) {
            if ($type != "print") {
              foreach ($stylesheets as $name => $path) {
                // Checks if less module exists...
                if (strstr($path, '.less') && module_exists('less')) {
                  $path = 'sites/default/files/less/' . $path; // append the less file path
                  $path = str_replace('.less', '', $path); // remove the .less
                }
                if (file_exists($path)) {
                  $css_files[$name] = $host . $path . $query_string;
                  // Grab rtl stylesheets ( will get rtl css files when thay are named with suffix "-rtl.css" (ex: fusion baased themes) )
                  if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL && substr($path, 0, -8) != "-rtl.css") {
                    $rtl_path = substr($path, 0, -4) . "-rtl.css";
                    if (file_exists($rtl_path)) {
                      $css_files[$name . "-rtl"] = $host . $rtl_path . $query_string;
                    }
                  }
                }
                elseif (!empty($css_files[$name])) {
                  unset($css_files[$name]);
                }
              }
            }
          }
        }
        // Grab stylesheets local.css and local-rtl.css if they exist (fusion based themes)
        if (file_exists($themepath . 'css/local.css')) {
          $css_files[] = $host . $themepath . 'css/local.css' . $query_string;
        }
        if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL && file_exists($themepath . 'css/local-rtl.css')) {
          $css_files[] = $host . $themepath . 'css/local-rtl.css' . $query_string;
        }

        // Grab stylesheets from color module
        $color_paths = variable_get('color_' . $current_theme . '_stylesheets', array());
        if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL) {
          if (!empty($color_paths[1])) {
            $css_files[] = $host . $color_paths[1] . $query_string;
          }
        }
        elseif (!empty($color_paths[0])) {
          $css_files[] = $host . $color_paths[0] . $query_string;
        }
      }
      else {
        if (!file_exists($themepath . 'ckeditor.css') && file_exists($themepath . 'style.css')) {
          $css_files[] = $host . $themepath . 'style.css' . $query_string;
        }
      }
      if (file_exists($module_drupal_local_path . '/css/ckeditor.css')) {
        $css_files[] = $module_drupal_path . '/css/ckeditor.css' . $query_string;
      }
      if (file_exists($themepath . 'ckeditor.css')) {
        $css_files[] = $host . $themepath . 'ckeditor.css' . $query_string;
      }
      break;

    case 'self':
      if (file_exists($module_drupal_local_path . '/css/ckeditor.css')) {
        $css_files[] = $module_drupal_path . '/css/ckeditor.css' . $query_string;
        if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL) {
          if (file_exists($module_drupal_local_path . '/css/ckeditor-rtl.css')) {
            $css_files[] = $module_drupal_path . '/css/ckeditor-rtl.css' . $query_string;
          }
        }
      }
      foreach (explode(',', $conf['css_path']) as $css_path) {
        $css_path = trim(str_replace("%h%t", "%t", $css_path));
        $css_files[] = str_replace(array('%h', '%t'), array($host, $host . $themepath), $css_path) . $query_string;
      }
      break;

    case 'none':
      if (file_exists($module_drupal_local_path . '/css/ckeditor.css')) {
        $css_files[] = $module_drupal_path . '/css/ckeditor.css' . $query_string;
        if (defined('LANGUAGE_RTL') && $language->direction == LANGUAGE_RTL) {
          if (file_exists($module_drupal_local_path . '/css/ckeditor-rtl.css')) {
            $css_files[] = $module_drupal_path . '/css/ckeditor-rtl.css' . $query_string;
          }
        }
      }
      if ($editor_local_path != '<URL>') {
        if (file_exists($editor_local_path . '/contents.css')) {
          $css_files[] = $editor_path . '/contents.css' . $query_string;
        }
      }
      else {
        $editor_url_path = ckeditor_path('url');
        $css_files[] = $editor_url_path . '/contents.css' . $query_string;
      }
      break;
  }

  if (isset($conf['ckeditor_load_method']) && $conf['ckeditor_load_method'] == 'ckeditor_source.js') {
    foreach ($css_files as $k => $v) {
      $css_files[$k] = $v . '&t=' . time();
    }
  }

  $settings['contentsCss'] = array_values($css_files);

  if (!empty($conf['uicolor']) && $conf['uicolor'] == "custom" && !empty($conf['uicolor_user'])) {
    $settings['uiColor'] = $conf['uicolor_user'];
  }

  if (!empty($conf['uicolor']) && strpos($conf['uicolor'], "color_") === 0) {
    if (function_exists('color_get_palette')) {
      $palette = @color_get_palette($current_theme, FALSE); //[#652274]
      $color = str_replace("color_", "", $conf['uicolor']);
      if (!empty($palette[$color])) {
        $settings['uiColor'] = $palette[$color];
      }
    }
  }

  if (!empty($settings['loadPlugins'])) {
    // Remove orphaned plugins (coming from disabled modules).
    $available_plugins = array_keys(ckeditor_load_plugins());
    $settings['loadPlugins'] = array_intersect_key($settings['loadPlugins'], array_flip($available_plugins));
  }

  // Allow modules to modify the settings.
  drupal_alter('ckeditor_settings', $settings, $conf);

  return $settings;
}

/**
 * Load CKEditor for field ID
 *
 * @param object $field
 * @param string $format
 *
 * @return object
 *
 */
function ckeditor_load_by_field($field, $format, $show_toggle = TRUE, $add_fields_to_toggle = FALSE) {
  global $user, $theme;
  static $processed_ids = array();
  static $is_running = FALSE;
  $use_ckeditor = FALSE;
  $format_arr = FALSE;
  $suffix = '';

  if (is_array($format)) {
    $format_arr = $format;
    $format = isset($format_arr['#value']) ? $format_arr['#value'] : $format_arr['#default_value'];
  }

  if (!isset($field['#id'])) {
    return $field;
  }

  if (isset($processed_ids[$field['#id']])) {
    return $field;
  }

  if (key_exists('#wysiwyg', $field) && !$field['#wysiwyg']) {
    return $field;
  }

  if (isset($field['#access']) && !$field['#access']) {
    return $field;
  }

  if ($field['#id'] == "edit-log") {
    return $field;
  }

  if (isset($field['#attributes']['disabled']) && $field['#attributes']['disabled'] == 'disabled') {
    return $field;
  }

  drupal_add_js(array('ckeditor' => array('textarea_default_format' => array($field['#id'] => $format))), 'setting');

  if (!isset($processed_ids[$field['#id']])) {
    $processed_ids[$field['#id']] = array();
  }

  $global_profile = ckeditor_profile_load('CKEditor Global Profile');
  $profile = ckeditor_get_profile($format);
  $host = base_path();

  if ($profile === FALSE) {
    $ckeditor_in_default_format = FALSE;
    foreach ((array) $format_arr['#options'] as $key => $val) {
      if ($key == $format)
        continue;
      if ($profile = ckeditor_get_profile($key)) {
        $use_ckeditor = $key;
        break;
      }
    }
    if ($use_ckeditor === FALSE) {
      return $field;
    }
  }
  else {
    $ckeditor_in_default_format = TRUE;
  }

  if ($settings = ckeditor_profiles_compile($format)) {
    $ckeditor_on = ($settings['default'] == 't' && $profile->settings['default'] == 't') ? TRUE : FALSE;
  }
  elseif ($settings = ckeditor_profiles_compile($use_ckeditor)) {
    $ckeditor_on = FALSE;
  }
  else {
    return $field;
  }

  // Attach the editor css.
  $field['#attached']['css'][] = drupal_get_path('module', 'ckeditor') . '/css/ckeditor.editor.css';

  if ($settings) {
    $textarea_id = $field['#id'];
    $class[] = 'ckeditor-mod';
    $_ckeditor_ids[] = $textarea_id;

    //settings are saved as strings, not booleans
    if ($settings['show_toggle'] == 't' && $show_toggle) {

      if ($add_fields_to_toggle !== FALSE) {
        if (is_array($add_fields_to_toggle)) {
          $toggle_fields = "['" . $textarea_id . "','" . implode("','", $add_fields_to_toggle) . "']";
        }
        else {
          $toggle_fields = "['" . $textarea_id . "','" . $add_fields_to_toggle . "']";
        }
      }
      else {
        $toggle_fields = "['{$textarea_id}']";
      }

      $wysiwyg_link = '';
      $wysiwyg_link .= "<a class=\"ckeditor_links\" style=\"display:none\" href=\"javascript:void(0);\" onclick=\"javascript:Drupal.ckeditorToggle({$toggle_fields},'" . str_replace("'", "\\'", t('Switch to plain text editor')) . "','" . str_replace("'", "\\'", t('Switch to rich text editor')) . "');\" id=\"switch_{$textarea_id}\">";
      $wysiwyg_link .= $ckeditor_on ? t('Switch to plain text editor') : t('Switch to rich text editor');
      $wysiwyg_link .= '</a>';

      // Make sure to append to #suffix so it isn't completely overwritten
      $suffix .= $wysiwyg_link;
    }

    $editor_local_path = ckeditor_path('local');
    $editor_url_path = ckeditor_path('url');

    if (!$is_running) {
      // By default sessions are not started automatically for anonymous users.
      // Start one for editing content so that we had a consistent token that is used in XSS filter.
      if (isset($field['#entity']) && !empty($field['#entity']->created) && empty($user->uid)) {
        drupal_session_start();
        $_SESSION['ckeditor_anonymous_user'] = true;
        drupal_page_is_cacheable(FALSE);
      }

      if (!$ckeditor_in_default_format) {
        $load_method = 'ckeditor_basic.js';
        $load_time_out = 0;
      }
      elseif (isset($profile->settings['ckeditor_load_method'])) {
        $load_method = $profile->settings['ckeditor_load_method'];
        $load_time_out = $profile->settings['ckeditor_load_time_out'];
      }
      if ($editor_local_path != '<URL>') {
        drupal_add_js('window.CKEDITOR_BASEPATH = "' . ckeditor_path('relative') . '/"', array('type' => 'inline', 'weight' => -100));
      }
      drupal_add_js(ckeditor_module_path('url') . '/includes/ckeditor.utils.js', array('type' => 'file', 'scope' => 'footer'));

      $preprocess = FALSE;
      if (isset($global_profile->settings['ckeditor_aggregate']) && $global_profile->settings['ckeditor_aggregate'] == 't') {
        $preprocess = TRUE;
      }

      if ($editor_local_path == '<URL>') {
        drupal_add_js($editor_url_path . '/ckeditor.js', array('type' => 'external', 'scope' => 'footer'));
      }
      else if (isset($load_method) && file_exists($editor_local_path . '/' . $load_method)) {
        drupal_add_js($editor_url_path . '/' . $load_method, array('type' => 'file', 'scope' => 'footer', 'preprocess' => $preprocess));
        if ($load_method == 'ckeditor_basic.js') {
          drupal_add_js('CKEDITOR.loadFullCoreTimeout = ' . $load_time_out . ';', array('type' => 'inline', 'scope' => 'footer'));
          drupal_add_js(array('ckeditor' => array('load_timeout' => TRUE)), 'setting');
        }
      }
      else {
        drupal_add_js($editor_url_path . '/ckeditor.js', array('type' => 'file', 'scope' => 'footer', 'preprocess' => $preprocess));
      }
      $ckeditor_url = ckeditor_path('relative');
      if ($ckeditor_url == '<URL>') {
        $ckeditor_url = ckeditor_path('url');
      }
      $ckeditor_url .= '/';
      drupal_add_js(array('ckeditor' => array('module_path' => ckeditor_module_path('relative'), 'editor_path' => $ckeditor_url)), 'setting');
      if (module_exists('paging')) {
        drupal_add_js(array('ckeditor' => array('pagebreak' => TRUE)), 'setting');
      }
      if (module_exists('linktocontent_node')) {
        drupal_add_js(array('ckeditor' => array('linktocontent_node' => TRUE)), 'setting');
      }
      if (module_exists('linktocontent_menu')) {
        drupal_add_js(array('ckeditor' => array('linktocontent_menu' => TRUE)), 'setting');
      }
      if (module_exists('pagebreak')) {
        drupal_add_js(array('ckeditor' => array('pagebreak' => TRUE)), 'setting');
      }
      if (module_exists('smart_paging')) {
        drupal_add_js(array('ckeditor' => array('pagebreak' => TRUE)), 'setting');
      }
      drupal_add_js(array('ckeditor' => array('ajaxToken' => drupal_get_token('ckeditorAjaxCall'), 'xss_url' => url('ckeditor/xss'))), 'setting');
      $is_running = TRUE;
    }

    drupal_add_js(array('ckeditor' => array('theme' => $theme)), 'setting');
    if (!empty($settings)) {
      drupal_add_js(array('ckeditor' => array('elements' => array($textarea_id => $format))), 'setting');
    }
    if (!empty($ckeditor_on)) {
      drupal_add_js(array('ckeditor' => array('autostart' => array($textarea_id => $ckeditor_on))), 'setting');
    }
    //[#1473010]
    if (isset($settings['scayt_sLang'])) {
      drupal_add_js(array('ckeditor' => array('scayt_language' => $settings['scayt_sLang'])), 'setting');
    }
    elseif (!empty($field["#language"]) && $field["#language"] != LANGUAGE_NONE) {
      drupal_add_js(array('ckeditor' => array('scayt_language' => ckeditor_scayt_langcode($field["#language"]))), 'setting');
    }

    // Remember extra information and reuse it during "Preview"
    $processed_ids[$field['#id']]['suffix'] = $suffix;
    $processed_ids[$field['#id']]['class'] = $class;

    if (empty($field['#suffix'])) {
      $field['#suffix'] = $suffix;
    }
    else {
      $field['#suffix'] .= $suffix;
    }

    if (empty($field['#attributes']['class'])) {
      $field['#attributes']['class'] = $class;
    }
    else {
      $field['#attributes']['class'] = array_merge($field['#attributes']['class'], $class);
    }
  }

  return $field;
}

/**
 * Return all modules that provide security filters.
 */
function ckeditor_security_filters() {
  $security_filters = array();

  $security_filters['modules'] = array(
    'htmLawed' => array(
      'title' => 'htmLawed',
      'project_page' => 'http://drupal.org/project/htmLawed',
      'weight' => 0,
      'installed' => FALSE,
      'filters' => array()
    ),
    'htmltidy' => array(
      'title' => 'Htmltidy',
      'project_page' => 'http://drupal.org/project/htmltidy',
      'weight' => 0,
      'installed' => FALSE,
      'filters' => array()
    ),
    'htmlpurifier' => array(
      'title' => 'HTML Purifier',
      'project_page' => 'http://drupal.org/project/htmlpurifier',
      'weight' => 0,
      'installed' => FALSE,
      'filters' => array()
    ),
    'wysiwyg_filter' => array(
      'title' => 'WYSIWYG Filter',
      'project_page' => 'http://drupal.org/project/wysiwyg_filter',
      'weight' => 0,
      'installed' => FALSE,
      'filters' => array()
    )
  );

  $security_filters['filters'] = array();

  foreach ($security_filters['modules'] as $module_name => $module_conf) {
    if (module_exists($module_name)) {
      $security_filters['modules'][$module_name]['installed'] = TRUE;
      $module_filters = module_invoke($module_name, 'filter_info');
      foreach ($module_filters as $module_filter_name => $module_filter_conf) {
        $security_filters['modules'][$module_name]['filters'][$module_filter_name] = $module_filter_conf;
        $security_filters['filters'][$module_filter_name] = TRUE;
      }
    }
  }

  //add filters from Drupal core
  $security_filters['modules']['__drupal'] = array(
    'title' => 'Drupal core',
    'project_page' => FALSE,
    'weight' => -1,
    'installed' => TRUE,
    'filters' => array(
      'filter_html' => array(
        'title' => 'Limit allowed HTML tags',
        'description' => 'Removes the attributes that the built-in "Limit allowed HTML tags"-filter does not allow inside HTML elements/tags'
      )
    )
  );
  $security_filters['filters']['filter_html'] = TRUE;

  //load security filters added by API
  $external_module_filters = module_invoke_all('ckeditor_security_filter');
  if (count($external_module_filters) > 0) {
    $security_filters['modules']['__external'] = array(
      'title' => 'External filters',
      'project_page' => FALSE,
      'weight' => 1,
      'installed' => TRUE,
      'filters' => array()
    );
    foreach ($external_module_filters as $module_filter_name => $module_filter_conf) {
      $security_filters['modules']['__external']['filters'][$module_filter_name] = $module_filter_conf;
      $security_filters['filters'][$module_filter_name] = TRUE;
    }
  }
  drupal_alter('ckeditor_security_filter', $security_filters);

  return $security_filters;
}
