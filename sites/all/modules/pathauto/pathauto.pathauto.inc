<?php

/**
 * @file
 * Pathauto integration for core modules.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_path_alias_types().
 *
 * Used primarily by the bulk delete form.
 */
function pathauto_path_alias_types() {
  $objects['user/'] = t('Users');
  $objects['node/'] = t('Content');
  if (module_exists('blog')) {
    $objects['blog/'] = t('User blogs');
  }
  if (module_exists('taxonomy')) {
    $objects['taxonomy/term/'] = t('Taxonomy terms');
  }
  if (module_exists('forum')) {
    $objects['forum/'] = t('Forums');
  }
  return $objects;
}

/**
 * Implements hook_pathauto().
 *
 * This function is empty so that the other core module implementations can be
 * defined in this file. This is because in pathauto_module_implements_alter()
 * we add pathauto to be included first. The module system then peforms a
 * check on any subsequent run if this function still exists. If this does not
 * exist, than this file will not get included and the core implementations
 * will never get run.
 *
 * @see pathauto_module_implements_alter().
 */
function pathauto_pathauto() {
  // Empty hook; see the above comment.
}

/**
 * Implements hook_pathauto().
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['groupheader'] = t('Content paths');
      $settings['patterndescr'] = t('Default path pattern (applies to all content types with blank patterns below)');
      $settings['patterndefault'] = 'content/[node:title]';
      $settings['batch_update_callback'] = 'node_pathauto_bulk_update_batch_process';
      $settings['batch_file'] = drupal_get_path('module', 'pathauto') . '/pathauto.pathauto.inc';

      $languages = array();
      if (module_exists('locale')) {
        $languages = array(LANGUAGE_NONE => t('language neutral')) + locale_language_list('name');
      }

      foreach (node_type_get_names() as $node_type => $node_name) {
        if (count($languages) && variable_get('language_content_type_' . $node_type, 0)) {
          $settings['patternitems'][$node_type] = t('Default path pattern for @node_type (applies to all @node_type content types with blank patterns below)', array('@node_type' => $node_name));
          foreach ($languages as $lang_code => $lang_name) {
            $settings['patternitems'][$node_type . '_' . $lang_code] = t('Pattern for all @language @node_type paths', array('@node_type' => $node_name, '@language' => $lang_name));
          }
        }
        else {
          $settings['patternitems'][$node_type] = t('Pattern for all @node_type paths', array('@node_type' => $node_name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Batch processing callback; Generate aliases for nodes.
 */
