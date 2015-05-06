<?php

/**
 * @file
 * Callbacks referenced in hook implementations.
 */

/**
 * Maps bundle machine names to their human friendly label.
 *
 * @param array $values
 *   An array of indexed values being mapped.
 * @param array $options
 *   An associative array of map options containing:
 *   - entities: An array of entities that $values are bundles for.
 *
 * @return array
 *   An array mapping the indexed values to human readable values.
 */
function facetapi_map_bundle(array $values, array $options) {
  if (empty($options['entities'])) {
    $options['entities'] = array('node');
  }
  foreach ($options['entities'] as $entity_type) {
    if ($info = entity_get_info($entity_type)) {
      foreach ($info['bundles'] as $bundle_name => $bundle_info) {
        $names[$bundle_name] = $bundle_info['label'];
      }
    }
  }
  return array_intersect_key($names, array_flip($values));
}

/**
 * Map callback for node authors.
 */
function facetapi_map_author(array $values) {
  $map = array();
  $users = user_load_multiple($values);
  foreach ($users as $user) {
    $map[$user->uid] = format_username($user);
  }
  if (isset($map[0])) {
    $map[0] = variable_get('anonymous', t('Anonymous'));
  }
  return $map;
}

/**
 * Map callback for languages.
 */
function facetapi_map_language(array $values) {
  $map = array();
  $language_list = language_list();
  foreach ($values as $language) {
    if (isset($language_list[$language])) {
      $map[$language] = t($language_list[$language]->name);
    }
    else {
      $map[$language] = t('Language neutral');
    }
  }
  return $map;
}

/**
 * Maps date ranges to human readable dates.
 *
 * @param array $values
 *   An array of indexed values being mapped.
 * @param array $options
 *   An associative array of map options containing:
 *   - format callback: The callback used to format the date, defaults to
 *     "facetapi_format_timestamp".
 *
 * @return array
 *   An array mapping the indexed values to human readable values.
 */
function facetapi_map_date(array $values, array $options) {
  if (empty($options['format callback'])) {
    $options['format callback'] = 'facetapi_format_timestamp';
  }
  $map = array();
  foreach ($values as $value) {
    $range = explode(' TO ', trim($value, '{[]}'));
    if (isset($range[1])) {
      $gap = facetapi_get_date_gap($range[0], $range[1]);
      $map[$value] = facetapi_format_date($range[0], $gap, $options['format callback']);
    }
  }
  return $map;
}

/**
 * Callback that returns the minimum date in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_min_date(array $facet) {
  $query = db_select('node', 'n')->condition('status', 1);
  $query->addExpression('MIN(' . $facet['name'] . ')', 'max');
  return $query->execute()->fetch()->max;
}

/**
 * Callback that returns the minimum value in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_max_date(array $facet) {
  $query = db_select('node', 'n')->condition('status', 1);
  $query->addExpression('MAX(' . $facet['name'] . ')', 'max');
  return $query->execute()->fetch()->max;
}

/**
 * Map callback for taxonomy terms.
 */
function facetapi_map_taxonomy_terms(array $values) {
  $map = array();
  $terms = taxonomy_term_load_multiple($values);
  foreach ($terms as $term) {
    $map[$term->tid] = entity_label('taxonomy_term', $term);
  }
  return $map;
}

/**
 * Gets parent information for taxonomy terms.
 *
 * @param array $values
 *   An array containing the term ids.
 *
 * @return
 *   An associative array keyed by term ID to parent ID.
 */
function facetapi_get_taxonomy_hierarchy(array $values) {
  $result = db_select('taxonomy_term_hierarchy', 'th')
    ->fields('th', array('tid', 'parent'))
    ->condition('th.parent', '0', '>')
    ->condition(db_or()
      ->condition('th.tid', $values, 'IN')
      ->condition('th.parent', $values, 'IN')
    )
    ->execute();

  $parents = array();
  foreach ($result as $record) {
    $parents[$record->tid][] = $record->parent;
  }
  return $parents;
}
