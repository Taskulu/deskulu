<?php

/**
 * @file
 * Builds placeholder replacement tokens for taxonomy terms and vocabularies.
 */

/**
 * Implements hook_token_info().
 */
function i18n_taxonomy_token_info() {

  // Taxonomy term related variables.
  $term['i18n-name'] = array(
    'name' => t("Name (localized)"),
    'description' => t("The name of the taxonomy term."),
  );
  $term['i18n-description'] = array(
    'name' => t("Description (localized)"),
    'description' => t("The optional description of the taxonomy term."),
  );

  // Taxonomy vocabulary related variables.
  $vocabulary['i18n-name'] = array(
    'name' => t("Name (localized)"),
    'description' => t("The name of the taxonomy vocabulary."),
  );
  $vocabulary['i18n-description'] = array(
    'name' => t("Description (localized)"),
    'description' => t("The optional description of the taxonomy vocabulary."),
  );

  // Chained tokens for taxonomies
  $term['i18n-vocabulary'] = array(
    'name' => t("Vocabulary (localized)"),
    'description' => t("The vocabulary the taxonomy term belongs to."),
    'type' => 'vocabulary',
  );
  $term['i18n-parent'] = array(
    'name' => t("Parent term (localized)"),
    'description' => t("The parent term of the taxonomy term, if one exists."),
    'type' => 'term',
  );

  return array(
    'tokens' => array(
      'term' => $term,
      'vocabulary' => $vocabulary,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function i18n_taxonomy_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $sanitize = !empty($options['sanitize']);
  $langcode = isset($options['language']) ? $options['language']->language : i18n_langcode();

  if ($type == 'term' && !empty($data['term'])) {
    $term = $data['term'];

    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'i18n-name':
          $name = i18n_taxonomy_term_name($term, $langcode);
          $replacements[$original] = $sanitize ? check_plain($name) : $name;
          break;

        case 'i18n-description':
          $replacements[$original] = i18n_string_text(array('taxonomy', 'term', $term->tid, 'description'), $term->description, array('langcode' => $langcode, 'format' => $term->format, 'sanitize' => $sanitize, 'cache' => TRUE));
          break;

        case 'i18n-vocabulary':
          $vocabulary = taxonomy_vocabulary_load($term->vid);
          $replacements[$original] = check_plain(i18n_taxonomy_vocabulary_name($vocabulary, $langcode));
          break;

        case 'i18n-parent':
          if ($parents = taxonomy_get_parents($term->tid)) {
            $parent = array_pop($parents);
            $replacements[$original] = check_plain(i18n_taxonomy_term_name($parent, $langcode));
          }
          break;
      }
    }
  }

  elseif ($type == 'vocabulary' && !empty($data['vocabulary'])) {
    $vocabulary = $data['vocabulary'];

    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'i18n-name':
          $name = i18n_taxonomy_vocabulary_name($vocabulary, $langcode);
          $replacements[$original] = $sanitize ? check_plain($name) : $name;
          break;

        case 'i18n-description':
          $description = i18n_object_langcode($vocabulary) ? $vocabulary->description : i18n_string(array('taxonomy', 'vocabulary', $vocabulary->vid, 'description'), $vocabulary->description, array('langcode' => $langcode));
          $replacements[$original] = $sanitize ? filter_xss($description) : $description;
          break;
      }
    }
  }

  return $replacements;
}
