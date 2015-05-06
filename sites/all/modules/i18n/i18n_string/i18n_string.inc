<?php
/**
 * @file
 *   API for internationalization strings
 */

/**
 * String object that contains source and translations.
 *
 * Note all database operations must go through textgroup object so we can switch storage at some point.
 */
class i18n_string_object {
  // Updated source string
  public $string;
  // Properties from locale source
  public $lid;
  public $source;
  public $textgroup;
  public $location;
  public $context;
  public $version;
  // Properties from i18n_tring
  public $type;
  public $objectid;
  public $property;
  public $objectkey;
  public $format;
  // Properties from metadata
  public $title;
  // Array of translations to multiple languages
  public $translations = array();
  // Textgroup object
  protected $_textgroup;

  /**
   * Class constructor
   */
  public function __construct($data = NULL) {
    if ($data) {
      $this->set_properties($data);
    }
    // Attempt to re-build the  data from the persistent cache.
    $this->rebuild_from_cache($data);
  }

  /**
   * Rebuild the object data based on the persistent cache.
   *
   * Since the textgroup defines if a string is cacheable or not the caching
   * of the string objects happens in the textgroup handler itself.
   *
   * @see i18n_string_textgroup_cached::__destruct()
   */
  protected function rebuild_from_cache($data = NULL) {
    // Check if we've the required information to repopulate the cache and do so
    // if possible.
    $meta_data_exist = isset($this->textgroup) && isset($this->type) && isset($this->objectid) && isset($this->property);
    if ($meta_data_exist && ($cache = cache_get($this->get_cid())) && !empty($cache->data)) {
      // Re-spawn the cached data.
      // @TODO do we need a array_diff to ensure we don't overwrite the data
      // provided by the $data parameter?
      $this->set_properties($cache->data);
    }
  }

  /**
   * Reset cache, needed for tests.
   */
  public function cache_reset() {
    $this->translations = array();
    // Ensure a possible persistent cache of this object is cleared too.
    cache_clear_all($this->get_cid(), 'cache', TRUE);
  }

  /**
   * Returns the caching id for this object.
   *
   * @return string
   *   The caching id.
   */
  public function get_cid() {
    return 'i18n:string:obj:' . $this->get_name();
  }

  /**
   * Get message parameters from context and string.
   */
  public function get_args() {
    return array(
      '%location' => $this->location,
      '%textgroup' => $this->textgroup,
      '%string' => ($string = $this->get_string()) ? $string : t('[empty string]'),
    );
  }
  /**
   * Set context properties
   */
  public function set_context($context) {
    $parts = is_array($context) ? $context : explode(':', $context);
    $this->context = is_array($context) ? implode(':', $context) : $context;
    // Location will be the full string name
    $this->location = $this->textgroup . ':' . $this->context;
    $this->type = array_shift($parts);
    $this->objectid = $parts ? array_shift($parts) : '';
    $this->objectkey = (int)$this->objectid;
    // Remaining elements glued again with ':'
    $this->property = $parts ? implode(':', $parts) : '';

    // Attempt to re-build the other data from the persistent cache.
    $this->rebuild_from_cache();

    return $this;
  }
  /**
   * Get string name including textgroup and context
   */
  public function get_name() {
    return $this->textgroup . ':' . $this->type . ':' . $this->objectid . ':' . $this->property;
  }
  /**
   * Get source string
   */
  public function get_string() {
    if (isset($this->string)) {
      return $this->string;
    }
    elseif (isset($this->source)) {
      return $this->source;
    }
    elseif ($this->textgroup()->debug) {
      return empty($this->lid) ? t('[Source not found]') : t('[String not found]');
    }
    else {
      return '';
    }
  }
  /**
   * Set source string
   *
   * @param $string
   *   Plain string or array with 'string', 'format', etc...
   */
  public function set_string($string) {
    if (is_array($string)) {
      $this->string = isset($string['string']) ? $string['string'] : NULL;
      if (isset($string['format'])) {
        $this->format = $string['format'];
      }
      if (isset($string['title'])) {
        $this->title = $string['title'];
      }
    }
    else {
      $this->string = $string;
    }

    return $this;
  }
  /**
   * Get string title.
   */
  public function get_title() {
    return isset($this->title) ? $this->title : t('String');
  }
  /**
   * Get translation to language from string object
   */
  public function get_translation($langcode) {
    if (!isset($this->translations[$langcode])) {
      $translation = $this->textgroup()->load_translation($this, $langcode);
      if ($translation && isset($translation->translation)) {
        $this->set_translation($translation, $langcode);
      }
      else {
        // No source, no translation
        $this->translations[$langcode] = FALSE;
      }
    }
    // Which doesn't mean we've got a translation, only that we've got the result cached
    return $this->translations[$langcode];
  }
  /**
   * Set translation for language
   *
   * @param $translation
   *   Translation object (from database) or string
   */
  public function set_translation($translation, $langcode = NULL) {
    if (is_object($translation)) {
      $langcode = $langcode ? $langcode : $translation->language;
      $string = isset($translation->translation) ? $translation->translation : FALSE;
      $this->set_properties($translation);
    }
    else {
      $string = $translation;
    }
    $this->translations[$langcode] = $string;
    return $this;
  }

