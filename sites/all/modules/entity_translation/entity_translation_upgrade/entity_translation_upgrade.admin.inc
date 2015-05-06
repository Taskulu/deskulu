<?php

/**
 * @file
 * Converts node translations into field-based translations.
 */

/**
 * The number of node translations that should be processed for each batch step.
 */
define('ENTITY_TRANSLATION_UPGRADE_BATCH_SIZE', 10);

/**
 * Starts the batch process to perform the upgrade.
 */
function entity_translation_upgrade_start($types) {
  $batch = array(
    'operations' => array(
      array('entity_translation_upgrade_do', array($types)),
      array('entity_translation_upgrade_complete', array()),
    ),
    'finished' => 'entity_translation_upgrade_end',
    'title' => t('Entity Translation Upgrade'),
    'init_message' => t('Entity Translation Upgrade is starting.'),
    'error_message' => t('Entity Translation Upgrade has encountered an error.'),
    'file' => drupal_get_path('module', 'entity_translation_upgrade') . '/entity_translation_upgrade.admin.inc',
  );
  batch_set($batch);
  batch_process('admin/config/regional/entity_translation');
}

/**
 * Finshed batch callback.
 */
function entity_translation_upgrade_end($success, $results, $operations, $elapsed) {
  if (!empty($results)) {
    $message = format_plural($results, '1 node translation successfully upgraded.', '@count node translations successfully upgraded.');
    watchdog('entity translation upgrade', '@count node translations successfully upgraded.', array('@count' => $results), WATCHDOG_INFO);
  }
  else {
    $message = t('No node translation available for the upgrade.');
  }

  drupal_set_message($message);
}

/**
 * Batch process to convert node translations to field-based translations.
 */
function entity_translation_upgrade_do($types, &$context) {
  $query = db_select('node', 'n');
  $query->addJoin('LEFT', 'entity_translation_upgrade_history', 'etuh', 'n.nid = etuh.nid');
  $query
    ->fields('n', array('nid', 'tnid'))
    ->where('etuh.complete IS NULL OR etuh.complete = 0')
    ->condition('n.tnid', 0, '<>')
    ->condition('n.tnid <> n.nid', array(), '')
    ->condition('n.type', $types)
    ->orderBy('n.nid');

  // Initialize the batch process.
  if (empty($context['sandbox'])) {
    $total = $query
      ->countQuery()
      ->execute()
      ->fetchField();

    $context['sandbox']['count'] = 0;
    $context['sandbox']['total'] = $total;
    $context['finished'] = $total == 0;
  }
  else {
    $batch_size = variable_get('entity_translation_upgrade_batch_size', ENTITY_TRANSLATION_UPGRADE_BATCH_SIZE);
    $result = $query
      ->range($context['sandbox']['count'], $batch_size)
      ->execute()
      ->fetchAllKeyed();

    // Here we load original nodes and translations all together, but the batch
    // size is determined only by node translations.
    $nids = array_keys($result);
    $nodes = node_load_multiple($nids + array_unique($result));

    $updated_nodes = array();
    $node_translations = array();
    $node_translation_sets = array();
    $instances = array();
    $field_info = array();

    foreach ($nids as $nid) {
      $node = $nodes[$nid];
      $original = $nodes[$node->tnid];
      $handler = entity_translation_get_handler('node', $original);

      if (!isset($instances[$node->type])) {
        $instances[$node->type] = field_info_instances('node', $node->type);
      }

      reset($instances[$node->type]);

      foreach ($instances[$node->type] as $instance) {
        $field_name = $instance['field_name'];
        $field = isset($field_info[$field_name]) ? $field_info[$field_name] : $field_info[$field_name] = field_info_field($field_name);

        // Copy field data.
        if ($field['translatable']) {
          $langcode = isset($node->{$field_name}[$node->language]) ? $node->language : LANGUAGE_NONE;
          if (isset($node->{$field_name}[$langcode])) {
            $original->{$field_name}[$node->language] = $node->{$field_name}[$langcode];
          }
        }
      }

      // Add the new translation.
      $handler->setTranslation(array(
        'translate' => $node->translate,
        'status' => $node->status,
        'language' => $node->language,
        'source' => $original->language,
        'uid' => $node->uid,
        'created' => $node->created,
        'changed' => $node->changed,
      ));

      // Build a list of updated nodes. They will be saved after all the node
      // translation conversions.
      $updated_nodes[$original->nid] = $original;

      // Build a list of obsolete node translations to be unpublished.
      $node_translations[$node->nid] = $node;

      // Build a list of obsolete translations sets to be passed to module hook
      // implementations.
      $node_translation_sets[$original->nid][$node->nid] = $node;

      $context['sandbox']['count']++;
    }

    // Ensure that the multilingual support configuration is set to the right
    // value for the current node type.
    foreach ($instances as $type_name => $data) {
      variable_set("language_content_type_$type_name", ENTITY_TRANSLATION_ENABLED);
    }

    // Save field data and translations for the updated nodes.
    foreach ($updated_nodes as $nid => $node) {
      field_attach_presave('node', $node);
      field_attach_update('node', $node);
      entity_translation_get_handler('node', $node)->saveTranslations();

      foreach ($node_translation_sets[$nid] as $translation) {
        // Allow modules to upgrade their node additions, if possible.
        module_invoke_all('entity_translation_upgrade', $node, $translation);
      }
    }

    if (!empty($node_translations)) {
      $nids = array_keys($node_translations);

      // Unpublish the obsolete node translations.
      db_update('node')
        ->fields(array('status' => 0))
        ->condition('nid', $nids)
        ->execute();

      db_update('node_revision')
        ->fields(array('status' => 0))
        ->condition('nid', $nids)
        ->execute();

      // Populate history table.
      $columns = array('nid', 'tnid', 'language');
      $query = db_insert('entity_translation_upgrade_history')->fields($columns);

      foreach ($node_translations as $node) {
        $query->values((array) $node);
      }

      $query->execute();

      // Flush the modified nodes from the entity load cache.
      entity_get_controller('node')->resetCache($nids);
    }

    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];

    if ($context['finished'] >= 1) {
      $context['results'] = $context['sandbox']['total'];
    }
  }
}

