<?php

/**
 * @file
 * Base widget plugin class and helper functions for facet sorting.
 */

/**
 * Abstract class extended by widget plugins.
 *
 * Widgets are responsible for altering the render arrays to achieve some user
 * interface component. For example, the render arrays could produce a list of
 * clickable links or even clickable charts.
 */
abstract class FacetapiWidget {

  /**
   * The machine name of the plugin associated with this instance.
   *
   * @var string
   */
  protected $id;

  /**
   * The realm definition as returned by facetapi_realm_load().
   *
   * @var array
   */
  protected $realm;

  /**
   * The facet object containing the facet definition and required contexts.
   *
   * @var FacetapiFacet
   */
  protected $facet;

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
   * The render that alterations are being applied to.
   *
   * The base render array is constructed via FacetapiFacetProcessor::process()
   * and has already been passed to the enabled filter plugins. The render array
   * is further processed by FacetapiWidget::init() prior to the plugin acting
   * on it in the FacetapiWidget::execute() implementation.
   *
   * @var array
   */
  protected $build = array();

  /**
   * The key of the facet's render array added to the realm's render array.
   *
   * After the render array is acted on by FacetapiWidget::execute(), it is
   * retrieved by FacetapiWidget::getBuild() and appended to the realm's render
   * array using this key as the element name. The default key is the "field
   * alias" in the facet's definition.
   *
   * @var string
   *
   * @see FacetapiFacet::build()
   * @see FacetapiAdapter::buildRealm()
   */
  protected $key;

  /**
   * The widget's JavaScript settings that are passed to drupal_add_js().
   *
   * For widgets that need make configurations available toe JavaScript files,
   * this is the best method for doing so. An example is the "soft limit" for
   * facets rendered as a list of links which uses jQuery to limit the list on
   * the client side.
   *
   * @var array
   */
  protected $jsSettings = array();

  /**
   * Constructs a FacetapiWidget object.
   *
   * @param string $id
   *   The machine name of the plugin associated with this instance.
   * @param array $realm
   *   The realm definition as returned by facetapi_realm_load() that this facet
   *   is being rendered in.
   * @param FacetapiFacet $facet
   *   The facet being rendered.
   * @param stdClass $settings
   *   The facet's realm specific settings as returned by
   *   FacetapiAdapter::getFacetSettings().
   */
  public function __construct($id, array $realm, FacetapiFacet $facet, stdClass $settings) {
    $this->id = $id;
    $this->realm = $realm;
    $this->settings = $settings;
    $this->settings->settings += $this->getDefaultSettings();
    $this->facet = $facet;

    // Set the default key as the facet's alias.
    $this->key = $facet['field alias'];
  }

