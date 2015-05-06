<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18n_taxonomy_i18n_object_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    'class' => 'i18n_taxonomy_term',
    'entity' => 'taxonomy_term',
    'key' => 'tid',
    'placeholders' => array(
      '%taxonomy_term' => 'tid',
    ),
    // Auto generate edit path
    'edit path' => 'taxonomy/term/%taxonomy_term/edit',
    // Auto-generate translate tab
    'translate tab' => 'taxonomy/term/%taxonomy_term/translate',
    'translation set' => TRUE,
    'string translation' => array(
      'textgroup' => 'taxonomy',
      'type' => 'term',
      'properties' => array(
        'name' => t('Name'),
        'description' => array(
          'title' => t('Description'),
          'format' => 'format',
        ),
      ),
    )
  );
  $info['taxonomy_vocabulary'] = array(
    'title' => t('Vocabulary'),
    'entity' => 'taxonomy_vocabulary',
    'key' => 'vid',
    'placeholders' => array(
      '%taxonomy_vocabulary_machine_name' => 'machine_name',
    ),
    // Auto generate edit path
    'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/edit',
    // Auto-generate translate tab
    'translate tab' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/translate',
    // We can easily list all these objects
    'list callback' => 'taxonomy_get_vocabularies',
    // Metadata for string translation
    'string translation' => array(
      'textgroup' => 'taxonomy',
      'type' => 'vocabulary',
      'properties' => array(
        'name' => t('Name'),
        'description' => t('Description'),
      ),
    ),
    'translation container' => array(
      'name' => t('vocabulary'),
      'item type' => 'taxonomy_term',
      'item name' => t('terms'),
      'options' => array(I18N_MODE_NONE, I18N_MODE_LOCALIZE, I18N_MODE_TRANSLATE, I18N_MODE_LANGUAGE),
    ),
  );
  return $info;
}

/**
 * Implements hook_i18n_translation_set_info()
 */
function i18n_taxonomy_i18n_translation_set_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    'class' => 'i18n_taxonomy_translation_set',
    'table' => 'taxonomy_term_data',
    'field' => 'i18n_tsid',
    'parent' => 'taxonomy_vocabulary',
    'placeholder' => '%i18n_taxonomy_translation_set',
    'list path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets',
    'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/edit/%i18n_taxonomy_translation_set',
    'delete path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/delete/%i18n_taxonomy_translation_set',
    'page callback' => 'i18n_taxonomy_term_translation_page',
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info()
 */
function i18n_taxonomy_i18n_string_info() {
  $groups['taxonomy'] = array(
    'title' => t('Taxonomy'),
    'description' => t('Vocabulary titles and term names for localizable vocabularies.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => FALSE, // This group cannot list all strings
    'refresh callback' => 'i18n_taxonomy_i18n_string_refresh',
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_objects()
 */
function i18n_taxonomy_i18n_string_objects($type) {
  if ($type == 'taxonomy_term') {
    $terms = array();
    foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) { 
      if (isset($vocabulary->i18n_mode) && $vocabulary->i18n_mode & I18N_MODE_LOCALIZE) {
        foreach (taxonomy_get_tree($vid, 0) as $term) {
          $terms[] = $term;
        }
      }
    }
    return $terms;
  }
}

/**
 * Callback for term translation tab.
 * 
 * @param $type
 *   Should be always 'taxonomy_term'
 * @pram $term
 *   Taxonomy term object
 */
function i18n_taxonomy_term_translation_page($type, $term) {
  module_load_include('admin.inc', 'i18n_taxonomy');
  $vocabulary = taxonomy_vocabulary_load($term->vid);
  $translation_set = !empty($term->i18n_tsid) ? i18n_translation_set_load($term->i18n_tsid) : NULL;

  $translation_overview = i18n_taxonomy_translation_term_overview($term);

  $translation_term_form = drupal_get_form('i18n_taxonomy_translation_term_form', $vocabulary, $translation_set, $term);

  return $translation_overview + $translation_term_form;
}
