<?php

/**
 * @file
 * Block realm code and hook implementations.
 */

/**
 * Implements hook_block_info().
 *
 * @see facetapi_get_block_info()
 */
function facetapi_block_info() {
  return facetapi_get_block_info(array('block'));
}

/**
 * Helper function to get block info for all block-like realms.
 *
 * @param array $realm_names
 *   An array of machine readable realm names blocks are being collected for.
 *
 * @return array
 *   An array of block information.
 *
 * @see http://api.drupal.org/api/drupal/modules%21block%21block.api.php/function/hook_block_info/7
 */
function facetapi_get_block_info(array $realm_names) {
  $blocks = array();
  foreach ($realm_names as $realm_name) {

    // Gets delta map, iterates over all enabled facets.
    $map = facetapi_get_delta_map();
    foreach (facetapi_get_searcher_info() as $searcher => $info) {

      // Gets cache settings for the searcher.
      $cache = facetapi_get_block_cache_setting($searcher, $realm_name);

      // Adds blocks for facets that are enabled or whose delta mapping is forced.
      foreach (facetapi_get_delta_map_queue($searcher, $realm_name) as $facet_name) {
        if ($facet = facetapi_facet_load($facet_name, $searcher)) {
          // Gets the delta from the delta map.
          $string = facetapi_build_delta($searcher, $realm_name, $facet_name);
          $delta = array_search($string, $map);

          // Builds the info. For non-block realm, append the realm name.
          $subject = 'Facet API: ' . $info['label'] . ' : ' . $facet['label'];
          if ('block' != $realm_name && ($realm = facetapi_realm_load($realm_name))) {
            $subject .= ' (' . $realm['label'] . ')';
          }

          // Defines the block.
          $blocks[$delta] = array(
            'info' => $subject,
            'cache' => $cache,
          );
        }
      }
    }
  }

  // Returns available blocks.
  return $blocks;
}

/**
 * Implements hook_ctools_block_info().
 *
 * @see http://drupal.org/node/1669918
 */
function facetapi_ctools_block_info($module, $delta, &$info) {
  // Give the Facet API blocks their own category.
  $info['category'] = t('Facet API');
  // Allow blocks to be used before the search results in Panels.
  $info['render last'] = TRUE;
}

/**
 * Returns block cache settings.
 *
 * @param $searcher
 *   The machine readable name of searcher.
 * @param string $realm_name
 *   The machine readable name of the realm.
 *
 * @return int
 *   The constands defined in includes/common.inc, defaults to DRUPAL_NO_CACHE.
 *
 * @see includes/common.inc
 */
function facetapi_get_block_cache_setting($searcher, $realm_name) {
  $name = 'facetapi:block_cache:' . $searcher;
  if ('block' != $realm_name) {
    $name .= ':' . $realm_name;
  }
  return variable_get($name, DRUPAL_NO_CACHE);
}

/**
 * Implements hook_block_view().
 */
function facetapi_block_view($delta = '') {
  $builds = &drupal_static(__FUNCTION__, array());
  $parsed = &drupal_static('facetapi_parsed_deltas', array());

  // Test block visibility if not already tested.
  if (!isset($parsed[$delta]) && !facetapi_check_block_visibility($delta)) {
    return;
  }

  list($searcher, $realm_name, $facet_name) = $parsed[$delta];

  // Builds and caches the entire realm per searcher / realm combination.
  $group = $searcher . ':' . $realm_name;
  if (!isset($builds[$group])) {
    $builds[$group] = facetapi_build_realm($searcher, $realm_name);
  }

  // Returns the individual block.
  if (isset($builds[$group][$facet_name])) {

    // Adds contextual links.
    $builds[$group][$facet_name]['#contextual_links'] = array(
      'facetapi' => array('admin/config/search/facetapi', array($searcher, $realm_name, $facet_name)),
    );

    // Returns the subject and content of the block.
    $variables = array('title' => $builds[$group][$facet_name]['#title'], 'facet' => $builds[$group][$facet_name]);
    return array(
      'subject' => theme('facetapi_title', $variables),
      'content' => $builds[$group][$facet_name]
    );
  }
}

/**
 * Checks whether the block should be displayed.
 *
 * In cases where modules like Context are being used, hook_block_list_alter()
 * is not invoked and we get fatal errors. We have to test whether or not the
 * hook has been invoked and call this function manually otherwise.
 *
 * @param $delta
 *   The block delta.
 *
 * @return
 *   A boolean flagging whether to display this block or not.
 */
