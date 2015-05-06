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
 * Main administrative page
 */
function ckeditor_admin_main() {
  global $base_url;
  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
  $editor_path = ckeditor_path('local');
  $ckconfig_file = $editor_path . '/config.js';

  //check if CKEditor plugin is installed
  if ($editor_path != '<URL>' && !_ckeditor_requirements_isinstalled()) {
    drupal_set_message(t(
            'Checking for !filename or !file.', array(
          '!filename' => '<code>' . $ckconfig_file . '</code>',
          '!file' => '<code>sites/all/libraries/ckeditor/ckeditor.js</code>'
            )
        ));
    drupal_set_message(t(
            'The CKEditor component is not installed correctly. Please go to the !ckeditorlink in order to download the latest version. After that you must extract the files to the !ckeditorpath or !librarypath directory and make sure that the !ckeditorfile or !ckeditorlibrary file exists. Refer to the !readme file for more information.', array(
          '!ckeditorlink' => l(t('CKEditor homepage'), 'http://ckeditor.com/download'),
          '!readme' => l(t('README.txt'), $base_url . '/' . drupal_get_path('module', 'ckeditor') . '/README.txt', array('absolute' => TRUE)),
          '!ckeditorpath' => '<code>sites/all/modules/ckeditor/ckeditor</code>',
          '!ckeditorfile' => '<code>sites/all/modules/ckeditor/ckeditor/ckeditor.js</code>',
          '!ckeditorlibrary' => '<code>sites/all/libraries/ckeditor/ckeditor.js</code>',
          '!librarypath' => '<code>sites/all/libraries/ckeditor</code>'
            )
        ), 'error');
    drupal_set_message(t(
            'If you have CKEditor already installed, edit the <strong>!editg</strong> and update the CKEditor path.', array(
          '!editg' => l(t('CKEditor Global Profile'), 'admin/config/content/ckeditor/editg')
            )
        ), 'warning');

    return '';
  }

  if (module_exists('wysiwyg')) {
    drupal_set_message(t(
            'The WYSIWYG module was detected. Using both modules at the same time may cause problems. It is recommended to turn the WYSIWYG module off (!wysiwygdisablelink).', array(
          '!wysiwygdisablelink' => l(t('click here to disable'), 'ckeditor/disable/wysiwyg/' . drupal_get_token('ckeditorDisableWysiwyg'))
            )
        ), 'warning');
  }

  //find profile other than Global
  $result = db_select('ckeditor_settings', 's')->fields('s', array('name'))->condition('name', 'CKEditor Global Profile', '<>')->range(0, 1)->execute()->fetchAssoc();
  if (!$result) {
    drupal_set_message(t('No CKEditor profiles found. Right now nobody is able to use CKEditor. Create a new profile below.'), 'error');
  }

  return ckeditor_profile_overview();
}

/**
 * Controller for CKEditor profiles.
 */
function ckeditor_profile_overview() {
  $output = '';
  $skins = ckeditor_load_skin_options();
  $global_profile = ckeditor_profile_load('CKEditor Global Profile');
  if (isset($global_profile->settings['skin']) && !array_key_exists($global_profile->settings['skin'], $skins)) {
    drupal_set_message(t('The <em>CKEditor Global Profile</em> profile is using %skin skin which cannot be found. Please <a href="@profile_settings">update your settings</a>.', array('%skin' => $global_profile->settings['skin'], '@profile_settings' => url('admin/config/content/ckeditor/editg'))), 'warning');
  }
  $profiles = ckeditor_profile_load();
  if ($profiles) {
    $access_ckeditor_roles = user_roles(FALSE, 'access ckeditor');
    $header = array(t('Profile'), t('Input format'), t('Operations'));
    $plugins = ckeditor_load_plugins();
    $disabled_plugins = array();
    foreach ($profiles as $p) {
      if ($p->name !== "CKEditor Global Profile") {
        if (isset($p->settings['loadPlugins']) && is_array($p->settings['loadPlugins']) && count($p->settings['loadPlugins']) > 0) {
          $changed = FALSE;
          foreach ($p->settings['loadPlugins'] as $plugin_name => $plugin_settings) {
            if (!array_key_exists($plugin_name, $plugins)) {
              if (isset($plugin_settings['active']) && $plugin_settings['active'] == 0) {
                continue;
              }
              if (!isset($disabled_plugins[$p->name])) {
                $disabled_plugins[$p->name] = array();
              }
              $p->settings['loadPlugins'][$plugin_name]['active'] = 0;
              $disabled_plugins[$p->name][] = $plugin_name;
              $changed = TRUE;
            }
          }
          if ($changed === TRUE) {
            db_update('ckeditor_settings')
                ->fields(array(
                  'settings' => serialize($p->settings)
                ))
                ->condition('name', $p->name, '=')
                ->execute();
          }
        }
        $rows[] = array(
          array('data' => $p->name, 'valign' => 'top'),
          array('data' => implode("<br />\n", $p->input_formats)),
          array(
            'data' =>
            l(t('edit'), 'admin/config/content/ckeditor/edit/' . urlencode($p->name)) . ' ' .
            l(t('clone'), 'admin/config/content/ckeditor/clone/' . urlencode($p->name)) . ' ' .
            l(t('delete'), 'admin/config/content/ckeditor/delete/' . urlencode($p->name)),
            'valign' => 'top'
          )
        );
      }
    }

    if (count($disabled_plugins) > 0) {
      $msg = t("The following plugins could not be found and were automatically disabled in CKEditor profiles:");
      foreach ($disabled_plugins as $profile_name => $profile_plugins) {
        $msg .= "<br/><br/>";
        $msg .= t("<b>Profile</b>: %profile_name", array("%profile_name" => $profile_name));
        $msg .= "<br/>";
        $msg .= t("<b>Plugins</b>: %profile_plugins", array("%profile_plugins" => implode(', ', $profile_plugins)));
      }
      drupal_set_message($msg, 'warning');
    }

    $output .= '<h3>' . t('Profiles') . '</h3>';
    $output .= theme('table', array("header" => $header, "rows" => $rows));
    $output .= '<p>' . l(t('Create a new profile'), 'admin/config/content/ckeditor/add') . '</p>';
  }
  else {
    drupal_set_message(t(
            'No profiles found. Click here to !create.', array(
          '!create' => l(t('create a new profile'), 'admin/config/content/ckeditor/add')
            )
        ), 'warning');
  }

  $rows = array();
  if (!isset($profiles['CKEditor Global Profile'])) {
    drupal_set_message(t(
            'The global profile can not be found. Click here to !create.', array(
          '!create' => l(t('create the global profile'), 'admin/config/content/ckeditor/addg')
            )
        ), 'warning');
  }
  else {
    $output .= "<h3>" . t("Global settings") . "</h3>";
    $rows[] = array(
      array('data' => t('CKEditor Global Profile')),
      array('data' => l(t('edit'), 'admin/config/content/ckeditor/editg'), 'valign' => 'top')
    );
    $output .= theme('table', array("header" => array(t('Profile'), t('Operations')), "rows" => $rows));
  }
  return $output;
}

/**
 * Form builder for a global profile
 */