  /**
   * Format the resulting translation or the default string applying callbacks
   *
   * There's a hidden variable, 'i18n_string_debug', that when set to TRUE will display additional info
   */
  public function format_translation($langcode, $options = array()) {
    $options += array('langcode' => $langcode, 'sanitize' => TRUE, 'cache' => FALSE, 'debug' => $this->textgroup()->debug);
    if ($translation = $this->get_translation($langcode)) {
      $string = $translation;
      if (isset($options['filter'])) {
        $string = call_user_func($options['filter'], $string);
      }
    }
    else {
      // Get default source string if no translation.
      $string = $this->get_string();
      $options['sanitize'] = !empty($options['sanitize default']);
    }
    if (!empty($this->format)) {
      $options += array('format' => $this->format);
    }
    // Add debug information if enabled
    if ($options['debug']) {
      $info = array($langcode, $this->textgroup, $this->context);
      if (!empty($this->format)) {
        $info[] = $this->format;
      }
      $options += array('suffix' => '');
      $options['suffix'] .= ' [' . implode(':', $info) . ']';
    }
    // Finally, apply options, filters, callback, etc...
    return i18n_string_format($string, $options);
  }

  /**
   * Get source string provided a string object.
   *
   * @return
   *   String object if source exists.
   */
  public function get_source() {
    // If already searched and not found we don't have a source,
    if (isset($this->lid) && !$this->lid) {
      return NULL;
    }
    elseif (!isset($this->lid) || !isset($this->source)) {
      // We may have lid from loading a translation but not loaded the source yet.
      if ($source = $this->textgroup()->load_source($this)) {
        // Set properties but don't override existing ones
        $this->set_properties($source, FALSE, FALSE);
        if (!isset($this->string)) {
          $this->string = $source->source;
        }
        return $this;
      }
      else {
        $this->lid = FALSE;
        return NULL;
      }
    }
    else {
      return $this;
    }
  }

  /**
   * Set properties from object or array
   *
   * @param $properties
   *   Obejct or array of properties
   * @param $set_null
   *   Whether to set null properties too
   * @param $override
   *   Whether to set properties that are already set in this object
   */
  public function set_properties($properties, $set_null = TRUE, $override = TRUE) {
    foreach ((array)$properties as $field => $value) {
      if (property_exists($this, $field) && ($set_null || isset($value)) && ($override || !isset($this->$field))) {
        $this->$field = $value;
      }
    }
    return $this;
  }
  /**
   * Access textgroup object
   */
  protected function textgroup() {
    if (!isset($this->_textgroup)) {
      $this->_textgroup = i18n_string_textgroup($this->textgroup);
    }
    return $this->_textgroup;
  }
  /**
   * Update this string.
   */
  public function update($options = array()) {
    return $this->textgroup()->string_update($this, $options);
  }
  /**
   * Delete this string.
   */
  public function remove($options = array()) {
    return $this->textgroup()->string_remove($this, $options);
  }
  /**
   * Check whether there is any problem for the  user to translate a this string.
   *
   * @param $account
   *   Optional user account, defaults to current user.
   *
   * @return
   *   None if the user has access to translate the string.
   *   Error message if the user cannot translate that string.
   */
  public function check_translate_access($account = NULL) {
    return i18n_string_translate_check_string($this, $account);
  }
}

/**
 * Textgroup handler for i18n_string API
 */
class i18n_string_textgroup_default {
  // Text group name
  public $textgroup;
  // Debug flag, set to true to print out more information.
  public $debug;
  // Cached or preloaded string objects
  public $strings = array();
  // Multiple translations search map
  protected $cache_multiple = array();

  /**
   * Class constructor.
   *
   * There are to hidden variables to produce debugging information:
   * - 'i18n_string_debug', generic for all text groups.
   * - 'i18n_string_debug_TEXTGROUP', enable debug only for TEXTGROUP.
   */
  public function __construct($textgroup) {
    $this->textgroup = $textgroup;
    $this->debug = variable_get('i18n_string_debug', FALSE) || variable_get('i18n_string_debug_' . $textgroup, FALSE);
  }
  /**
   * Build string object
   *
   * @param $context
   *   Context array or string
   * @param $string string
   *   Current value for string source
   */
  public function build_string($context, $string = NULL) {
    // First try to locate string on cache
    $context = is_array($context) ? implode(':', $context) : $context;
    if ($cached = $this->cache_get($context)) {
      $i18nstring = $cached;
    }
    else {
      $i18nstring = new i18n_string_object();
      $i18nstring->textgroup = $this->textgroup;
      $i18nstring->set_context($context);
      $this->cache_set($context, $i18nstring);
    }
    if (isset($string)) {
      $i18nstring->set_string($string);
    }
    return $i18nstring;
  }
  /**
   * Add source string to the locale tables for translation.
   *
   * It will also add data into i18n_string table for faster retrieval and indexing of groups of strings.
   * Some string context doesn't have a numeric oid (I.e. content types), it will be set to zero.
   *
   * This function checks for already existing string without context for this textgroup and updates it accordingly.
   * It is intended for backwards compatibility, using already created strings.
   *
   * @param $i18nstring
   *   String object
   * @param $format
   *   Text format, for strings that will go through some filter
   * @return
   *   Update status.
   */
  protected function string_add($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE);
    // Default return status if nothing happens
    $status = -1;
    $source = NULL;
    $location = $i18nstring->location;
    // The string may not be allowed for translation depending on its format.
    if (!$this->string_check($i18nstring, $options)) {
      // The format may have changed and it's not allowed now, delete the source string
      return $this->string_remove($i18nstring, $options);
    }
    elseif ($source = $i18nstring->get_source()) {
      if ($source->source != $i18nstring->string || $source->location != $location) {
        $i18nstring->location = $location;
        // String has changed, mark translations for update
        $status = $this->save_source($i18nstring);
        db_update('locales_target')
          ->fields(array('i18n_status' => I18N_STRING_STATUS_UPDATE))
          ->condition('lid', $source->lid)
          ->execute();
      }
      elseif (empty($source->version)) {
        // When refreshing strings, we've done version = 0, update it
        $this->save_source($i18nstring);
      }
    }
    else {
      // We don't have the source object, create it
      $status = $this->save_source($i18nstring);
    }
    // Make sure we have i18n_string part, create or update
    // This will also create the source object if doesn't exist
    $this->save_string($i18nstring);

