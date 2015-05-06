<?php

/**
 * @file
 * Current Search default hooks.
 */

/**
 * Implements hook_current_search_default_items().
 */
function facetapi_test_current_search_default_items() {
  $items = array();

  $item = new stdClass;
  $item->disabled = FALSE;
  $item->api_version = 1;
  $item->name = 'second';
  $item->label = 'Second';
  $item->settings = array(
    'items' => array(
      'results' => array(
        'id' => 'text',
        'label' => 'Results',
        'text' => 'Second current search block',
        'plural' => 0,
        'text_plural' => '',
        'plural_condition' => 'facetapi_results:result-count',
        'wrapper' => 1,
        'element' => 'h3',
        'css' => 0,
        'classes' => '',
        'weight' => '-50',
      ),
    ),
    'advanced' => array(
      'empty_searches' => 0,
    ),
  );
  $items[$item->name] = $item;

  return $items;
}
