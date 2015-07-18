<?php

/**
 * @file
 * Rules integration.
 */

/**
 * Implements hook_rules_event_info().
 */
function feeds_rules_event_info() {
  // General events definitions.
  $info = array(
    'feeds_before_import' => array(
      'label' => t('Before importing feed'),
      'group' => t('Feeds'),
      'variables' => array('source' => array('type' => 'feeds_source', 'label' => 'Feeds source')),
    ),
    'feeds_after_import' => array(
      'label' => t('After importing feed'),
      'group' => t('Feeds'),
      'variables' => array('source' => array('type' => 'feeds_source', 'label' => 'Feeds source')),
    ),
  );
  // Per importer events definitions.
  $entity_info = entity_get_info();
  foreach (feeds_importer_load_all() as $importer) {
    $config = $importer->getConfig();
    $processor = feeds_plugin($config['processor']['plugin_key'], $importer->id);

    // It's possible to get FeedsMissingPlugin here which will break things
    // since it doesn't implement FeedsProcessor::entityType().
    if (!$processor instanceof FeedsProcessor) {
      continue;
    }

    $entity_type = $processor->entityType();
    $label = isset($entity_info[$entity_type]['label']) ? $entity_info[$entity_type]['label'] : $entity_type;

    $info['feeds_import_'. $importer->id] = array(
      'label' => t('Before saving an item imported via @name.', array('@name' => $importer->config['name'])),
      'group' => t('Feeds'),
      'variables' => array(
        $entity_type => array(
          'label' => t('Imported @label', array('@label' => $label)),
          'type' => $entity_type,
          // Saving is handled by feeds anyway (unless the skip action is used).
          'skip save' => TRUE,
        ),
      ),
      'access callback' => 'feeds_rules_access_callback',
    );
    // Add bundle information if the node processor is used.
    if ($processor instanceof FeedsNodeProcessor) {
      $info['feeds_import_'. $importer->id]['variables'][$entity_type]['bundle'] = $processor->bundle();
    }
  }
  return $info;
}

/**
 * Implements of hook_rules_action_info().
 */
function feeds_rules_action_info() {
  return array(
    'feeds_skip_item' => array(
      'base' => 'feeds_action_skip_item',
      'label' => t('Skip import of feeds item'),
      'group' => t('Feeds'),
      'parameter' => array(
        'entity' => array('type' => 'entity', 'label' => t('The feeds import item to be marked as skipped')),
      ),
      'access callback' => 'feeds_rules_access_callback',
    ),
  );
}

/**
 * Implements hook_rules_data_info().
 */
function feeds_rules_data_info() {
  return array(
    'feeds_source' => array(
      'label' => t('Feeds source'),
      'group' => t('Feeds'),
      'wrap' => TRUE,
      'property info' => array(
        'id' => array(
          'label' => t('ID'),
          'type' => 'text',
          'description' => t("The machine readable name of the source importer."),
        ),
        'imported' => array(
          'label' => t('Date imported'),
          'type' => 'date',
          'description' => t("The date the source was last imported."),
        ),
        // @TODO: fetcher, parser, state ...
      ),
    ),
  );
}

/**
 * Mark feeds import item as skipped.
 */
function feeds_action_skip_item($entity_wrapper) {
  $entity = $entity_wrapper->value();
  if (isset($entity->feeds_item)) {
    $entity->feeds_item->skip = TRUE;
  }
}

/**
 * Help callback for the skip action.
 */
function feeds_action_skip_item_help() {
  return t("This action allows skipping certain feed items during feeds processing, i.e. before an imported item is saved. Once this action is used on a item, the changes to the entity of the feed item are not saved.");
}

/**
 * Access callback for the feeds rules integration.
 */
function feeds_rules_access_callback() {
  return user_access('administer feeds');
}
