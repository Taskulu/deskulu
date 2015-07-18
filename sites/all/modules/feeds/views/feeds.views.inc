<?php

/**
 * @file
 * Views integration for Feeds tables.
 *
 * @see http://drupal.org/project/views
 */

/**
 * Implements hook_views_data().
 */
function feeds_views_data() {
  $data = array();

  /**
   * Expose feeds_source table to views.
   */
  $data['feeds_source']['table'] = array(
    'group' => 'Feeds source',
    'base' => array(
      'field' => array('feed_nid', 'id'),
      'title' => 'Feeds source',
      'help' => 'The source information for a feed, for example its URL',
    ),
  );
  $data['feeds_source']['feed_nid'] = array(
    'title' => 'Feed node id',
    'help' => 'Contains the node id of a feed node if the feed\'s configuration is attached to a content type, otherwise contains 0.',
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
      'help' => 'Filter on a Feeds Source\'s feed_nid field.',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => 'Argument on a Feeds Source\'s feed_nid field.',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
      'help' => 'Sort Feeds Source\'s feed_nid field.',
    ),
  );
  $data['feeds_source']['config'] = array(
    'title' => 'Source',
    'help' => 'The resource identifier for this feed. For instance a URL.',
    'field' => array(
      'handler' => 'feeds_views_handler_field_source',
      'click sortable' => FALSE,
    ),
  );
  $data['feeds_source']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'feed_nid',
      'type' => 'LEFT',
    ),
  );

  /**
   * Expose feeds_node_item table to views.
   */
  $data['feeds_item']['table'] = array(
    'group' => 'Feeds item',
  );
  $data['feeds_item']['feed_nid'] = array(
    'title' => t('Owner feed nid'),
    'help' => t('The node id of the owner feed node if available.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
      'help' => t('Filter on Feed Items by the Feed they were generated from using the Node Id of the Feed Node.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => t('Argument on Feed Items by the Feed they were generated from using the Node Id of the Feed Node.'),
    ),
    'sort' => array(
      'help' => t('Sort Feed Items by the Feed they were generated from using the Node Id of the Feed Node.'),
    ),
    'relationship' => array(
      'title' => t('Owner feed'),
      'help' => t('Relate a feed item to its owner feed node if available.'),
      'label' => t('Owner feed'),
      'base' => 'node',
      'base field' => 'nid',
    ),
  );
  $data['feeds_item']['url'] = array(
    'title' => t('Item URL'),
    'help' => t('Contains the URL of the feed item.'),
    'field' => array(
      'handler' => 'views_handler_field_url',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'allow empty' => TRUE,
      'help' => t('Filter on a Feeds Item\'s URL field.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => t('Argument on a Feeds Item\'s URL field.'),
    ),
    'sort' => array(
      'help' => t('Sort on a Feeds Item\'s URL field.'),
    ),
  );
  $data['feeds_item']['guid'] = array(
    'title' => t('Item GUID'),
    'help' => t('Contains the GUID of the feed item.'),
    'field' => array(
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'allow empty' => TRUE,
      'help' => t('Filter on a Feeds Item\'s GUID field.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => t('Argument on a Feeds Item\'s GUID field.'),
    ),
    'sort' => array(
      'help' => t('Sort on a Feeds Item\'s GUID field.'),
    ),
  );
  $data['feeds_item']['imported'] = array(
    'title' => t('Import date'),
    'help' => t('Contains the import date of the feed item.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
      'help' => t('Sort on a Feeds Item\'s import date field.'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
      'help' => t('Filter on a Feeds Item\'s import date field.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => t('Argument on a Feeds Item\'s import date field.'),
    ),
  );

  // Add a relationship for each entity type relating the entity's base table
  // to the feeds_item table whre feeds_item.entity_type = 'entity_type'.
  foreach (entity_get_info() as $entity_type => $info) {
    if (!empty($info['base table'])) {
      $data['feeds_item']['table']['join'][$info['base table']] = array(
        'left_field' => $info['entity keys']['id'],
        'field' => 'entity_id',
        'type' => 'LEFT',
        'extra' => array(
          array(
            'field' => 'entity_type',
            'value' => $entity_type,
          ),
        ),
      );
    }
  }

  /**
   * Expose feeds_log table to views.
   */
  $data['feeds_log']['table'] = array(
    'group' => 'Feeds log',
    'base' => array(
      'field' => array('flid'),
      'title' => 'Feeds log',
      'help' => 'Logs events during importing, clearing, expiry.',
    ),
  );
  $data['feeds_log']['id'] = array(
    'title' => 'Importer id',
    'help' => 'The id of an importer.',
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'allow empty' => TRUE,
      'help' => 'Filter on an importer id.',
    ),
    'argument' => array(
      'handler' => 'feeds_views_handler_argument_importer_id',
      'help' => 'Filter on an importer id.',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
      'help' => 'Sort by importer id.',
    ),
    'relationship' => array(
      'title' => t('Importer'),
      'help' => t('Relate a log entry to its importer if available.'),
      'label' => t('Importer'),
      'base' => 'feeds_importer',
      'base field' => 'id',
    ),
  );
  $data['feeds_log']['importer_name'] = array(
    'real field' => 'id',
    'title' => 'Importer name',
    'help' => 'The human readable name of an importer.',
    'field' => array(
      'handler' => 'feeds_views_handler_field_importer_name',
    ),
  );
  $data['feeds_log']['feed_nid'] = array(
    'title' => 'Feed node id',
    'help' => 'Contains the node id of a feed node if the feed\'s configuration is attached to a content type, otherwise contains 0.',
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
      'help' => 'Filter on a Feeds Source\'s feed_nid field.',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'nid',
      'help' => 'Argument on a Feeds Source\'s feed_nid field.',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
      'help' => 'Sort Feeds Source\'s feed_nid field.',
    ),
    'relationship' => array(
      'title' => t('Feed node'),
      'help' => t('Relate a log entry to its feed node if available.'),
      'label' => t('Feed node'),
      'base' => 'node',
      'base field' => 'nid',
    ),
  );
  $data['feeds_log']['log_time'] = array(
    'title' => t('Log time'),
    'help' => t('The time of the event.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $data['feeds_log']['request_time'] = array(
    'title' => t('Request time'),
    'help' => t('The time of the page request of an event.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $data['feeds_log']['message'] = array(
    'title' => 'Log message',
    'help' => 'The message logged by the event.',
    'field' => array(
      'handler' => 'feeds_views_handler_field_log_message',
      'click sortable' => FALSE,
      'additional fields' => array(
        'variables',
      ),
    ),
  );
  $data['feeds_log']['severity'] = array(
    'title' => 'Severity',
    'help' => 'The severity of the event logged.',
    'field' => array(
      'handler' => 'feeds_views_handler_field_severity',
      'click sortable' => FALSE,
    ),
    'filter' => array(
      'handler' => 'feeds_views_handler_filter_severity',
      'allow empty' => TRUE,
      'help' => 'Filter on the severity of a log message.',
    ),
  );
  $data['feeds_log']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'feed_nid',
      'type' => 'LEFT',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_plugins().
 */
function feeds_views_plugins() {
  return array(
    'argument validator' => array(
      'feed_nid' => array(
        'title' => t('Feed node id'),
        'handler' => 'feeds_views_plugin_argument_validate_feed_nid',
      ),
    ),
  );
}
