<?php

/**
 * @file
 * Provides tokens for entity properties which have no token yet.
 */

/**
 * Defines the types of properties to be added as token.
 *
 * @return
 *   An array mapping token types to the usual (entity) type names.
 */
function entity_token_types() {
  $return = entity_token_types_chained();
  return $return + drupal_map_assoc(array('text', 'integer', 'decimal', 'duration', 'boolean', 'uri'));
}

/**
 * Defines a list of token types that need to be chained.
 *
 * @return
 *   If a (token) type is given, whether the given type needs to be chained.
 *   Else a full list of token types to be chained as returned by
 *   entity_token_token_types().
 */
function entity_token_types_chained($type = NULL) {
  // This functions gets called rather often when replacing tokens, thus
  // we statically cache $types using the advanced drupal static pattern.
  static $drupal_static_fast;
  if (!isset($drupal_static_fast)) {
    $drupal_static_fast['types'] = &drupal_static(__FUNCTION__, array());
  }
  $types = &$drupal_static_fast['types'];

  if (!$types) {
    // Add entities.
    foreach (entity_get_info() as $entity_type => $info) {
      if ($token_type = isset($info['token type']) ? $info['token type'] : $entity_type) {
        $types[$token_type] = $entity_type;
      }
    }
    // Add 'date' and 'site' tokens.
    $types['date'] = 'date';
    $types['site'] = 'site';
    // Add a 'struct' type.
    $types['struct'] = 'struct';
  }

  if (isset($type)) {
    return isset($types[$type]) || entity_property_list_extract_type($type);
  }
  return $types;
}

/**
 * Gets the right token type for a given property info array.
 */
function _entity_token_map_to_token_type($property_info) {
  $lookup = &drupal_static(__FUNCTION__);

  if (!$lookup) {
    // Initialize a lookup array mapping property types to token types.
    $lookup = array_flip(entity_token_types());
  }

  $type = isset($property_info['type']) ? $property_info['type'] : 'text';
  // Just use the type 'struct' for all structures.
  if (!empty($property_info['property info'])) {
    $type = 'struct';
  }

  if ($item_type = entity_property_list_extract_type($type)) {
    return isset($lookup[$item_type]) ? "list<$lookup[$item_type]>" : FALSE;
  }
  return isset($lookup[$type]) ? $lookup[$type] : FALSE;
}

/**
 * Implements hook_token_info_alter().
 */
