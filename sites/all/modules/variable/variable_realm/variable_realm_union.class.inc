<?php
/**
 * @file
 * Classes for Realm Union.
 */

/**
 * Default Realm Union class
 */
class VariableRealmUnionController extends VariableRealmDefaultController implements VariableRealmHooks {
  protected $union_realms;
  /**
   * Implementation of VariableRealmControllerInterface::__construct().
   */
  public function __construct($realm_name) {
    parent::__construct($realm_name);
    // Get / create realm controllers for each of the union realms.
    foreach ($this->getParentRealms() as $realm_name) {
      $this->union_realms[$realm_name] = variable_realm_controller($realm_name);
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::start().
   */
  public function start($realm_key = NULL) {
    if (!isset($this->current_key)) {
      if (isset($realm_key)) {
        return $this->current_key = $realm_key;
      }
      elseif ($request_key = $this->getRequestKey()) {
        return $this->current_key = $request_key;
      }
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getAllKeys().
   */
  public function getAllKeys() {
    $all_realm_keys = $this->invokeUnionRealms('getAllKeys');
    // First build all combinations of realm keys.
    $combine = array(array('keys' => array(), 'names' => array()));
    foreach ($all_realm_keys as $realm => $realm_keys) {
      $new_combine = array();
      foreach ($combine as $index => $data) {
        foreach ($realm_keys as $new_key => $new_name) {
          $keys = $data['keys'];
          $names = $data['names'];
          $keys[$realm] = $new_key;
          $names[$realm] = $new_name;
          $new_combine[] = array('keys' => $keys, 'names' => $names);
        }
      }
      $combine = $new_combine;
    }
    // Now build all realm keys for the combinations.
    $keys = array();
    foreach ($combine as $data) {
      $key = $this->buildUnionKey($data['keys']);
      $name = $this->buildUnionName($data['names']);
      $keys[$key] = $name;
    }
    return $keys;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getDefaultKey().
   */
  public function getDefaultKey() {
    $keys = $this->invokeUnionRealms('getDefaultKey');
    return $this->buildUnionKey($keys);
  }
  /**
   * Implementation of VariableRealmControllerInterface::getRequestKey().
   */
  public function getRequestKey() {
    // We'll have a request key if union realms have a current key.
    $keys = $this->invokeUnionRealms('getKey');
    return $this->buildUnionKey($keys);
  }
  /**
   * Implementation of VariableRealmControllerInterface::getAvailableVariables().
   */
  public function getAvailableVariables() {
    $variables = $this->invokeUnionRealms('getAvailableVariables');
    return call_user_func_array('array_intersect', $variables);
  }
  /**
   * Implementation of VariableRealmControllerInterface::getEnabledVariables().
   */
  public function getEnabledVariables() {
    $variables = $this->invokeUnionRealms('getEnabledVariables');
    return call_user_func_array('array_intersect', $variables);
  }
  /**
   * Implementation of VariableRealmControllerInterface::getParentRealms().
   */
  public function getParentRealms() {
    return $this->getInfo('union', array());
  }
  /**
   * Get union realms controllers.
   */
  protected function getUnionRealms() {
    return $this->union_realms;
  }
  /**
   * Implementation of VariableRealmHooks::variableRealmEnable()
   */
  public function variableRealmEnable($realm_name, $realm_key) {
    // If this realm is enabled but not active, try to find a key.
    if ($this->isEnabled() && !$this->isActive() && $this->isUnionRealm($realm_name) && $union_key = $this->getRequestKey()) {
      $this->setKey($union_key);
    }
  }
  /**
   * Implementation of VariableRealmHooks::variableRealmSwitch()
   */
  public function variableRealmSwitch($realm_name, $realm_key) {
    // If the this realm is active, try to find new key.
    if ($this->isActive() && $this->isUnionRealm($realm_name) && ($union_key = $this->getRequestKey())) {
      $this->setKey($union_key);
    }
  }
  /**
   * Check whether a realm belongs to the union realms.
   */
  protected function isUnionRealm($realm_name) {
    return isset($this->union_realms[$realm_name]);
  }
  /**
   * Invoke function on all realms.
   */
  protected function invokeUnionRealms($method) {
    return _variable_realm_invoke($this->getUnionRealms(), $method);
  }
  /**
   * Build key from union realm keys.
   */
  protected static function buildUnionKey($keys) {
    if (in_array(FALSE, $keys, TRUE)) {
      return FALSE;
    }
    else {
      // Make sure values are in correct order
      ksort($keys);
      // implode values
      return implode(':', $keys);
    }
  }
  /**
   * Build key name from union realm key names.
   */
  protected static function buildUnionName($names) {
    return implode(', ', $names);
  }
}
