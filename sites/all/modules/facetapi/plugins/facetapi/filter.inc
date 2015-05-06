<?php

/**
 * @file
 * Base filter class and core implementations.
 */

/**
 * Abstract class extended by filter plugins.
 *
 * Filter plugins are executed in FacetapiFacet::build() after the render array
 * is initialized and before the array is passed to the widget plugin. This
 * allows for runtime data alterations such as hiding active items or only
 * displaying facets in the current level of a hierarchical data set.
 *
 * Although it is most common to remove items from the render array, it is also
 * possible to add items to the array. However, the filter plugin is executed
 * after FacetapiFacetProcessor::process(), so the hierarchical data has already
 * been processed and index values have already been mapped to their human
 * readable values. If you are looking to add items to the render array that
 * also go through these processes, adding a callback to the facet's "alter
 * callbacks" is probably the best route to take. See hook_facetapi_facet_info()
 * for more information on alter callbacks.
 *
 * @see http://drupal.org/node/1156606
 */
abstract class FacetapiFilter {

  /**
   * The machine name of the plugin associated with this instance.
   *
   * @var string
   */
  protected $id;

  /**
   * The adapter associated with facet being filtered.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet settings.
   *
   * @var stdClass
   */
  protected $settings;

  /**
   * Constructs a FacetapiFilter object.
   *
   * Sets the necessary information required to filter the facet.
   *
   * @param string $id
   *   The machine name of the filter plugin being instantiated as defined in
   *   hook_facetapi_filters() implementations.
   * @param FacetapiAdapter $adapter
   *   The adapter associated with facet being filtered.
   * @param stdClass $settings
   *   The facet's realm specific settings as returned by
   *   FacetapiAdapter::getFacetSettings().
   */
  public function __construct($id, FacetapiAdapter $adapter, stdClass $settings) {
    $this->id = $id;
    $this->adapter = $adapter;
    $this->settings = $settings;
    $this->settings->settings += $this->getDefaultSettings();
  }

  /**
   * Executes the filter by returning an altered render array.
   *
   * @param array $build
   *   The facet's base render array.
   *
   * @return array
   *   The altered render array that will be passed to the widget plugin.
   */
  abstract public function execute(array $build);

  /**
   * Allows the plugin to add settings to the dependency form.
   *
   * @see facetapi_facet_filters_form()
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do.
  }

  /**
   * Provides default values for the plugin settings.
   *
   * All settings added via FacetapiFilter::settingsForm() should have
   * corresponding defaults in this method.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }
}

/**
 * Plugin that filters active items.
 */
class FacetapiFilterActiveItems extends FacetapiFilter {

  /**
   * Implements FacetapiFilter::execute().
   */
  public function execute(array $build) {
    return array_filter($build, 'facetapi_filter_active');
  }
}

/**
 * Callback for array_filter() that strips out active items.
 *
 * @param arrat $build
 *   The facet item's render array.
 *
 * @return bool
 *   A boolean flagging whether the value should remain in the array.
 */
function facetapi_filter_active(array $build) {
  return empty($build['#active']);
}

/**
 * Plugin that filters all items not in current hierarchy.
 *
 * For hierarchical facets such as taxonomy, it might be adventageous to only
 * display the current level of the hierarchy to implement the progressive
 * disclosure pattern inside of the facet. For example, if your taxonomy tree
 * is Make -> Model -> Year for cars, you might want to only display one level
 * at a time. After selecting a make, users should only be presented with the
 * available models.
 */
class FacetapiFilterCurrentDepth extends FacetapiFilter {

  /**
   * Implements FacetapiFilter::execute().
   */
  public function execute(array $build) {
    foreach ($build as $item) {
      if ($item['#active'] && !empty($item['#item_children'])) {
        return $this->execute($item['#item_children']);
      }
    }
    return $build;
  }
}