function ckeditor_admin_global_profile_form($form, $form_state, $mode = 'add') {
  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
  if ($mode == 'edit') {
    $profile = ckeditor_profile_load('CKEditor Global Profile');

    $form['_profile'] = array(
      '#type' => 'value',
      '#value' => $profile,
    );
  }
  else {
    $profile = new stdClass();
  }

  if ($mode == 'add') {
    $data = ckeditor_profile_load('CKEditor Global Profile');
    if (!empty($data)) {
      drupal_set_message(t('The global profile already exists. Only one global profile is allowed.'), 'error');
      drupal_not_found();
    }

    $btn = t('Create a global profile');
  }
  else {
    $btn = t('Update the global profile');
  }

  $orig_formats = filter_formats();
  $formats = array();
  foreach ($orig_formats AS $format) {
    $formats[$format->format] = $format->name;
  }

  $drupal_base_path = ckeditor_base_path('relative');
  $drupal_library_path = ckeditor_library_path('relative');
  $module_drupal_path = ckeditor_module_path('relative');
  $skin_options = ckeditor_load_skin_options();

  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Editor appearance'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  //if skin is not set or not exists select default skin (moono or skin) or first on the skin list
  if (empty($profile->settings['skin']) || (!empty($profile->settings['skin']) && !array_key_exists($profile->settings['skin'], $skin_options))) {
    $profile->settings['skin'] = ckeditor_default_skin();
  }

  $form['appearance']['skin'] = array(
    '#type' => 'select',
    '#title' => t('Skin'),
    '#default_value' => $profile->settings['skin'],
    '#options' => $skin_options,
    '#description' => t('Choose a CKEditor skin.'),
  );

  $form['ckeditor_advanced_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $current_path = ckeditor_path('relative');
  if ($current_path == '<URL>') {
    $current_path = ckeditor_path('url');
  }
  $form['ckeditor_advanced_settings']['ckeditor_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to CKEditor'),
    '#default_value' => !empty($profile->settings['ckeditor_path']) ? $profile->settings['ckeditor_path'] : '%m/ckeditor',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t(
        'The path to CKEditor (the WYSIWYG rich text editor downloaded from !ckeditorcom). Provide a path relative to the document root, unless CKEditor is located on an external URL. In such case enter the full URL to a folder with CKEditor.<br />The simplest way to have the editor running is to use CDN, to use it set: !cdn', array(
      '!ckeditorcom' => l(t('ckeditor.com'), 'http://ckeditor.com/download'),
      '!cdn' => '<strong>//cdn.ckeditor.com/4.4.3/full-all</strong>'
        )
    ) .
    '<br /><br /> ' .
    t('Available placeholders:!b &ndash; path of the Drupal installation (!base) !m &ndash; path where the CKEditor module is stored (!module).!l &ndash; path to the libraries directory (!library).', array(
      '!b' => '<br /><code>%b</code>',
      '!m' => '<br /><code>%m</code>',
      '!l' => '<br /><code>%l</code>',
      '!base' => '<code>' . $drupal_base_path . '</code>',
      '!module' => '<code>' . $module_drupal_path . '</code>',
      '!library' => '<code>' . $drupal_library_path . '</code>',
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . $current_path . '</code>'
        )
    ),
    '#required' => TRUE
  );

  $local_path = ckeditor_path('local');
  if ($local_path == '<URL>') {
    $local_path = t('CKEditor is loaded from URL. Local path is not available.');
  }
  $form['ckeditor_advanced_settings']['ckeditor_local_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path to CKEditor'),
    '#default_value' => isset($profile->settings['ckeditor_local_path']) ? $profile->settings['ckeditor_local_path'] : '',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t(
        'The path to the local directory (on the server) that points to the path defined above. Enter either an absolute server path or a path relative to the !indexphp file. If left empty, the CKEditor module will try to find the right path.', array(
      '!indexphp' => '<code>index.php</code>'
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . $local_path . '</code>'
        )
    )
  );

  $form['ckeditor_advanced_settings']['ckeditor_plugins_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to the CKEditor plugins directory'),
    '#default_value' => !empty($profile->settings['ckeditor_plugins_path']) ? $profile->settings['ckeditor_plugins_path'] : '%m/plugins',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t('Path to the CKEditor plugins directory relative to the document root.') .
    '<br />' .
    t('Available placeholders:!b &ndash; path of the Drupal installation (!base) !m &ndash; the base URL path where the CKEditor module is stored (!module).!l &ndash; the base URL path to the libraries directory (!library)', array(
      '!b' => '<br /><code>%b</code>',
      '!m' => '<br /><code>%m</code>',
      '!l' => '<br /><code>%l</code>',
      '!base' => '<code>' . $drupal_base_path . '</code>',
      '!module' => '<code>' . $module_drupal_path . '</code>',
      '!library' => '<code>' . $drupal_library_path . '</code>',
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . ckeditor_plugins_path('relative') . '</code>'
        )
    )
  );

  $form['ckeditor_advanced_settings']['ckeditor_plugins_local_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path to the CKEditor plugins directory'),
    '#default_value' => isset($profile->settings['ckeditor_plugins_local_path']) ? $profile->settings['ckeditor_plugins_local_path'] : '',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t(
        'The path to the local directory (on the server) that points to the path defined above. Enter either an absolute server path or a path relative to the !indexphp file. If left empty, the CKEditor module will try to find the right path.', array(
      '!indexphp' => '<code>index.php</code>'
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . ckeditor_plugins_path('local') . '</code>'
        )
    )
  );

  $form['ckeditor_advanced_settings']['ckfinder_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to CKFinder'),
    '#default_value' => !empty($profile->settings['ckfinder_path']) ? $profile->settings['ckfinder_path'] : '%m/ckfinder',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t(
        'The path to CKFinder (AJAX based file manager downloaded from !ckfindercom) relative to the document root.', array(
      '!ckfindercom' => l(t('ckfinder.com'), 'http://cksource.com/ckfinder')
        )
    ) .
    '<br />' .
    t('Available placeholders:!b &ndash; path of the Drupal installation (!base) !m &ndash; path where the CKEditor module is stored (!module).!l &ndash; path to the libraries directory (!library)', array(
      '!b' => '<br /><code>%b</code>',
      '!m' => '<br /><code>%m</code>',
      '!l' => '<br /><code>%l</code>',
      '!base' => '<code>' . $drupal_base_path . '</code>',
      '!module' => '<code>' . $module_drupal_path . '</code>',
      '!library' => '<code>' . $drupal_library_path . '</code>'
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . ckfinder_path('relative') . '</code>'
        )
    )
  );

  $form['ckeditor_advanced_settings']['ckfinder_local_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path to CKFinder'),
    '#default_value' => isset($profile->settings['ckfinder_local_path']) ? $profile->settings['ckfinder_local_path'] : '',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t(
        'The path to the local directory (on the server) that points to the path defined above. Enter either an absolute server path or a path relative to the !indexphp file. If left empty, the CKEditor module will try to find the right path.', array(
      '!indexphp' => '<code>index.php</code>'
        )
    ) .
    '<br /> ' .
    t('Current path: !path', array(
      '!path' => '<code>' . ckfinder_path('local') . '</code>'
        )
    )
  );

  //@todo DOWNLOAD API
  if (variable_get('file_default_scheme', '') == 'private') {
    $form['ckeditor_advanced_settings']['ckeditor_allow_download_private_files'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable access to files located in the private folder'),
      '#default_value' => !empty($profile->settings['ckeditor_allow_download_private_files']),
      '#return_value' => 't',
      '#description' => t(
          '<strong>Use this option with care.</strong> If checked, CKEditor will allow anyone knowing the URL to view a file located inside of the private path (!private_path), but only if there is no information about the file in the Drupal database. If the path below is specified, anyone will have access only to that location.', array(
        '!private_path' => '<code>' . variable_get('file_private_path', '') . '</code>'
          )
      ),
      '#required' => FALSE
    );
    $current_private_dir = !empty($profile->settings['private_dir']) ? $profile->settings['private_dir'] : '';
    $form['ckeditor_advanced_settings']['private_dir'] = array(
      '#type' => 'textfield',
      '#title' => t('Location of files uploaded with CKEditor to the private folder'),
      '#default_value' => !empty($profile->settings['private_dir']) ? $profile->settings['private_dir'] : '',
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('The path relative to the location of the private directory where CKEditor should store uploaded files.') .
      '<br />' .
      t('Available wildcard characters:!u &ndash; User ID.!n &ndash; Username', array(
        '!u' => '<br/><code>%u</code>',
        '!n' => '<br /><code>%n</code>'
          )
      ) .
      '<br />' .
      t('System path to the private folder is: !system_path.', array(
        '!system_path' => '<code>' . realpath(variable_get('file_private_path', conf_path() . '/files')) . DIRECTORY_SEPARATOR . '</code>'
          )
      )
    );
  }

  if (function_exists('linktocontent_node_menu') && function_exists('pathfilter_filter')) {
    $form['ckeditor_advanced_settings']['linktoc'] = array(
      '#type' => 'select',
      '#options' => array('p' => t('Link to paths only'), 'n' => t('Link using internal: links'), 'pn' => t('Allow the user to select between paths and internal links')),
      '#title' => t('Path Filter & Link To Content integration'),
      '#default_value' => empty($profile->settings['linktoc']) ? 'p' : $profile->settings['linktoc'],
      '#description' => t(
          'With the !plink extension it is possible to use internal: links. By default the !link extension is linking to nodes using paths.', array(
        '!plink' => l(t('Path Filter'), 'http://drupal.org/project/pathfilter'),
        '!link' => l(t('Link To Content'), 'http://drupal.org/project/linktocontent')
          )
      )
    );
  }

  $form['ckeditor_advanced_settings']['ckeditor_aggregate'] = array(
    '#type' => 'radios',
    '#title' => t('Aggregate <code>ckeditor.js</code>'),
    '#default_value' => !empty($profile->settings['ckeditor_aggregate']) ? $profile->settings['ckeditor_aggregate'] : 'f',
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled')
    ),
    '#description' => t('When enabled, <code>ckeditor.js</code> will be aggregated if JavaScript aggregation is enabled. <strong>Not recommended</strong>.'),
  );

  $form['ckeditor_advanced_settings']['toolbar_wizard'] = array(
    '#type' => 'radios',
    '#title' => t('Use toolbar Drag&Drop feature'),
    '#default_value' => !empty($profile->settings['toolbar_wizard']) ? $profile->settings['toolbar_wizard'] : 't',
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled')
    ),
    '#description' => t('When enabled, the toolbar can be built by using the drag-and-drop feature. Otherwise you will need to enter the toolbar configuration manually to the text box.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $btn
  );

  return $form;
}

/**
 * Form validation for a global profile
 */
function ckeditor_admin_global_profile_form_validate($form, &$form_state) {

}

/**
 * Submit form for a global profile
 */
function ckeditor_admin_global_profile_form_submit($form, &$form_state) {
  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
  $edit = & $form_state['values'];
  $edit['name'] = 'CKEditor Global Profile';

  if (isset($edit['_profile'])) {
    ckeditor_profile_delete($edit['_profile']->name);
  }

  //strip whitespaces
  if (empty($edit['ckeditor_local_path'])) {
    $edit['ckeditor_local_path'] = '';
  }
  else {
    $edit['ckeditor_local_path'] = trim($edit['ckeditor_local_path']);
  }

  //strip slash from the end
  if (empty($edit['ckeditor_path'])) {
    $edit['ckeditor_path'] = '';
  }
  $edit['ckeditor_path'] = trim(rtrim($edit['ckeditor_path'], "/"));
  if ($edit['ckeditor_path'] && 0 !== strpos($edit['ckeditor_path'], "/") && 0 !== strpos($edit['ckeditor_path'], "%") && !preg_match("|^http(s)?://|i", $edit['ckeditor_path'])) {
    //ensure that slash is at the beginning
    $edit['ckeditor_path'] = "/" . $edit['ckeditor_path'];
  }
  //no slash at the end
  $edit['ckeditor_local_path'] = trim(rtrim($edit['ckeditor_local_path'], "/"));

  //strip whitespaces
  if (empty($edit['ckeditor_plugins_local_path'])) {
    $edit['ckeditor_plugins_local_path'] = '';
  }
  else {
    $edit['ckeditor_plugins_local_path'] = trim($edit['ckeditor_plugins_local_path']);
  }

  //strip slash from the end
  if (empty($edit['ckeditor_plugins_path'])) {
    $edit['ckeditor_plugins_path'] = '';
  }
  $edit['ckeditor_plugins_path'] = trim(rtrim($edit['ckeditor_plugins_path'], "/"));
  if ($edit['ckeditor_plugins_path'] && 0 !== strpos($edit['ckeditor_plugins_path'], "/") && 0 !== strpos($edit['ckeditor_plugins_path'], "%")) {
    //ensure that slash is at the beginning
    $edit['ckeditor_plugins_path'] = "/" . $edit['ckeditor_plugins_path'];
  }
  //no slash at the end
  $edit['ckeditor_plugins_path'] = trim(rtrim($edit['ckeditor_plugins_path'], "/"));

  //strip slash from the end
  if (empty($edit['ckfinder_path'])) {
    $edit['ckfinder_path'] = '';
  }
  $edit['ckfinder_path'] = trim(rtrim($edit['ckfinder_path'], "/"));
  if ($edit['ckfinder_path'] && 0 !== strpos($edit['ckfinder_path'], "/") && 0 !== strpos($edit['ckfinder_path'], "%")) {
    //ensure that slash is at the beginning
    $edit['ckfinder_path'] = "/" . $edit['ckfinder_path'];
  }

  //no slash at the end
  $edit['ckfinder_local_path'] = trim(rtrim($edit['ckfinder_local_path'], "/"));

  $settings = ckeditor_admin_values_to_settings($edit);
  db_insert('ckeditor_settings')
      ->fields(array(
        "name" => $edit["name"],
        "settings" => $settings
      ))
      ->execute();

  drupal_set_message(t('The CKEditor global profile was saved.'));
  $form_state['redirect'] = 'admin/config/content/ckeditor';
}

/**
 * Form builder for a profile
 */
