<?php

/**
 * @file
 * Internationalization (i18n) package. Synchronization of translations
 *
 * Node synchronization.
 */

/**
 * Synchronizes fields for node translation.
 *
 * There's some specific handling for known fields like:
 * - files, for file attachments.
 * - iid (CCK node attachments, translations for them will be handled too).
 *
 * All the rest of the fields will be just copied over.
 * The 'revision' field will have the special effect of creating a revision too for the translation.
 *
 * @param $node
 *   Source node being edited.
 * @param $translations
 *   Node translations to synchronize, just needs nid property.
 * @param $fields
 *   List of fields to synchronize.
 * @param $op
 *   Node operation (insert|update).
 */
function i18n_sync_node_translation($node, $translations, $field_names, $op) {
  $total = count($translations);
  $count = 0;
  // Disable language selection and synchronization temporarily, enable it again later
  $i18n_select = i18n_select(FALSE);
  i18n_sync(FALSE);
  foreach ($translations as $translation) {
    // If translation is the same node, we cannot synchronize with itself
    if ($node->nid == $translation->nid) {
      $total--;
      continue;
    }
    // Load full node, we need all data here.
    entity_get_controller('node')->resetCache(array($translation->nid));
    $translation = node_load($translation->nid);
    $i18n_options = i18n_sync_node_options($node->type);
    // Invoke callback for each field, the default is just copy over
    foreach ($field_names as $field) {
      if (!empty($i18n_options[$field]['field_name'])) {
        i18n_sync_field_translation_sync('node', $node->type, $translation, $translation->language, $node, $node->language, $i18n_options[$field]['field_name']);
      }
      elseif (isset($node->$field)) {
        // Standard node field, just copy over.
        $translation->$field = $node->$field;
      }
    }
    // Give a chance to other modules for additional sync
    module_invoke_all('i18n_sync_translation', 'node', $translation, $translation->language, $node, $node->language, $field_names);
    node_save($translation);
    $count++;
    // Flush each entity from the load cache after processing, to
    // avoid exceeding PHP memory limits. It should be safe to keep
    // at least one, however; so we retain the final translation in
    // the cache after saving it.
    if ($count < $total) {
      entity_get_controller('node')->resetCache(array($translation->nid));
    }
  }
  i18n_sync(TRUE);
  i18n_select($i18n_select);
  drupal_set_message(format_plural($count, 'One node translation has been synchronized.', 'All @count node translations have been synchronized.'));
}

/**
 * Node attachments (CCK) that may have translation.
 */
function i18n_sync_node_translation_attached_node(&$node, &$translation, $field) {
  if ($attached = node_load($node->$field)) {
    $translation->$field = i18n_sync_node_translation_reference_field($attached, $node->$field, $translation->language);
  }
}

/**
 * Translating a nodereference field (cck).
 */
function i18n_sync_node_translation_nodereference_field(&$node, &$translation, $field) {
  $translated_references = array();
  foreach ($node->$field as $reference) {
    if ($reference_node = node_load($reference['nid'])) {
      $translated_references[] = array(
        'nid' => i18n_sync_node_translation_reference_field($reference_node, $reference['nid'], $translation->language)
      );
    }
  }
  $translation->$field = $translated_references;
}

/**
 * Helper function to which translates reference field. We try to use translations for reference, otherwise fallback.
 * Example:
 *   English A references English B and English C.
 *   English A and B are translated to German A and B, but English C is not.
 *   The syncronization from English A to German A would it German B and English C.
 */
function i18n_sync_node_translation_reference_field(&$reference_node, $default_value, $langcode) {
  if (isset($reference_node->tnid) && translation_supported_type($reference_node->type)) {
    // This content type has translations, find the one.
    if (($reference_trans = translation_node_get_translations($reference_node->tnid)) && isset($reference_trans[$langcode])) {
      return $reference_trans[$langcode]->nid;
    }
    else {
      // No requested language found, just copy the field.
      return $default_value;
    }
  }
  else {
    // Content type without language, just copy the field.
    return $default_value;
  }
}

/**
 * Synchronize configurable field
 *
 * @param $field_info
 *   Field API field information.
 */
function i18n_sync_node_translation_default_field($node, $translation, $field, $field_info) {
  switch ($field_info['field']['type']) {
    case 'file':
    case 'image':
      i18n_sync_node_translation_file_field($node, $translation, $field);
      break;
    default:
      // For fields that don't need special handling, just copy it over if defined.
      // Field languages are completely unconsistent, for not to say broken
      // both in Drupal core and entity translation. Let's hope this works.
      $source_lang = field_language('node', $node, $field);
      $translation_lang = field_language('node', $translation, $field);
      if (isset($node->{$field}[$source_lang])) {
        $translation->{$field}[$translation_lang] = $node->{$field}[$source_lang];
      }
      break;
  }
}

/**
 * Sync a file or image field:
 * - file-id's (fid) are synced
 * - order of fid's is synced
 * - description, alt, title is kept if already existing, copied otherwise
 *
 * @param object $node the node whose changes are to be synced
 * @param object $translation a node to which the changes need to be synced
 * @param string $field field name
 */
function i18n_sync_node_translation_file_field($node, $translation, $field) {
  if (isset($node->{$field}[$node->language])) {
    // Build a copy of the existing files in the translation node
    // indexed by fid for easy retrieval in the copy loop below
    $existing_files = array();
    if (isset($translation->{$field}[$translation->language])) {
      foreach ($translation->{$field}[$translation->language] as $delta => $file) {
        $existing_files[$file['fid']] = $file;
      }
    }

    // Start creating the translated copy
    $translated_files = $node->{$field}[$node->language];
    foreach ($translated_files as $delta => &$file) {
      // keep alt, title, description if they already exist
      if (array_key_exists($file['fid'], $existing_files)) {
        foreach (array('title', 'description', 'alt') as $property) {
          if (!empty($existing_files[$file['fid']][$property])) {
            $file[$property] = $existing_files[$file['fid']][$property];
          }
        }
      }
    }
    $translation->{$field}[$translation->language] = $translated_files;
  }
}