    if ($options['watchdog']) {
      switch ($status) {
        case SAVED_UPDATED:
          watchdog('i18n_string', 'Updated string %location for textgroup %textgroup: %string', $i18nstring->get_args());
          break;
        case SAVED_NEW:
          watchdog('i18n_string', 'Created string %location for text group %textgroup: %string', $i18nstring->get_args());
          break;
      }
    }
    return $status;
  }

  /**
   * Check if string is ok for translation
   */
  protected static function string_check($i18nstring, $options = array()) {
    $options += array('messages' => FALSE, 'watchdog' => TRUE);
    if (!empty($i18nstring->format) && !i18n_string_allowed_format($i18nstring->format)) {
      // This format is not allowed, so we remove the string, in this case we produce a warning
      drupal_set_message(t('The string %location for textgroup %textgroup is not allowed for translation because of its text format.', $i18nstring->get_args()), 'warning');
      return FALSE;
    }
    else {
      return TRUE;
    }
  }

  /**
   * Filter array of strings
   *
   * @param array $string_list
   *   Array of strings to be filtered.
   * @param array $filter
   *   Array of name value conditions.
   *
   * @return array
   *   Strings from $string_list that match the filter conditions.
   */
  protected static function string_filter($string_list, $filter) {
    // Remove 'language' and '*' conditions.
    if (isset($filter['language'])) {
      unset($filter['language']);
    }
    while ($field = array_search('*', $filter)) {
      unset($filter[$field]);
    }
    foreach ($string_list as $key => $string) {
      foreach ($filter as $field => $value) {
        if ($string->$field != $value) {
          unset($string_list[$key]);
          break;
        }
      }
    }
    return $string_list;
  }

  /**
   * Build query for i18n_string table
   */
  protected static function string_query($context, $multiple = FALSE) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    $query = db_select('i18n_string', 's')->fields('s');
    if (!empty($context->lid)) {
      $query->condition('s.lid', $context->lid);
    }
    else {
      $query->condition('s.textgroup', $context->textgroup);
      if (!$multiple) {
        $query->condition('s.context', $context->context);
      }
      else {
        // Query multiple strings
        foreach (array('type', 'objectid', 'property') as $field) {
          if (!empty($context->$field)) {
            $query->condition('s.' . $field, $context->$field);
          }
        }
      }
    }
    return $query;
  }

  /**
   * Remove string object.
   *
   * @return
   *   SAVED_DELETED | FALSE (If the operation failed because no source)
   */
  public function string_remove($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE, 'messages' => $this->debug);
    if ($source = $i18nstring->get_source()) {
      db_delete('locales_target')->condition('lid', $source->lid)->execute();
      db_delete('i18n_string')->condition('lid', $source->lid)->execute();
      db_delete('locales_source')->condition('lid', $source->lid)->execute();
      $this->cache_set($source->context, NULL);
      if ($options['watchdog']) {
        watchdog('i18n_string', 'Deleted string %location for text group %textgroup: %string', $i18nstring->get_args());
      }
      if ($options['messages']) {
        drupal_set_message(t('Deleted string %location for text group %textgroup: %string', $i18nstring->get_args()));
      }
      return SAVED_DELETED;
    }
    else {
      if ($options['messages']) {
        drupal_set_message(t('Cannot delete string, not found %location for text group %textgroup: %string', $i18nstring->get_args()));
      }
      return FALSE;
    }
  }

  /**
   * Translate string object
   *
   * @param $i18nstring
   *   String object
   * @param $options
   *   Array with aditional options
   */
  protected function string_translate($i18nstring, $options = array()) {
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // Search for existing translation (result will be cached in this function call)
    $i18nstring->get_translation($langcode);
    return $i18nstring;
  }

  /**
   * Update / create / remove string.
   *
   * @param $name
   *   String context.
   * @pram $string
   *   New value of string for update/create. May be empty for removing.
   * @param $format
   *   Text format, that must have been checked against allowed formats for translation
   * @param $options
   *   Processing options, the ones used here are:
   *   - 'watchdog', whether to produce watchdog messages.
   *   - 'messages', whether to produce user messages.
   *   - 'check', whether to check string format and then update/delete if not allowed.
   * @return status
   *   SAVED_UPDATED | SAVED_NEW | SAVED_DELETED | FALSE (If the string is to be removed but has no source)
   */
  public function string_update($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE, 'messages' => $this->debug, 'check' => TRUE);
    if ((!$options['check'] || $this->string_check($i18nstring, $options)) && $i18nstring->get_string()) {
      // String is ok, has a value so we store it into the database.
      $status = $this->string_add($i18nstring, $options);
    }
    elseif ($i18nstring->get_source()) {
      // Just remove it if we already had a source created before.
      $status = $this->string_remove($i18nstring, $options);
    }
    else {
      // String didn't pass validation or we have an empty string but was not stored anyway.
      $status = FALSE;
    }
    if ($options['messages']) {
      switch ($status) {
        case SAVED_UPDATED:
          drupal_set_message(t('Updated string %location for text group %textgroup: %string', $i18nstring->get_args()));
          break;
        case SAVED_NEW:
          drupal_set_message(t('Created string %location for text group %textgroup: %string', $i18nstring->get_args()));
          break;
      }
    }
    if ($options['watchdog']) {
      switch ($status) {
        case SAVED_UPDATED:
          watchdog('i18n_string', 'Updated string %location for text group %textgroup: %string', $i18nstring->get_args());
          break;
        case SAVED_NEW:
          watchdog('i18n_string', 'Created string %location for text group %textgroup: %string', $i18nstring->get_args());
          break;
      }
    }
    return $status;
  }

  /**
   * Set string object into cache
   */
  protected function cache_set($context, $string) {
    $this->strings[$context] = $string;
  }

  /**
   * Get translation from cache
   */
  protected function cache_get($context) {
    return isset($this->strings[$context]) ? $this->strings[$context] : NULL;
  }

  /**
   * Reset cache, needed for tests
   */
  public function cache_reset() {
    $this->strings = array();
    $this->string_format = array();

    // Reset the persistent caches.
    cache_clear_all('i18n:string:tgroup:' . $this->textgroup , 'cache', TRUE);
    // Reset the complete string object cache too.
    cache_clear_all('i18n:string:obj:', 'cache', TRUE);
  }

  /**
   * Load multiple strings.
   *
   * @return array
   *   List of strings indexed by full string name.
   */
  public function load_strings($conditions = array()) {
    // Add textgroup condition and load all
    $conditions['textgroup'] = $this->textgroup;
    $list = array();
    foreach (i18n_string_load_multiple($conditions) as $string) {
      $list[$string->get_name()] = $string;
      $this->cache_set($string->context, $string);
    }
    return $list;
  }

  /**
   * Load string source from db
   */
  public static function load_source($i18nstring) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    $query = db_select('locales_source', 's')->fields('s');
    $query->leftJoin('i18n_string', 'i', 's.lid = i.lid');
    $query->fields('i', array('format', 'objectid', 'type', 'property', 'objectindex'));
    if (!empty($i18nstring->lid)) {
      $query->condition('s.lid', $i18nstring->lid);
    }
    else {
      $query->condition('s.textgroup', $i18nstring->textgroup);
      $query->condition('s.context', $i18nstring->context);
    }
    // Speed up the query, we just need one row
    return $query->range(0, 1)->execute()->fetchObject();
  }

  /**
   * Load translation from db
   *
   * @todo Optimize when we've already got the source string
   */
  public static function load_translation($i18nstring, $langcode) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    if (!empty($i18nstring->lid)) {
      // We've already got lid, we just need translation data
      $query = db_select('locales_target', 't');
      $query->condition('t.lid', $i18nstring->lid);
    }
    else {
      // Still don't have lid, load string properties too
      $query = db_select('i18n_string', 's')->fields('s');
      $query->leftJoin('locales_target', 't', 's.lid = t.lid');
      $query->condition('s.textgroup', $i18nstring->textgroup);
      $query->condition('s.context', $i18nstring->context);
    }
    // Add translation fields
    $query->fields('t', array('translation', 'i18n_status'));
    $query->condition('t.language', $langcode);
    // Speed up the query, we just need one row
    $query->range(0, 1);
    return $query->execute()->fetchObject();
  }

  /**
   * Save / update string object
   *
   * There seems to be a race condition sometimes so skip errors, #277711
   *
   * @param $string
   *   Full string object to be saved
   * @param $source
   *   Source string object
   */
  protected function save_string($string, $update = FALSE) {
    if (!$string->get_source()) {
      // Create source string so we get an lid
      $this->save_source($string);
    }
    if (!isset($string->objectkey)) {
      $string->objectkey = (int)$string->objectid;
    }
    if (!isset($string->format)) {
      $string->format = '';
    }
    $status = db_merge('i18n_string')
      ->key(array('lid' => $string->lid))
      ->fields(array(
          'textgroup' => $string->textgroup,
          'context' => $string->context,
          'objectid' => $string->objectid,
          'type' => $string->type,
          'property' => $string->property,
          'objectindex' => $string->objectkey,
          'format' => $string->format,
      ))
      ->execute();
    return $status;
  }

  /**
   * Save translation to the db
   *
   * @param $string
   *   Full string object with translation data (language, translation)
   */
  protected function save_translation($string, $langcode) {
    db_merge('locales_target')
      ->key(array('lid' => $string->lid, 'language' => $langcode))
      ->fields(array('translation' => $string->get_translation($langcode)))
      ->execute();
  }

  /**
   * Save source string (create / update)
   */
  protected static function save_source($source) {
    if (isset($source->string)) {
      $source->source = $source->string;
    }
    if (empty($source->version)) {
      $source->version = 1;
    }
    return drupal_write_record('locales_source', $source, !empty($source->lid) ? 'lid' : array());
  }

  /**
   * Remove source and translations for user defined string.
   *
   * Though for most strings the 'name' or 'string id' uniquely identifies that string,
   * there are some exceptions (like profile categories) for which we need to use the
   * source string itself as a search key.
   *
   * @param $context
   *   Textgroup and location glued with ':'.
   * @param $string
   *   Optional source string (string in default language).
   */
  public function context_remove($context, $string = NULL, $options = array()) {
    $options += array('messages' => $this->debug);
    $i18nstring = $this->build_string($context, $string);
    $status = $this->string_remove($i18nstring, $options);

    return $this;
  }

  /**
   * Translate source string
   */
  public function context_translate($context, $string, $options = array()) {
    $i18nstring = $this->build_string($context, $string);
    return $this->string_translate($i18nstring, $options);
  }

  /**
   * Update / create translation source for user defined strings.
   *
   * @param $name
   *   Textgroup and location glued with ':'.
   * @param $string
   *   Source string in default language. Default language may or may not be English.
   * @param $options
   *   Array with additional options:
   *   - 'format', String format if the string has text format.
   *   - 'messages', Whether to print out status messages.
   *   - 'check', whether to check string format and then update/delete if not allowed.
   */
  public function context_update($context, $string, $options = array()) {
    $options += array('format' => FALSE, 'messages' => $this->debug, 'watchdog' => TRUE, 'check' => TRUE);
    $i18nstring = $this->build_string($context, $string);
    $i18nstring->format = $options['format'];
    $this->string_update($i18nstring, $options);
    return $this;
  }

  /**
   * Build combinations of an array of arrays respecting keys.
   *
   * Example:
   *   array(array(a,b), array(1,2)) will translate into
   *   array(a,1), array(a,2), array(b,1), array(b,2)
   */
  protected static function multiple_combine($properties) {
    $combinations = array();
    // Get first key, value. We need to make sure the array pointer is reset.
    $value = reset($properties);
    $key = key($properties);
    array_shift($properties);
    $values = is_array($value) ? $value : array($value);
    foreach ($values as $value) {
      if ($properties) {
        foreach (self::multiple_combine($properties) as $merge) {
          $combinations[] = array_merge(array($key => $value), $merge);
        }
      }
      else {
        $combinations[] = array($key => $value);
      }
    }
    return $combinations;
  }

  /**
   * Get multiple translations with search conditions.
   *
   * @param $translations
   *   Array of translation objects as loaded from the db.
   * @param $langcode
   *   Language code, array of language codes or * to search all translations.
   *
   * @return array
   *   Array of i18n string objects.
   */
  protected function multiple_translation_build($translations, $langcode) {
    $strings = array();
    foreach ($translations as $translation) {
      // The string object may be already in list
      if (isset($strings[$translation->context])) {
        $string = $strings[$translation->context];
      }
      else {
        $string = $this->build_string($translation->context);
        $string->set_properties($translation);
        $strings[$string->context] = $string;
      }
      // If this is a translation we set it there too
      if ($translation->language && $translation->translation) {
        $string->set_translation($translation);
      }
      elseif ($langcode) {
        // This may only happen when we have a source string but not translation.
        $string->set_translation(FALSE, $langcode);
      }
    }
    return $strings;
  }

  /**
   * Load multiple translations from db
   *
   * @todo Optimize when we've already got the source object
   *
   * @param $conditions
   *   Array of field values to use as query conditions.
   * @param $langcode
   *   Language code to search.
   * @param $index
   *   Field to use as index for the result.
   * @return array
   *   Array of string objects with translation set.
   */
  protected function multiple_translation_load($conditions, $langcode) {
    $conditions += array(
      'language' => $langcode,
      'textgroup' => $this->textgroup
    );
    // We may be querying all translations at the same time or just one language.
    // The language field needs some special treatment though.
    $query = db_select('i18n_string', 's')->fields('s');
    $query->leftJoin('locales_target', 't', 's.lid = t.lid');
    $query->fields('t', array('translation', 'language', 'i18n_status'));
    foreach ($conditions as $field => $value) {
      // Single array value, reduce array
      if (is_array($value) && count($value) == 1) {
        $value = reset($value);
      }
      if ($value === '*') {
        continue;
      }
      elseif ($field == 'language') {
        $query->condition('t.language', $value);
      }
      else {
        $query->condition('s.' . $field, $value);
      }
    }
    return $this->multiple_translation_build($query->execute()->fetchAll(), $langcode);
  }

  /**
   * Search multiple translations with key combinations.
   *
   * Each $context field may be a single value, an array of values or '*'.
   * Example:
   * 	array('term', array(1,2), '*')
   * This will be mapped into the following conditions (provided language code is 'es')
   *  array('type' => 'term', 'objectid' => array(1,2), 'property' => '*', 'language' => 'es')
   * And will result in these combinations to search for
   *  array('type' => 'term', 'objectid' => 1, 'property' => '*', 'language' => 'es')
   *  array('type' => 'term', 'objectid' => 2, 'property' => '*', 'language' => 'es')
   *
   * @param $context array
   *   Array with String context conditions.
   *
   * @return
   *   Array of translation objects indexed by context.
   */
  public function multiple_translation_search($context, $langcode) {
    // First, build conditions and identify the variable field.
    $keys = array('type', 'objectid', 'property');
    $conditions = array_combine($keys, $context) + array('language' => $langcode);
    // Find existing searches in cache, compile remaining ones.
    $translations = $search = array();
    foreach ($this->multiple_combine($conditions) as $combination) {
      $cached = $this->multiple_cache_get($combination);
      if (isset($cached)) {
        // Cache hit. Merge and remove value from search.
        $translations += $cached;
      }
      else {
        // Not in cache, add to search conditions skipping duplicated values.
        // As array_merge_recursive() has some bug in PHP 5.2, http://drupal.org/node/1244598
        // we use our simplified version here, instead of $search = array_merge_recursive($search, $combination);
        foreach ($combination as $key => $value) {
          if (!isset($search[$key]) || !in_array($value, $search[$key], TRUE)) {
            $search[$key][] = $value;
          }
        }
      }
    }
    // If we've got any search values left, find translations.
    if ($search) {
      // Load translations for conditions and set them to the cache
      $loaded = $this->multiple_translation_load($search, $langcode);
      if ($loaded) {
        $translations += $loaded;
      }
      // Set cache for each of the multiple search keys.
      foreach ($this->multiple_combine($search) as $combination) {
        $list = $loaded ? $this->string_filter($loaded, $combination) : array();
        $this->multiple_cache_set($combination, $list);
      }
    }
    return $translations;
  }

  /**
   * Set multiple cache.
   *
   * @param $context
   *   String context with language property at the end.
   * @param $strings
   *   Array of strings (may be empty) to cache.
   */
  protected function multiple_cache_set($context, $strings) {
    $cache_key = implode(':', $context);
    $this->cache_multiple[$cache_key] = $strings;
  }

  /**
   * Get strings from multiple cache.
   *
   * @param $context array
   *   String context as array with language property at the end.
   *
   * @return mixed
   *   Array of strings (may be empty) if we've got a cache hit.
   *   Null otherwise.
   */
  protected function multiple_cache_get($context) {
    $cache_key = implode(':', $context);
    if (isset($this->cache_multiple[$cache_key])) {
      return $this->cache_multiple[$cache_key];
    }
    else {
      // Now we try more generic keys. For instance, if we are searching 'term:1:*'
      // we may try too 'term:*:*' and filter out the results.
      foreach ($context as $key => $value) {
        if ($value != '*') {
          $try = array_merge($context, array($key => '*'));
          $cache_key = implode(':', $try);
          if (isset($this->cache_multiple[$cache_key])) {
            // As we've found some more generic key, we need to filter using original conditions.
            $strings = $this->string_filter($this->cache_multiple[$cache_key], $context);
            return $strings;
          }
        }
      }
      // If we've reached here, we didn't find any cache match.
      return NULL;
    }
  }

  /**
   * Translate array of source strings
   *
   * @param $context
   *   Context array with placeholders (*)
   * @param $strings
   *   Optional array of source strings indexed by the placeholder property
   *
   * @return array
   *   Array of string objects (with translation) indexed by the placeholder field
   */
  public function multiple_translate($context, $strings = array(), $options = array()) {
    // First, build conditions and identify the variable field
    $search = $context = array_combine(array('type', 'objectid', 'property'), $context);
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // If we've got keyed source strings set the array of keys on the placeholder field
    // or if not, remove that condition so we search all strings with that keys.
    foreach ($search as $field => $value) {
      if ($value === '*') {
        $property = $field;
        if ($strings) {
          $search[$field] = array_keys($strings);
        }
      }
    }
    // Now we'll add the language code to conditions and get the translations indexed by the property field
    $result = $this->multiple_translation_search($search, $langcode);
    // Remap translations using property field. If we've got strings it is important that they are in the same order.
    $translations = $strings;
    foreach ($result as $key => $i18nstring) {
      $translations[$i18nstring->$property] = $i18nstring;
    }
    // Set strings as source or create
    foreach ($strings as $key => $source) {
      if (isset($translations[$key]) && is_object($translations[$key])) {
        $translations[$key]->set_string($source);
      }
      else {
        // Not found any string for this property, create it to map in the response
        // But make sure we set this language's translation to FALSE so we don't search again
        $newcontext = $context;
        $newcontext[$property] = $key;
        $translations[$key] = $this->build_string($newcontext)
          ->set_string($source)
          ->set_translation(FALSE, $langcode);
      }
    }
    return $translations;
  }

  /**
   * Update string translation, only if source exists.
   *
   * @param $context
   *   String context as array
   * @param $langcode
   *   Language code to create the translation for
   * @param $translation
   *   String translation for this language
   */
  function update_translation($context, $langcode, $translation) {
    $i18nstring = $this->build_string($context);
    if ($source = $i18nstring->get_source()) {
      $source->set_translation($translation, $langcode);
      $this->save_translation($source, $langcode);
      return $source;
    }
  }

  /**
   * Recheck strings after update
   */
  public function update_check() {
    // Find strings in locales_source that have no data in i18n_string
    $query = db_select('locales_source', 'l')
    ->fields('l')
    ->condition('l.textgroup', $this->textgroup);
    $alias = $query->leftJoin('i18n_string', 's', 'l.lid = s.lid');
    $query->isNull('s.lid');
    foreach ($query->execute()->fetchAll() as $string) {
      $i18nstring = $this->build_string($string->context, $string->source);
      $this->save_string($i18nstring);
    }
  }

}

