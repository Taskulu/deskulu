<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_translation_set {
  public $tsid = NULL;
  public $type;
  public $bundle = '';
  public $status = 0;
  public $master_id = 0;
  // It may optionally have a title
  public $title = '';
  // Translations indexed by language
  protected $translations = NULL;
  // Translation languages indexed by oid
  protected $object_languages = array();
  // Related translation sets indexed by tsid
  // Keep track of old translation sets objects belong to.
  protected $related_translations = array();
  /**
   * Constructor from object/array
   */
  public function __construct($translation = NULL) {
    if ($translation) {
      foreach ((array)$translation as $key => $value) {
        $this->$key = $value;
      }
    }
  }

  /**
   * Delete a translation set
   * 
   * @param $delete_translations
   *   Whether to unlink translations from the set. Detaults to TRUE.
   */
  public function delete($delete_translations = TRUE) {
    db_delete('i18n_translation_set')
      ->condition('tsid', $this->tsid)
      ->execute();
    if ($delete_translations) {
      $this->delete_translations();
    }
    $this->invoke_all('delete');
    $this->tsid = NULL;
  }

  /**
   * Invoke all modules
   */
  public function invoke_all($op) {
    module_invoke_all('i18n_translation_set_' . $op, $this);
    module_invoke_all('entity_' . $op, $this, 'i18n_translation');
  }

  /**
   * Create a new translation set
   * 
   * @param $save_translations
   *   Whether to update linked objects so they belong to this set.
   */
  public function insert($save_translations = TRUE) {
    $this->created = $this->changed = REQUEST_TIME;
    $status = drupal_write_record('i18n_translation_set', $this);
    if ($save_translations) {
      $this->save_translations();
      $this->update_related();
    }
    $this->invoke_all('insert');
    return $status;
  }

  /**
   * Save translation set
   * 
   * @param $save_translations
   *   Whether to update linked objects so they belong to this set.
   */
  public function save($save_translations = TRUE) {
    $this->invoke_all('presave');
    return empty($this->tsid) ? $this->insert($save_translations) : $this->update($save_translations);
  }

  /**
   * Update a translation set
   * 
   * @param $update_translations
   *   Whether to update objects linked to this set.
   */
  public function update($update_translations = TRUE) {
    $this->changed = REQUEST_TIME;
    $status = drupal_write_record('i18n_translation_set', $this, 'tsid');
    if ($update_translations) {
      $this->clean_translations();
      $this->save_translations();
      $this->update_related();
    }
    $this->invoke_all('update');
    return $status;
  }

  /**
   * Update a translation set or delete if empty.
   */
  public function update_delete() {
    if ($this->get_translations()) {
      $result = $this->save(TRUE);
      // Update related translation sets too.
      $this->update_related();
      return $result;
    }
    else {
      return $this->delete(TRUE);
    }
  }

  /**
   * Update related translation sets
   */
  protected function update_related($op = 'update_delete') {
    foreach ($this->related_translations as $translation_set) {
      $translation_set->$op();
    }    
  }
  
  /**
   * Clean all items in this translation set
   *
   * Unlink other items (not current translations from this translation set)
   */
  public function clean_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      $query = db_update($table)
        ->fields(array($field => 0))
        ->condition($field, $this->tsid);
      if ($translations = $this->get_translations()) {
        $query->condition('language', array_keys($translations), 'NOT IN');
      }
      return $query->execute();
    }
  }

  /**
   * Save translations in this translation set
   */
  public function save_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      if ($keys = $this->get_translations_keys()) {
        return db_update($table)
          ->fields(array($field => $this->tsid))
          ->condition($this->key_field(), $keys)
          ->execute();
      }
      else {
        return $this->delete_translations();
      }
    }
  }

  /**
   * Delete translations in this translation set
   *
   * It won't delete objects, just unlink them from translation set
   */
  public function delete_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      return db_update($table)
        ->fields(array($field => 0))
        ->condition($field, $this->tsid)
        ->execute();
    }
  }

  /**
   * Get translations, indexed by language
   */
  public function get_translations() {
    $translations = array();
    foreach ($this->get_objects() as $lang => $object) {
      $translations[$lang] = $object->get_object();
    }
    return $translations;
  }
  /**
   * Reset translations, set empty array or new array of translations.
   * 
   * @param $translations array
   *   Array of langcode => item
   */
  public function reset_translations($translations = array()) {
    $this->translations = array();
    $this->add_translations($translations);
    return $this;
  }
  /**
   * Get translations as i18n objects, indexed by language
   */
  public function get_objects() {
    if (!isset($this->translations)) {
      $this->translations = array();
      // Disable selection query altering, just in case
      $previous = i18n_select(FALSE);
      $this->add_translations($this->load_translations());
      i18n_select($previous);
    }
    return $this->translations;
  }

  /**
   * Get item for language
   */
  public function get_item($langcode) {
    if (($translations = $this->get_translations()) && isset($translations[$langcode])) {
      return $translations[$langcode];
    }
    else {
      return NULL;
    }
  }
  /**
   * Get translations keys, indexed by language
   */
  public function get_translations_keys() {
    $keys = array();
    foreach ($this->get_objects() as $lang => $object) {
      if ($id = $object->get_key()) {
        $keys[$lang] = $id;
      }
    }
    return array_filter($keys);
  }
  /**
   * Get edit path for this translation set
   */
  public function get_edit_path() {
    if ($path = $this->get_info('edit path')) {
      return strtr($path, $this->get_path_placeholders('delete'));
    }
    else {
      return '';
    }
  }
  /**
   * Get operations as renderable links
   */
  public function get_operations() {
    $destination = drupal_get_destination();
    $operations = array();
    if ($path = $this->get_edit_path()) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => $path,
        'query' => $destination,
      );
    }
    if ($path = $this->get_delete_path()) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => $path,
        'query' => $destination,
      );
    }
    return $operations;  
  }
  /**
   * Get items as renderable links
   */
  public function get_links() {
    $language_list = language_list();
    $items = array();
    foreach ($this->get_objects() as $langcode => $object) {
      $title = $object->get_title();
      $path = $object->get_path();
      $language = isset($language_list[$langcode]) ? $language_list[$langcode] : NULL;
      $items[$langcode] = array(
        'title' => $title,
        'href' => $path ? $path : NULL,
        'language' => $language,
      );
      if ($language && function_exists('languageicons_link_add')) {
        languageicons_link_add($items[$langcode]);
      }
    }
    return $items;    
  }
  /**
   * Get overview (list) path for this translation set
   */
  public function get_list_path() {
    return $this->get_info('list path');
  }
  /**
   * Get delete path for this translation set
   */
  public function get_delete_path() {
    if ($path = $this->get_info('delete path')) {
      return strtr($path, $this->get_path_placeholders('delete'));
    }
    else {
      return '';
    }
  }
  /**
   * Get placeholder values for path replacement
   */
  function get_path_placeholders($op = 'list') {
    $values['%operation'] = $op;
    $values['%type'] = $this->type;
    $values['%i18n_translation_set'] = $this->tsid;
    if ($placeholder = $this->get_info('placeholder')) {
      $values[$placeholder] = $this->tsid;
    }
    return $values;
  }
  
  /**
   * Get field on translations table that stores the translation set id (tsid)
   */
  protected function get_field() {
    return $this->get_info('field');
  }

  /**
   * Get property from translation set info
   */
  public function get_info($property, $default = NULL) {
    $info = i18n_translation_set_info($this->type);
    return $info && isset($info[$property]) ? $info[$property] : $default;
  }

  /**
   * Get table name, where translation items are stored.
   */
  protected function get_table() {
    return $this->get_info('table');
  }

  /**
   * Get title for this set
   */
  public function get_title() {
    if (!empty($this->title)) {
      return $this->title;
    }
    elseif ($translations = $this->get_objects()) {
      foreach ($translations as $object) {
        $names[] = $object->get_title();
      }
      return implode(' / ', $names);
    }
    else {
      return t('Undefined');
    }
  }

  /**
   * Get item list
   */
  public function item_list() {
    $language_list = language_list();
    $items = array();
    foreach ($this->get_objects() as $langcode => $object) {
      $title = $object->get_title();
      $path = $object->get_path();
      if ($title && $path) {
        $options = isset($language_list[$langcode]) ? array('language' => $language_list[$langcode]) : array();
        $items[$langcode] = l($title, $path, $options);
      }
      elseif ($title) {
        $items[$langcode] = check_plain($title);
      }
    }
    return $items;
  }

  /**
   * Add array of translation items
   * 
   * @param $translations array
   *   Translation items indexed by language code
   */
  public function add_translations($translations) {
    foreach ($translations as $langcode => $item) {
      $this->add_item($item, $langcode);
    }
    return $this;
  }

  /**
   * Add translation item
   */
  public function add_item($item, $langcode = NULL) {
    $object = i18n_object($this->type, $item);
    $langcode = $langcode ? $langcode : $object->get_langcode();
    // Check whether this item belongs to another translation set
    $old_tsid = $object->get_tsid();
    if ($old_tsid && $old_tsid != $this->tsid) {
      $this->related_translations[$old_tsid] = i18n_translation_set_load($old_tsid);
      $this->related_translations[$old_tsid]->remove_object($object);
    }    
    if ($langcode) {
      $this->get_translations();
      $object->set_tsid($this->tsid);
      $this->translations[$langcode] = $object;
      $this->object_languages[$object->get_index()] = $langcode;
    }
    return $this;
  }

  /**
   * Remove item / language from translation set
   * 
   * @param $item
   *   Item to remove from this translation set, it must have a language property.
   */
  public function remove_item($item) {
    $this->remove_object(i18n_object($this->type, $item));
    return $this;
  }
  
  /**
   * Remove i18n object from translation set.
   */
  public function remove_object($object) {
    // As object's language may have changed, we use our object_languages index better.
    $index = $object->get_index();
    $this->get_translations();
    if (isset($this->object_languages[$index])) {
      $langcode = $this->object_languages[$index];
      unset($this->translations[$langcode]);
      unset($this->object_languages[$index]);
    }
    return $this;
  }

  /**
   * Remove language from translation set.
   * 
   * @param $langcode
   *   Language to remove from this translation set.
   */
  public function remove_language($langcode) {
    $this->get_translations();
    if (isset($this->translations[$langcode])) {
      $this->remove_object($this->translations[$langcode]);
    }
    return $this;   
  }

  /**
   * Load all translations as objects indexed by language
   */
  public function load_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      return db_select($table, 't')
        ->fields('t')
        ->condition('t.' . $field, $this->tsid)
        ->execute()
        ->fetchAllAssoc('language');
    }
    else {
      return array();
    }
  }

  /**
   * Get key field for this translation items
   */
  protected function key_field() {
    $info = i18n_object_info($this->type);
    return $info['key'];
  }
}