  /**
   * Initializes the build, must be invoked prior to executing this widget.
   *
   * This method is called automatically by FacetapiFacet::build() and shoud
   * rarely be invoked outside of that context. It is responsible for getting
   * the base render array stored in the FacetapiFacet object and appending it
   * to the container that will be returned, sorting the facets, and adding some
   * common JavaScript settings common across all widgets.
   *
   * @return FacetapiWidget
   *   An instance of this class.
   */
  public function init() {

    // Capture searcher for code readability.
    $searcher = $this->facet->getAdapter()->getSearcher();

    // Initialize the render array.
    $this->build = array(
      '#title' => $this->facet['label'],
      '#description' => $this->facet['description'],
      '#weight' => $this->facet['weight'],
      '#adapter' => $this->facet->getAdapter(),
      '#realm_name' => $this->realm['name'],
      '#facet' => $this->facet->getFacet(),
      '#settings' => $this->settings,
      $this->facet['field alias'] => $this->facet->getBuild(),
      '#attributes' => array(
        'class' => array(
          drupal_html_class("facetapi-" . $this->id),
          drupal_html_class("facetapi-facet-{$this->facet['name']}"),
        ),
        'id' => drupal_html_id("facetapi-facet-$searcher-{$this->realm['name']}-{$this->facet['name']}"),
      ),
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'facetapi') . '/facetapi.js'),
      ),
    );

    // Apply sorting algorithms to the render array.
    $this->sortFacet($this->build);

    // Initialize JavaScript settings for this facet build.
    $this->jsSettings += array(
      'id' => $this->build['#attributes']['id'],
      'searcher' => $searcher,
      'realmName' => $this->realm['name'],
      'facetName' => $this->facet['name'],
      'queryType' => $this->facet['query type'],
      'widget' => $this->settings->settings['widget'],
      'showMoreText' => $this->settings->settings['facet_more_text'],
      'showFewerText' => $this->settings->settings['facet_fewer_text'],
    );

    return $this;
  }

  /**
   * Executes the widget plugin by altering FacetapiWidget::build.
   *
   * Implementing plugins use this method to transform the initialized render
   * array into something that will product the desired UI component. See the
   * documentation at http://drupal.org/node/930760 for more information on how
   * to work with render arrays.
   *
   * For code readability, it is best practice to add the following snippet as
   * the first line of the method so developers can act directly on the $element
   * variable as opposed to a nested array.
   *
   * <code>
   * public function execute() {
   *   $element = &$this->build[$this->facet['field alias']];
   *
   * }
   * </code>
   *
   * @see http://drupal.org/node/930760
   */
  abstract public function execute();

  /**
   * Allows the plugin to add settings to the display form.
   *
   * @see facetapi_facet_display_form()
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do ...
  }

  /**
   * Provides default values for the plugin settings.
   *
   * All settings added via FacetapiWidget::settingsForm() should have
   * corresponding defaults in this method.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
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
   * Returns the altered render array acted on by FacetapiWidget::execute().
   *
   * @return array
   *   The render array.
   */
  public function getBuild() {
    return $this->build;
  }

  /**
   * Gets key used to append FacetapiWidget::build to the realm's render array.
   *
   * @return string
   *   The key used as the element name for this facet's render array.
   */
  public function getKey() {
    return $this->key;
  }

  /**
   * Returns the JavaScript settings that are passed to drupal_add_js().
   *
   * @return array
   *   The JavaScript settings.
   */
  public function getJavaScriptSettings() {
    return $this->jsSettings;
  }

  /**
   * Applies sorting algorithms to the items in the facet's render array.
   *
   * This method applies the sorting algorithms configured via the display form
   * to the items in the facet's initialized render array.
   *
   * @param array &$build
   *   Reference to the facet's render array.
   *
   * @see FacetapiWidget::init()
   * @see FacetapiWidget::applySorts()
   */
  function sortFacet(array &$build) {
    $settings = $build['#settings']->settings;

    // Gets active sort definitions.
    $this->sorts = array_intersect_key(
      facetapi_get_sort_info(),
      array_filter($settings['active_sorts'])
    );

    // Finalizes sort definitions with settings or defaults.
    foreach ($this->sorts as $name => &$info) {
      $info['weight'] = $settings['sort_weight'][$name];
      $info['order'] = $settings['sort_order'][$name];
    }

    if ($this->sorts) {
      // Orders the sorts, applies sorting algorithms in that order.
      uasort($this->sorts, 'drupal_sort_weight');
      $this->applySorts($build[$this->facet['field alias']]);
    }
  }

  /**
   * Applies the sorts to the facet items recursively.
   *
   * The recursion in this function is useful for sorting hierarchical data sets
   * one level at a time.
   *
   * @param array &$build
   *   Reference to the items in the facet's render array that are being sorted.
   *
   * @see FacetapiWidget::sortFacet()
   * @see FacetapiWidget::sortCallback()
   */
  protected function applySorts(&$build) {
    foreach (element_children($build) as $value) {
      if (!empty($build[$value]['#item_children'])) {
        $this->applySorts($build[$value]['#item_children']);
      }
    }
    // Uses FacetapiWidget::sortCallback() as the uasort() callback.
    uasort($build, array($this, 'sortCallback'));
  }

  /**
   * Applies sort information via the callback in the sort definition.
   *
   * Each definition in hook_facetapi_sort_info() specifices a "callback", which
   * is the function that returns the sorting information expected by normal
   * uasort() callbacks.
   *
   * @see uasort()
   */
  protected function sortCallback(array $a, array $b) {
    $return = 0;
    foreach ($this->sorts as $sort) {
      if ($return = $sort['callback']($a, $b)) {
        if (SORT_DESC == $sort['order']) {
          $return *= -1;
        }
        break;
      }
    }
    return $return;
  }
}

/**
 * Sorts by whether or not a facet is active.
 *
 * @see uasort()
 */
function facetapi_sort_active(array $a, array $b) {
  $a_active = (isset($a['#active'])) ? $a['#active'] : 0;
  $b_active = (isset($b['#active'])) ? $b['#active'] : 0;
  if ($a_active == $b_active) {
    return 0;
  }
  return ($a_active < $b_active) ? -1 : 1;
}

/**
 * Sorts by facet count.
 *
 * @see uasort()
 */
function facetapi_sort_count(array $a, array $b) {
  $a_count = (isset($a['#count'])) ? $a['#count'] : 0;
  $b_count = (isset($b['#count'])) ? $b['#count'] : 0;
  if ($a_count == $b_count) {
    return 0;
  }
  return ($a_count < $b_count) ? -1 : 1;
}

/**
 * Sorts by raw indexed value.
 *
 * @see uasort()
 */
function facetapi_sort_indexed(array $a, array $b) {
  $a_value = (isset($a['#indexed_value'])) ? $a['#indexed_value'] : '';
  $b_value = (isset($b['#indexed_value'])) ? $b['#indexed_value'] : '';
  if ($a_value == $b_value) {
    return 0;
  }
  return ($a_value < $b_value) ? -1 : 1;
}

/**
 * Sorts by display value.
 *
 * @see uasort()
 */
function facetapi_sort_display(array $a, array $b) {
  $a_count = (isset($a['#markup'])) ? $a['#markup'] : '';
  $b_count = (isset($b['#markup'])) ? $b['#markup'] : '';
  return strcasecmp($a['#markup'], $b['#markup']);
}