/**
 * String object wrapper
 */
class i18n_string_object_wrapper extends i18n_object_wrapper {
  // Text group object
  protected $textgroup;
  // Properties for translation
  protected $properties;

  /**
   * Get object strings for translation
   *
    * This will return a simple array of string objects, indexed by full string name.
    *
    * @param $options
    *   Array with processing options.
    *   - 'empty', whether to return empty strings, defaults to FALSE.
   */
  public function get_strings($options = array()) {
    $options += array('empty' => FALSE);
    $strings = array();
    foreach ($this->get_properties() as $textgroup => $textgroup_list) {
      foreach ($textgroup_list as $type => $type_list) {
        foreach ($type_list as $object_id => $object_list) {
          foreach ($object_list as $key => $string) {
            if ($options['empty'] || !empty($string['string'])) {
              // Build string object, that will trigger static caches everywhere.
              $i18nstring = i18n_string_textgroup($textgroup)
                ->build_string(array($type, $object_id, $key))
                ->set_string($string);
              $strings[$i18nstring->get_name()] = $i18nstring;
            }
          }
        }
      }
    }
    return $strings;
  }
  /**
   * Get object translatable properties
   *
   * This will return a big array indexed by textgroup, object type, object id and string key.
   * Each element is an array with string information, and may have these properties:
   * - 'string', the string itself, will be NULL if the object doesn't have that string
   * - 'format', string format when needed
   * - 'title', string readable name
   */
  public function get_properties() {
    if (!isset($this->properties)) {
      $this->properties = $this->build_properties();
      // Call hook_i18n_string_list_TEXTGROUP_alter(), last chance for modules
      drupal_alter('i18n_string_list_' . $this->get_textgroup(), $this->properties, $this->type, $this->object);

    }
    return $this->properties;
  }

