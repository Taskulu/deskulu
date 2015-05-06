<?php

/**
 * @file
 * Interfaces and base classes for query type plugins.
 */

/**
 * Interface implemented by all query type plugins.
 *
 * Facet API does not perform any facet calculations on it's own. The query type
 * plugin system provides Facet API with a consistent way to tell backends what
 * type of query to execute in order to return the appropriate data required
 * for the faceted navigation display.
 *
 * Query type plugins are implemented by the backends and perform the
 * alterations of their internal search engine query mechanisms to execute the
 * filters and retrieve facet data. For example, modules that integrate with
 * Apache Solr will set the necessary params for faceting, whereas modules that
 * extend the core Search module will add SQL joins, filter clauses, and COUNT
 * queries in order to implement faceting.
 *
 * Although the actual method of querying the search engine is vastly different
 * per backend, Facet API operates under the assumption that the types of
 * queries are the same. For example, a "term" query is assumed to be a straight
 * filter, whereas a "range" query is assumed to be a search between two values.
 * Although the common query types such as "term" and "date" should be available
 * to all backends, it is expected that some backends will have additional query
 * types based on capability. For example, backends integrating with the Apache
 * Solr engine might have a "geospatial" query type that modules integrating
 * with the core Search won't have.
 */
interface FacetapiQueryTypeInterface {

  /**
   * Returns the query type associated with the plugin.
   *
   * Query types must be standard across all backends. For example, the common
   * "term" query type must execute the same type of query for backends that
   * integrate with Apache Solr, the core Search module, or any other search
   * engine that implementing modules connect to.
   *
   * It is recommended that the strings returned by this method contain only
   * lowercase letters with optional underscores.
   *
   * @return string
   *   The query type.
   */
  static public function getType();

  /**
   * Alters the backend's native query object to execute the facet query.
   *
   * As an example, modules that integrate with Apache Solr will set the
   * necessary params for faceting, whereas modules that extend the core Search
   * module will add SQL joins, filter clauses, and COUNT queries in order to
   * implement faceting.
   *
   * @param mixed $query
   *   The backend's native query object.
   */
  public function execute($query);

  /**
   * Gets data from the server and adds values to the facet's render array.
   *
   * At a minimum this method should add the index values returned by the
   * search server as keys containing associative arrays with the "#count" key.
   * The end result will be an array structured like the one below:
   *
   * <code>
   * $build = array(
   *   'index-value-1' => array('#count' => 3),
   *   'index-value-2' => array('#count' => 19),
   *   'index-value-3' => array('#count' => 82),
   *   ...
   * );
   * </code>
   *
   * See the return of the FacetapiFacetProcessor::initializeBuild() for all
   * possible values that could be populated. Query type plugins such as "date"
   * types will populte the #item_children, #item_parents, and #active keys in
   * addition to #count.
   *
   * @return array
   *   The initialized render array. For all possible values of the structure of
   *   the array, see the FacetapiFacetProcessor::initializeBuild() docblock.
   *   Usually only the #count key is used.
   *
   * @see FacetapiFacetProcessor::initializeBuild()
   */
  public function build();
}

/**
 * Base class for query type plugins.
 */
class FacetapiQueryType {

  /**
   * The adapter associated with facet being queried.
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
   * Constructs a FacetapiQueryType object.
   *
   * @param FacetapiAdapter $adapter
   *   The adapter object associated with facet being queried.
   * @param array $facet
   *   The facet definition as returned by facetapi_facet_load().
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->adapter = $adapter;
    $this->facet = $facet;
  }

  /**
   * Adds additional information to the array active items.
   *
   * Active facet items are stored in the FacetapiAdapter::activeItems property
   * as associative arrays. See the docblock for the structure. Queries such as
   * ranges can add additional info such as the "start" and "end" values for
   * more efficient processing of facet data.
   *
   * @param array $item
   *   The active item. See FacetapiAdapter::activeItems for the structure of
   *   the active item array.
   *
   * @return array
   *   An associative array addition information to add to the active item.
   */
  public function extract(array $item) {
    return array();
  }

  /**
   * Convenience method to get the facet's global and per relam settings.
   *
   * @param string|array $realm
   *   The machine readable name of the realm or an array containing the realm
   *   definition. Pass NULL to return the facet's global settings.
   *
   * @return stdClass
   *   An object containing the settings.
   *
   * @see FacetapiFacet::getSettings()
   */
  public function getSettings($realm = NULL) {
    return $this->adapter->getFacet($this->facet)->getSettings($realm);
  }

  /**
   * Returns the facet's active items.
   *
   * @return array
   *   The facet's active items. See FacetapiAdapter::activeItems for the
   *   structure of the active item array.
   *
   * @see FacetapiAdapter::activeItems
   */
  public function getActiveItems() {
    return $this->adapter->getActiveItems($this->facet);
  }
}

/**
 * Base class for date query type plugins.
 */
class FacetapiQueryTypeDate extends FacetapiQueryType {

  /**
   * Overrides FacetapiQueryType::__construct().
   *
   * Lazy loads the date processing functions.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    module_load_include('inc', 'facetapi', 'facetapi.date');
    parent::__construct($adapter, $facet);
  }

  /**
   * Overrides FacetapiQueryType::extract().
   *
   * Adds the "start" and "end" values for the date range.
   */
  public function extract(array $item) {
    $return = array();
    if (preg_match(FACETAPI_REGEX_DATE_RANGE, $item['value'], $matches)) {
      $return['start'] = $matches[1];
      $return['end'] = $matches[8];
    }
    return $return;
  }
}

/**
 * Base class for range query type plugins.
 */
class FacetapiQueryTypeRange extends FacetapiQueryType {

  /**
   * Overrides FacetapiQueryType::extract().
   *
   * Adds the "start" and "end" values for the range.
   */
  public function extract(array $item) {
    $return = array();
    if (preg_match(FACETAPI_REGEX_RANGE, $item['value'], $matches)) {
      $return['start'] = $matches[1];
      $return['end'] = $matches[2];
    }
    return $return;
  }
}
