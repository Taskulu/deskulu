<?php

/**
 * @file
 * Search API Rules integration.
 */


/**
 * Implements hook_rules_action_info().
 */
function search_api_rules_action_info() {
  $items['search_api_index'] = array (
   'parameter' => array(
      'entity' => array(
        'type' => 'entity',
        'label' => t('Entity'),
        'description' => t('The item to index.'),
      ),
      'index' => array(
        'type' => 'search_api_index',
        'label' => t('Index'),
        'description' => t('The index on which the item should be indexed. Leave blank to index on all indexes for this item type.'),
        'optional' => TRUE,
        'options list' => 'search_api_index_options_list',
      ),
      'index_immediately' => array(
        'type' => 'boolean',
        'label' => t('Index immediately'),
        'description' => t('Activate for indexing the item right away, otherwise it will only be marked as dirty and indexed during the next cron run.'),
        'optional' => TRUE,
        'default value' => TRUE,
        'restriction' => 'input',
      ),
    ),
    'group' => t('Search API'),
    'access callback' => '_search_api_rules_access',
    'label' => t('Index an entity'),
    'base' => '_search_api_rules_action_index',
  );
  return $items;
}

/**
 * Rules access callback for search api actions.
 */
function _search_api_rules_access() {
  return user_access('administer search_api');
}

/**
 * Rules action for indexing an item.
 */
function _search_api_rules_action_index(EntityDrupalWrapper $wrapper, SearchApiIndex $index = NULL, $index_immediately = TRUE) {
  // If we do not have an index, we need to guess the item type to use.
  // @todo Since this can only be used with entities anyways, we can just loop
  //   over the item type information and use all types with that entity type.
  $type = $wrapper->type();
  $item_ids = array($wrapper->getIdentifier());

  if (empty($index) && !$index_immediately) {
    search_api_track_item_change($type, $item_ids);
    return;
  }

  if ($index) {
    $type = $index->item_type;
    $indexes = array($index);
  }
  else {
    $conditions = array(
      'enabled' => 1,
      'item_type' => $type,
      'read_only' => 0,
    );
    $indexes = search_api_index_load_multiple(FALSE, $conditions);
    if (!$indexes) {
      return;
    }
  }
  if ($index_immediately) {
    foreach ($indexes as $index) {
      search_api_index_specific_items_delayed($index, $item_ids);
    }
  }
  else {
    search_api_get_datasource_controller($type)->trackItemChange($item_ids, $indexes);
  }
}

function _search_api_rules_action_index_help() {
  return t('Queues an item for reindexing. If "index immediately" is disabled then the item will be indexed during the next cron run.');
}