function ckeditor_admin_profile_form($form, $form_state, $task, $profile = NULL) {
  global $theme;

  if ($profile != NULL) {
    $form['_profile'] = array(
      '#type' => 'value',
      '#value' => $profile,
    );
  }
  else {
    $profile = new stdClass();
  }

  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');

  $editor_path = ckeditor_path('relative');
  $editor_local_path = ckeditor_path('local');
  $editor_url_path = ckeditor_path('url');
  $module_drupal_path = ckeditor_module_path('relative');
  $module_drupal_url_path = ckeditor_module_path('url');

  drupal_add_js(array('ckeditor_version' => ckeditor_get_version()), 'setting');

  $lang_options = ckeditor_load_lang_options();

  // Attach the administration CSS.
  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('module', 'ckeditor') . '/css/ckeditor.admin.css',
    ),
  );

  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic setup'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  switch ($task) {
    case 'clone':
      //load all profiles to check their names
      $profiles = ckeditor_profile_load();
      $oldname = $profile->name;
      $maxsize = 128;   //default max name length

      $res = array();
      $pat = "/^(.*?)_([0-9]+)$/";
      if (preg_match($pat, $oldname, $res)) {     // oldname like 'name_nr'
        $name = $res[1];
        $num = $res[2] + 1;
      }
      else {
        $name = $oldname;
        $num = 2;
      }

      $newname = substr($name, 0, $maxsize - 3) . '_' . $num;   // +limit
      while (isset($profiles[$newname])) {            //find next free number
        $num++;
        $newname = substr($name, 0, $maxsize - 3) . '_' . $num;
      }
      break;
    case 'edit':
      $newname = $profile->name;
      break;
  }

  $global_profile = ckeditor_profile_load('CKEditor Global Profile');
  $toolbar_wizard = !empty($global_profile->settings['toolbar_wizard']) ? $global_profile->settings['toolbar_wizard'] : 't';
  drupal_add_js(array('ckeditor_toolbar_wizard' => $toolbar_wizard), 'setting');

  $form['basic']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile name'),
    '#default_value' => !empty($profile->name) ? $newname : '',
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t('Enter a name for this profile. This name is only visible within the CKEditor administration page.'),
    '#required' => TRUE
  );

  $orig_formats = filter_formats();

  if ($task == 'edit' && !empty($profile->name)) {
    $used_formats = db_select('ckeditor_input_format', 'f')->fields('f', array("format"))->distinct()->condition("f.name", array($profile->name), 'NOT IN')->execute()->fetchAllAssoc('format');
  }
  else {
    $profile->input_formats = array();
    $used_formats = db_select('ckeditor_input_format', 'f')->fields('f', array("format"))->distinct()->execute()->fetchAllAssoc('format');
  }

  $formats = array();
  $input_format_config_links = array();
  foreach ($orig_formats AS $format) {
    if (($task == 'edit' && !empty($profile->input_formats) && array_key_exists($format->format, $profile->input_formats)) || !array_key_exists($format->format, $used_formats)) {
      $formats[$format->format] = $format->name;
      $input_format_config_links[$format->format] = array("name" => $format->name, "config_url" => url("admin/config/content/formats/" . $format->format));
    }
  }

  $form['basic']['input_formats'] = array(
    '#id' => 'input-formats',
    '#type' => 'checkboxes',
    '#title' => t('Text formats'),
    '#default_value' => !empty($profile->input_formats) ? array_keys((array) $profile->input_formats) : array(),
    '#options' => $formats,
    '#description' => t('Choose the text formats where you want to load CKEditor.')
  );

  $form['security'] = array(
    '#type' => 'fieldset',
    '#title' => t('Security'),
    '#description' => '<p>' . t('The CKEditor security system protects you from executing malicious code that is already in your database. In plain textareas database content is harmless because it is not executed, but the CKEditor WYSIWYG editor interprets HTML like a web browser and thus the content needs to be filtered before it is loaded.') . '</p>',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $form['security']['filters'] = array(
    '#id' => 'security-filters',
    '#type' => 'container'
  );

  $form['security']['filters']['__header'] = array(
    '#type' => 'item',
    '#title' => t('Security filters'),
    '#description' => t(
      'Below is a list of security filters along with their status for each text format.<br /><br />The following statuses are available: !img_enabled - the filter is configured for a given text format and will be run on the content during the filtering process. !img_disabled - the filter is disabled for a given text format and will not be run on the content during the filtering process.<br /><br />In order to modify the security filters settings for each of the text formats, you need to visit the text format configuration sections as listed below: !elem_format_config', array(
      '!img_enabled' => '</br><img src="' . $module_drupal_path . '/images/tick.png"/>',
      '!img_disabled' => '</br><img src="' . $module_drupal_path . '/images/delete.png"/>',
      '!elem_format_config' => '<ul class="text-formats-config"></ul>'
        )
    ),
    '#weight' => -10
  );

  //get text formats filters
  $input_format_filters = array();
  foreach ($formats AS $key => $value) {
    $input_format_filters[$key] = array();
    $filters = filter_list_format($key);
    foreach ($filters as $filter_name => $filter_settings) {
      if ($filter_settings->status == 1) {
        $input_format_filters[$key][$filter_name] = TRUE;
      }
    }
  }
  drupal_add_js(array('text_format_filters' => $input_format_filters, 'text_formats_config_links' => $input_format_config_links), 'setting');

  $security_filters = ckeditor_security_filters();
  _ckeditor_admin_profile_form_security_filters($form, $profile, $security_filters);

  $form['security']['ss'] = array(
    '#type' => 'radios',
    '#title' => t('Security settings'),
    '#default_value' => isset($profile->settings['ss']) ? $profile->settings['ss'] : '2',
    '#options' => array(
      '2' => t('Always run security filters for CKEditor.'),
      '1' => t('Run security filters only when CKEditor is set to start automatically.'),
    ),
    '#description' => t('There are two ways of starting CKEditor: automatically and manually (via toggle). If you decide to apply security filters only when CKEditor starts automatically, you will not be protected when toggling manually from plain textarea to CKEditor. Choose this option only if you can detect various attacks (mainly XSS) by yourself just by looking at the HTML code.'),
  );

  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Editor appearance'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $ui_colors = array(
    "default" => t('CKEditor default'),
    "custom" => t('Select manually')
  );
  if (function_exists('color_get_palette')) {
    // apparently $theme is not initialized (?)
    if (empty($theme)) {
      init_theme();
    }
    $palette = @color_get_palette($theme, FALSE); //[#652274]
    $color_palette['default'] = '#D3D3D3';
    if (!empty($palette)) {
      if (!empty($palette['base'])) {
        $color_palette['color_base'] = $palette['base'];
        $ui_colors["color_base"] = t('Color module: base');
      }
      if (!empty($palette['top'])) {
        $color_palette['color_top'] = $palette['top'];
        $ui_colors["color_top"] = t('Color module: top');
      }
      if (!empty($palette['bottom'])) {
        $color_palette['color_bottom'] = $palette['bottom'];
        $ui_colors["color_bottom"] = t('Color module: bottom');
      }
    }
    drupal_add_js(array('ckeditor_uicolor' => $color_palette), 'setting');
  }

  if ($editor_path == '<URL>') {
    drupal_add_js($editor_url_path . '/ckeditor.js', 'external');
  }
  else {
    drupal_add_js('window.CKEDITOR_BASEPATH = "' . $editor_path . '/"', array('type' => 'inline', 'weight' => -100));
    drupal_add_js($editor_url_path . '/ckeditor.js', array('type' => 'file', 'preprocess' => FALSE));
  }
  drupal_add_js($module_drupal_url_path . '/ckeditor.config.js', 'file');
  drupal_add_js($module_drupal_url_path . '/includes/ckeditor.admin.js', 'file');

  if ($toolbar_wizard == 't') {
    drupal_add_library('system', 'ui.widget');
    drupal_add_library('system', 'ui.sortable');
    drupal_add_js($module_drupal_url_path . '/includes/jqueryUI/sort.js', 'file');
  }

  $form['appearance']['toolbar'] = array(
    '#type' => 'textarea',
    '#title' => t('Toolbar'),
    '#default_value' => !empty($profile->settings['toolbar']) ? $profile->settings['toolbar'] : '',
    '#description' => t('Load sample toolbar: !toolbars', array("!toolbars" => '<a href="#" id="cke_toolbar_DrupalBasic" class="cke_load_toolbar">' . t('Basic') . '</a> | <a href="#" id="cke_toolbar_DrupalAdvanced" class="cke_load_toolbar">' . t('Advanced') . '</a> | <a href="#" id="cke_toolbar_DrupalFull" class="cke_load_toolbar">' . t('Full') . '</a>')),
    '#wysiwyg' => FALSE,
    '#rows' => 15
  );

  if ($toolbar_wizard == 't') {
    $form['appearance']['toolbar_wizzard_used'] = array(
      '#markup' => '<div>' . t('Current toolbar') . '</div><div class="sortableList" id="groupLayout"></div><br/>',
      '#description' => t('Currently used buttons'),
    );

    drupal_add_js(array('cke_toolbar_buttons_all' => ckeditor_toolbar_buttons_all()), 'setting');

    $form['appearance']['toolbar_wizzard_all'] = array(
      '#markup' => '<div>' . t('Available buttons') . '</div><div id="allButtons" class="sortableList"></div><br/>',
      '#description' => t('All available buttons'),
    );
  }

  $form['appearance']['expand'] = array(
    '#type' => 'radios',
    '#title' => t('Toolbar state on startup'),
    '#default_value' => !empty($profile->settings['expand']) ? $profile->settings['expand'] : 't',
    '#options' => array(
      't' => t('Expanded'),
      'f' => t('Collapsed')
    ),
    '#description' => t('The toolbar will start in an expanded or collapsed state.'),
  );

  $form['appearance']['default'] = array(
    '#type' => 'radios',
    '#title' => t('Default editor state'),
    '#default_value' => !empty($profile->settings['default']) ? $profile->settings['default'] : 't',
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled')
    ),
    '#description' => t('Default editor state. If disabled, the rich text editor may still be enabled by using toggle.'),
  );

  $form['appearance']['show_toggle'] = array(
    '#type' => 'radios',
    '#title' => t('Show the disable/enable rich text editor toggle'),
    '#default_value' => !empty($profile->settings['show_toggle']) ? $profile->settings['show_toggle'] : 't',
    '#options' => array(
      't' => t('Show'),
      'f' => t('Hide')
    ),
    '#description' => t('Whether or not to show the disable/enable rich text editor toggle below the textarea.'),
  );

  $form['appearance']['uicolor'] = array(
    '#type' => 'select',
    '#title' => t('User interface color'),
    '#default_value' => !empty($profile->settings['uicolor']) ? $profile->settings['uicolor'] : 'default',
    '#options' => $ui_colors,
  );

  if (!isset($global_profile->settings['skin']) || !file_exists($editor_local_path . '/skins/' . $global_profile->settings['skin'])) {
    $global_profile->settings['skin'] = ckeditor_default_skin();
  }
  $skinframe_url = url('admin/config/content/ckeditor/skinframe', array(
    'query' => array(
      'token' => drupal_get_token('ckeditorSkinframeCall'),
      'skin' => $global_profile->settings['skin'],
      'uicolor' => (!empty($profile->settings['uicolor_user']) && $profile->settings['uicolor'] == 'custom') ? ltrim($profile->settings['uicolor_user'], '#') : 'D3D3D3'
    )
  ));
  $form['appearance']['skinframe'] = array(
    '#markup' => '<iframe frameborder="0" id="skinframe" marginheight="0" marginwidth="0" src="' . $skinframe_url . '" style="height:430px;width:700px;"></iframe><br/>',
    '#description' => '',
  );
  $form['appearance']['uicolor_user'] = array(
    '#type' => 'hidden',
    '#default_value' => !empty($profile->settings['uicolor_user']) ? $profile->settings['uicolor_user'] : 'default',
  );

  $plugin_list = ckeditor_load_plugins();
  $plugins = array();
  if (isset($profile->settings['loadPlugins'])) {
    foreach ($plugin_list AS $key => $val) {
      $plugins[$key] = $val['desc'];
    }
  }
  else {
    $default_plugins = array();
    foreach ($plugin_list AS $key => $val) {
      $plugins[$key] = $val['desc'];
      if (isset($val['default']) && $val['default'] == 't') {
        $default_plugins[] = $key;
      }
    }
  }

  $form['appearance']['loadPlugins'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Plugins'),
    '#default_value' => isset($profile->settings['loadPlugins']) ? array_keys((array) $profile->settings['loadPlugins']) : $default_plugins,
    '#options' => $plugins,
    '#description' => t('Choose the plugins that you want to enable in CKEditor.')
  );

  if ($editor_local_path != '<URL>' && file_exists($editor_local_path . '/plugins/divarea/plugin.js')) {
    $form['appearance']['use_divarea'] = array(
      '#type' => 'radios',
      '#title' => t('"Div-based" editor'),
      '#default_value' => !empty($profile->settings['use_divarea']) ? $profile->settings['use_divarea'] : 'f',
      '#options' => array(
        't' => t('Enabled'),
        'f' => t('Disabled')
      ),
      '#description' => t("Loading editor into <code>div</code> instead of <code>iframe</code>.")
    );
  }

  $form['appearance']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Editor width'),
    '#default_value' => !empty($profile->settings['width']) ? $profile->settings['width'] : '100%',
    '#description' => t("Editor interface width in pixels or percent. Examples: 400, 100%."),
    '#size' => 40,
    '#maxlength' => 128,
  );

  $form['appearance']['lang'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#default_value' => !empty($profile->settings['lang']) ? $profile->settings['lang'] : 'en',
    '#options' => $lang_options,
    '#description' => t('The language for the CKEditor user interface.')
  );

  $form['appearance']['auto_lang'] = array(
    '#type' => 'radios',
    '#title' => t('Auto-detect language'),
    '#default_value' => !empty($profile->settings['auto_lang']) ? $profile->settings['auto_lang'] : 't',
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled')
    ),
    '#description' => t('Automatically detect the user language.')
  );

  $form['appearance']['language_direction'] = array(
    '#type' => 'select',
    '#title' => t('Language direction'),
    '#default_value' => !empty($profile->settings['language_direction']) ? $profile->settings['language_direction'] : 'default',
    '#options' => array(
      'default' => t('Get from current locale (default)'),
      'ltr' => t('Left-To-Right'), // language like English
      'rtl' => t('Right-To-Left') // languages like Arabic
    ),
    '#description' => t(
        'Choose the language direction used in the editing area. Even when CKEditor automatically detects the user language and adjusts its user interface, the editing area is not automatically changed into the LTR or RTL mode. To be able to type LTR (like English) and RTL (like Arabic, Hebrew, Persian) content at the same time, please make sure that the <strong>!bidiltr</strong> and <strong>!bidirtl</strong> buttons are enabled in the toolbar.', array(
      '!bidiltr' => 'BidiLtr',
      '!bidirtl' => 'BidiRtl'
        )
    )
  );

  $form['acf'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced content filter'),
    '#description' => t('Configure input data filtering in CKEditor. See !link for more information about ACF.', array(
        '!link' => l(t('this blog post'), 'http://ckeditor.com/blog/Upgrading-to-CKEditor-4.1')
      )),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $form['acf']['allowed_content'] = array(
    '#type' => 'radios',
    '#title' => t('Advanced content filter'),
    '#description' => t('<strong>Example:</strong> if only Bold and Link buttons are enabled, only !strong and !a tags will be allowed. Any !b tags will automatically be converted into !strong. The !img tag will not be allowed, unless the Image button is added to the toolbar.', array(
    '!strong' => '<code>&lt;strong&gt;</code>',
    '!a' => '<code>&lt;a&gt;</code>',
    '!img' => '<code>&lt;img&gt;</code>',
    '!b' => '<code>&lt;b&gt;</code>'
  )).'<br /><br />'.
    t('<strong>Warning:</strong> CKEditor will not use the set of tags associated with text format. Only the buttons enabled in the toolbar and optional plugins will determine which tags/attributes/styles are handled by CKEditor. Enabling ACF in CKEditor does not affect server-side filtering done by Drupal in any way.'),

    '#default_value' => !empty($profile->settings['allowed_content']) ? $profile->settings['allowed_content'] : 't',
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled'),
    ),
  );
  $form['acf']['allowed_content']['t']['#description'] = t('CKEditor will only allow tags/attributes/styles provided by CKEditor features. Disallowed content will be removed directly by CKEditor. When configured properly, it helps to ensure a true WYSIWYG experience.');
  $form['acf']['allowed_content']['f']['#description'] = t('CKEditor will not filter any tags and as a result, the user will be able to insert any content into the editor. The content will only be filtered by server-side filters associated with input formats.');

  $form['acf']['extraAllowedContent'] = array(
    '#type' => 'textarea',
    '#title' => t('Extra allowed content'),
    '#default_value' => !empty($profile->settings['extraAllowedContent']) ? $profile->settings['extraAllowedContent'] : "",
    '#cols' => 60,
    '#rows' => 5,
    '#description' => t(
      'Set additional allowed content rules for CKEditor. Separate rules with a semicolon. See !docs for more information.', array(
        '!docs' => l(t('CKEditor documentation'), 'http://docs.ckeditor.com/#!/api/CKEDITOR.config-cfg-allowedContent')
       )).'<p>'.
       t('<strong>Example:</strong> !rules', array(
         '!rules' => "<code>object[width,height];param[name,value];embed[*];</code>"
       )).'</p>',
    '#wysiwyg' => FALSE,
  );

  $form['output'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cleanup and output'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['output']['enter_mode'] = array(
    '#type' => 'select',
    '#title' => t('Enter mode'),
    '#default_value' => !empty($profile->settings['enter_mode']) ? $profile->settings['enter_mode'] : 'p',
    '#options' => array(
      'p' => '<p>',
      'br' => '<br>',
      'div' => '<div>'
    ),
    '#description' => t('Set which tag should be used by CKEditor when the <em>Enter</em> key is pressed.')
  );

  $form['output']['shift_enter_mode'] = array(
    '#type' => 'select',
    '#title' => t('Shift+Enter mode'),
    '#default_value' => !empty($profile->settings['shift_enter_mode']) ? $profile->settings['shift_enter_mode'] : 'br',
    '#options' => array(
      'p' => '<p>',
      'br' => '<br>',
      'div' => '<div>'
    ),
    '#description' => t('Set which tag should be used by CKEditor when the <em>Shift+Enter</em> key combination is pressed.')
  );

  $form['output']['font_format'] = array(
    '#type' => 'textfield',
    '#title' => t('Font formats'),
    '#default_value' => !empty($profile->settings['font_format']) ? $profile->settings['font_format'] : 'p;div;pre;address;h1;h2;h3;h4;h5;h6',
    '#size' => 40,
    '#maxlength' => 250,
    '#description' => t(
        'Semicolon-separated list of HTML font formats. Allowed values are: !allowed_values', array(
      '!allowed_values' => '<code>p;div;pre;address;h1;h2;h3;h4;h5;h6</code>'
        )
    )
  );

  if (!empty($profile->settings['formatting']['custom_formatting_options'])) {
    foreach ($profile->settings['formatting']['custom_formatting_options'] as $k => $v) {
      if ($v === 0) {
        unset($profile->settings['formatting']['custom_formatting_options'][$k]);
      }
    }
  }

  $form['output']['custom_formatting'] = array(
    '#type' => 'radios',
    '#title' => t('Use custom formatting options'),
    '#default_value' => !empty($profile->settings['custom_formatting']) ? $profile->settings['custom_formatting'] : 'f',
    '#options' => array(
      't' => t('Yes'),
      'f' => t('No'),
    ),
  );

  $form['output']['formatting'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom formatting options'),
    '#tree' => TRUE,
  );

  $form['output']['formatting']['custom_formatting_options'] = array(
    '#type' => 'checkboxes',
    '#default_value' => isset($profile->settings['formatting']['custom_formatting_options']) ? array_keys((array) $profile->settings['formatting']['custom_formatting_options']) : array('indent' => 'indent', 'breakBeforeOpen' => 'breakBeforeOpen', 'breakAfterOpen' => 'breakAfterOpen', 'breakAfterClose' => 'breakAfterClose'),
    '#options' => array(
      'indent' => t('Indent the element contents.'),
      'breakBeforeOpen' => t('Break line before the opening tag.'),
      'breakAfterOpen' => t('Break line after the opening tag.'),
      'breakBeforeClose' => t('Break line before the closing tag.'),
      'breakAfterClose' => t('Break line after the closing tag.'),
      'pre_indent' => t('Indent the <code>&lt;pre&gt;</code> element contents.'),
    ),
  );

  $form['css'] = array(
    '#type' => 'fieldset',
    '#title' => t('CSS'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $current_theme = variable_get('theme_default', $theme);
  $theme_path = drupal_get_path('theme', $current_theme);

  $form['css']['css_mode'] = array(
    '#type' => 'select',
    '#title' => t('Editor CSS'),
    '#default_value' => !empty($profile->settings['css_mode']) ? $profile->settings['css_mode'] : 'theme',
    '#options' => array(
      'theme' => t('Use theme CSS'),
      'self' => t('Define CSS'),
      'none' => t('CKEditor default')
    ),
    '#description' => t(
        'Defines the CSS to be used in the editor area.!title_theme_css &ndash; load the !style_css file from the current site theme.!title_define_css &ndash; enter the CSS file path below.!title_ckeditor_default &ndash; use the default editor CSS.', array(
      '!title_theme_css' => '<br /><strong>' . t('Use theme CSS') . '</strong>',
      '!title_define_css' => '<br /><strong>' . t('Define CSS') . '</strong>',
      '!title_ckeditor_default' => '<br /><strong>' . t('CKEditor default') . '</strong>',
      '!style_css' => '<code>style.css</code>'
        )
    )
  );

  $form['css']['css_path'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS file path'),
    '#default_value' => !empty($profile->settings['css_path']) ? $profile->settings['css_path'] : "",
    '#size' => 40,
    '#maxlength' => 255,
    '#description' => t(
        'Enter the path to the CSS file (Example: !example1) or a list of CSS files separated with a comma (Example: !example2). Make sure you select the !title_define_css option above.', array(
      '!example1' => '<code>"css/editor.css"</code>',
      '!example2' => '<code>"/themes/garland/style.css,http://example.com/style.css"</code>',
      '!title_define_css' => '<strong>' . t('Define CSS') . '</strong>'
        )
    ) .
    '<br />' .
    t('Available placeholders:!h &ndash; host name (!host).!t &ndash; path to theme (!theme).', array(
      '!h' => '<br /><code>%h</code>',
      '!t' => '<br /><code>%t</code>',
      '!host' => '<code>' . base_path() . '</code>',
      '!theme' => '<code>' . base_path() . $theme_path . '/</code>'
        )
    )
  );

  $form['css']['css_style'] = array(
    '#type' => 'select',
    '#title' => t('Predefined styles'),
    '#default_value' => !empty($profile->settings['css_style']) ? $profile->settings['css_style'] : 'theme',
    '#options' => array(
      'theme' => t('Use theme ckeditor.styles.js'),
      'self' => t('Define path to ckeditor.styles.js'),
      'default' => t('CKEditor default')
    ),
    '#description' => t(
        'Define the location of the !ckeditor_styles_js_file file. It is used by the <strong>Style</strong> drop-down list available in the default toolbar. Copy the !ckeditor_styles_js_path file into your theme directory (!theme) and adjust it to your needs.', array(
      '!ckeditor_styles_js_file' => '<code>ckeditor.styles.js</code>',
      '!ckeditor_styles_js_path' => '<code>' . drupal_get_path('module', 'ckeditor') . '/ckeditor.styles.js</code>',
      '!theme' => '<code>' . $theme_path . '/ckeditor.styles.js' . '</code>'
        )
    )
  );

  $form['css']['styles_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Predefined styles path'),
    '#default_value' => !empty($profile->settings['styles_path']) ? $profile->settings['styles_path'] : "",
    '#size' => 40,
    '#maxlength' => 255,
    '#description' => t(
        'Enter the path to a file with predefined styles (Example: !example1). Make sure you select the !style_define_path option above.', array(
      '!style_define_path' => '<strong>' . t('Define path to ckeditor.styles.js') . '</strong>',
      '!ckeditor_styles_js_file' => '<code>ckeditor.styles.js</code>',
      '!example1' => '<code>/ckeditor.styles.js</code>'
        )
    ) .
    '<br />' .
    t('Available placeholders:!h &ndash; host name (!host).!t &ndash; path to theme (!theme).!m &ndash; path to the CKEditor module (!module).', array(
      '!h' => '<br /><code>%h</code>',
      '!t' => '<br /><code>%t</code>',
      '!m' => '<br /><code>%m</code>',
      '!host' => '<code>' . base_path() . '</code>',
      '!theme' => '<code>' . base_path() . $theme_path . '/</code>',
      '!module' => '<code>' . drupal_get_path('module', 'ckeditor') . '</code>'
        )
    )
  );

  $form['ckeditor_upload_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('File browser settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t(
        'Set the file browser settings. A file browser will allow you to browse the files stored on the server and embed them as links, images, or Flash movies. CKEditor is compatible with such Drupal modules as !imce or !elfinder. CKEditor can be also integrated with !ckfinder, an advanced Ajax file manager.', array(
      '!imce' => l(t('IMCE'), 'http://drupal.org/project/imce'),
      '!elfinder' => l(t('elFinder'), 'http://drupal.org/project/elfinder'),
      '!ckfinder' => l(t('CKFinder'), 'http://cksource.com/ckfinder')
        )
    )
  );

  $filebrowsers = array(
    'none' => t('None'),
    'ckfinder' => t('CKFinder'),
  );

  $filebrowsers_dialogs = array(
    '' => t('Same as in the Link dialog window'),
    'ckfinder' => t('CKFinder'),
  );

  if (module_exists('imce')) {
    $filebrowsers['imce'] = t('IMCE');
    $filebrowsers_dialogs['imce'] = t('IMCE');
  }

  if (module_exists('elfinder')) {
    $filebrowsers['elfinder'] = t('elFinder');
    $filebrowsers_dialogs['elfinder'] = t('elFinder');
  }

  /* MODULES NOT PORTED TO D7
    if (module_exists('tinybrowser')) {
    $filebrowsers['tinybrowser'] = t('TinyBrowser');
    $filebrowsers_dialogs['tinybrowser'] = t('TinyBrowser');
    }

    if (module_exists('imagebrowser')) {
    $filebrowsers['ib'] = t('Image Browser');
    $filebrowsers_dialogs['ib'] = t('Image Browser');
    }

    if (module_exists('webfm_popup')) {
    $filebrowsers['webfm'] = t('Web File Manager');
    $filebrowsers_dialogs['webfm'] = t('Web File Manager');
    }
   */
  $form['ckeditor_upload_settings']['filebrowser'] = array(
    '#type' => 'select',
    '#title' => t('File browser type (Link dialog window)'),
    '#default_value' => !empty($profile->settings['filebrowser']) ? $profile->settings['filebrowser'] : 'none',
    '#options' => $filebrowsers,
    '#description' => t('Select the file browser that you would like to use to upload files.'),
  );

  $form['ckeditor_upload_settings']['filebrowser_image'] = array(
    '#type' => 'select',
    '#title' => t('File browser type (Image dialog window)'),
    '#default_value' => !empty($profile->settings['filebrowser_image']) ? $profile->settings['filebrowser_image'] : 'none',
    '#options' => $filebrowsers_dialogs,
    '#description' => t('Select the file browser that you would like to use to upload images.'),
  );

  $form['ckeditor_upload_settings']['filebrowser_flash'] = array(
    '#type' => 'select',
    '#title' => t('File browser type (Flash dialog window)'),
    '#default_value' => !empty($profile->settings['filebrowser_flash']) ? $profile->settings['filebrowser_flash'] : 'none',
    '#options' => $filebrowsers_dialogs,
    '#description' => t('Select the file browser that you would like to use to upload Flash movies.'),
  );

  if (variable_get('file_default_scheme', '') != 'private') {
    $current_user_files_path = empty($profile->settings['UserFilesPath']) ? "%b%f/" : $profile->settings['UserFilesPath'];
    $current_user_files_path = strtr($current_user_files_path, array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => "UID", "%b" => base_path(), "%n" => "UNAME"));

    $current_user_files_absolute_path = empty($profile->settings['UserFilesAbsolutePath']) ? "%d%b%f/" : $profile->settings['UserFilesAbsolutePath'];
    $current_user_files_absolute_path = strtr($current_user_files_absolute_path, array("%f" => variable_get('file_public_path', conf_path() . '/files'), "%u" => "UID", "%b" => base_path(), "%d" => ckeditor_get_document_root_full_path(), "%n" => "UNAME"));


    $form['ckeditor_upload_settings']['UserFilesPath'] = array(
      '#type' => 'textfield',
      '#prefix' => '<fieldset><legend>' . t('CKFinder settings') . '</legend>',
      '#title' => t('Path to uploaded files'),
      '#default_value' => !empty($profile->settings['UserFilesPath']) ? $profile->settings['UserFilesPath'] : "%b%f/",
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('Path to uploaded files relative to the document root.') .
      '<br />' .
      t('Available placeholders:!b &ndash; the base URL path of the Drupal installation (!base).!f &ndash; the Drupal file system path where the files are stored (!files).!u &ndash; User ID.!n &ndash; Username.', array(
        '!n' => '<br /><code>%n</code>',
        '!u' => '<br /><code>%u</code>',
        '!f' => '<br/><code>%f</code>',
        '!b' => '<br/><code>%b</code>',
        '!files' => '<code>' . variable_get('file_public_path', conf_path() . '/files') . '</code>',
        '!base' => '<code>' . base_path() . '</code>'
          )
      ) .
      '<br />' .
      t('Current path: !path', array(
        '!path' => '<code>' . $current_user_files_path . '</code>'
          )
      )
    );

    $form['ckeditor_upload_settings']['UserFilesAbsolutePath'] = array(
      '#type' => 'textfield',
      '#title' => t('Absolute path to uploaded files'),
      '#default_value' => !empty($profile->settings['UserFilesAbsolutePath']) ? $profile->settings['UserFilesAbsolutePath'] : "%d%b%f/",
      '#size' => 40,
      '#maxlength' => 255,
      '#suffix' => '</fieldset>',
      '#description' => t('The path to the local directory (on the server) which points to the path defined above. If left empty, CKEditor will try to discover the right path.') .
      '<br />' .
      t('Available placeholders:!d &ndash; the server path to the document root (!root).!b &ndash; the base URL path of the Drupal installation (!base).!f &ndash; the Drupal file system path where the files are stored (!files).!u &ndash; User ID.!n &ndash; Username.', array(
        '!u' => '<br /><code>%u</code>',
        '!n' => '<br /><code>%n</code>',
        '!d' => '<br/><code>%d</code>',
        '!b' => '<br /><code>%b</code>',
        '!f' => '<br/><code>%f</code>',
        '!files' => '<code>' . variable_get('file_public_path', conf_path() . '/files') . '</code>',
        '!base' => '<code>' . base_path() . '</code>',
        '!root' => '<code>' . ckeditor_get_document_root_full_path() . '</code>'
          )
      ) .
      '<br />' .
      t('Current path: !path', array(
        '!path' => '<code>' . $current_user_files_absolute_path . '</code>'
          )
      )
    );
  }
  if (variable_get('file_default_scheme', '') == 'private') {
    $form['ckeditor_upload_settings']['private_path_descrption'] = array(
      '#markup' => '<div>' . t(
          'Setting a relative path to uploaded files was disabled because private downloads are enabled and thus this path is calculated automatically. To change the location of uploaded files in the private file system, edit the <strong>!url</strong>.', array(
        '!url' => l(t('CKEditor Global Profile'), 'admin/config/content/ckeditor/editg')
          )
      ) . '</div>',
    );
  }

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  if (ckeditor_get_version(TRUE) == 3) {
    $form['advanced']['ckeditor_load_method'] = array(
      '#type' => 'select',
      '#title' => t('Loading method'),
      '#default_value' => !empty($profile->settings['ckeditor_load_method']) ? $profile->settings['ckeditor_load_method'] : 'ckeditor.js',
      '#options' => _ckeditor_load_methods(),
      '#description' => t('Select the loading method of CKEditor. If the !ckeditor_basic_js file is used, only a small file is loaded initially and the rest of the editor is loaded later (see <strong>Loading timeout</strong>). This might be useful if CKEditor is disabled by default.', array(
        '!ckeditor_basic_js' => '<code>ckeditor_basic.js</code>'
          )
      )
    );
    $form['advanced']['ckeditor_load_time_out'] = array(
      '#type' => 'textfield',
      '#title' => t('Loading timeout'),
      '#default_value' => !empty($profile->settings['ckeditor_load_time_out']) ? $profile->settings['ckeditor_load_time_out'] : "0",
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('The time to wait (in seconds) to load the full editor code after the page is loaded, if the !ckeditor_basic_js file is used. If set to zero, the editor is loaded on demand.', array(
        '!ckeditor_basic_js' => '<code>ckeditor_basic.js</code>'
          )
      )
    );
  }

  $form['advanced']['forcePasteAsPlainText'] = array(
    '#type' => 'select',
    '#title' => t('Force pasting as plain text'),
    '#default_value' => !empty($profile->settings['forcePasteAsPlainText']) ? $profile->settings['forcePasteAsPlainText'] : "f",
    '#options' => array(
      't' => t('Enabled'),
      'f' => t('Disabled')
    ),
    '#description' => t('If enabled, HTML content will be automatically changed to plain text when pasting.'),
  );
  $form['advanced']['html_entities'] = array(
    '#type' => 'radios',
    '#title' => t('HTML Entities'),
    '#default_value' => !empty($profile->settings['html_entities']) ? $profile->settings['html_entities'] : 'f',
    '#description' => t('Convert all applicable characters to HTML entities.'),
    '#options' => array(
      'f' => t('No'),
      't' => t('Yes')
    ),
  );
  $form['advanced']['scayt_autoStartup'] = array(
    '#type' => 'radios',
    '#title' => t('Spellchecker'),
    '#default_value' => !empty($profile->settings['scayt_autoStartup']) ? $profile->settings['scayt_autoStartup'] : 'f',
    '#description' => t('If enabled, turns on SCAYT (Spell Check As You Type) automatically after loading the editor.'),
    '#options' => array(
      'f' => t('No'),
      't' => t('Yes')
    ),
  );
  $form['advanced']['theme_config_js'] = array(
    '#type' => 'radios',
    '#title' => t('Load !ckeditor_config_js from the theme path', array(
      '!ckeditor_config_js' => '<code>ckeditor.config.js</code>'
        )
    ),
    '#default_value' => !empty($profile->settings['theme_config_js']) ? $profile->settings['theme_config_js'] : 'f',
    '#options' => array(
      't' => t('Yes'),
      'f' => t('No')
    ),
    '#description' => t('When enabled, the editor will try to load the !ckeditor_config_js file from the theme directory.', array(
      '!ckeditor_config_js' => '<code>ckeditor.config.js</code>'
        )
    )
  );
  $form['advanced']['js_conf'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom JavaScript configuration'),
    '#default_value' => !empty($profile->settings['js_conf']) ? $profile->settings['js_conf'] : "",
    '#cols' => 60,
    '#rows' => 5,
    '#description' => t(
        'In order to change CKEditor configuration globally, you should modify the !ckeditor_config configuration file. Sometimes it is required to change the CKEditor configuration for a single profile only. Use this box to define settings that are unique for this profile. Available options are listed in the !docs. Add the following code snippet to change the fonts available in the CKEditor <strong>Font</strong> and <strong>Size</strong> drop-down lists: <pre>@code</pre><strong>Warning</strong>: If you make a mistake here, CKEditor may not load correctly.', array(
      '!ckeditor_config' => '<code>' . drupal_get_path('module', 'ckeditor') . "/ckeditor.config.js</code>",
      '!docs' => l(t('CKEditor documentation'), 'http://docs.cksource.com/ckeditor_api/symbols/CKEDITOR.config.html'),
      '@code' => "config.font_names = 'Arial;Times New Roman;Verdana';\nconfig.fontSize_sizes = '16/16px;24/24px;48/48px;';"
        )
    ),
    '#wysiwyg' => FALSE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  return $form;
}

