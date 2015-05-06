<?php

/**
 * @file
 * Base empty behavior plugin class and default implementation.
 */

/**
 * Abstract class extended by empty behavior plugins.
 *
 * Empty behaviors allow administrators to provide alternate facet displays
 * when a facet has no items. The most common use case is displaying a message
 * such as "This facet has no items.".
 */
abstract class FacetapiEmptyBehavior {

  /**
   * The machine readable name of facet configuration.
   *
   * @var string
   */
  protected $configName;

  /**
   * An array of facet settings.
   *
   * This is the the "settings" property of the facet's realm specific settings
   * returned by FacetapiAdapter::getFacetSettings().
   *
   * @var array
   */
  protected $settings;

  /**
   * Constructs a FacetapiEmptyBehavior object.
   *
   * Captures a subset of the facet's settings and applies the plugin defaults.
   *
   * @param stdClass $settings
   *   The facet's realm specific settings as returned by
   *   FacetapiAdapter::getFacetSettings().
   */
  public function __construct(stdClass $settings) {
    $this->configName = $settings->name;
    $this->settings = $settings->settings;
    $this->settings += $this->getDefaultSettings();
  }

  /**
   * Returns the render array used for the facet that is empty, or has no items.
   *
   * @return
   *   The element's render array.
   */
  abstract public function execute();

  /**
   * Allows the plugin to add settings to the display form.
   *
   * @see facetapi_facet_display_form()
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do...
  }

  /**
   * Provides default values for the plugin settings.
   *
   * All settings added via FacetapiEmptyBehavior::settingsForm() should have
   * corresponding defaults in this method.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }

  /**
   * Helper function for translating strings.
   *
   * @param string $key
   *   The array key of the form element under $form['widget']['empty'] in the
   *   facetapi_translate_string() form containing the setting being translated.
   * @param string $string
   *   The string being translated.
   *
   * @return string
   *   The translated string.
   *
   * @see facetapi_translate_string()
   */
  public function translate($key, $string) {
    $config_name = preg_replace('@[^a-zA-Z0-9]@', '_', $this->configName);
    $name = 'facetapi:' . $config_name . ':empty_text:' . $key;
    return facetapi_translate_string($name, $string);
  }
}

/**
 * Empty behavior plugin that returns an empty array.
 *
 * This is the default behavior for empty facets. Returning an empty array will
 * prevent the facet from being displayed if it contains no items. For example,
 * if facets are displayed as blocks, the empty array will prevent the block
 * from rendering.
 */
class FacetapiEmptyBehaviorNone extends FacetapiEmptyBehavior {

  /**
   * Implements FacetapiEmptyBehavior::execute().
   */
  public function execute() {
    return array();
  }
}
