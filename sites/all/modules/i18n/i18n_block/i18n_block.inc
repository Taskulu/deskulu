<?php
/**
 * Blocks textgroup handler
 */

/**
 * Block object
 */
class i18n_block_object extends i18n_string_object_wrapper {

  /**
   * Load a block object.
   *
   * @param $object
   *  An array with module and delta.
   */
  function load_object($object) {
    $this->object = call_user_func_array($this->get_info('load callback'), $object);
    return $this->get_object();
  }

  /**
   * Get base keys for translating this object
   */
  public function get_string_context() {
    return array($this->object->module, $this->object->delta);
  }

  /**
   * Get object strings for translation
   */
  protected function build_properties() {
    if ($this->object->module == 'block' && !isset($this->object->body)) {
      $block = (object) block_custom_block_get($this->object->delta);
      $this->object->body = $block->body;
      $this->object->format = $block->format;
    }
    $properties = parent::build_properties();
    // Body is available only for custom blocks.
    if ($this->object->module != 'block') {
      unset($properties[$this->get_textgroup()][$this->object->module][$this->object->delta]['body']);
    }
    return $properties;
  }

  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    return !empty($this->object->i18n_mode) ? I18N_MODE_LOCALIZE : I18N_MODE_NONE;
  }
}
