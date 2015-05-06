<?php

/**
 * @file
 * Test adapter and plugins.
 */

/**
 * Adapter for our test searcher.
 *
 * The test searcher does not connect to a backend, it just renders dummy data
 * every tom a search is executed. Facet API is not responsible for calculating
 * the facets, so for testing purposes this works just fine.
 */
class FacetapiTestAdapter extends FacetapiAdapter {

  /**
   * The total number of results in the query.
   */
  protected $resultCount = 10;

  /**
   * Implements FacetapiAdapter::searchExecuted().
   */
  public function searchExecuted() {
    return !empty($this->keys);
  }

  /**
   * Implements FacetapiAdapter::suppressOutput().
   */
  public function suppressOutput($realm_name) {
    return FALSE;
  }

  /**
   * Overrides FacetapiAdapter::getSearchPath().
   */
  public function getSearchPath() {
    return 'facetapi_test/search';
  }

  /**
   * Sets the result count.
   *
   * The test searcher does not use the core pager APIs, so we use this method
   * to set a dummy value.
   *
   * @param int $count
   *   The result count.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setResultCount($count) {
    $this->resultCount = $count;
    return $this;
  }

  /**
   * Overrides FacetapiAdapter::getResultCount().
   */
  public function getResultCount() {
    return $this->resultCount;
  }
}

/**
 * Plugin for "term" query type.
 */
class FacetapiTestTerm extends FacetapiQueryType implements FacetapiQueryTypeInterface {

  /**
   * Implements FacetapiQueryTypeInterface::getType().
   */
  static public function getType() {
    return 'term';
  }

  /**
   * Implements FacetapiQueryTypeInterface::execute().
   */
  public function execute($query) {
    // Nothing to do...
  }

  /**
   * Implements FacetapiQueryTypeInterface::build().
   */
  public function build() {
    $build = array();

    $build['testthree'] = array('#count' => 3);
    $build['testone'] = array('#count' => 1);
    $build['testtwo'] = array('#count' => 2);

    // Returns initialized build.
    return $build;
  }
}

/**
 * Plugin for "nonterm" query type.
 */
class FacetapiTestNonterm extends FacetapiQueryType implements FacetapiQueryTypeInterface {

  /**
   * Implements FacetapiQueryTypeInterface::getType().
   */
  static public function getType() {
    return 'nonterm';
  }

  /**
   * Implements FacetapiQueryTypeInterface::execute().
   */
  public function execute($query) {
    // Nothing to do...
  }

  /**
   * Implements FacetapiQueryTypeInterface::build().
   */
  public function build() {
    $build = array();

    $build['testfour'] = array('#count' => 4);
    $build['testsix'] = array('#count' => 6);
    $build['testfive'] = array('#count' => 5);

    // Returns initialized build.
    return $build;
  }
}

/**
 * Plugin for dummy widget that only supports the "nonterm" query type.
 */
class FacetapiTestWidgetNonterm extends FacetapiWidgetLinks {

}
