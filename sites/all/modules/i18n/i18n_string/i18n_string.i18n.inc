<?php
/**
 * @file
 * Implementation of i18n hooks
 */

/**
 * Implements hook_i18n_string_objects().
 * 
 * Automate object list for object types that have a 'table' property
 */
function i18n_string_i18n_string_objects($type) {
  if ($function = i18n_object_info($type, 'list callback')) {
    return call_user_func($function);
  }
  elseif ($table = i18n_string_object_info($type, 'table')) {
    $query = db_select($table, 's')->fields('s');
    return $query->execute()->fetchAll();
  }
}

/**
 * Implements hook_i18n_string_list().
 * 
 * Collect all strings from objects of this group.
 */
function i18n_string_i18n_string_list($group) {
  $strings = array();
  // It may be for one group or all groups
  $groups = $group == 'all' ? array_keys(i18n_string_group_info()) : array($group);
  foreach ($groups as $group) {  
    // Compile strings for object types for this group
    foreach (i18n_string_group_object_types($group) as $type) {
      $type_strings = i18n_string_object_type_string_list($type);
      if ($type_strings && !empty($type_strings[$group])) {
        $strings[$group] = isset($strings[$group]) ? i18n_string_array_merge($strings[$group], $type_strings[$group]) : $type_strings[$group];
      }
    }
  }
  return $strings;
}

/**
 * Get object types for text group
 */
function i18n_string_group_object_types($group) {
  $types = array();
  foreach (i18n_object_info() as $type => $type_info) {
    if (!empty($type_info['string translation']) && $type_info['string translation']['textgroup'] == $group) {
      $types[] = $type;
    }
  }
  return $types;
}

/**
 * Get object string list that are in this text group.
 * 
 * @param $type
 *   Object type
 */
function i18n_string_object_type_string_list($type) {
  $strings = array();
  if ($objects = module_invoke_all('i18n_string_objects', $type)) {
    foreach ($objects as $object) {
      if ($object_strings = i18n_object($type, $object)->get_properties()) {
        $strings = i18n_string_array_merge($strings, $object_strings);
      }
    }
  }
  return $strings;
}

/**
 * Merges multiple arrays, recursively, and returns the merged array.
 *
 * This function is not equivalent to PHP's array_merge_recursive(),
 * as this version leaves integer keys intact.
 *
 * @see drupal_array_merge_deep(), @see array_merge_recursive()
 *
 * @param ...
 *   Arrays to merge.
 * @return
 *   The merged array.
 */
function i18n_string_array_merge() {
  $arrays = func_get_args();
  $result = array();

  foreach ($arrays as $array) {
    foreach ($array as $key => $value) {
      // Recurse when both values are arrays.
      if (isset($result[$key]) && is_array($result[$key]) && is_array($value)) {
        $result[$key] = i18n_string_array_merge($result[$key], $value);
      }
      // Otherwise, use the latter value, overriding any previous value.
      else {
        $result[$key] = $value;
      }
    }
  }

  return $result;
}