/**
 * Form validation for a profile.
 */
function ckeditor_admin_profile_form_validate($form, &$form_state) {
  $edit = & $form_state['values'];
  /*
    if ($edit['default'] == 't' && $edit['popup'] == 't') {
    form_set_error('popup', t('If CKEditor is enabled by default, the popup window must be disabled.'));
    }

    if ($edit['show_toggle'] == 't' && $edit['popup'] == 't') {
    form_set_error('popup', t('If toggle is enabled, the popup window must be disabled.'));
    }
   */
  if (!$edit['name']) {
    form_set_error('name', t('You must give a profile name.'));
  }
  elseif (!preg_match('/^[A-Za-z0-9_]+$/', $edit['name'])) {
    form_set_error('name', t('Enter a valid profile name. Only alphanumeric and underscore characters are allowed.'));
  }
  elseif ($edit['name'] == 'CKEditor Global Profile') {
    form_set_error('name', t('This profile name is reserved. Please choose a different name.'));
  }
  elseif (!isset($edit['_profile']) || ($edit['_profile']->name != $edit['name'])) {
    $result = ckeditor_profile_load($edit['name']);
    if (!empty($result)) {
      form_set_error('name', t('The profile name must be unique. A profile with this name already exists.'));
    }
  }

  if (!preg_match('/^\d+%?$/', $edit['width'])) {
    form_set_error('width', t('Enter a valid width value. Examples: 400, 100%.'));
  }

  if (!empty($edit['css_path'])) {
    if ($edit['css_mode'] != 'self') {
      form_set_error('css_path', t('The CSS path is not empty. Please set the <strong>Editor CSS</strong> option to the <strong>Define CSS</strong> mode.'));
    }
    elseif (FALSE !== strpos($edit['css_path'], '"')) {
      form_set_error('css_path', t('Double quotes are not allowed in the CSS path.'));
    }
    elseif (substr($edit['css_path'], 0, 1) == "'" && substr($edit['css_path'], -1) == "'") {
      form_set_error('css_path', t('Enter a valid CSS path, do not surround it with quotes.'));
    }
  }

  if (!empty($edit['styles_path'])) {
    if ($edit['css_style'] != 'self') {
      form_set_error('styles_path', t('The path to predefined styles is not empty. Please set the <strong>Predefined styles</strong> option to the <strong>Define path to ckeditor.styles.js</strong> mode.'));
    }
    elseif (FALSE !== strpos($edit['styles_path'], '"')) {
      form_set_error('styles_path', t('Double quotes are not allowed in the styles path.'));
    }
    elseif (substr($edit['styles_path'], 0, 1) == "'" && substr($edit['styles_path'], -1) == "'") {
      form_set_error('styles_path', t('Enter a valid styles path, do not surround it with quotes.'));
    }
  }

  if (!empty($edit['font_format'])) {
    if (!preg_match("/^((p|div|pre|address|h1|h2|h3|h4|h5|h6);)*(p|div|pre|address|h1|h2|h3|h4|h5|h6)$/", $edit['font_format'])) {
      form_set_error('font_format', t('Enter a valid, semicolon-separated list of HTML font formats (no semicolon at the end of the list is expected).'));
    }
  }
  // @todo DOWNLOAD API
  if (!empty($edit['UserFilesAbsolutePath']) && empty($edit['UserFilesPath'])) {
    form_set_error('UserFilesPath', t('The path to uploaded files is required.'));
  }
  if (!empty($edit['UserFilesPath']) && empty($edit['UserFilesAbsolutePath'])) {
    form_set_error('UserFilesPath', t('An absolute path to uploaded files is required.'));
  }

  if (ckeditor_get_version(TRUE) == 3) {
    $load_methods = _ckeditor_load_methods();
    if (!isset($load_methods[$edit['ckeditor_load_method']])) {
      form_set_error('ckeditor_load_method', t('Set a valid loading method.'));
    }
    if (!preg_match('#\d+#', $edit['ckeditor_load_time_out'])) {
      form_set_error('ckeditor_load_time_out', t('Enter a valid loading timeout in seconds.'));
    }
  }
  $toolbar = $edit['toolbar'];
  $toolbar = str_replace("'", '"', $toolbar);
  $toolbar = preg_replace('/(\w*)\s*\:/', '"${1}":', $toolbar);

  if (strpos($toolbar, 'CodeSnippet') !== false && empty($edit['loadPlugins']['codesnippet'])) {
    form_set_error('loadPlugins][codesnippet', t('The Code Snippet plugin must be enabled if the CodeSnippet button is enabled in the toolbar.'));
  }
  if (strpos($toolbar, 'Mathjax') !== false && empty($edit['loadPlugins']['mathjax'])) {
    form_set_error('loadPlugins][mathjax', t('The Mathjax plugin must be enabled if the Mathjax button is enabled in the toolbar.'));
  }
  if (!json_decode($toolbar)) {
    form_set_error('toolbar', t('Enter a valid toolbar configuration.'));
  }
}

