<?php
/**
 * @file
 * Variable realm controller
 */

/**
 * Realm Controller Interface.
 */
interface VariableRealmControllerInterface {
  /**
   * Class constructor.
   *
   * @param $realm_name
   *   Realm name.
   * @param $store_class
   *   Realm key.
   */
  public function __construct($realm_name);
  /**
   * Check whether the realm is enabled (has a key set).
   */
  public function isEnabled();
  /**
   * Check whether the realm is active (has a valid key set).
   */
  public function isActive();
  /**
   * Start this realm with request key.
   *
   * This will only change the realm key if the realm is not initialized yet.
   */
  public function enable($realm_key = NULL);
  /**
   * Get title for this realm.
   */
  public function getTitle();
  /**
   * Get name for variables in this realm.
   */
  public function getVariableName();
  /**
   * Add store for realm key.
   *
   * @param $variables
   *   Optional array with variable values to initialize the realm key store.
   */
  public function addStore($realm_key, $variables = NULL);
  /**
   * Get store for realm key.
   */
  public function getStore($realm_key);
  /**
   * Set store for realm key.
   */
  public function setStore($realm_key, $realm_store);
  /**
   * Get current realm key.
   */
  public function getKey();
  /**
   * Get default key for this realm.
   */
  public function getDefaultKey();
  /**
   * Get key for this page request.
   */
  public function getRequestKey();
  /**
   * Set current realm key.
   */
  public function setKey($realm_key);
  /**
   * Get all keys for this realm.
   */
  public function getAllKeys();
  /**
   * Get other realms this one may depend upon.
   */
  public function getParentRealms();
  /**
   * Get current realm weight.
   */
  public function getWeight();
  /**
   * Set current realm weight.
   */
  public function setWeight($weight);
  /**
   * Implementation of VariableRealmControllerInterface::getWeight().
   */
  public function getDefaultWeight();
  /**
   * Get current variable store.
   */
  public function getCurrentStore();
  /**
   * List all current variable values.
   */
  public function getCurrentVariables();
  /**
   * Get information for this realm.
   *
   * Return information provided by hook_variable_realm_info()
   */
  public function getInfo($property = NULL, $default = NULL);
  /**
   * Get list of variables available for this realm.
   */
  public function getAvailableVariables();
  /**
   * Get list of variables enabled for this realm.
   */
  public function getEnabledVariables();
  /**
   * Get system variable for this realm.
   */
  public function getRealmVariable($name, $default = NULL);
  /**
   * Set system variable for this realm.
   */
  public function setRealmVariable($name, $value);
  /**
   * Delete variable for all keys this realm.
   *
   * @param $variable_name
   *   Variable name to delete.
   */
  public function deleteVariable($variable_name);
}

/**
 * Variable Realm Hooks.
 */
interface VariableRealmHooks {
  /**
   * Callback function that will be invoked when a realm is enabled.
   */
  public function variableRealmEnable($realm_name, $realm_key);
  /**
   * Callback method that will be invoked when a realm is switched.
   */
  public function variableRealmSwitch($realm_name, $realm_key);
}

/**
 * Realm Store Interface.
 */
interface VariableRealmStoreInterface {
  /**
   * Class constructor.
   *
   * @param $realm
   *   Realm name.
   * @param $key
   *   Realm key.
   * @param $variables
   *   Optional array of variables to initialize the realm with.
   */
  public function __construct($realm, $key, $variables = NULL);
  /**
   * Initialize variables.
   */
  public function variable_init();
  /**
   * Get single variable.
   *
   * @param $name
   *   Variable name
   * @param $default
   *   Default value
   */
  public function variable_get($name, $default = NULL);
  /**
   * Set single variable.
   *
   * @param $name
   *   Variable name
   * @param $value
   *   Variable value
   */
  public function variable_set($name, $value);
  /**
   * Delete single variable.
   *
   * @param $name
   *   Variable name
   */
  public function variable_del($name);
  /**
   * Add single variable to the realm for current request.
   *
   * While variable set actually sets the variable on whatever realm storage
   * we are using, this function just sets a runtime value.
   *
   * @param $name
   *   Variable name
   * @param $value
   *   Variable value
   */
  public function variable_add($name, $value);
  /**
   * Check whether a variable exists in the realm.
   */
  public function variable_exists($name);
  /**
   * List all current variable values.
   */
  public function variable_list();
}

