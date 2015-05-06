<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18n_block_i18n_object_info() {
  $info['block'] = array(
    'title' => t('Block'),
    'class' => 'i18n_block_object',
    'load callback' => 'block_load',
    'key' => array('module', 'delta'),
    'placeholders' => array(
    	'%module' => 'module',
      '%delta' => 'delta',
    ),
    'edit path' => 'admin/structure/block/manage/%module/%delta/configure',
    'string translation' => array(
      'textgroup' => 'blocks',
      'properties' => array(
        'title' => array(
          'title' => t('Title'),
          'empty' => '<none>',
        ),
        'body' => array(
          'title' => t('Body'),
          'format' => 'format',
        ),
      ),
      'translate path' => 'admin/structure/block/manage/%module/%delta/translate/%i18n_language',
    )
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info().
 */
function i18n_block_i18n_string_info() {
  $groups['blocks'] = array(
    'title' => t('Blocks'),
    'description' => t('Configurable blocks titles and content.'),
    'format' => TRUE, // This group has strings with format (block body)
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_objects().
 */
function i18n_block_i18n_string_objects($type) {
  if ($type == 'block') {
    $query = db_select('block', 'b')
      ->distinct()
      ->fields('b', array('module', 'delta', 'title', 'i18n_mode'))
      ->fields('bc', array('body', 'format'))
      ->condition('i18n_mode', I18N_MODE_LOCALIZE);
    $query->leftJoin('block_custom', 'bc', 'b.bid = bc.bid');
    return $query->execute()->fetchAll(PDO::FETCH_OBJ);
  }
}