function entity_token_token_info_alter(&$info) {
  $entity_info = entity_get_info();
  $token_types = entity_token_types_chained();

  // Loop over all chain-able token types, as those may contain further tokens,
  // e.g. entity types or 'site'.
  foreach ($token_types as $token_type => $type) {
    // Just add all properties regardless whether it's in a bundle, but only if
    // there is no token of the property yet.
    foreach (entity_get_all_property_info($type) as $name => $property) {
      $name = str_replace('_', '-', $name);
      $property += array('type' => 'text', 'description' => $property['label']);
      $property_token_type = _entity_token_map_to_token_type($property);

      if (!isset($info['tokens'][$token_type][$name]) && $property_token_type) {

        $info['tokens'][$token_type][$name] = array(
          'name' => $property['label'],
          'description' => $property['description'],
          'type' => $property_token_type,
          // Mark the token so we know we have to provide the value afterwards.
          'entity-token' => TRUE,
        );
      }
      if ($property_token_type == 'struct' && !empty($property['property info'])) {
        $info['tokens'][$token_type][$name]['dynamic'] = TRUE;
        $help = array();
        foreach ($property['property info'] as $key => $property_info) {
          $help[] = $key . ' (' . $property_info['label'] . ')';
        }
        $info['tokens'][$token_type][$name]['description'] .= ' ' . t('The following properties may be appended to the token: @keys',
          array('@keys' => implode(', ', $help))
        );
      }
    }
  }

  // Make sure all chain-able token types we support are registered.
  foreach ($token_types as $token_type => $type) {

    if (!empty($info['tokens'][$token_type]) && !isset($info['types'][$token_type])) {
      if (isset($entity_info[$type])) {
        $info['types'][$token_type] = array(
          'name' => $entity_info[$type]['label'],
          'description' => t('Tokens related to the "@name" entities.', array('@name' => $entity_info[$type]['label'])),
          'needs-data' => $token_type,
        );
      }
      else {
        $info['types'][$token_type] = array(
          'name' => drupal_strtoupper($token_type),
          'description' => t('@name tokens.', array('@name' => drupal_strtoupper($token_type))),
          'needs-data' => $token_type,
        );
      }
    }
    if (!empty($info['tokens'][$token_type]) && !isset($info['types']["list<$token_type>"]) && $token_type != 'site') {
      if (isset($entity_info[$type])) {
        $info['types']["list<$token_type>"] = array(
          'name' => t('List of @entities', array('@entities' => isset($entity_info[$type]['plural label']) ? $entity_info[$type]['plural label'] : $entity_info[$type]['label'] . 's')),
          'description' => t('Tokens related to the "@name" entities.', array('@name' => $entity_info[$type]['label'])),
          'needs-data' => "list<$token_type>",
        );
      }
      else {
        $info['types']["list<$token_type>"] = array(
          'name' => t('List of @type values', array('@type' => $token_type)),
          'description' => t('Tokens for lists of @type values.', array('@type' => $token_type)),
          'needs-data' => "list<$token_type>",
        );
      }
      // Also add some basic token replacements for lists...
      for ($i = 0; $i < 4; $i++) {
        $info['tokens']["list<$token_type>"][$i] = array(
          'name' => t('@type with delta @delta', array('@delta' => $i, '@type' => $info['types'][$token_type]['name'])),
          'description' => t('The list item with delta @delta. Delta values start from 0 and are incremented by one per list item.', array('@delta' => $i)),
          'type' => $token_type,
        );
      }
    }
  }
}

/**
 * Implements hook_tokens().
 */
function entity_token_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $token_types = entity_token_types_chained();
  $replacements = array();

  if (isset($token_types[$type]) && (!empty($data[$type]) || $type == 'site')) {
    $data += array($type => FALSE);

    // Make use of token module's token cache if available.
    $info = module_exists('token') ? token_get_info() : token_info();
    foreach ($tokens as $name => $original) {
      // Provide the token for all properties marked to stem from us.
      if (!empty($info['tokens'][$type][$name]['entity-token']) || $type == 'struct') {
        $wrapper = !isset($wrapper) ? _entity_token_wrap_data($type, $token_types[$type], $data[$type], $options) : $wrapper;
        $property_name = str_replace('-', '_', $name);
        try {
          if (isset($wrapper->$property_name)) {
            $replacement = _entity_token_get_token($wrapper->$property_name, $options);
            if (isset($replacement)) {
              $replacements[$original] = $replacement;
            }
          }
        }
        catch (EntityMetadataWrapperException $e) {
          // If tokens for not existing values are requested, just do nothing.
        }
      }
    }

    // Properly chain everything of a type marked as needs chaining.
    $info['tokens'] += array($type => array());
    foreach ($info['tokens'][$type] as $name => $token_info) {
      if (!empty($token_info['entity-token']) && isset($token_info['type']) && entity_token_types_chained($token_info['type'])) {

        if ($chained_tokens = token_find_with_prefix($tokens, $name)) {
          $wrapper = !isset($wrapper) ? _entity_token_wrap_data($type, $token_types[$type], $data[$type], $options) : $wrapper;
          $property_name = str_replace('-', '_', $name);

          try {
            // Pass on 'struct' properties wrapped, else un-wrap the data.
            $value = ($token_info['type'] == 'struct') ? $wrapper->$property_name : $wrapper->$property_name->value();
            $replacements += token_generate($token_info['type'], $chained_tokens, array($token_info['type'] => $value), $options);
          }
          catch (EntityMetadataWrapperException $e) {
            // If tokens for not existing values are requested, just do nothing.
          }
        }
      }
    }
  }
  // Add support for evaluating tokens for "list<type"> types.
  elseif ($item_token_type = entity_property_list_extract_type($type)) {
    foreach ($tokens as $name => $original) {
      // Care about getting entries of a list.
      if (is_numeric($name)) {
        $wrapper = !isset($wrapper) ? _entity_token_wrap_data($type, "list<$token_types[$item_token_type]>", $data[$type], $options) : $wrapper;
        try {
          $replacement = _entity_token_get_token($wrapper->get($name), $options);
          if (isset($replacement)) {
            $replacements[$original] = $replacement;
          }
        }
        catch (EntityMetadataWrapperException $e) {
          // If tokens for not existing values are requested, just do nothing.
        }
      }
      // Care about generating chained tokens for list-items.
      else {
        $parts = explode(':', $name, 2);
        $delta = $parts[0];

        if (is_numeric($delta) && $chained_tokens = token_find_with_prefix($tokens, $delta)) {
          $wrapper = !isset($wrapper) ? _entity_token_wrap_data($type, "list<$token_types[$item_token_type]>", $data[$type], $options) : $wrapper;
          try {
            $replacements += token_generate($item_token_type, $chained_tokens, array($item_token_type => $wrapper->get($delta)->value()), $options);
          }
          catch (EntityMetadataWrapperException $e) {
            // If tokens for not existing values are requested, just do nothing.
          }
        }
      }
    }
  }

  // Add support for chaining struct data. As struct data has no registered
  // tokens, we have to chain based upon wrapper property info.
  if ($type == 'struct') {
    $wrapper = $data[$type];
    foreach ($wrapper as $name => $property) {
      $token_type = _entity_token_map_to_token_type($property->info());

      if (entity_token_types_chained($token_type) && $chained_tokens = token_find_with_prefix($tokens, $name)) {
        try {
          // Pass on 'struct' properties wrapped, else un-wrap the data.
          $value = ($token_type == 'struct') ? $property : $property->value();
          $replacements += token_generate($token_type, $chained_tokens, array($token_type => $value), $options);
        }
        catch (EntityMetadataWrapperException $e) {
          // If tokens for not existing values are requested, just do nothing.
        }
      }
    }
  }

  return $replacements;
}

