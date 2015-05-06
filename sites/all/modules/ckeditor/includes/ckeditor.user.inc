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
function ckeditor_user_customize(&$form, &$form_state, $form_id) {

  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');

  $data = $form['#user']->data;

  $default = ckeditor_user_get_setting_default();
  $lang_options = ckeditor_load_lang_options();

  // because the settings are saved as strings we need to test for the string 'true'
  if (user_access('customize ckeditor')) {
    $form['ckeditor'] = array(
      '#type' => 'fieldset',
      '#title' => t('Rich text editor settings'),
      '#weight' => 10,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );

    $form['ckeditor']['ckeditor_default'] = array(
      '#type' => 'radios',
      '#title' => t('Default state'),
      '#default_value' => isset($data['ckeditor_default']) ? $data['ckeditor_default'] : $default['default'],
      '#options' => array(
        't' => t('Enabled'),
        'f' => t('Disabled')
      ),
      '#description' => t('Should rich text editing be enabled or disabled by default in textarea fields? If disabled, the rich text editor may still be enabled by using toggle.'),
    );

    $form['ckeditor']['ckeditor_show_toggle'] = array(
      '#type' => 'radios',
      '#title' => t('Show the disable/enable rich text editor toggle'),
      '#default_value' => isset($data['ckeditor_show_toggle']) ? $data['ckeditor_show_toggle'] : $default['show_toggle'],
      '#options' => array(
        't' => t('Yes'),
        'f' => t('No')
      ),
      '#description' => t('Whether or not to show the disable/enable rich text editor toggle below the textarea.'),
    );

    $form['ckeditor']['ckeditor_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Editor width'),
      '#default_value' => isset($data['ckeditor_width']) ? $data['ckeditor_width'] : $default['width'],
      '#description' => t('Editor interface width in pixels or percent.') . ' ' . t('Examples') . ': 400 ' . t('or') . ' 100%.',
      '#size' => 40,
      '#maxlength' => 128,
    );

    $form['ckeditor']['ckeditor_lang'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => isset($data['ckeditor_lang']) ? $data['ckeditor_lang'] : $default['lang'],
      '#options' => $lang_options,
      '#description' => t('The language for the CKEditor interface.')
    );

    $form['ckeditor']['ckeditor_auto_lang'] = array(
      '#type' => 'radios',
      '#title' => t('Auto-detect language'),
      '#default_value' => isset($data['ckeditor_auto_lang']) ? $data['ckeditor_auto_lang'] : $default['auto_lang'],
      '#options' => array(
        't' => t('Yes'),
        'f' => t('No')
      ),
      '#description' => t('Automatically detect the user language.')
    );

    $form['#validate'][] = 'ckeditor_user_customize_form_validate';
  }
}

function ckeditor_user_customize_form_validate(&$form, &$form_state) {
  /*
    if (isset($form_state['values']['ckeditor_default'], $form_state['values']['ckeditor_popup']) && $form_state['values']['ckeditor_default'] == 't' && $form_state['values']['ckeditor_popup'] == 't') {
    form_set_error('ckeditor_popup', t('If CKEditor is enabled by default, the popup window must be disabled.'));
    }

    if (isset($form_state['values']['ckeditor_show_toggle'], $form_state['values']['ckeditor_popup']) && $form_state['values']['ckeditor_show_toggle'] == 't' && $form_state['values']['ckeditor_popup'] == 't') {
    form_set_error('ckeditor_popup', t('If toggle is enabled, the popup window must be disabled.'));
    }
   */
  if (isset($form_state['values']['ckeditor_width']) && !preg_match('/^\d+%?$/', $form_state['values']['ckeditor_width'])) {
    form_set_error('ckeditor_width', t('Enter a valid width value.') . ' ' . t('Examples:') . ': 400 ' . t('or') . ' 100%.');
  }
}

function ckeditor_user_presave(&$edit, $account, $category) {
  if (user_access('customize ckeditor')) {
    module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
    $default = ckeditor_user_get_setting_default();

    $edit['data']['ckeditor_default'] = isset($edit['ckeditor_default']) ? $edit['ckeditor_default'] : $default['default'];
    $edit['data']['ckeditor_show_toggle'] = isset($edit['ckeditor_show_toggle']) ? $edit['ckeditor_show_toggle'] : $default['show_toggle'];
    $edit['data']['ckeditor_width'] = isset($edit['ckeditor_width']) ? $edit['ckeditor_width'] : $default['width'];
    $edit['data']['ckeditor_lang'] = isset($edit['ckeditor_lang']) ? $edit['ckeditor_lang'] : $default['lang'];
    $edit['data']['ckeditor_auto_lang'] = isset($edit['ckeditor_auto_lang']) ? $edit['ckeditor_auto_lang'] : $default['auto_lang'];
  }
}
