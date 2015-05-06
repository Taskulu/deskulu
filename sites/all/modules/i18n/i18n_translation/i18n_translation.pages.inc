<?php
/**
 * @file
 * Generic translation page for objects
 */

/**
 * Translate object, create translation set
 */
function i18n_translation_object_translate_page($type, $object) {
  $page = i18n_translation_set_info($type, 'page callback');
  return call_user_func($page, $type, $object);
}