/**
 * Wraps the given data by correctly obeying the options.
 */
function _entity_token_wrap_data($token_type, $type, $data, $options) {
  if ($type == 'site') {
    $wrapper = entity_metadata_site_wrapper();
  }
  elseif ($type == 'struct') {
    // 'struct' data items are passed on wrapped.
    $wrapper = $data;
  }
  else {
    $wrapper = entity_metadata_wrapper($type, $data);
  }
  if (isset($options['language']) && $wrapper instanceof EntityStructureWrapper) {
    $wrapper->language($options['language']->language);
  }
  return $wrapper;
}

/**
 * Gets the token replacement by correctly obeying the options.
 */
function _entity_token_get_token($wrapper, $options) {

  if (!$wrapper || $wrapper->value() === NULL) {
    // Do not provide a replacement if there is no value.
    return NULL;
  }

  if (empty($options['sanitize'])) {
    // When we don't need sanitized tokens decode already sanitizied texts.
    $options['decode'] = TRUE;
  }
  $langcode = isset($options['language']) ? $options['language']->language : NULL;

  // If there is a label for a property, e.g. defined by an options list or an
  // entity label, make use of it.
  if ($label = $wrapper->label()) {
    return empty($options['sanitize']) ? $label : check_plain($label);
  }

  switch ($wrapper->type()) {
    case 'integer':
      return $wrapper->value();
    case 'decimal':
      return number_format($wrapper->value(), 2);
    case 'date':
      return format_date($wrapper->value(), 'medium', '', NULL, $langcode);
    case 'duration':
      return format_interval($wrapper->value(), 2, $langcode);
    case 'boolean':
      return $wrapper->value() ? t('true') : t('false');
    case 'uri':
    case 'text':
      return $wrapper->value($options);
  }

  // Care for outputing list values.
  if ($wrapper instanceof EntityListWrapper) {
    $output = array();
    foreach ($wrapper as $item) {
      $output[] = _entity_token_get_token($item, $options);
    }
    return implode(', ', $output);
  }
  // Else we do not have a good string to output, e.g. for struct values. Just
  // output the string representation of the wrapper.
  return (string) $wrapper;
}
