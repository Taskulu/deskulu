<?php 
/**
 * @file
 * Synchronization API documentation
 */

/**
 * Provide information about which fields to synchronize for each entity type.
 * 
 * @see i18n_sync_options()
 * 
 * Field definitions defined on hook_field_info() may contain a synchronization
 * callback used for that field to be synchronized. This callback can be set by:
 * $field['i18n_sync_callback'] = 'sychcronize_function_callback
 * 
 * This callback will be invoked with the following parameters
 * - $entity_type, $entity, $field, $instance, $langcode, $items, $source_entity, $source_langcode);
 * 
 * @see i18n_sync_field_info_alter()
 * @see i18n_sync_field_file_sync()
 * 
 * @return array
 *   Array of fields indexed by field name that will be presented as options 
 *   to be synchronized. Each element is an array with the following keys:
 *   - 'title', Field title to be displayed
 *   - 'description', Field description to be displayed.
 *   - 'field_name', Field name for configurable Fields.
 *   - 'group', Group for the UI only to display this field.
 *   
 */
function hook_i18n_sync_options($entity_type, $bundle_name) {
  if ($entity_type == 'node') {
    return array(
      'parent' => array(
        'title' => t('Book outline'),
        'description' => t('Set the translated parent for each translation if possible.')
      ),
    );
  }
}

/**
 * Alter information about synchronization options for entities/field
 * 
 * @see hook_i18n_sync_options()
 */
function hook_i18n_sync_options_alter(&$fields, $entity_type, $bundle_name) {

}

/**
 * Perform aditional synchronization on entities
 * 
 * @param $entity_type
 * @param $translation
 *   Translated entity.
 * @param $translation_language
 *   Translated entity language code.
 * @param $source
 *   Source entity.
 * @param $source_language
 *   Source entity language code.
 * @param $field_names
 *   Array of field names to synchronize.
 */
function hook_i18n_sync_translation($entity_type, $translation, $translation_language, $source, $source_language, $field_names) {
  
}