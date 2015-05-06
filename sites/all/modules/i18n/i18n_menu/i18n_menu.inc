<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_menu_link_translation_set extends i18n_translation_set {
  /**
   * Load all path translations
   */
  public function load_translations() {
    $translations = array();
    $query = db_select('menu_links', 'ml');
    $query->leftJoin('menu_router', 'm', 'm.path = ml.router_path');
    $query->fields('ml');
    // Weight should be taken from {menu_links}, not {menu_router}.
    $query->addField('ml', 'weight', 'link_weight');
    $query->fields('m');
    $query->condition('ml.i18n_tsid', $this->tsid);
    foreach ($query->execute()->fetchAll(PDO::FETCH_ASSOC) as $item) {
      $item['weight'] = $item['link_weight'];
      _menu_link_translate($item);
      $translations[$item['language']] = $item;
    }
    return $translations;
  }
}

/**
 * Menu link object
 */
class i18n_menu_link extends i18n_string_object_wrapper {
  /**
   * Class constructor
   */
  public function __construct($type, $key, $object) {
    // Unserialize options if not done
    if (isset($object['options']) && !is_array($object['options'])) {
      $object['options'] = unserialize($object['options']);
    }
    parent::__construct($type, $key, $object);
  }

  /**
   * Get path for item
   */
  public function get_path() {
    return $this->object['link_path'];
  }

  /**
   * Get title from item
   */
  public function get_title() {
    return $this->object['title'];
  }

  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    $mode = i18n_menu_mode($this->object['menu_name']);
    if ($this->get_langcode()) {
      return $mode & I18N_MODE_TRANSLATE;
    }
    elseif (!empty($this->object['customized'])) {
      return $mode & I18N_MODE_LOCALIZE;
    }
    else {
      return I18N_MODE_NONE;
    }
  }

  /**
   * Access to object translation. This should check object properties and permissions
   */
  protected function translate_access() {
    return user_access('administer menu') && user_access('translate interface');
  }

  /**
   * Get translatable properties.
   *
   * Check whether title or description are to be translated by default menu
   * system.
   */
  protected function build_properties() {
    $properties = parent::build_properties();
    if ($properties) {
      $strings = &$properties['menu']['item'][$this->get_key()];
      $localizable = _i18n_menu_link_localizable_properties($this->object);
      foreach ($strings as $key => $data) {
        if (!in_array($key, $localizable)) {
          unset($strings[$key]);
        }
      }
    }
    return $properties;
  }
}
