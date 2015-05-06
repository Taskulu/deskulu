<?php
/**
 * @file
 * Variable realm controller
 */

class VariableStoreRealmStore extends VariableRealmDefaultStore {
  /**
   * Initialize realm.
   */
  public function variable_init() {
    if (!isset($this->variables)) {
      $this->variables = &variable_store($this->realm, $this->key);
    }
  }
  /**
   * Set single variable.
   *
   * @param $name
   *   Variable name
   * @param $value
   *   Variable value
   */
  public function variable_set($name, $value) {
    // Since $variables is a reference we just need to set the store value.
    variable_store_set($this->realm, $this->key, $name, $value);
  }
  /**
   * Delete single variable.
   *
   * @param $name
   *   Variable name
   */
  public function variable_del($name) {
    // Since $variables is a reference we just need to delete the store value.
    variable_store_del($this->realm, $this->key, $name);
  }
  /**
   * Implements 'magic' _sleep method.
   *
   * If serialized, variables should not be saved, but rebuilt from store on wake up.
   */
  public function __sleep(){
    return array('realm', 'key');
  }
}