/**
 * Form submit for a profile
 */
function ckeditor_admin_profile_form_submit($form, &$form_state) {
  $edit = & $form_state['values'];

  if (isset($edit['_profile'])) {
    ckeditor_profile_delete($edit['_profile']->name);
    drupal_set_message(t('Your CKEditor profile was updated.'));
  }
  else {
    drupal_set_message(t('Your CKEditor profile was created.'));
  }

  $settings = ckeditor_admin_values_to_settings($edit);
  db_insert('ckeditor_settings')
      ->fields(array(
        "name" => $edit['name'],
        "settings" => $settings
      ))
      ->execute();

  if (!empty($edit['input_formats'])) {
    foreach (array_keys($edit['input_formats']) as $format) {
      if ($edit['input_formats'][$format] != '0') {
        db_insert('ckeditor_input_format')->fields(array("name" => $edit['name'], "format" => $format))->execute();
      }
    }
  }

  $form_state['redirect'] = 'admin/config/content/ckeditor';
}

/**
 * Form builder for a clone profile
 */
function ckeditor_admin_profile_clone_form($form, $form_state, $task, $oldprofile) {
  return ckeditor_admin_profile_form($form, $form_state, $task, $oldprofile);
}

/**
 * Form validation for a clone profile
 */
function ckeditor_admin_profile_clone_form_validate($form_state, $oldprofile) {
  ckeditor_admin_profile_form_validate($form_state, $oldprofile);
}

