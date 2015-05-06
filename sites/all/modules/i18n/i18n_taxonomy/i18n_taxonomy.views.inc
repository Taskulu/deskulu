<?php
/**
 * @file
 * Views integration for i18n_taxonomy fields.
 */

/**
 * Implements hook_field_views_data().
 */
function i18n_taxonomy_field_views_data($field) {
  return taxonomy_field_views_data($field);
}

/**
 * Implements hook_field_views_data_views_data_alter().
 */
function i18n_taxonomy_field_views_data_views_data_alter(&$data, $field) {
  return taxonomy_field_views_data_views_data_alter($data, $field);
}