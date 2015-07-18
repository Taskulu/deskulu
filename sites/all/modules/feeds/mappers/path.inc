<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for path.module.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function path_feeds_processor_targets($entity_type, $bundle_name) {
  $targets = array();

  switch ($entity_type) {
    case 'node':
    case 'taxonomy_term':
    case 'user':
      $targets['path_alias'] = array(
        'name' => t('Path alias'),
        'description' => t('URL path alias of the node.'),
        'callback' => 'path_feeds_set_target',
        'summary_callbacks' => array('path_feeds_summary_callback'),
        'form_callbacks' => array('path_feeds_form_callback'),
      );
      break;
  }

  return $targets;
}

/**
 * Callback for mapping path aliases.
 */
function path_feeds_set_target(FeedsSource $source, $entity, $target, array $values, array $mapping) {
  $alias = FALSE;
  // Path alias cannot be multi-valued, so use the first non-empty value.
  foreach ($values as $value) {
    $value = ltrim(trim($value), '/');
    if (strlen($value)) {
      $alias = $value;
      break;
    }
  }

  $entity->path = array();

  $entity_type = $source->importer->processor->entityType();

  list($id, , ) = entity_extract_ids($entity_type, $entity);

  if ($id) {
    $uri = entity_uri($entity_type, $entity);

    // Check for existing aliases.
    if ($path = path_load($uri['path'])) {
      $entity->path = $path;
    }
  }

  // Allow pathauto to set the path alias if the option is set, and the value is
  // empty.
  $entity->path['pathauto'] = !empty($mapping['pathauto_override']) && $alias === FALSE;

  $entity->path['alias'] = (string) $alias;
}

/**
 * Mapping configuration summary for path.module.
 */
function path_feeds_summary_callback(array $mapping, $target, array $form, array $form_state) {
  if (!module_exists('pathauto')) {
    return;
  }

  if (empty($mapping['pathauto_override'])) {
    return t('Do not allow Pathauto if empty.');
  }

  else {
    return t('Allow Pathauto if empty.');
  }
}

/**
 * Settings form callback.
 */
function path_feeds_form_callback(array $mapping, $target, array $form, array $form_state) {
  return array(
    'pathauto_override' => array(
      '#type' => 'checkbox',
      '#title' => t('Allow Pathauto to set the alias if the value is empty.'),
      '#default_value' => !empty($mapping['pathauto_override']),
    ),
  );
}
