<?php

/**
 * @file
 * Facet API hook implementations.
 */

/**
 * Implements hook_facetapi_realm_info().
 */
function facetapi_test_facetapi_realm_info() {
  return array(
    'nodisplay' => array(
      'label' => t('No display'),
      'weight' => -5,
      'sortable' => FALSE,
      'default widget' => 'facetapi_links',
      'element type' => 'links',
      'description' => t('Dummy realm that never displays facets.'),
    ),
  );
}

/**
 * Implements hook_facetapi_adapters().
 */
function facetapi_test_facetapi_adapters() {
  return array(
    'facetapi_test' => array(
      'handler' => array(
        'class' => 'FacetapiTestAdapter',
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_query_types().
 */
function facetapi_test_facetapi_query_types() {
  return array(
    'facetapi_test_term' => array(
      'handler' => array(
        'class' => 'FacetapiTestTerm',
        'adapter' => 'facetapi_test',
      ),
    ),
    'facetapi_test_nonterm' => array(
      'handler' => array(
        'class' => 'FacetapiTestNonterm',
        'adapter' => 'facetapi_test',
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_widgets().
 */
function facetapi_test_facetapi_widgets() {
  return array(
    'facetapi_nonterm' => array(
      'handler' => array(
        'label' => t('Nonterm Widget'),
        'class' => 'FacetapiTestWidgetNonterm',
        'query types' => array('nonterm'),
      ),
    ),
  );
}
