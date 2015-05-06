<?php

/**
 * @file
 * This file provides the rules integration for this module.
 */

/**
 * Implements of hook_rules_event_info().
 */
function entity_translation_rules_event_info() {
  $events = array(
    'entity_translation_insert' => array(
      'label' => t('After adding an entity translation'),
      'group' => t('Entity'),
      'variables' => array(
        'entity_type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
        ),
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
        ),
        'language_code' => array(
          'type' => 'text',
          'label' => t('Language code'),
        ),
        'values' => array(
          'type' => 'list',
          'label' => t('Raw values'),
          'optional' => TRUE,
        ),
      ),
    ),
    'entity_translation_update' => array(
      'label' => t('After updating an entity translation'),
      'group' => t('Entity'),
      'variables' => array(
        'entity_type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
        ),
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
        ),
        'language_code' => array(
          'type' => 'text',
          'label' => t('Language code'),
        ),
        'values' => array(
          'type' => 'list',
          'label' => t('Raw values'),
          'optional' => TRUE,
        ),
      ),
    ),
    'entity_translation_delete' => array(
      'label' => t('After deleting an entity translation'),
      'group' => t('Entity'),
      'variables' => array(
        'entity_type' => array(
          'type' => 'text',
          'label' => t('Entity type'),
        ),
        'entity' => array(
          'type' => 'entity',
          'label' => t('Entity'),
        ),
        'language_code' => array(
          'type' => 'text',
          'label' => t('Language code'),
        ),
      ),
    ),
  );
  return $events;
}