  /**
   * Build properties from object.
   */
  protected function build_properties() {
    list($string_type, $object_id) = $this->get_string_context();
    $object_keys = array(
      $this->get_textgroup(),
      $string_type,
      $object_id,
    );
    $strings = array();
    foreach ($this->get_string_info('properties', array()) as $field => $info) {
      $info = is_array($info) ? $info : array('title' => $info);
      $field_name = isset($info['field']) ? $info['field'] : $field;
      $value = $this->get_field($field_name);
      $strings[$this->get_textgroup()][$string_type][$object_id][$field] = array(
        'string' => is_array($value) || isset($info['empty']) && $value === $info['empty'] ? NULL : $value,
        'title' => $info['title'],
        'format' => isset($info['format']) ? $this->get_field($info['format']) : NULL,
        'name' => array_merge($object_keys, array($field)),
      );
    }
    return $strings;
  }

  /**
   * Get string context
   */
  public function get_string_context() {
    return array($this->get_string_info('type'), $this->get_key());
  }

  /**
   * Get translate path for object
   *
   * @param $langcode
   * 	 Language code if we want ti for a specific language
   */
  public function get_translate_path($langcode = NULL) {
    $replacements = array('%i18n_language' => $langcode ? $langcode : '');
    if ($path = $this->get_string_info('translate path')) {
      return $this->path_replace($path, $replacements);
    }
    elseif ($path = $this->get_info('translate tab')) {
      // If we've got a translate tab path, we just add language to it
      return $this->path_replace($path . '/%i18n_language', $replacements);
    }
  }

  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    return !$this->get_langcode() ? I18N_MODE_LOCALIZE : I18N_MODE_NONE;
  }

  /**
   * Get textgroup name
   */
  public function get_textgroup() {
    return $this->get_string_info('textgroup');
  }

  /**
   * Get textgroup object
   */
  protected function textgroup() {
    if (!isset($this->textgroup)) {
      $this->textgroup = i18n_string_textgroup($this->get_textgroup());
    }
    return $this->textgroup;
  }

  /**
   * Translate object.
   *
   * Translations are cached so it runs only once per language.
   *
   * @return object/array
   *   A clone of the object with its properties translated.
   */
  public function translate($langcode, $options = array()) {
    // We may have it already translated. As objects are statically cached, translations are too.
    if (!isset($this->translations[$langcode])) {
      $this->translations[$langcode] = $this->translate_object($langcode, $options);
    }
    return $this->translations[$langcode];
  }

  /**
   * Translate access (localize strings)
   */
  protected function localize_access() {
    // We could check also whether the object has strings to translate:
    //   && $this->get_strings(array('empty' => TRUE))
    // However it may be better to display the 'No available strings' message
    // for the user to have a clue of what's going on. See i18n_string_translate_page_object()
    return user_access('translate interface') && user_access('translate user-defined strings');
  }

  /**
   * Translate all properties for object.
   *
   * On top of object strings we search for all textgroup:type:objectid:* properties
   *
   * @param $langcode
   *   A clone of the object or array
   */
  protected function translate_object($langcode, $options) {
    // Clone object or array so we don't affect the original one.
    $object = is_object($this->object) ? clone $this->object : $this->object;
    // Get object strings for translatable properties.
    if ($strings = $this->get_strings()) {
      // We preload some of the property translations with a single query.
      if ($context = $this->get_translate_context($langcode, $options)) {
        $found = $this->textgroup()->multiple_translation_search($context, $langcode);
      }
      // Replace all strings in object.
      foreach ($strings as $i18nstring) {
        $this->translate_field($object, $i18nstring, $langcode, $options);
      }
    }
    return $object;
  }

  /**
   * Context to be pre-loaded before translation.
   */
  protected function get_translate_context($langcode, $options) {
    // One-query translation of all textgroup:type:objectid:* properties
    $context = $this->get_string_context();
    $context[] = '*';
    return $context;
  }

  /**
   * Translate object property.
   *
   * Mot often, this is a direct field set, but sometimes fields may have different formats.
   */
  protected function translate_field(&$object, $i18nstring, $langcode, $options) {
    $field_name = $i18nstring->property;
    $translation = $i18nstring->format_translation($langcode, $options);
    if (is_object($object)) {
      $object->$field_name = $translation;
    }
    elseif (is_array($object)) {
      $object[$field_name] = $translation;
    }
  }

  /**
   * Remove all strings for this object.
   */
  public function strings_remove($options = array()) {
    $result = array();
    foreach ($this->load_strings() as $key => $string) {
      $result[$key] = $string->remove($options);
    }
    return _i18n_string_result_count($result);
  }

  /**
   * Update all strings for this object.
   */
  public function strings_update($options = array()) {
    $options += array('empty' => TRUE, 'update' => TRUE);
    $result = array();
    $existing = $this->load_strings();
    // Update object strings
    foreach ($this->get_strings($options) as $key => $string) {
      $result[$key] = $string->update($options);
      unset($existing[$key]);
    }
    // Delete old existing strings.
    foreach ($existing as $key => $string) {
      $result[$key] = $string->remove($options);
    }
    return _i18n_string_result_count($result);
  }

  /**
   * Load all existing strings for this object.
   */
  public function load_strings() {
    list($type, $id) = $this->get_string_context();
    return $this->textgroup()->load_strings(array('type' => $type, 'objectid' => $id));
  }
}


