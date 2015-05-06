<?php

/**
 * @file
 * Builds placeholder replacement tokens for content types.
 */

/**
 * Implements hook_token_info().
 */
function i18n_node_token_info() {
  $content_type['i18n-name'] = array(
    'name' => t("Name (localized)"),
    'description' => t("The name of the content type."),
  );
  $content_type['i18n-description'] = array(
    'name' => t("Description (localized)"),
    'description' => t("The optional description of the content type."),
  );

  return array(
    'tokens' => array(
      'content-type' => $content_type,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function i18n_node_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $sanitize = !empty($options['sanitize']) ? TRUE : FALSE;
  $langcode = isset($options['language']) ? $options['language']->language : i18n_langcode();

  if ($type == 'content-type' && !empty($data['node_type'])) {

    $node_type = $data['node_type'];

    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'i18n-name':
          $name = array('node', 'type', $node_type->type, 'name');
          $options = array('sanitize' => $sanitize, 'langcode' => $langcode);
          $name = i18n_string_translate($name, $node_type->name, $options);
          $replacements[$original] = $name;
          break;

        case 'i18n-description':
          $description = array('node', 'type', $node_type->type, 'description');
          $options = array('sanitize' => $sanitize, 'langcode' => $langcode);
          $description = i18n_string_translate($description, $node_type->description, $options);
          $replacements[$original] = $description;
          break;
      }
    }
  }

  return $replacements;
}
