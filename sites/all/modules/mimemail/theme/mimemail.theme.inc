<?php

/**
 * @file
 * The theme system, which controls the output of the messages.
 */

function mimemail_theme_theme() {
  $path = drupal_get_path('module', 'mimemail') . '/theme';

  return array(
    'mimemail_message' => array(
      'variables' => array('module' => NULL, 'key' => NULL, 'recipient' => NULL, 'subject' => NULL, 'body' => NULL),
      'template' => 'mimemail-message',
      'pattern' => 'mimemail_message__',
      'file' => 'mimemail.theme.inc',
      'mail theme' => TRUE,
      'path' => $path,
    )
  );
}

/**
 * A preprocess function for theme('mimemail_message').
 *
 * The $variables array initially contains the following arguments:
 * - $recipient: The recipient of the message
 * - $key:  The mailkey associated with the message
 * - $subject: The message subject
 * - $body:  The message body
 *
 * @see mimemail-message.tpl.php
 */
function template_preprocess_mimemail_message(&$variables) {
  $theme = mailsystem_get_mail_theme();
  $themepath = drupal_get_path('theme', $theme);

  $sitestyle = variable_get('mimemail_sitestyle', 1);
  $mailstyles = file_scan_directory($themepath, '#^mail\.css*$#');

  // Check recursively for the existence of a mail.css file in the theme folder.
  if (!empty($mailstyles)) {
    foreach ($mailstyles as $mailstyle) {
      $styles = $mailstyle->uri;
    }
  }
  // If no mail.css was found and the site style sheets including is enabled,
  // gather all style sheets and embed a version of all style definitions.
  elseif ($sitestyle) {
    // Grab local.css if it exists (support for Fusion based themes).
    $local = $themepath . '/css/local.css';
    if (@file_exists($local)) {
      $css_all = drupal_add_css($local, array('group' => CSS_THEME));
    }
    else {
      $css_all = drupal_add_css();
    }
    $css_files = array();
    foreach ($css_all as $key => $options) {
      if ($options['group'] == CSS_THEME && $options['type'] == 'file' &&
        ($options['media'] == 'all' || $options['media'] == 'screen')) {
        $css_files[$key] = $options;
      }
    }
    if (variable_get('preprocess_css', FALSE)) {
      $pattern = '|<link.*href="' . $GLOBALS['base_url'] . '/([^"?]*)[?"].*|';
      $replacement = '\1';
    }
    else {
      $pattern = array(
        '/<([^<>]*)>/', // Remove the style tag.
        '/@import\s+url\("([^"]+)"\);+/', // Remove the import directive.
        '|' . $GLOBALS['base_url'] . '/([^"?]*)[?"].*|' // Remove the base URL.
      );
      $replacement = array('', '\1', '\1');
    }
    $styles = preg_replace($pattern, $replacement, drupal_get_css($css_files));
  }

  $css = '';
  if (isset($styles)) {
    // Process each style sheet.
    foreach (explode("\n", $styles) as $style) {
      if (!empty($style)) {
        $css .= drupal_load_stylesheet($style, TRUE);
      }
    }

    // Wordwrap to adhere to RFC821
    $css = wordwrap($css, 700);
  }

  // Set styles for the message.
  $variables['css'] = $css;

  // Set template alternatives.
  $variables['theme_hook_suggestions'][] = 'mimemail_message__' . str_replace('-', '_', $variables['key']);

   // Process identifiers to be proper CSS classes.
  $variables['module'] = str_replace('_', '-', $variables['module']);
  $variables['key'] = str_replace('_', '-', $variables['key']);
}
