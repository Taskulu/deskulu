<?php

/**
 * @file
 * Base url processor plugin class.
 */

/**
 * Abstract class extended by url processor plugins.
 *
 * Url processor plugins provide a pluggable method of retrieving facet data.
 * Most commonly facet data is retrieved from a query string variable via $_GET,
 * however custom plugis can be written to retrieve data from the path as well.
 * In addition to facet data retrieval, the url processor plugin is also
 * responsible for building facet links and setting breadcrumb trails.
 *
 * Each adapter instance is associated with a single url processor plugin. The
 * plugin is associated with the adapter via hook_facetapi_searcher_info()
 * implementations.
 */
abstract class FacetapiUrlProcessor {

  /**
   * The adapter that the url processor plugin is associated with.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * An array of facet params, usually $_GET.
   *
   * @var array.
   */
  protected $params = array();

  /**
   * The array key in FacetapiUrlProcessor::params containing the facet data.
   *
   * @var string
   */
  protected $filterKey = 'f';

  /**
   * Constructs a FacetapiUrlProcessor object.
   *
   * @param FacetapiAdapter $adapter
   *   The adapter that the url processor plugin is associated with.
   */
  public function __construct(FacetapiAdapter $adapter) {
    $this->adapter = $adapter;
  }

  /**
   * Fetches parameters from the source, usually $_GET.
   *
   * This method is invoked in FacetapiAdapter::__construct().
   *
   * @return array
   *   An associative array containing the params, usually $_GET.
   */
  abstract public function fetchParams();

  /**
   * Normalizes the array returned by FacetapiAdapter::fetchParams().
   *
   * When extracting data from a source such as $_GET, there are certain items
   * that you might nor want, for example the "q" or "page" keys. This method is
   * useful for filtering those out. In addition, plugins that do not get data
   * from $_GET can use this method to normalize the data into an associative
   * array that closely matches the data structure of $_GET.
   *
   * @param array $params
   *   An array of keyed params, usually as $_GET.
   * @param string $filter_key
   *   The array key in $params containing the facet data, defaults to "f".
   *
   * @return array
   *   An associative array containing the normalized params.
   */
  abstract public function normalizeParams(array $params, $filter_key = 'f');

  /**
   * Returns the query string variables for a facet item.
   *
   * The return array must be able to be passed as the "query" key of the
   * options array passed as the second argument to the url() function. See
   * http://api.drupal.org/api/drupal/includes%21common.inc/function/url/7 for
   * more details.
   *
   * @param array $facet
   *   The facet definition as returned by facetapi_facet_load().
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not. 1 if the item is
   *   active, 0 if it is not.
   *
   * @return array
   *   The query string variables.
   */
  abstract public function getQueryString(array $facet, array $values, $active);

  /**
   * Returns the path for a facet item.
   *
   * @param array $facet
   *   The facet definition.
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @return string
   *   The path of the facet.
   */
  public function getFacetPath(array $facet, array $values, $active) {
    return $this->adapter->getSearchPath();
  }

  /**
   * Sets the breadcrumb trail for active searches.
   *
   * This method is called by FacetapiAdapter::processFacets(), which is called
   * directly by the backend to search the chain of Facet API events.
   */
  abstract public function setBreadcrumb();

  /**
   * Sets the normalized parameters.
   *
   * This method is usually called by FacetapiAdapter::setParams() and is very
   * rarely called directly.
   *
   * @param array $params
   *   An array of normalized params hat have already been passed through
   *   FacetapiUrlProcessor::normalizeParams().
   * @param string $filter_key
   *   The array key in $params containing the facet data, defaults to "f".
   *
   * @return FacetapiUrlParser
   *   An instance of this class.
   */
  public function setParams(array $params, $filter_key = 'f') {
    $this->params = $params;
    $this->filterKey = $filter_key;
    if (!isset($this->params[$this->filterKey]) || !is_array($this->params[$this->filterKey])) {
      $this->params[$this->filterKey] = array();
    }
    return $this;
  }

  /**
   * Returns the params.
   *
   * @return array
   *   An array containing the params.
   */
  public function getParams() {
    return $this->params;
  }

  /**
   * Removes an item from the $this->params array.
   *
   * @param int $pos
   *   The zero-based position of the value in the source data.
   */
  public function removeParam($pos) {
    unset($this->params[$this->filterKey][$pos]);
  }

  /**
   * Returns the filter key.
   *
   * @return string
   *   A string containing the filter key.
   */
  public function getFilterKey() {
    return $this->filterKey;
  }

  /**
   * Allows for processor specific overrides to the settings form.
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do...
  }

  /**
   * Provides default values for the backend specific settings.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }
}
