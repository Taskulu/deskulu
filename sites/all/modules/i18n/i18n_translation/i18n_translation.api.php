<?php
/**
 * @file
 * API documentation for Internationalization module
 *
 * Most i18n hooks can be placed on each module.i18n.inc file but in this case
 * such file must be listed in the module.info file.
 */

/**
 * Provide information about translation sets and involved objects.
 *
 * @see i18n_translation_set_info()
 *
 * @see hook_i18n_object_info()
 *
 * This feature relies on object information provided by i18n_object_info().
 */
function hook_i18n_translation_set_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    // The class that handles this translation sets
    'class' => 'i18n_taxonomy_translation_set',
    // Table and field into that table that keeps the translation set id for each item.
    'table' => 'taxonomy_term_data',
    'field' => 'i18n_tsid',
    // This is the parent object (i18n object type).
    'parent' => 'taxonomy_vocabulary',
    // Placeholders and path information for generating translation set pages for administration.
    'placeholder' => '%i18n_taxonomy_translation_set',
    'list path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets',
    'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/edit/%i18n_taxonomy_translation_set',
    'delete path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/delete/%i18n_taxonomy_translation_set',
    'page callback' => 'i18n_taxonomy_term_translation_page',
  );
  return $info;
}

/**
 * Alter i18n object information provided by modules with the previous hook
 *
 * @see i18n_translation_set_info()
 */
function hook_i18n_translation_set_info_alter(&$info) {
}