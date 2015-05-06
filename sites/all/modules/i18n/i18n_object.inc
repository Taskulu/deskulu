<?php
/**
 * @file
 * i18n Object Class
 */

/**
 * Object wrapper
 */
class i18n_object_wrapper {
  protected $type;
  protected $key;
  protected $object;
  // Object translations, static cache.
  protected $translations;

  /**
   * Class constructor
   */
  public function __construct($type, $key, $object) {
    $this->type = $type;
    $this->key = $key;
    $this->object = $object ? $object : $this->load_object($key);
  }

  /**
   * Get edit path for object
   */
  function get_edit_path() {
    return $this->path_replace($this->get_info('edit path'));
  }

  /**
   * Get field value from object/array
   */
  function get_field($field, $default = NULL) {
    return i18n_object_field($this->object, $field, $default);
  }

  /**
   * Set field value to object/array
   */
  function set_field($field, $value) {
    if (is_object($this->object)) {
      $this->object->$field = $value;
    }
    elseif (is_array($this->object)) {
      $this->object[$field] = $value;
    }
    return $this;
  }

  /**
   * Get string numeric key for indexing.
   */
  function get_index() {
    $key = $this->get_key();
    return is_array($key) ? implode(':', $key) : $key;
  }

  /**
   * Get key value from object/array
   */
  function get_key($default = NULL) {
    if ($field = $this->get_info('key')) {
      return $this->get_field($field, $default);
    }
    else {
      return $default;
    }
  }

  /**
   * Get language code
   */
  public function get_langcode() {
    return i18n_object_langcode($this->object);
  }

  /**
   * Get real object or array.
   */
  public function get_object() {
    return $this->object;
  }

  /**
   * Load real object or array.
   *
   * @param $object
   */
  function load_object($object) {
    if ($callback = $this->get_info('load callback', NULL)) {
      $this->object = call_user_func($callback, $object);
    }
    elseif ($entity_type = $this->get_info('entity', NULL)) {
      $entity = entity_load($entity_type, array($object));
      $this->object = $entity ? reset($entity) : FALSE;
    }

    return $this->get_object();
  }

  /**
   * Get menu placehoders for object
   */
  protected function get_placeholders() {
    $placeholders = $this->get_info('placeholders', array());
    foreach ($placeholders as $name => $field) {
      $placeholders[$name] = $this->get_field($field);
    }
    return $placeholders;
  }

  /**
   * Get link for item
   */
  public function get_path() {
    if ($uri = entity_uri($this->type, $this->object)) {
      return $uri['path'];
    }
  }

  /**
   * Get title from item
   */
  public function get_title() {
    return entity_label($this->type, $this->object);
  }
  /**
   * Get object type
   */
  public function get_type() {
    return $this->type;
  }

  /**
   * Menu access callback for mixed translation tab
   */
  function get_translate_access() {
    switch ($this->get_translate_mode()) {
      case I18N_MODE_TRANSLATE:
        return $this->translate_access();
      case I18N_MODE_LOCALIZE:
        return $this->localize_access();
      default:
        return FALSE;
    }
  }

  /**
   * Get translate or localize mode for object
   */
  function get_translate_mode() {
    return I18N_MODE_NONE;
  }

  /**
   * Get translation set id if any
   */
  function get_tsid() {
    return $this->get_field($this->get_translation_info('field', 'i18n_tsid'));
  }

  /**
   * Set translation set id
   */
  function set_tsid($tsid) {
    return $this->set_field($this->get_translation_info('field', 'i18n_tsid'), $tsid);
  }

  /**
   * Localize object if localizable.
   */
  function localize($langcode, $options = array()) {
    if ($this->get_translate_mode() == I18N_MODE_LOCALIZE) {
      return $this->translate($langcode, $options);
    }
    else {
      return $this->object;
    }
  }

  /**
   * Translate object if translatable.
   */
  function translate($langcode, $options = array()) {
    if (isset($this->translations[$langcode])) {
      return $this->translations[$langcode];
    }
    else {
      return $this->object;
    }
  }

  /**
   * Translate access (translation sets)
   */
  protected function translate_access() {
    return FALSE;
  }

  /**
   * Translate access (localize strings)
   */
  protected function localize_access() {
    return FALSE;
  }

  /**
   * Replace path with placeholders
   *
   * @param $path
   *   Path to replace
   * @param $replacements
   *   Replacement variables to override or add to placeholders
   */
  protected function path_replace($path, $replacements = array()) {
    if ($path) {
      $path = strtr($path, $replacements + $this->get_placeholders());
      // Clean up duplicated and final '/' (empty placeholders)
      $path = strtr($path, array('//' => '/'));
      return trim($path, '/');
    }
    else {
      return '';
    }
  }
  /**
   * Get object info
   */
  public function get_info($property, $default = NULL) {
    return i18n_object_info($this->type, $property, $default);
  }
  /**
   * Get object translation set info
   */
  public function get_translation_info($property, $default = NULL) {
    return function_exists('i18n_translation_set_info') ? i18n_translation_set_info($this->type, $property, $default) : $default;
  }
  /**
   * Get object string translation info
   */
  public function get_string_info($property, $default = NULL) {
    $info = $this->get_info('string translation');
    return $info && isset($info[$property]) ? $info[$property] : $default;
  }
}
