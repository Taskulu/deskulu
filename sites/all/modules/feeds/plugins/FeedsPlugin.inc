<?php

/**
 * @file
 * Definition of FeedsPlugin class.
 */

/**
 * Base class for a fetcher, parser or processor result.
 */
class FeedsResult {}

/**
 * Implement source interface for all plugins.
 *
 * Note how this class does not attempt to store source information locally.
 * Doing this would break the model where source information is represented by
 * an object that is being passed into a Feed object and its plugins.
 */
abstract class FeedsPlugin extends FeedsConfigurable implements FeedsSourceInterface {

  /**
   * The plugin definition.
   *
   * @var array
   */
  protected $pluginDefinition;

  /**
   * Constructs a FeedsPlugin object.
   *
   * A copy of FeedsConfigurable::__construct() that doesn't call
   * configDefaults() so that we avoid circular dependencies.
   *
   * @param string $id
   *   The importer id.
   */
  protected function __construct($id) {
    $this->id = $id;
    $this->export_type = FEEDS_EXPORT_NONE;
    $this->disabled = FALSE;
  }

  /**
   * Instantiates a FeedsPlugin object.
   *
   * Don't use directly, use feeds_plugin() instead.
   *
   * @see feeds_plugin()
   */
  public static function instance($class, $id, array $plugin_definition = array()) {
    if (!strlen($id)) {
      throw new InvalidArgumentException(t('Empty configuration identifier.'));
    }

    $instances = &drupal_static(__METHOD__, array());

    if (!isset($instances[$class][$id])) {
      $instance = new $class($id);

      // The ordering here is important. The plugin definition should be usable
      // in getConfig().
      $instance->setPluginDefinition($plugin_definition);
      $instance->setConfig($instance->configDefaults());
      $instances[$class][$id] = $instance;
    }

    return $instances[$class][$id];
  }

  /**
   * Returns the type of plugin.
   *
   * @return string
   *   One of either 'fetcher', 'parser', or 'processor'.
   */
  abstract public function pluginType();

  /**
   * Returns the plugin definition.
   *
   * @return array
   *   The plugin definition array.
   *
   * @see ctools_get_plugins()
   */
  public function pluginDefinition() {
    return $this->pluginDefinition;
  }

  /**
   * Sets the plugin definition.
   *
   * This is protected since we're only using it in FeedsPlugin::instance().
   *
   * @param array $plugin_definition
   *   The plugin definition.
   */
  protected function setPluginDefinition(array $plugin_definition) {
    $this->pluginDefinition = $plugin_definition;
  }

  /**
   * Save changes to the configuration of this object.
   * Delegate saving to parent (= Feed) which will collect
   * information from this object by way of getConfig() and store it.
   */
  public function save() {
    feeds_importer($this->id)->save();
  }

  /**
   * Returns TRUE if $this->sourceForm() returns a form.
   */
  public function hasSourceConfig() {
    $form = $this->sourceForm(array());
    return !empty($form);
  }

  /**
   * Implements FeedsSourceInterface::sourceDefaults().
   */
  public function sourceDefaults() {
    $values = array_flip(array_keys($this->sourceForm(array())));
    foreach ($values as $k => $v) {
      $values[$k] = '';
    }
    return $values;
  }

  /**
   * Callback methods, exposes source form.
   */
  public function sourceForm($source_config) {
    return array();
  }

  /**
   * Validation handler for sourceForm.
   */
  public function sourceFormValidate(&$source_config) {}

  /**
   * A source is being saved.
   */
  public function sourceSave(FeedsSource $source) {}

  /**
   * A source is being deleted.
   */
  public function sourceDelete(FeedsSource $source) {}

  /**
   * Loads on-behalf implementations from mappers/ directory.
   *
   * FeedsProcessor::map() does not load from mappers/ as only node and user
   * processor ship with on-behalf implementations.
   *
   * @see FeedsNodeProcessor::map()
   * @see FeedsUserProcessor::map()
   *
   * @todo: Use CTools Plugin API.
   */
  public static function loadMappers() {
    static $loaded = FALSE;
    if (!$loaded) {
      $path = drupal_get_path('module', 'feeds') . '/mappers';
      $files = drupal_system_listing('/.*\.inc$/', $path, 'name', 0);
      foreach ($files as $file) {
        if (strstr($file->uri, '/mappers/')) {
          require_once(DRUPAL_ROOT . '/' . $file->uri);
        }
      }
    }
    $loaded = TRUE;
  }