function facetapi_check_block_visibility($delta) {
  $map = facetapi_get_delta_map();

  // Store parsed deltas so we only calculate once. This also lets us know
  // whether hook_block_list_alter() was called or not.
  $parsed = &drupal_static('facetapi_parsed_deltas', array());

  // Ensures the delta is mapped.
  if (!isset($map[$delta])) {
    $parsed[$delta] = FALSE;
    return FALSE;
  }

  // Parses the raw delta, extracts variables for code readability.
  $parsed[$delta] = facetapi_parse_delta($map[$delta]);
  list($searcher, $realm_name, $facet_name) = $parsed[$delta];

  // Checks whether block should be displayed.
  if (!facetapi_is_active_searcher($searcher)) {
    return FALSE;
  }
  if (!facetapi_facet_enabled($searcher, $realm_name, $facet_name)) {
    return FALSE;
  }
  if (!$adapter = facetapi_adapter_load($searcher)) {
    return FALSE;
  }
  if ($adapter->suppressOutput($realm_name)) {
    return FALSE;
  }

  // We have facets!
  return TRUE;
}

/**
 * Returns a cached delta map of hashes to names.
 *
 * Sometimes deltas are longer than 32 chars and need to be passed to hash().
 * Due to the block table's schema, deltas cannot be longer than 32 characters.
 * However, hashes are nasty as CSS IDs, so we can use the map to convert
 * the hashes back to a nicer value in facetapi_preprocess_block().
 *
 * @return
 *   An array containing the delta map.
 */
function facetapi_get_delta_map() {
  $map = &drupal_static(__FUNCTION__);
  if (NULL === $map) {
    if ($data = cache_get('facetapi:delta_map')) {
      $map = $data->data;
    }
    else {

      $map = array();

      // Maps deltas for all realms. This is a hack since not all realms display
      // facets in blocks, but it doesn't hurt to store the extra data. The map
      // is cached anyways, so it shouldn't cause too much additional resource
      // consumption. It is also an edge-case to have non-block facets.
      $searcher_info = facetapi_get_searcher_info();
      foreach (facetapi_get_realm_info() as $realm_name => $realm_info) {
        foreach ($searcher_info as $searcher => $info) {
          foreach (facetapi_get_delta_map_queue($searcher, $realm_name) as $facet_name) {
            $delta = facetapi_build_delta($searcher, $realm_name, $facet_name);
            $map[facetapi_hash_delta($delta)] = $delta;
          }
        }
      }

      // Caches the map so we don't have to do this repeatedly.
      cache_set('facetapi:delta_map', $map, 'cache', CACHE_TEMPORARY);
    }
  }

  return $map;
}

/**
 * Build a delta from the searcher, realm name, and facet name.
 *
 * @param $searcher
 *   The machine readable name of the searcher.
 * @param $realm_name
 *   The machine readable name of the realm.
 * @param $facet_name
 *   The machine readable name of the facet.
 *
 * @return
 *   A string containing the raw delta.
 */
function facetapi_build_delta($searcher, $realm_name, $facet_name) {
  return $searcher . ':' . $realm_name . ':' . urlencode($facet_name);
}

/**
 * Parses a raw delta into parts.
 *
 * @param $raw_delta
 *   A string containing the raw delta prior to being hashed.
 *
 * @return
 *   An array containing the searcher, realm_name, and facet name in that order.
 */
function facetapi_parse_delta($raw_delta) {
  $parsed = array();

  // Splits by ":", finds each part.
  $parts = explode(':', $raw_delta);
  $facet_name = array_pop($parts);
  $facet_name = rawurldecode($facet_name);
  $realm_name = array_pop($parts);
  $searcher = implode(':', $parts);

  // Returns array with parsed info.
  return array($searcher, $realm_name, $facet_name);
}

/**
 * Hashing code for deltas.
 *
 * @param $delta
 *   A string containing the delta.
 *
 * @return
 *   The hashed delta value.
 */
function facetapi_hash_delta($delta) {
  // Ensure hashes are URL safe and alpha-numeric.
  // @see http://drupal.org/node/1355270
  $hash = substr(drupal_hash_base64($delta), 0, 32);
  $hash = strtr($hash, array('-' => '0', '_' => '1'));
  drupal_alter('facetapi_hash', $hash, $delta);
  return $hash;
}

/**
 * Returns facets that are enabled or whose delta mapping is forced.
 *
 * @param $searcher
 *   The machine readable name of the searcher.
 * @param $realm_name
 *   The machine readable name of the realm.
 *
 * @return array
 *   A list of machine readable facet names.
 */
function facetapi_get_delta_map_queue($searcher, $realm_name) {
  static $forced;
  if (NULL === $forced) {
    $forced = module_invoke_all('facetapi_force_delta_mapping');
  }

  $enabled = array_keys(facetapi_get_enabled_facets($searcher, $realm_name));
  if (!isset($forced[$searcher][$realm_name])) {
    $forced[$searcher][$realm_name] = array();
  }

  // Merges enabled facets and facets whose mapping is forced.
  return array_unique(array_merge($enabled, $forced[$searcher][$realm_name]));
}
