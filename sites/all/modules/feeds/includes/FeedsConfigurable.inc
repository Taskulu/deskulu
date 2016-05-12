<?php

/**
 * @file
 * FeedsConfigurable and helper functions.
 */

/**
 * Used when an object does not exist in the DB or code but should.
 */
class FeedsNotExistingException extends Exception {
}

/**
 * Base class for configurable classes. Captures configuration handling, form
 * handling and distinguishes between in-memory configuration and persistent
 * configuration.
 *
 * Due to the magic method __get(), protected properties from this class and
 * from classes that extend this class will be publicly readable (but not
 * writeable).
 */
abstract class FeedsConfigurable {

  // Holds the actual configuration information.
  protected $config;

  // A unique identifier for the configuration.
  protected $id;

  /*
  CTools export type of this object.

  @todo Should live in FeedsImporter. Not all child classes
  of FeedsConfigurable are exportable. Same goes for $disabled.

  Export type can be one of
  FEEDS_EXPORT_NONE - the configurable only exists in memory
  EXPORT_IN_DATABASE - the configurable is defined in the database.
  EXPORT_IN_CODE - the configurable is defined in code.
  EXPORT_IN_CODE | EXPORT_IN_DATABASE - the configurable is defined in code, but
                                        overridden in the database.*/
  protected $export_type;

  /**
   * CTools export enabled status of this object.
   */
  protected $disabled;

  /**
   * Instantiates a FeedsConfigurable object.
   *
   * Don't use directly, use feeds_importer() or feeds_plugin() instead.
   *
   * @see feeds_importer()
   * @see feeds_plugin()
   */
  public static function instance($class, $id) {
    if (!strlen($id)) {
      throw new InvalidArgumentException(t('Empty configuration identifier.'));
    }

    $instances = &drupal_static(__METHOD__, array());

    if (!isset($instances[$class][$id])) {
      $instances[$class][$id] = new $class($id);
    }
    return $instances[$class][$id];
  }

  /**
   * Constructor, set id and load default configuration.
   */
  protected function __construct($id) {
    // Set this object's id.
    $this->id = $id;
    // Per default we assume that a Feeds object is not saved to
    // database nor is it exported to code.
    $this->export_type = FEEDS_EXPORT_NONE;
    // Make sure configuration is populated.
    $this->config = $this->configDefaults();
    $this->disabled = FALSE;
  }

  /**
   * Override magic method __isset(). This is needed due to overriding __get().
   */
  public function __isset($name) {
    return isset($this->$name) ? TRUE : FALSE;
  }

  /**
   * Determine whether this object is persistent and enabled. I. e. it is
   * defined either in code or in the database and it is enabled.
   */
  public function existing() {
    if ($this->export_type == FEEDS_EXPORT_NONE) {
      throw new FeedsNotExistingException(t('Object is not persistent.'));
    }
    if ($this->disabled) {
      throw new FeedsNotExistingException(t('Object is disabled.'));
    }
    return $this;
  }

  /**
   * Save a configuration. Concrete extending classes must implement a save
   * operation.
   */
  public abstract function save();

  /**
   * Copy a configuration.
   */
  public function copy(FeedsConfigurable $configurable) {
    $this->setConfig($configurable->config);
  }

  /**
   * Set configuration.
   *
   * @param $config
   *   Array containing configuration information. Config array will be filtered
   *   by the keys returned by configDefaults() and populated with default
   *   values that are not included in $config.
   */
  public function setConfig($config) {
    $defaults = $this->configDefaults();
    $this->config = array_intersect_key($config, $defaults) + $defaults;
  }

  /**
   * Similar to setConfig but adds to existing configuration.
   *
   * @param $config
   *   Array containing configuration information. Will be filtered by the keys
   *   returned by configDefaults().
   */
  public function addConfig($config) {
    $this->config = is_array($this->config) ? array_merge($this->config, $config) : $config;
    $default_keys = $this->configDefaults();
    $this->config = array_intersect_key($this->config, $default_keys);
  }

  /**
   * Overrides magic method __get().
   *
   * - Makes sure that external reads of FeedsConfigurable::config go through
   *   ::getConfig();
   * - Makes private and protected properties from this class and protected
   *   properties from child classes publicly readable.
   * - Prevents warnings when accessing non-existent properties.
   */
  public function __get($name) {
    if ($name == 'config') {
      return $this->getConfig();
    }
    return isset($this->$name) ? $this->$name : NULL;
  }

