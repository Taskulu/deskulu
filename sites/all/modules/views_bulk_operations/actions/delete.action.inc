<?php

/**
 * @file
 * Implements a generic entity delete action. Uses Entity API if available.
 */

function views_bulk_operations_delete_action_info() {
  return array(
    'views_bulk_operations_delete_item' => array(
      'type' => 'entity',
      'label' => t('Delete item'),
      'configurable' => FALSE,
      'behavior' => array('deletes_property'),
      'triggers' => array('any'),
    ),
    'views_bulk_operations_delete_revision' => array(
      'type' => 'entity',
      'label' => t('Delete revision'),
      'configurable' => FALSE,
      'behavior' => array('deletes_property'),
      'triggers' => array('any'),
    ),
  );
}

function views_bulk_operations_delete_item($entity, $context) {
  $info = entity_get_info($context['entity_type']);
  $entity_id = $entity->{$info['entity keys']['id']};

  entity_delete($context['entity_type'], $entity_id);
}

function views_bulk_operations_delete_revision($entity, $context) {
  $info = entity_get_info($context['entity_type']);
  $revision_id = $entity->{$info['entity keys']['revision']};
  entity_revision_delete($context['entity_type'], $revision_id);
}
