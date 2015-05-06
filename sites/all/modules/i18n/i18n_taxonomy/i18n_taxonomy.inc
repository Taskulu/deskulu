<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_taxonomy_translation_set extends i18n_translation_set {
  /**
   * Load all term translations
   */
  public function load_translations() {
    return i18n_translation_set_index(taxonomy_term_load_multiple(array(), array('i18n_tsid' => $this->tsid)));
  }

  /**
   * Get placeholder values for path replacement
   */
  function get_path_placeholders($op = 'list') {
    $values = parent::get_path_placeholders($op);
    if (!empty($this->bundle)) {
      $values['%taxonomy_vocabulary_machine_name'] = $this->bundle;
    }
    return $values;
  }
}

/**
 * Taxonomy textgroup handler
 */
class i18n_taxonomy_term extends i18n_string_object_wrapper {
  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    $mode = i18n_taxonomy_vocabulary_mode($this->object->vid);
    if ($this->get_langcode()) {
      return $mode & I18N_MODE_TRANSLATE;
    }
    else {
      return $mode & I18N_MODE_LOCALIZE;
    }
  }
  /**
   * Access to object translation. This should check object properties and permissions
   */
  protected function translate_access() {
    return taxonomy_term_edit_access($this->object) && $this->get_langcode() && user_access('translate interface');
  }
}