  /**
   * Implements getConfig().
   *
   * Return configuration array, ensure that all default values are present.
   */
  public function getConfig() {
    $defaults = $this->configDefaults();
    return $this->config + $defaults;
  }

  /**
   * Return default configuration.
   *
   * @todo rename to getConfigDefaults().
   *
   * @return
   *   Array where keys are the variable names of the configuration elements and
   *   values are their default values.
   */
  public function configDefaults() {
    return array();
  }

  /**
   * Returns whether or not the configurable has a config form.
   *
   * @return bool
   *   True if the configurable has a config form, and false if not.
   */
  public function hasConfigForm() {
    $form_state = array();
    return (bool) $this->configForm($form_state);
  }

  /**
   * Return configuration form for this object. The keys of the configuration
   * form must match the keys of the array returned by configDefaults().
   *
   * @return
   *   FormAPI style form definition.
   */
  public function configForm(&$form_state) {
    return array();
  }

  /**
   * Validation handler for configForm().
   *
   * Set errors with form_set_error().
   *
   * @param $values
   *   An array that contains the values entered by the user through configForm.
   */
  public function configFormValidate(&$values) {
  }

  /**
   *  Submission handler for configForm().
   *
   *  @param $values
   */
  public function configFormSubmit(&$values) {
    $this->addConfig($values);
    $this->save();
    drupal_set_message(t('Your changes have been saved.'));
    feeds_cache_clear(FALSE);
  }

  /**
   * Returns an array of required modules.
   *
   * @return array
   *   The modules that this configurable requires.
   */
  public function dependencies() {
    return array();
  }
}

/**
 * Config form wrapper. Use to render the configuration form of
 * a FeedsConfigurable object.
 *
 * @param $configurable
 *   FeedsConfigurable object.
 * @param $form_method
 *   The form method that should be rendered.
 *
 * @return
 *   Config form array if available. NULL otherwise.
 */
function feeds_get_form($configurable, $form_method) {
  if (method_exists($configurable, $form_method)) {
    return drupal_get_form(get_class($configurable) . '_feeds_form', $configurable, $form_method);
  }
}

/**
 * Config form callback. Don't call directly, but use
 * feeds_get_form($configurable, 'method') instead.
 *
 * @param
 *   FormAPI $form_state.
 * @param
 *   FeedsConfigurable object.
 * @param
 *   The object to perform the save() operation on.
 * @param $form_method
 *   The $form_method that should be rendered.
 */
function feeds_form($form, &$form_state, $configurable, $form_method) {
  $form = $configurable->$form_method($form_state);
  $form['#configurable'] = $configurable;
  $form['#feeds_form_method'] = $form_method;
  $form['#validate'] = array('feeds_form_validate');
  $form['#submit'] = array('feeds_form_submit');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );
  return $form;
}

/**
 * Validation handler for feeds_form().
 */
function feeds_form_validate($form, &$form_state) {
  _feeds_form_helper($form, $form_state, 'Validate');
}

/**
 * Submit handler for feeds_form().
 */
function feeds_form_submit($form, &$form_state) {
  _feeds_form_helper($form, $form_state, 'Submit');
}

/**
 * Helper for Feeds validate and submit callbacks.
 *
 * @todo This is all terrible. Remove.
 */
function _feeds_form_helper($form, &$form_state, $action) {
  $method = $form['#feeds_form_method'] . $action;
  $class = get_class($form['#configurable']);
  $id = $form['#configurable']->id;

  // Re-initialize the configurable object. Using feeds_importer() and
  // feeds_plugin() will ensure that we're using the same instance. We can't
  // reuse the previous form instance because feeds_importer() is used to save.
  // This will re-initialize all of the plugins anyway, causing some tricky
  // saving issues in certain cases.
  // See http://drupal.org/node/1672880.
  if ($class == variable_get('feeds_importer_class', 'FeedsImporter')) {
    $form['#configurable'] = feeds_importer($id);
  }
  else {
    $importer = feeds_importer($id);
    $plugin_key = $importer->config[$form['#configurable']->pluginType()]['plugin_key'];
    $form['#configurable'] = feeds_plugin($plugin_key, $id);
  }

  if (method_exists($form['#configurable'], $method)) {
    $form['#configurable']->$method($form_state['values']);
  }
}