/**
 * Textgroup handler for i18n_string API which integrated persistent caching.
 */
class i18n_string_textgroup_cached extends i18n_string_textgroup_default {

  /**
   * Defines the timeout for the persistent caching.
   * @var int
   */
  public $caching_time = CACHE_TEMPORARY;

  /**
   * Extends the existing constructor with a cache handling.
   *
   * @param string $textgroup
   *   The name of this textgroup.
   */
  public function __construct($textgroup) {
    parent::__construct($textgroup);
    // Fetch persistent caches, the persistent caches contain only metadata.
    // Those metadata are processed by the related cache_get() methods.
    foreach (array('cache_multiple', 'strings') as $caches_type) {
      if (($cache = cache_get('i18n:string:tgroup:' . $this->textgroup . ':' . $caches_type)) && !empty($cache->data)) {
        $this->{$caches_type} = $cache->data;
      }
    }
  }

  /**
   * Class destructor.
   *
   * Updates the persistent caches for the next usage.
   * This function not only stores the data of the textgroup objects but also
   * of the string objects. That way we ensure that only cacheable string object
   * go into the persistent cache.
   */
  public function __destruct() {
    // Reduce size to cache by removing NULL values.
    $this->strings = array_filter($this->strings);


    $strings_to_cache = array();
    // Store the persistent caches. We just store the metadata the translations
    // are stored by the string object itself. However storing the metadata
    // reduces the number of DB queries executed during runtime.
    $cache_data = array();
    foreach ($this->strings as $context => $i18n_string_object) {
      $cache_data[$context] = $context;
      $strings_to_cache[$context] = $i18n_string_object;
    }
    cache_set('i18n:string:tgroup:' . $this->textgroup . ':strings', $cache_data, 'cache', $this->caching_time);

    $cache_data = array();
    foreach ($this->cache_multiple as $pattern => $strings) {
      foreach ($strings as $context => $i18n_string_object) {
        $cache_data[$pattern][$context] = $context;
        $strings_to_cache[$context] = $i18n_string_object;
      }
    }
    cache_set('i18n:string:tgroup:' . $this->textgroup . ':cache_multiple', $cache_data, 'cache', $this->caching_time);

    // Cache the string objects related to this textgroup.
    // Store only the public visible data into the persistent cache.
    foreach ($strings_to_cache as $i18n_string_object) {
      // If this isn't an object it's an unprocessed cache item and doesn't need
      // to be stored again.
      if (is_object($i18n_string_object)) {
        cache_set($i18n_string_object->get_cid(), get_object_vars($i18n_string_object), 'cache', $this->caching_time);
      }
    }
  }