  /**
   * Get all available plugins.
   */
  public static function all() {
    ctools_include('plugins');
    $plugins = ctools_get_plugins('feeds', 'plugins');

    $result = array();
    foreach ($plugins as $key => $info) {
      if (!empty($info['hidden'])) {
        continue;
      }
      $result[$key] = $info;
    }

    // Sort plugins by name and return.
    uasort($result, 'feeds_plugin_compare');
    return $result;
  }

  /**
   * Determines whether given plugin is derived from given base plugin.
   *
   * @param $plugin_key
   *   String that identifies a Feeds plugin key.
   * @param $parent_plugin
   *   String that identifies a Feeds plugin key to be tested against.
   *
   * @return
   *   TRUE if $parent_plugin is directly *or indirectly* a parent of $plugin,
   *   FALSE otherwise.
   */
  public static function child($plugin_key, $parent_plugin) {
    ctools_include('plugins');
    $plugins = ctools_get_plugins('feeds', 'plugins');
    $info = $plugins[$plugin_key];

    if (empty($info['handler']['parent'])) {
      return FALSE;
    }
    elseif ($info['handler']['parent'] == $parent_plugin) {
      return TRUE;
    }
    else {
      return self::child($info['handler']['parent'], $parent_plugin);
    }
  }

  /**
   * Determines the type of a plugin.
   *
   * @todo PHP5.3: Implement self::type() and query with $plugin_key::type().
   *
   * @param $plugin_key
   *   String that identifies a Feeds plugin key.
   *
   * @return
   *   One of the following values:
   *   'fetcher' if the plugin is a fetcher
   *   'parser' if the plugin is a parser
   *   'processor' if the plugin is a processor
   *   FALSE otherwise.
   */
  public static function typeOf($plugin_key) {
    if (self::child($plugin_key, 'FeedsFetcher')) {
      return 'fetcher';
    }
    elseif (self::child($plugin_key, 'FeedsParser')) {
      return 'parser';
    }
    elseif (self::child($plugin_key, 'FeedsProcessor')) {
      return 'processor';
    }
    return FALSE;
  }

  /**
   * Gets all available plugins of a particular type.
   *
   * @param $type
   *   'fetcher', 'parser' or 'processor'
   */
  public static function byType($type) {
    $plugins = self::all();

    $result = array();
    foreach ($plugins as $key => $info) {
      if ($type == self::typeOf($key)) {
        $result[$key] = $info;
      }
    }
    return $result;
  }

  /**
   * Implements FeedsConfigurable::dependencies().
   */
  public function dependencies() {
    $dependencies = parent::dependencies();

    // Find out which module provides this plugin.
    $plugin_info = $this->pluginDefinition();
    if (isset($plugin_info['module'])) {
      $dependencies[$plugin_info['module']] = $plugin_info['module'];
    }

    return $dependencies;
  }
}

/**
 * Used when a plugin is missing.
 */
class FeedsMissingPlugin extends FeedsPlugin {
  public function pluginType() {
    return 'missing';
  }

  public function save() {}

  /**
   * Fetcher methods.
   */
  public function fetch(FeedsSource $source) {
    return new FeedsFetcherResult('');
  }

  public function clear(FeedsSource $source) {}

  public function request($feed_nid = 0) {
    drupal_access_denied();
  }

  public function menuItem() {
    return array();
  }

  public function subscribe(FeedsSource $source) {}

  public function unsubscribe(FeedsSource $source) {}

  public function importPeriod(FeedsSource $source) {}

  /**
   * Parser methods.
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    return new FeedsParserResult();
  }

  public function getMappingSources() {
    return array();
  }

  /**
   * Processor methods.
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {}

  public function entityType() {}

  public function bundle() {}

  public function bundleOptions() {
    return array();
  }

  public function getLimit() {
    return 0;
  }

  public function getMappings() {
    return array();
  }

  public function getMappingTargets() {
    return array();
  }

  public function expire(FeedsSource $source, $time = NULL) {}

  public function itemCount(FeedsSource $source) {
    return 0;
  }

  public function expiryTime() {
    return FEEDS_EXPIRE_NEVER;
  }

}

/**
 * Sort callback for FeedsPlugin::all().
 */
function feeds_plugin_compare($a, $b) {
  return strcasecmp($a['name'], $b['name']);
}
