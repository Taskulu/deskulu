<?php

/**
 * @file
 * CKEditor hooks implemented by the CKEditor module.
 */

/**
 * Implements hook_ckeditor_plugin().
 */
function ckeditor_ckeditor_plugin() {
  $plugins = array();
  $editor_path = '%editor_path%';
  $plugin_dir = '%plugin_dir%';
  $plugin_dir_additional = '%plugin_dir_extra%';
  $pattern = '#\.addButton\([\s]*[\'"](.*?)[\'"][\s]*\,[\s]*\{[\s]*(.*?)[\s]*\}#s';

  /**
   * CKEditor built-in plugins.
   */
  $_editor_path = ckeditor_path('local');
  if ($_editor_path != '<URL>') {
    if (file_exists($_editor_path . '/plugins/tableresize/plugin.js')) {
      $plugins['tableresize'] = array(
        'name' => 'tableresize',
        'desc' => t('Table Resize plugin'),
        'path' => $editor_path . '/plugins/tableresize/',
        'buttons' => FALSE,
        'default' => 't'
      );
    }

    if (file_exists($_editor_path . '/plugins/autogrow/plugin.js')) {
      $plugins['autogrow'] = array(
        'name' => 'autogrow',
        'desc' => t('Auto Grow plugin'),
        'path' => $editor_path . '/plugins/autogrow/',
        'buttons' => FALSE,
        'default' => 'f'
      );
    }

    if (file_exists($_editor_path . '/plugins/stylesheetparser/plugin.js')) {
      $plugins['stylesheetparser'] = array(
        'name' => 'stylesheetparser',
        'desc' => t('Stylesheet Parser plugin'),
        'path' => $editor_path . '/plugins/stylesheetparser/',
        'buttons' => FALSE,
        'default' => 'f'
      );
    }
  }
  else {
    $_editor_url = ckeditor_path('url');
    if (preg_match("/\/(standard|full)-all/", $_editor_url)) {
      $plugins['tableresize'] = array(
        'name' => 'tableresize',
        'desc' => t('Table Resize plugin. See <a href="@ckdocs-tableresize">addon page</a> for more details.',
          array('@ckdocs-tableresize' => 'http://ckeditor.com/addon/tableresize')
        ),
        'path' => $_editor_url . '/plugins/tableresize/',
        'buttons' => FALSE,
        'default' => 't'
      );
      $plugins['autogrow'] = array(
        'name' => 'autogrow',
        'desc' => t('Auto Grow plugin. See <a href="@ckdocs-autogrow">addon page</a> for more details.',
          array('@ckdocs-autogrow' => 'http://ckeditor.com/addon/autogrow')
        ),
        'path' => $_editor_url . '/plugins/autogrow/',
        'buttons' => FALSE,
        'default' => 'f'
      );
      $plugins['stylesheetparser'] = array(
        'name' => 'stylesheetparser',
        'desc' => t('Stylesheet Parser plugin. See <a href="@ckdocs-stylesheetparser">addon page</a> for more details.',
          array('@ckdocs-stylesheetparser' => 'http://ckeditor.com/addon/stylesheetparser')
        ),
        'path' => $_editor_url . '/plugins/stylesheetparser/',
        'buttons' => FALSE,
        'default' => 'f'
      );
      $plugins['codesnippet'] = array(
        'name' => 'codesnippet',
        'desc' => t('Plugin for inserting Code Snippets. See <a href="@ckdocs-codesnippet">addon page</a> for more details. See <a href="@ckdocs-codesnippet-help">help</a> for additional instructions.',
          array(
            '@ckdocs-codesnippet' => 'http://ckeditor.com/addon/codesnippet',
            '@ckdocs-codesnippet-help' => url('admin/help/ckeditor', array('fragment' => 'codesnippet')),
          )
        ),
        'path' => $_editor_url . '/plugins/codesnippet/',
        'buttons' => array(
          'CodeSnippet' => array(
            'icon' => 'icons/codesnippet.png',
            'label' => t('Insert Code Snippet'),
          )
        ),
        'default' => 'f'
      );
      $plugins['mathjax'] = array(
        'name' => 'mathjax',
        'desc' => t('Plugin for inserting Mathematical Formula (MathJax). See <a href="@ckdocs-mathjax">addon page</a> for more details. See <a href="@ckdocs-mathjax-help">help</a> for additional instructions.',
          array(
            '@ckdocs-mathjax' => 'http://ckeditor.com/addon/mathjax',
            '@ckdocs-mathjax-help' => url('admin/help/ckeditor', array('fragment' => 'mathjax')),
        )),
        'path' => $_editor_url . '/plugins/mathjax/',
        'buttons' => array(
          'Mathjax' => array(
            'icon' => 'icons/mathjax.png',
            'label' => t('Insert Mathematical Formulas'),
          )
        ),
        'default' => 'f'
      );
      $plugins['image2'] = array(
        'name' => 'image2',
        'desc' => t('Enhanced Image plugin. See <a href="@ckdocs-image2">addon page</a> for more details.',
          array('@ckdocs-image2' => 'http://ckeditor.com/addon/image2')
        ),
        'path' => $_editor_url . '/plugins/image2/',
        'buttons' => array(
          'Enhanced Image' => array(
            'icon' => 'icons/image.png',
            'label' => t('Insert Enhanced Image'),
          )
        ),
        'default' => 't'
      );
    }
  }

  /**
   * Plugins located directly in the CKEditor module folder.
   */
  $_plugin_dir = ckeditor_module_path('local') . '/plugins/';
  if ($handle = opendir($_plugin_dir)) {
    while (false !== ($file = readdir($handle))) {
      if (is_dir($_plugin_dir . $file) && file_exists($_plugin_dir . $file . '/plugin.js')) {
        $source = file_get_contents($_plugin_dir . $file . '/plugin.js');
        $buttons = array();
        if (preg_match_all($pattern, $source, $matches)) {
          foreach ($matches[1] as $i => $button_name) {
            if (preg_match('#(icon)[\s]*\:[\s]*([^\,\n]*)#', $matches[2][$i], $matches2)) {
              $buttons[$button_name] = array();
              $buttons[$button_name]['label'] = $button_name;
              $matches2[2] = str_replace(array('this.path', '+', '\'', '"'), array('', '', '', ''), $matches2[2]);
              $buttons[$button_name]['icon'] = trim($matches2[2]);
            }
          }
        }
        if (preg_match('#@file ([^\n\r]*)#', $source, $matches)) {
          $plugins[$file] = array(
            'name' => $file,
            'desc' => check_plain($matches[1]),
            'path' => $plugin_dir . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
        else {
          $plugins[$file] = array(
            'name' => $file,
            'desc' => t('Plugin file: @plugin_filename', array('@plugin_filename' => $file)),
            'path' => $plugin_dir . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
      }
    }
    closedir($handle);
  }

  /**
   * Plugins located in a folder specified in the CKEditor Global Profile.
   */
  $_plugin_dir_additional = ckeditor_plugins_path('local') . '/';
  if ($_plugin_dir != $_plugin_dir_additional && is_dir($_plugin_dir_additional) && $handle = opendir($_plugin_dir_additional)) {
    while (false !== ($file = readdir($handle))) {
      if (is_dir($_plugin_dir_additional . $file) && file_exists($_plugin_dir_additional . $file . '/plugin.js')) {
        $source = file_get_contents($_plugin_dir_additional . $file . '/plugin.js');
        $buttons = array();
        if (preg_match_all($pattern, $source, $matches)) {
          foreach ($matches[1] as $i => $button_name) {
            if (preg_match('#(icon)[\s]*\:[\s]*([^\,\n]*)#', $matches[2][$i], $matches2)) {
              $buttons[$button_name] = array();
              $buttons[$button_name]['label'] = $button_name;
              $matches2[2] = str_replace(array('this.path', '+', '\'', '"'), array('', '', '', ''), $matches2[2]);
              $buttons[$button_name]['icon'] = trim($matches2[2]);
            }
          }
        }
        if (preg_match('#@file ([^\n\r]*)#', $source, $matches)) {
          $plugins[$file] = array(
            'name' => $file,
            'desc' => check_plain($matches[1]),
            'path' => $plugin_dir_additional . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
        else {
          $plugins[$file] = array(
            'name' => $file,
            'desc' => t('Plugin file: @plugin_filename', array('@plugin_filename' => $file)),
            'path' => $plugin_dir_additional . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
      }
    }

    closedir($handle);
  }

  // Remove the IMCE plugin if the IMCE module isn't installed.
  if (isset($plugins['imce']) && module_exists('imce') == FALSE) {
    unset($plugins['imce']);
  }

  // Enable the Drupal Breaks plugin by default if the plugin is available.
  if (isset($plugins['drupalbreaks'])) {
    $plugins['drupalbreaks']['default'] = 't';

    // Remove the page break button if there is no module to handle it.
    if (isset($plugins['drupalbreaks']['buttons']['DrupalPageBreak']) && !module_exists('paging') && !module_exists('pagebreak')) {
      unset($plugins['drupalbreaks']['buttons']['DrupalPageBreak']);
    }
  }

  return $plugins;
}
