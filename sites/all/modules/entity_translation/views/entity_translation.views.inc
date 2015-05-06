<?php
/**
 * @file
 *
 * Provide views data and handlers for entity_translation.
 */

/**
 * Implements hook_views_data().
 */
function entity_translation_views_data() {
  $data = array();
  $data['entity_translation']['table']['group'] = t('Entity translation');
  // Advertise this table as a possible base table.
  $data['entity_translation']['table']['base'] = array(
    'field' => 'entity_id',
    'title' => t('Entity translation'),
    'help' => t('Information about a translation of an entity.'),
  );
  $data['entity_translation']['entity_id'] = array(
    'title' => t('Entity id'),
    'help' => t('The entity id.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'entity_id',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['entity_type'] = array(
    'title' => t('Entity type'),
    'help' => t('The entity type.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'numeric' => FALSE,
      'validate type' => 'entity_type',
    ),
    'filter' => array(
      'handler' => 'entity_translation_handler_filter_entity_type',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['language'] = array(
    'title' => t('Language'),
    'help' => t('The language of this translation.'),
    'field' => array(
      'handler' => 'views_handler_field_locale_language',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_locale_language',
      'numeric' => FALSE,
      'validate type' => 'language',
    ),
    'filter' => array(
      'handler' => 'entity_translation_handler_filter_language',
      'allow empty' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['source'] = array(
    'title' => t('Source'),
    'help' => t('The source language.'),
    'field' => array(
      'handler' => 'views_handler_field_locale_language',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_locale_language',
      'name field' => 'title',
      'numeric' => FALSE,
      'validate type' => 'string',
    ),
    'filter' => array(
      'handler' => 'entity_translation_handler_filter_language',
      'allow empty' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['status'] = array(
    'title' => t('Translation status'),
    'help' => t('The status of this translation.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'boolean',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['translate'] = array(
    'title' => t('Needs update'),
    'help' => t('Indicates if the translation needs to be updated.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'boolean',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['entity_translation']['created'] = array(
    'title' => t('Created'),
    'help' => t('Created date.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
      'numeric' => FALSE,
      'validate type' => 'date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );
  $data['entity_translation']['changed'] = array(
    'title' => t('Changed'),
    'help' => t('Changed date.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
      'numeric' => FALSE,
      'validate type' => 'date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );
  $data['entity_translation']['translate_link'] = array(
    'title' => t('Translate link'),
    'help' => t('Link to translation overview page.'),
    'field' => array(
      'handler' => 'entity_translation_handler_field_translate_link',
    ),
  );
  $data['entity_translation']['translation_exists'] = array(
    'title' => t('Translation exists'),
    'help' => t('Link to translation overview page.'),
    'filter' => array(
      'handler' => 'entity_translation_handler_filter_translation_exists',
    ),
  );
  $data['entity_translation']['label'] = array(
    'title' => t('Label'),
    'help' => t('The label of the entity.'),
    'field' => array(
      'handler' => 'entity_translation_handler_field_label',
    ),
  );
  return $data;
}

/**
 * Implements hook_views_data_alter().
 *
 * Add entity translation information to the various entity tables.
 */
function entity_translation_views_data_alter(&$data) {
  foreach (entity_get_info() as $type =>  $info) {
    if ($info['fieldable'] && isset($data[$info['base table']])) {
      $table = &$data[$info['base table']];
      $table['entity_translations'] = array(
        'title' => t('Entity translation: translations'),
        'help' => t('Translation information.'),
        'relationship' => array(
          'label' => t('Translations'),
          'base' => 'entity_translation',
          'base field' => 'entity_id',
          'relationship field' => $info['entity keys']['id'],
          'handler' => 'entity_translation_handler_relationship',
          // We add our information here in the definition, so we can copy it
          // later.
          'left_table' => $info['base table'],
          'left_field' => $info['entity keys']['id'],
          'entity type' => $type,
        ),
      );
    }
  }

  // Expose all translatable fields, using a handler based off Views' default
  // field handler, that allows users to select the language to display the
  // field in.
  foreach (field_info_fields() as $field) {
    if ($field['translatable'] && $field['storage']['type'] == 'field_sql_storage') {
      // Set defaults and just change the handler, title, group, and help.
      $defaults = field_views_field_default_views_data($field);
      foreach ($defaults as $table_name => $table_data) {
        if (isset($data[$table_name][$field['field_name']])) {
          $field_title = $table_data[$field['field_name']]['title'];
          $table_data[$field['field_name']]['title'] = t('!title: translated', array('!title' => $field_title));
          $table_data[$field['field_name']]['group'] = t('Entity translation');
          $table_data[$field['field_name']]['help'] .= ' ' . t('Show the field !title translated into a specified language', array('!title' => $field_title));
          $table_data[$field['field_name']]['field']['handler'] = 'entity_translation_handler_field_field';
          $data[$table_name][$field['field_name'] . '_et'] = $table_data[$field['field_name']];
        }
      }
    }
  }
}