/**
 * Form submit for a clone profile
 */
function ckeditor_admin_profile_clone_form_submit($form, &$form_state) {
  $edit = & $form_state['values'];
  drupal_set_message(t('Your CKEditor profile was created.'));
  $settings = ckeditor_admin_values_to_settings($edit);
  db_insert('ckeditor_settings')
      ->fields(array(
        "name" => $edit['name'],
        "settings" => $settings
      ))
      ->execute();

  if (!empty($edit['input_formats'])) {
    foreach (array_keys($edit['input_formats']) as $format) {
      if ($edit['input_formats'][$format] != 0) {
        db_insert('ckeditor_input_format')->fields(array("name" => $edit['name'], "format" => $format))->execute();
      }
    }
  }

  $form_state['redirect'] = 'admin/config/content/ckeditor';
}

/**
 * Form builder for a profile delete
 */
function ckeditor_admin_profile_delete_form($form, $form_state, $profile) {
  $form = array();

  $form['_profile'] = array(
    '#type' => 'value',
    '#value' => $profile,
  );

  $form['question'] = array(
    '#type' => 'item',
    '#markup' => t(
        'Are you sure that you want to delete the CKEditor profile <strong>%profile</strong>?', array(
      '%profile' => $profile->name
        )
    ),
  );

  $form['delete'] = array(
    '#type' => 'submit',
    '#id' => 'delete',
    '#value' => t('Delete'),
  );

  $form['back'] = array(
    '#type' => 'submit',
    '#id' => 'back',
    '#value' => t('Cancel'),
  );

  return $form;
}

