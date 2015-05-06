<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_path_translation_set extends i18n_translation_set {
  /**
   * Add translation item
   */
  public function add_item($path, $langcode = NULL) {
    // Path may be object or plain string
    $item = is_object($path) ? $path : (object)array('path' => $path, 'language' => $langcode);
    return parent::add_item($item, $langcode);
  }
  /**
   * Clean path translations.
   *
   * Unlike other translation sets this actually deletes paths
   */
  public function clean_translations() {
    $delete = db_delete('i18n_path')
      ->condition('tsid', $this->tsid)
      ->condition('language', array_keys($this->get_translations()), 'NOT IN')
      ->execute();
  }
  /**
   * Delete translation set
   */
  public function delete_translations() {
    return db_delete('i18n_path')
      ->condition('tsid', $this->tsid)
      ->execute();
  }
  /**
   * Save all path translations
   */
  public function save_translations() {
    foreach ($this->get_translations() as $lang => $path) {
      $path = is_object($path) ? $path : (object) array('path' => $path, 'language' => $lang, 'tsid' => $this->tsid);
      drupal_write_record('i18n_path', $path, !empty($path->tpid) ? 'tpid' : array());
      $this->add_item($path, $lang);
    }
  }

}

/**
 * Path object
 */
class i18n_path_object extends i18n_object_wrapper {
  /**
   * Get title from item
   */
  public function get_title() {
    return $this->object->path;
  }

  /**
   * Get path for item
   */
  public function get_path() {
    return check_url($this->object->path);
  }

  /**
   * Get translate mode
   */
  public function get_translate_mode() {
    return I18N_MODE_TRANSLATE;
  }

}