  /**
   * Reset cache, needed for tests.
   *
   * Takes care of the persistent caches.
   */
  public function cache_reset() {
    // Reset the persistent caches.
    cache_clear_all('i18n:string:tgroup:' . $this->textgroup , 'cache', TRUE);
    // Reset the complete string object cache too. This will affect string
    // objects of other textgroups as well.
    cache_clear_all('i18n:string:obj:', 'cache', TRUE);

    return parent::cache_reset();
  }

  /**
   * Get translation from cache.
   *
   * Extends the original handler with persistent caching.
   *
   * @param string $context
   *   The context to look out for.
   * @return i18n_string_object|NULL
   *   The string object if available or NULL otherwise.
   */
  protected function cache_get($context) {
    if (isset($this->strings[$context])) {
      // If the cache contains a string re-build i18n_string_object.
      if (is_string($this->strings[$context])) {
        $i8n_string_object = new i18n_string_object(array('textgroup' => $this->textgroup));
        $i8n_string_object->set_context($context);
        $this->strings[$context] = $i8n_string_object;
      }
      // Now run the original handling.
      return parent::cache_get($context);
    }
    return NULL;
  }

  /**
   * Get strings from multiple cache.
   *
   * @param $context array
   *   String context as array with language property at the end.
   *
   * @return mixed
   *   Array of strings (may be empty) if we've got a cache hit.
   *   Null otherwise.
   */
  protected function multiple_cache_get($context) {
    // Ensure the values from the persistent cache are properly re-build.
    $cache_key = implode(':', $context);
    if (isset($this->cache_multiple[$cache_key])) {
      foreach ($this->cache_multiple[$cache_key] as $cached_context) {
        if (is_string($cached_context)) {
          $i8n_string_object = new i18n_string_object(array('textgroup' => $this->textgroup));
          $i8n_string_object->set_context($cached_context);
          $this->cache_multiple[$cache_key][$cached_context] = $i8n_string_object;
        }
      }
    }
    else {
      // Now we try more generic keys. For instance, if we are searching 'term:1:*'
      // we may try too 'term:*:*' and filter out the results.
      foreach ($context as $key => $value) {
        if ($value != '*') {
          $try = array_merge($context, array($key => '*'));
          return $this->multiple_cache_get($try);
        }
      }
    }
    return parent::multiple_cache_get($context);
  }

  public function string_update($i18nstring, $options = array()) {
    // Flush persistent cache.
    cache_clear_all($i18nstring->get_cid(), 'cache', TRUE);
    return parent::string_update($i18nstring, $options);
  }

  public function string_remove($i18nstring, $options = array()) {
    // Flush persistent cache.
    cache_clear_all($i18nstring->get_cid(), 'cache', TRUE);
    return parent::string_remove($i18nstring, $options);
  }
}
