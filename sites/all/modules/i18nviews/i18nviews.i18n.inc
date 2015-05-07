<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18nviews_i18n_object_info() {
  $info['views'] = array(
    'title' => t('Views'),
    'class' => 'i18nviews',
    'key' => 'name',
    'string translation' => array(
      'textgroup' => 'views',
      'type' => 'views',
    ),
    // List of all known objects of this type
    'list callback' => 'views_get_all_views',
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info().
 */
function i18nviews_i18n_string_info() {
  $groups['views'] = array(
    'title' => t('Views'),
    'description' => t('Views translation.'),
    'format' => FALSE,
    'list' => TRUE,
  );
  return $groups;
}