/**
 * Base class, keeps static list of variables.
 */
class VariableRealmDefaultController implements VariableRealmControllerInterface {
  // Unique realm name (language, domain..)
  public $realm_name;
  // Current realm key.
  public $current_key;
  // Current realm weight.
  public $current_weight;
  // Array of variable stores indexed by realm key.
  protected $store;
  /**
   * Implementation of VariableRealmControllerInterface::__construct().
   */
  public function __construct($realm_name) {
    $this->realm_name = $realm_name;
    $this->current_weight = $this->getDefaultWeight();
  }
  /**
   * Implementation of VariableRealmControllerInterface::isEnabled()
   */
  public function isEnabled() {
    return isset($this->current_key);
  }
  /**
   * Implementation of VariableRealmControllerInterface::isActive()
   */
  public function isActive() {
    return $this->isEnabled() && $this->current_key !== FALSE;
  }
  /**
   * Implementation of VariableRealmControllerInterface::enable().
   */
  public function enable($realm_key = NULL) {
    if (!isset($this->current_key)) {
      return $this->current_key = isset($realm_key) ? $realm_key : $this->getRequestKey();
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getTitle().
   */
  public function getTitle() {
    return $this->getInfo('title');
  }
  /**
   * Implementation of VariableRealmControllerInterface::getVariableName().
   */
  public function getVariableName() {
    return $this->getInfo('variable name');
  }
  /**
   * Implementation of VariableRealmControllerInterface::getStore().
   */
  public function getStore($realm_key) {
    if (isset($this->store[$realm_key])) {
      return $this->store[$realm_key];
    }
    else {
      return $this->addStore($realm_key);
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::addStore().
   */
  public function addStore($realm_key, $variables = NULL) {
    $store = $this->createStore($realm_key, $variables);
    $this->setStore($realm_key, $store);
    return $store;
  }
  /**
   * Create Store for key.
   */
  protected function createStore($realm_key, $variables = NULL) {
    $class = $this->getInfo('store class');
    $class = $class && class_exists($class) ? $class : 'VariableRealmDefaultStore';
    return new $class($this->realm_name, $realm_key, $variables);
  }
  /**
   * Set store for realm key.
   */
  public function setStore($realm_key, $realm_store) {
    $this->store[$realm_key] = $realm_store;
  }
  /**
   * Implementation of VariableRealmControllerInterface::setKey().
   */
  public function setKey($realm_key) {
    $this->current_key = $realm_key;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getKey().
   */
  public function getKey() {
    return isset($this->current_key) ? $this->current_key : FALSE;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getAllKeys().
   */
  public function getAllKeys() {
    return $this->getInfo('keys', array());
  }
  /**
   * Implementation of VariableRealmControllerInterface::getDefaultKey().
   */
  public function getDefaultKey() {
    return $this->getInfo('default key', FALSE);
  }
  /**
   * Implementation of VariableRealmControllerInterface::getRequestKey().
   */
  public function getRequestKey() {
    return FALSE;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getWeight().
   */
  public function getWeight() {
    return isset($this->current_weight) ? $this->current_weight : $this->controller_data['weight'];
  }
  /**
   * Implementation of VariableRealmControllerInterface::setWeight().
   */
  public function setWeight($weight) {
    $this->current_weight = $weight;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getWeight().
   */
  public function getDefaultWeight() {
    return $this->getRealmVariable('weight', $this->getInfo('weight', 0));
  }
  /**
   * Implementation of VariableRealmControllerInterface::getParentRealms().
   */
  public function getParentRealms() {
    return array();
  }
  /**
   * Implementation of VariableRealmControllerInterface::getCurrentVariables().
   */
  public function getCurrentVariables() {
    if ($store = $this->getCurrentStore()) {
      return $store->variable_list();
    }
    else {
      return array();
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getCurrentStore().
   */
  public function getCurrentStore() {
    if ($this->isActive()) {
      return $this->getStore($this->getKey());
    }
    else {
      return NULL;
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getAvailableVariables().
   */
  public function getAvailableVariables() {
    if ($options = $this->getInfo('options')) {
      return $options;
    }
    else {
      // Defaults to all available variables.
      return array_keys(variable_get_info());
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getEnabledVariables().
   */
  public function getEnabledVariables() {
    if ($this->getInfo('select')) {
      return $this->getRealmVariable('list', array());
    }
    else {
      // If the variable is not set it will default to all variables
      return $this->getAvailableVariables();
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getInfo().
   */
  public function getInfo($property = NULL, $default = NULL) {
    $info = variable_realm_info($this->realm_name);
    if ($property) {
      return isset($info[$property]) ? $info[$property] : $default;
    }
    else {
      return $info;
    }
  }
  /**
   * Implementation of VariableRealmControllerInterface::getRealmVariable().
   */
  public function getRealmVariable($name, $default = NULL) {
    return variable_get('variable_realm_' . $name . '_' . $this->realm_name, $default);
  }
  /**
   * Implementation of VariableRealmControllerInterface::setRealmVariable().
   */
  public function setRealmVariable($name, $value) {
    variable_realm_global_set('variable_realm_' . $name . '_' . $this->realm_name, $value);
  }
  /**
   * Implementation of VariableRealmControllerInterface::deleteVariable().
   */
  public function deleteVariable($variable_name) {
    foreach ($this->getAllKeys() as $key => $name) {
      variable_realm_del($this->realm_name, $key, $variable_name, FALSE);
    }
  }
}

/**
 * Base class, keeps static list of variables.
 */
class VariableRealmDefaultStore implements VariableRealmStoreInterface {
  public $realm;
  public $key;
  protected $variables;
  /**
   * Class constructor.
   */
  public function __construct($realm, $key, $variables = NULL) {
    $this->realm = $realm;
    $this->key = $key;
    $this->variables = $variables;
  }
  /**
   * Initialize variables.
   */
  public function variable_init() {
    if (!isset($this->variables)) {
      $this->variables = array();
    }
  }
  /**
   * Get single variable.
   *
   * @param $name
   *   Variable name
   * @param $default
   *   Default value
   */
  public function variable_get($name, $default = NULL) {
    $this->variable_init();
    return isset($this->variables[$name]) ? $this->variables[$name] : $default;
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
    $this->variable_init();
    $this->variables[$name] = $value;
  }
  /**
   * Delete single variable.
   *
   * @param $name
   *   Variable name
   */
  public function variable_del($name) {
    $this->variable_init();
    unset($this->variables[$name]);
  }
  /**
   * Implementation of VariableRealmStoreInterface::variable_add().
   */
  public function variable_add($name, $value) {
    $this->variable_init();
    $this->variables[$name] = $value;
  }
  /**
   * Implementation of VariableRealmStoreInterface::variable_exists().
   */
  public function variable_exists($name) {
    $this->variable_init();
    return isset($this->variables[$name]);
  }
  /**
   * List all current variable values.
   */
  public function variable_list() {
    $this->variable_init();
    return $this->variables;
  }
}

/**
 * Controller for default system variables.
 */
class VariableRealmGlobalStore extends VariableRealmDefaultStore {
  /**
   * Initialize variables.
   */
  public function variable_init() {
    if (!isset($this->variables)) {
      $this->variables = $GLOBALS['conf'];
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
    parent::variable_set($name, $value);
    variable_set($name, $value);
  }
  /**
   * Delete single variable.
   *
   * @param $name
   *   Variable name
   */
  public function variable_del($name) {
    parent::variable_del($name);
    variable_del($name);
  }
}