/**
 * Removes the translation sets for all the upgraded nodes.
 */
function entity_translation_upgrade_complete(&$context) {
  if ($nids = db_query('SELECT DISTINCT etuh.tnid FROM {entity_translation_upgrade_history} etuh WHERE etuh.complete = 0')->fetchCol()) {
    // Remove translation sets for migrated nodes.
    db_query('UPDATE {node} SET tnid = 0 WHERE nid IN (:nids)', array(':nids' => $nids));
    entity_get_controller('node')->resetCache($nids);
    // Mark nodes as migrated.
    db_query('UPDATE {entity_translation_upgrade_history} SET complete = 1 WHERE complete = 0');
  }
}

/**
 * Implementations of hook_entity_translation_upgrade() on behalf of core
 * modules.
 */

/**
 * Implements hook_entity_translation_upgrade().
 */
function path_entity_translation_upgrade($node, $translation) {
  // Update URL aliases.
  db_update('url_alias')
    ->fields(array('source' => 'node/' . $node->nid))
    ->condition('source', 'node/' . $translation->nid)
    ->execute();
}

/**
 * Implements hook_entity_translation_upgrade().
 */
function comment_entity_translation_upgrade($node, $translation) {
  // Attach comments to the original node.
  db_update('comment')
    ->fields(array('nid' => $node->nid, 'language' => $translation->language))
    ->condition('nid', $translation->nid)
    ->execute();

  // Update node-comment statistics.
  $ncs = db_select('node_comment_statistics', 'ncs')
    ->fields('ncs')
    ->condition('nid', array($node->nid, $translation->nid))
    ->execute()
    ->fetchAllAssoc('nid');

  $last = $ncs[$node->nid]->last_comment_timestamp > $ncs[$translation->nid]->last_comment_timestamp;
  $ncs_updated = $last ? $ncs[$node->nid] : $ncs[$translation->nid];
  $ncs_updated->nid = $node->nid;
  $ncs_updated->comment_count = $ncs[$node->nid]->comment_count + $ncs[$translation->nid]->comment_count;

  db_update('node_comment_statistics')
    ->fields((array) $ncs_updated)
    ->condition('nid', $node->nid)
    ->execute();
}