/**
 * Submit form for a profile delete
 */
function ckeditor_admin_profile_delete_form_submit($form, &$form_state) {
  $v = & $form_state['values'];

  if ($form_state['clicked_button']['#id'] == 'delete') {
    ckeditor_profile_delete($v['_profile']->name);
    drupal_set_message(t('The CKEditor profile was deleted.'));
  }

  $form_state['redirect'] = 'admin/config/content/ckeditor';
}

/**
 * Converts an array of form values to a serialized array that does not
 * contain Drupal Form API values
 */
function ckeditor_admin_values_to_settings($values) {
  $plugins = array();
  if (isset($values['loadPlugins'])) {
    $plugins = $values['loadPlugins'];
  }
  unset($values['name'], $values['input_formats'], $values['_profile'], $values['op'], $values['submit'], $values['form_build_id'], $values['form_token'], $values['form_id'], $values['loadPlugins']);

  // Replace CRLF with LF in toolbar settings
  if(isset($values['toolbar'])){
    $values['toolbar'] = str_replace("\r\n", "\n", $values['toolbar']);
  }

  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
  $plugin_list = ckeditor_load_plugins();
  $values['loadPlugins'] = array();
  if (!empty($plugins)) {
    foreach (array_keys($plugins) as $plugin) {
      if ($plugins[$plugin] != '0') {
        $values['loadPlugins'][$plugin] = $plugin_list[$plugin];
      }
    }
  }

  return serialize($values);
}

/**
 * Remove a profile from the database.
 */
function ckeditor_profile_delete($name) {
  db_delete('ckeditor_settings')
      ->condition('name', $name)
      ->execute();
  db_delete('ckeditor_input_format')
      ->condition('name', $name)
      ->execute();
}

/*
 * List of CKEditor librares to load
 */

function _ckeditor_load_methods() {
  $result = array('ckeditor.js' => 'ckeditor.js');
  $ckeditor_local_path = ckeditor_path('local');
  if ($ckeditor_local_path == '<URL>') {
    return $result;
  }
  if (file_exists($ckeditor_local_path . '/ckeditor_basic.js')) {
    $result['ckeditor_basic.js'] = 'ckeditor_basic.js';
  }
  if (file_exists($ckeditor_local_path . '/ckeditor_source.js')) {
    $result['ckeditor_source.js'] = 'ckeditor_source.js (' . t('for developers only') . ')';
  }
  return $result;
}

/*
 * Disable WYSIWYG module
 */

function ckeditor_disable_wysiwyg($token) {
  if (!drupal_valid_token($token, 'ckeditorDisableWysiwyg')) {
    exit();
  }
  module_disable(array('wysiwyg'));
  drupal_set_message(t('The WYSIWYG module is disabled.'));

  drupal_goto('admin/config/content/ckeditor');
}

/*
 * Get all available toolbar buttons
 */