function node_pathauto_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('node', 'n');
  $query->leftJoin('url_alias', 'ua', "CONCAT('node/', n.nid) = ua.source");
  $query->addField('n', 'nid');
  $query->isNull('ua.source');
  $query->condition('n.nid', $context['sandbox']['current'], '>');
  $query->orderBy('n.nid');
  $query->addTag('pathauto_bulk_update');
  $query->addMetaData('entity', 'node');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $nids = $query->execute()->fetchCol();

  pathauto_node_update_alias_multiple($nids, 'bulkupdate');
  $context['sandbox']['count'] += count($nids);
  $context['sandbox']['current'] = max($nids);
  $context['message'] = t('Updated alias for node @nid.', array('@nid' => end($nids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_pathauto().
 */
function taxonomy_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'taxonomy_term';
      $settings['token_type'] = 'term';
      $settings['groupheader'] = t('Taxonomy term paths');
      $settings['patterndescr'] = t('Default path pattern (applies to all vocabularies with blank patterns below)');
      $settings['patterndefault'] = '[term:vocabulary]/[term:name]';
      $settings['batch_update_callback'] = 'taxonomy_pathauto_bulk_update_batch_process';
      $settings['batch_file'] = drupal_get_path('module', 'pathauto') . '/pathauto.pathauto.inc';

      $vocabularies = taxonomy_get_vocabularies();
      if (count($vocabularies)) {
        $settings['patternitems'] = array();
        foreach ($vocabularies as $vid => $vocabulary) {
          if ($vid == variable_get('forum_nav_vocabulary', '')) {
            // Skip the forum vocabulary.
            continue;
          }
          $settings['patternitems'][$vocabulary->machine_name] = t('Pattern for all %vocab-name paths', array('%vocab-name' => $vocabulary->name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Batch processing callback; Generate aliases for taxonomy terms.
 */
function taxonomy_pathauto_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('taxonomy_term_data', 'td');
  $query->leftJoin('url_alias', 'ua', "CONCAT('taxonomy/term/', td.tid) = ua.source");
  $query->addField('td', 'tid');
  $query->isNull('ua.source');
  $query->condition('td.tid', $context['sandbox']['current'], '>');
  // Exclude the forums terms.
  if ($forum_vid = variable_get('forum_nav_vocabulary', '')) {
    $query->condition('td.vid', $forum_vid, '<>');
  }
  $query->orderBy('td.tid');
  $query->addTag('pathauto_bulk_update');
  $query->addMetaData('entity', 'taxonomy_term');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $tids = $query->execute()->fetchCol();

  pathauto_taxonomy_term_update_alias_multiple($tids, 'bulkupdate');
  $context['sandbox']['count'] += count($tids);
  $context['sandbox']['current'] = max($tids);
  $context['message'] = t('Updated alias for term @tid.', array('@tid' => end($tids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_pathauto() for forum module.
 */
function forum_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'forum';
      $settings['token_type'] = 'term';
      $settings['groupheader'] = t('Forum paths');
      $settings['patterndescr'] = t('Pattern for forums and forum containers');
      $settings['patterndefault'] = '[term:vocabulary]/[term:name]';
      $settings['batch_update_callback'] = 'forum_pathauto_bulk_update_batch_process';
      $settings['batch_file'] = drupal_get_path('module', 'pathauto') . '/pathauto.pathauto.inc';
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Batch processing callback; Generate aliases for forums.
 */
function forum_pathauto_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('taxonomy_term_data', 'td');
  $query->leftJoin('url_alias', 'ua', "CONCAT('forum/', td.tid) = ua.source");
  $query->addField('td', 'tid');
  $query->isNull('ua.source');
  $query->condition('td.tid', $context['sandbox']['current'], '>');
  $query->condition('td.vid', variable_get('forum_nav_vocabulary', ''));
  $query->orderBy('td.tid');
  $query->addTag('pathauto_bulk_update');
  $query->addMetaData('entity', 'taxonomy_term');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $tids = $query->execute()->fetchCol();

  pathauto_taxonomy_term_update_alias_multiple($tids, 'bulkupdate');
  $context['sandbox']['count'] += count($tids);
  $context['sandbox']['current'] = max($tids);
  $context['message'] = t('Updated alias for forum @tid.', array('@tid' => end($tids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_pathauto().
 */
function user_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'user';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User paths');
      $settings['patterndescr'] = t('Pattern for user account page paths');
      $settings['patterndefault'] = 'users/[user:name]';
      $settings['batch_update_callback'] = 'user_pathauto_bulk_update_batch_process';
      $settings['batch_file'] = drupal_get_path('module', 'pathauto') . '/pathauto.pathauto.inc';
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Batch processing callback; Generate aliases for users.
 */
function user_pathauto_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('users', 'u');
  $query->leftJoin('url_alias', 'ua', "CONCAT('user/', u.uid) = ua.source");
  $query->addField('u', 'uid');
  $query->isNull('ua.source');
  $query->condition('u.uid', $context['sandbox']['current'], '>');
  $query->orderBy('u.uid');
  $query->addTag('pathauto_bulk_update');
  $query->addMetaData('entity', 'user');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $uids = $query->execute()->fetchCol();

  pathauto_user_update_alias_multiple($uids, 'bulkupdate', array('alias blog' => FALSE));
  $context['sandbox']['count'] += count($uids);
  $context['sandbox']['current'] = max($uids);
  $context['message'] = t('Updated alias for user @uid.', array('@uid' => end($uids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_pathauto().
 */
function blog_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'blog';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('Blog paths');
      $settings['patterndescr'] = t('Pattern for blog page paths');
      $settings['patterndefault'] = 'blogs/[user:name]';
      $settings['batch_update_callback'] = 'blog_pathauto_bulk_update_batch_process';
      $settings['batch_file'] = drupal_get_path('module', 'pathauto') . '/pathauto.pathauto.inc';
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Batch processing callback; Generate aliases for blogs.
 */
function blog_pathauto_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('users', 'u');
  $query->leftJoin('url_alias', 'ua', "CONCAT('blog/', u.uid) = ua.source");
  $query->addField('u', 'uid');
  $query->isNull('ua.source');
  $query->condition('u.uid', $context['sandbox']['current'], '>');
  $query->orderBy('u.uid');
  $query->addTag('pathauto_bulk_update');
  $query->addMetaData('entity', 'user');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $uids = $query->execute()->fetchCol();

  $accounts = user_load_multiple($uids);
  foreach ($accounts as $account) {
    pathauto_blog_update_alias($account, 'bulkupdate');
  }

  $context['sandbox']['count'] += count($uids);
  $context['sandbox']['current'] = max($uids);
  $context['message'] = t('Updated alias for blog user @uid.', array('@uid' => end($uids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
