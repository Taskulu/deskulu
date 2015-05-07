<?php

class i18nviews extends i18n_string_object_wrapper {
  /**
   * Get string context
   */
  public function get_string_context() {
    return array('views', $this->object->name);
  }

  /**
   * Get translatable properties
   */
  protected function build_properties() {
    $strings = parent::build_properties();
    $properties = array();
    $this->object->init_display();
    foreach ($this->object->display as $display_id => $display) {
      $translatables = array();
      $display->handler->unpack_translatables($translatables);
      foreach ($translatables as $translatable) {
        $key = implode(':', $translatable['keys']);
        $properties[$display_id][$key] = array(
          'title' => $display_id . ' ' . $key,
          'string' => $translatable['value'],
        );
      }
    }
    $strings[$this->get_textgroup()][$this->object->name] = $properties;
    return $strings;
  }

  /**
   * Get edit path for oject.
   */
  public function get_edit_path() {
    return 'admin/structure/views/view/' . $this->object->name .'/edit';
  }

  /**
   * Get translate path for object
   */
  public function get_translate_path($langcode = NULL) {
    return 'admin/structure/views/view/' . $this->object->name .'/translate' . ($langcode ? '/' . $langcode : '');
  }
}
