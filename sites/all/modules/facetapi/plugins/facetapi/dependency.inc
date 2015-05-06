<?php

/**
 * @file
 * Base dependency plugin class.
 */

/**
 * Abstract class extended by dependency plugins.
 *
 * Dependencies are conditions that must be met for facets to be processed and
 * displayed. The dependency system allows site builders to implement the
 * "progressive disclosure" pattern, where facets are exposed only after some
 * action has been taken. For example, a common use case is to only expose
 * taxonomy facets when the bundle they are attached to is selected by the user
 * as an active facet.
 */
abstract class FacetapiDependency {

  /**
   * The machine name of the plugin associated with this instance.
   *
   * @var string
   */
  protected $id;

  /**
   * The adapter associated with facet whose dependencies are being checked.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition as returned by facetapi_facet_load().
   *
   * @var array
   */
  protected $facet;

  /**
   * An array of dependency settings.
   *
   * This is the "dependencies" key of the "settings" property of the facet's
   * global settings returned by FacetapiAdapter::getFacetSettingsGlobal().
   *
   * @var array
   */
  protected $settings;

  /**
   * The facet's active items as returned by FacetapiAdapter::getActiveItems().
   *
   * @var array
   */
  protected $activeItems;

  /**
   * Constructs a FacetapiDependency object.
   *
   * Sets the necessary information in order to check a facet's dependencies.
   *
   * @param array $id
   *   The machine name of the dependency plugin being instantiated as defined
   *   in hook_facetapi_dependencies() implementations.
   * @param FacetapiAdapter $adapter
   *   The adapter associated with facet whose dependencies are being checked.
   * @param array $facet
   *   The facet definition as returned by facetapi_facet_load().
   * @param array $active_items
   *   The facet's active items returned by FacetapiAdapter::getActiveItems().
   * @param stdClass $settings
   *   The facet's global settings.
   */
  public function __construct($id, FacetapiAdapter $adapter, array $facet, stdClass $settings, array $active_items = array()) {
    $this->id = $id;
    $this->adapter = $adapter;
    $this->facet = $facet;
    $this->activeItems = $active_items;

    // Capture dependency settings only, ensure defaults are set.
    if (empty($settings->settings['dependencies'])) {
      $settings->settings['dependencies'] = array();
    }
    $this->settings = $settings->settings['dependencies'];
    $this->settings += $this->getDefaultSettings();
  }

  /**
   * Gets the machine name of the plugin.
   *
   * @return string
   *   The machine name of the plugin.
   */
  public function getId() {
    return $this->id;
  }

  /**
   * Gets the facet definition as returned by facetapi_facet_load().
   *
   * @return array
   *   The facet definition.
   */
  public function getFacet() {
    return $this->facet;
  }

  /**
   * Performs the dependency check.
   *
   * The dependency system works similar to hook_node_access() in that returning
   * FASLE explicitly fails the dependency, returning NULL does not actively
   * pass or fail the dependency, and returning TRUE explicitly passes the
   * dependency even if other plugins already returned FALSE. Plugins will
   * usually return either NULL or FALSE in this method.
   *
   * @return boolean|NULL
   *   Return NULL to pass through to other dependency plugins, or return a
   *   boolean to explicitly set the result.
   */
  abstract public function execute();

  /**
   * Allows the plugin to add settings to the dependency form.
   *
   * @see facetapi_facet_dependencies_form()
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do.
  }

  /**
   * Provides default values for the plugin settings.
   *
   * All settings added via FacetapiDependency::settingsForm() should have
   * corresponding defaults in this method.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }
}