function ckeditor_toolbar_buttons_all() {
  $path = base_path() . drupal_get_path('module', 'ckeditor');

  //CKEditor default buttons
  $buttons = array(
    'Source' => array('name' => 'Source', 'icon' => $path . '/images/buttons/source.png', 'title' => 'Source', 'row' => 1),
    'Save' => array('name' => 'Save', 'icon' => $path . '/images/buttons/save.png', 'title' => 'Save', 'row' => 1),
    'NewPage' => array('name' => 'NewPage', 'icon' => $path . '/images/buttons/newpage.png', 'title' => 'New Page', 'row' => 1),
    'Preview' => array('name' => 'Preview', 'icon' => $path . '/images/buttons/preview.png', 'title' => 'Preview', 'row' => 1),
    'Templates' => array('name' => 'Templates', 'icon' => $path . '/images/buttons/templates.png', 'title' => 'Templates', 'row' => 1),
    'Cut' => array('name' => 'Cut', 'icon' => $path . '/images/buttons/cut.png', 'title' => 'Cut', 'row' => 1),
    'Copy' => array('name' => 'Copy', 'icon' => $path . '/images/buttons/copy.png', 'title' => 'Copy', 'row' => 1),
    'Paste' => array('name' => 'Paste', 'icon' => $path . '/images/buttons/paste.png', 'title' => 'Paste', 'row' => 1),
    'PasteText' => array('name' => 'PasteText', 'icon' => $path . '/images/buttons/pastetext.png', 'title' => 'Paste as plain text', 'row' => 1),
    'PasteFromWord' => array('name' => 'PasteFromWord', 'icon' => $path . '/images/buttons/pastefromword.png', 'title' => 'Paste from Word', 'row' => 1),
    'Print' => array('name' => 'Print', 'icon' => $path . '/images/buttons/print.png', 'title' => 'Print', 'row' => 1),
    'SpellChecker' => array('name' => 'SpellChecker', 'icon' => $path . '/images/buttons/spellchecker.png', 'title' => 'Check Spelling', 'row' => 1),
    'Scayt' => array('name' => 'Scayt', 'icon' => $path . '/images/buttons/scayt.png', 'title' => 'Spell Check As you Type', 'row' => 1), //TODO sprawdzic ta opcje
    'Undo' => array('name' => 'Undo', 'icon' => $path . '/images/buttons/undo.png', 'title' => 'Undo', 'row' => 1),
    'Redo' => array('name' => 'Redo', 'icon' => $path . '/images/buttons/redo.png', 'title' => 'Redo', 'row' => 1),
    'Find' => array('name' => 'Find', 'icon' => $path . '/images/buttons/find.png', 'title' => 'Find', 'row' => 1),
    'Replace' => array('name' => 'Replace', 'icon' => $path . '/images/buttons/replace.png', 'title' => 'Replace', 'row' => 1),
    'SelectAll' => array('name' => 'SelectAll', 'icon' => $path . '/images/buttons/selectall.png', 'title' => 'Select All', 'row' => 1),
    'RemoveFormat' => array('name' => 'RemoveFormat', 'icon' => $path . '/images/buttons/removeformat.png', 'title' => 'Remove Format', 'row' => 1),
    'Form' => array('name' => 'Form', 'icon' => $path . '/images/buttons/form.png', 'title' => 'Form', 'row' => 1),
    'Checkbox' => array('name' => 'Checkbox', 'icon' => $path . '/images/buttons/checkbox.png', 'title' => 'Checkbox', 'row' => 1),
    'Radio' => array('name' => 'Radio', 'icon' => $path . '/images/buttons/radio.png', 'title' => 'Radio Button', 'row' => 1),
    'TextField' => array('name' => 'TextField', 'icon' => $path . '/images/buttons/textfield.png', 'title' => 'Text Field', 'row' => 1),
    'Textarea' => array('name' => 'Textarea', 'icon' => $path . '/images/buttons/textarea.png', 'title' => 'Textarea', 'row' => 1),
    'Select' => array('name' => 'Select', 'icon' => $path . '/images/buttons/select.png', 'title' => 'Selection Field', 'row' => 1),
    'Button' => array('name' => 'Button', 'icon' => $path . '/images/buttons/button.png', 'title' => 'Button', 'row' => 1),
    'ImageButton' => array('name' => 'ImageButton', 'icon' => $path . '/images/buttons/imagebutton.png', 'title' => 'Image Button', 'row' => 1),
    'HiddenField' => array('name' => 'HiddenField', 'icon' => $path . '/images/buttons/hiddenfield.png', 'title' => 'Hidden Field', 'row' => 1),
    'Bold' => array('name' => 'Bold', 'icon' => $path . '/images/buttons/bold.png', 'title' => 'Bold', 'row' => 2),
    'Italic' => array('name' => 'Italic', 'icon' => $path . '/images/buttons/italic.png', 'type' => 'command', 'title' => 'Italic', 'row' => 2),
    'Underline' => array('name' => 'Underline', 'icon' => $path . '/images/buttons/underline.png', 'title' => 'Underline', 'row' => 2),
    'Strike' => array('name' => 'Strike', 'icon' => $path . '/images/buttons/strike.png', 'title' => 'Strike Through', 'row' => 2),
    'Subscript' => array('name' => 'Subscript', 'icon' => $path . '/images/buttons/subscript.png', 'title' => 'Subscript', 'row' => 2),
    'Superscript' => array('name' => 'Superscript', 'icon' => $path . '/images/buttons/superscript.png', 'title' => 'Superscript', 'row' => 2),
    'NumberedList' => array('name' => 'NumberedList', 'icon' => $path . '/images/buttons/numberedlist.png', 'title' => 'Insert/Remove Numbered List', 'row' => 2),
    'BulletedList' => array('name' => 'BulletedList', 'icon' => $path . '/images/buttons/bulletedlist.png', 'title' => 'Insert/Remove Bulleted List', 'row' => 2),
    'Outdent' => array('name' => 'Outdent', 'icon' => $path . '/images/buttons/outdent.png', 'title' => 'Decrease Indent', 'row' => 2),
    'Indent' => array('name' => 'Indent', 'icon' => $path . '/images/buttons/indent.png', 'title' => 'Increase Indent', 'row' => 2),
    'Blockquote' => array('name' => 'Blockquote', 'icon' => $path . '/images/buttons/blockquote.png', 'title' => 'Block Quote', 'row' => 2),
    'CreateDiv' => array('name' => 'CreateDiv', 'icon' => $path . '/images/buttons/creatediv.png', 'title' => 'Create Div Container', 'row' => 2),
    'JustifyLeft' => array('name' => 'JustifyLeft', 'icon' => $path . '/images/buttons/justifyleft.png', 'title' => 'Left Justify', 'row' => 2),
    'JustifyCenter' => array('name' => 'JustifyCenter', 'icon' => $path . '/images/buttons/justifycenter.png', 'title' => 'Center Justify', 'row' => 2),
    'JustifyRight' => array('name' => 'JustifyRight', 'icon' => $path . '/images/buttons/justifyright.png', 'title' => 'Right Justify', 'row' => 2),
    'JustifyBlock' => array('name' => 'JustifyBlock', 'icon' => $path . '/images/buttons/justifyblock.png', 'title' => 'Block Justify', 'row' => 2),
    'BidiLtr' => array('name' => 'BidiLtr', 'icon' => $path . '/images/buttons/bidiltr.png', 'title' => 'Text direction from left to right', 'row' => 2),
    'BidiRtl' => array('name' => 'BidiRtl', 'icon' => $path . '/images/buttons/bidirtl.png', 'title' => 'Text direction from right to left', 'row' => 2),
    'Language' => array('name' => 'Language', 'icon' => $path . '/images/buttons/language.png', 'title' => 'Set Language', 'row' => 2),
    'Link' => array('name' => 'Link', 'icon' => $path . '/images/buttons/link.png', 'title' => 'Link', 'row' => 2),
    'Unlink' => array('name' => 'Unlink', 'icon' => $path . '/images/buttons/unlink.png', 'title' => 'Unlink', 'row' => 2),
    'Anchor' => array('name' => 'Anchor', 'icon' => $path . '/images/buttons/anchor.png', 'title' => 'Anchor', 'row' => 2),
    'Image' => array('name' => 'Image', 'icon' => $path . '/images/buttons/image.png', 'title' => 'Image', 'row' => 2),
    'Flash' => array('name' => 'Flash', 'icon' => $path . '/images/buttons/flash.png', 'title' => 'Flash', 'row' => 2),
    'Table' => array('name' => 'Table', 'icon' => $path . '/images/buttons/table.png', 'title' => 'Table', 'row' => 2),
    'HorizontalRule' => array('name' => 'HorizontalRule', 'icon' => $path . '/images/buttons/horizontalrule.png', 'title' => 'Insert Horizontal Line', 'row' => 2),
    'Smiley' => array('name' => 'Smiley', 'icon' => $path . '/images/buttons/smiley.png', 'title' => 'Smiley', 'row' => 2),
    'SpecialChar' => array('name' => 'SpecialChar', 'icon' => $path . '/images/buttons/specialchar.png', 'title' => 'Insert Special Character', 'row' => 2),
    'PageBreak' => array('name' => 'PageBreak', 'icon' => $path . '/images/buttons/pagebreak.png', 'title' => 'Insert Page Break for Printing', 'row' => 2),
    'Styles' => array('name' => 'Styles', 'icon' => $path . '/images/buttons/styles.png', 'title' => 'Formatting Styles', 'row' => 3),
    'Format' => array('name' => 'Format', 'icon' => $path . '/images/buttons/format.png', 'title' => 'Paragraph Format', 'row' => 3),
    'Font' => array('name' => 'Font', 'icon' => $path . '/images/buttons/font.png', 'title' => 'Font Name', 'row' => 3),
    'FontSize' => array('name' => 'FontSize', 'icon' => $path . '/images/buttons/size.png', 'title' => 'Font Size', 'row' => 3),
    'TextColor' => array('name' => 'TextColor', 'icon' => $path . '/images/buttons/textcolor.png', 'title' => 'Text Color', 'row' => 3),
    'BGColor' => array('name' => 'BGColor', 'icon' => $path . '/images/buttons/bgcolor.png', 'title' => 'Background Color', 'row' => 3),
    'Maximize' => array('name' => 'Maximize', 'icon' => $path . '/images/buttons/maximize.png', 'title' => 'Maximize', 'row' => 3),
    'ShowBlocks' => array('name' => 'ShowBlocks', 'icon' => $path . '/images/buttons/showblocks.png', 'title' => 'Show Blocks', 'row' => 3),
    'Iframe' => array('name' => 'Iframe', 'icon' => $path . '/images/buttons/iframe.png', 'title' => 'IFrame', 'row' => 3),
    'About' => array('name' => 'About', 'icon' => $path . '/images/buttons/about.png', 'title' => 'About', 'row' => 3),
    '__spacer' => array('name' => FALSE, 'icon' => $path . '/images/buttons/spacer.png', 'title' => 'Spacer', 'row' => 4),
    '__group' => array('name' => FALSE, 'icon' => $path . '/images/buttons/group.png', 'title' => 'Group', 'row' => 4)
  );

  $plugins = ckeditor_load_plugins(TRUE);
  foreach ($plugins as $plugin_name => $plugin) {
    if (!isset($plugin['buttons']) || $plugin['buttons'] == FALSE)
      continue;
    foreach ((array) $plugin['buttons'] as $button_name => $button) {
      $buttons[$button_name] = array('name' => $button_name, 'icon' => $plugin['path'] . $button['icon'], 'title' => t($button['label']), 'row' => 4);
    }
  }

  return $buttons;
}

/**
 * Add security filters to the configuration panel available in the CKEditor profile administration section
 */
function _ckeditor_admin_profile_form_security_filters(&$form, $profile, $security_filters) {
  foreach ((array) $security_filters['modules'] as $module_name => $module_conf) {
    if (isset($module_conf['project_page']) && strlen($module_conf['project_page']) > 0) {
      $form['security']['filters'][$module_name] = array(
        '#type' => 'fieldset',
        '#title' => t(
            "!title <a href=\"!link\" target=\"_blank\">project page</a>", array(
          "!title" => $module_conf['title'],
          "!link" => $module_conf['project_page']
            )
        ),
        '#weight' => $module_conf['weight']
      );
    }
    else {
      $form['security']['filters'][$module_name] = array(
        '#type' => 'fieldset',
        '#title' => t("!title", array("!title" => $module_conf['title'])),
        '#weight' => $module_conf['weight']
      );
    }

    if ($module_conf['installed'] == TRUE) {
      foreach ((array) $module_conf['filters'] as $filter_name => $filter_conf) {
        $filter_text_formats = '<div class="filter-text-formats" filter="' . $filter_name . '"></div><div class="filter-info">';
        $form['security']['filters'][$module_name][$filter_name] = array(
          '#type' => 'item',
          '#title' => t("@data", array('@data' => $filter_conf['title'])),
          '#description' => t("!data", array('!data' => $filter_conf['description'])),
          '#prefix' => "<div class=\"filter-wrapper\"></div>" . $filter_text_formats,
          '#suffix' => "</div>"
        );
      }
    }
    else {
      $form['security']['filters'][$module_name]['not_installed'] = array(
        '#markup' => t("<i>Not installed</i>")
      );
    }
  }
}

function ckeditor_skinframe() {
  header('Content-Type: text/html; charset=utf-8');

  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], 'ckeditorSkinframeCall')) {
    exit;
  }

  $editor_path = ckeditor_path('relative');
  if ($editor_path == '<URL>')
    $editor_path = ckeditor_path('url');
  $module_drupal_path = ckeditor_module_path('relative');

  $html = "<!DOCTYPE html>";
  $html .= "<html>";
  $html .= "<head>";
  $html .= '<title>CKEditor UI Color Picker</title>';
  $html .= "<script type=\"text/javascript\" src=\"{$editor_path}/ckeditor.js\"></script>";
  $html .= "</head>";
  $html .= "<body>";
  $html .= "<input type=\"hidden\" name=\"uicolor_user\"/>";
  $html .= "<div id=\"editor\" style=\"height: 200px;\">Click the <strong>UI Color Picker</strong> button to set your color preferences.</div>";
  $html .= "<script type=\"text/javascript\">
var skin = window.location.search.match(/skin=([^&]*)/)[1];
var uicolor = window.location.search.match(/uicolor=([^&]*)/)[1];
var ckeditorUiColorOnChange = function() {
  var color = CKEDITOR.instances['editor'].getUiColor();
  if (typeof(color) != 'undefined') {
    document.getElementsByName('uicolor_user').value = parent.document.getElementsByName('uicolor_user')[0].value = color;
    var edit_uicolor = parent.document.getElementById('edit-uicolor');
    for (i=0; i<edit_uicolor.options.length; i++) {
      if (edit_uicolor[i].value == 'custom') {
        edit_uicolor.selectedIndex = i;
        edit_uicolor[i].selected = 'selected';
        break;
      }
    }
  }
};
CKEDITOR.on('dialogDefinition', function( ev ) {
  var dialogName = ev.data.name;
  var dialogDefinition = ev.data.definition;

  if (dialogName == 'uicolor') {
    dialogDefinition.getContents('tab1').get('configBox').style = 'display:none';
  }
});
CKEDITOR.replace('editor', {
  extraPlugins : 'uicolor',
  height: 330,
  uiColor: '#' + uicolor,
  toolbar : [[ 'Bold', 'Italic', '-', 'NumberedList', 'BulletedList'],[ 'UIColor' ]],
  skin: skin,
  on: {
    configLoaded: function(ev) {
      var ckeditor_ver = CKEDITOR.version.split('.')[0];
      if (ckeditor_ver == 3) {
        if (skin != 'kama') {
          ev.editor.config.toolbar = [[ 'Bold', 'Italic', '-', 'NumberedList', 'BulletedList']];
        }
      }
      else {
        CKEDITOR.plugins.addExternal('uicolor', '{$module_drupal_path}/includes/uicolor/', 'plugin.js');
        if (uicolor.length == 0 || uicolor == 'default') {
          delete ev.editor.config.uiColor;
        }
      }
    },
    blur: ckeditorUiColorOnChange,
    focus: ckeditorUiColorOnChange
  }
});
</script>";
  $html .= "</body>";
  $html .= "</html>";
  echo $html;
}
