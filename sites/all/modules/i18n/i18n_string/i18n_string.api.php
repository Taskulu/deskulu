<?php

/**
 * @file
 * API documentation file for String translation module.
 * 
 * Basically we are collecting translatable strings for each text group. There are two ways a
 * module can produce this list of strings. It should be one or the other, not both.
 * 
 * 1. Provide a list of objects that are translatable for that text group either defining a
 *    'list callback' for that object type or implementing hook_i18n_string_objects($type) for
 *    that object type.
 * 
 * 2. Provide a full list of strings for that text group by implementing
 *    hook_i18n_string_list()
 *    
 * Then we have hook_i18n_string_list_TEXTGROUP_alter() for other modules to alter either the
 * list of strings for a single object or the full list of strings at the end.
 */

/**
 * List text groups for string translation.
 * 
 * This information will be automatically produced later for hook_locale()
 */
function hook_i18n_string_info() {
  $groups['menu'] = array(
    'title' => t('Menu'),
    'description' => t('Translatable menu items: title and description.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Provide list of translatable strings for text group.

 * A module can provide either a list of translatable strings using hook_i18n_string_list() or
 * it can provide a list of objects using hook_i18n_string_objects() from which the string
 * list will be produced automatically. But not both.
 * 
 * @param $group
 *   Text group name.
 */
function hook_i18n_string_list($group) {
  if ($group == 'mygroup') {
    $strings['mygroup']['type1']['key1']['name'] = 'Name of object type1/key1';
    $strings['mygroup']['type1']['key1']['description'] = 'Description of object type1/key1';
    return $strings;
  }
}

/**
 * Alter string list for objects of text group.
 *
 * To build a list of translatable strings for a text group, we'll follow these steps:
 * 1. Invoke hook_i18n_string_list($textgroup), that will get us an array of strings
 * 2. Get the object types for that textgroup, collectin it from i18n object information.
 *    @see i18n_string_group_object_types()
 * 3. For each object type, collect the full list of objects invoking hook_i18n_string_objects($type)
 *    @see i18n_string_object_type_string_list()
 *    If an object defines a 'list callback' function that one will be called to get the list of strings.
 * 4. For each object, collect the properties for that specific object.
 *    $properties = i18n_object($type, $object)->get_properties();
 * 5. Run this hook to alter the strings for that specific object. In this case we'll pass the
 *    $type and $object parameters.
 * 6. Merge all strings from all objects in an array indexed by textgroup, type, id, name
 * 7. Run this hook once again to alter *all* strings for this textgroup. In this case we
 *    don't have a $type and $object parameters.
 *    
 * Thus this hook is really invoked once per object and once per textgroup on top of that.
 * 
 * @see i18n_string_group_string_list()
 * @see i18n_string_object_type_string_list()
 * @see i18n_menu_i18n_string_list_menu_alter()
 * 
 * @param $strings
 *   Associative array with current string list indexed by textgroup, type, id, name
 * @param $type
 *   Object type ad defined on i18n_object_info()
 * @param $object
 *   Object defined on i18n_object_info()
 *   
 * These last parameters are optional. If type and object are not present
 * we are altering the full list of strings for the text group that happens once at the end.
 */
function hook_i18n_string_list_TEXTGROUP_alter(&$strings, $type = NULL, $object = NULL) {
  if ($type == 'menu_link' && $object) {
    if (isset($object['options']['attributes']['title'])) {
    	$strings['menu']['item'][$object['mlid']]['title']['string'] = $object['link_title']; 
      $strings['menu']['item'][$object['mlid']]['description']['string'] = $object['options']['attributes']['title'];
    }  
  }
}

/**
 * List objects to collect translatable strings.
 * 
 * A module can provide either a list of translatable strings using hook_i18n_string_list() or
 * it can provide a list of objects using hook_i18n_string_objects() from which the string
 * list will be produced automatically. But not both.
 * 
 * @see i18n_object_info()
 * @see i18n_menu_i18n_string_objects()
 * @see i18n_string_i18n_string_list()
 * 
 * @param $type string
 *   Object type
 * @return $objects array
 *   Associative array of objects indexed by object id
 */
function hook_i18n_string_objects($type) {
  if ($type == 'menu_link') {
    // All menu items that have no language and are customized.
    return db_select('menu_links', 'm')
      ->fields('m')
      ->condition('language', LANGUAGE_NONE)
      ->condition('customized', 1)
      ->execute()
      ->fetchAllAssoc('mlid', PDO::FETCH_ASSOC);
  }
}