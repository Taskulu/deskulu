<?php

/**
 * @file
 * Views hook implementations for the Search API Views module.
 */

/**
 * Implements hook_views_data().
 */
function search_api_views_views_data() {
  try {
    $data = array();
    $entity_types = entity_get_info();
    foreach (search_api_index_load_multiple(FALSE) as $index) {
      // Fill in base data.
      $key = 'search_api_index_' . $index->machine_name;
      $table = &$data[$key];
      $type_info = search_api_get_item_type_info($index->item_type);
      $table['table']['group'] = t('Indexed @entity_type', array('@entity_type' => $type_info['name']));
      $table['table']['base'] = array(
        'field' => 'search_api_id',
        'index' => $index->machine_name,
        'title' => $index->name,
        'help' => t('Use the %name search index for filtering and retrieving data.', array('%name' => $index->name)),
        'query class' => 'search_api_views_query',
      );
      if (isset($entity_types[$index->getEntityType()])) {
        $table['table'] += array(
          'entity type' => $index->getEntityType(),
          'skip entity load' => TRUE,
        );
      }

      try {
        $wrapper = $index->entityWrapper(NULL, FALSE);
      }
      catch (EntityMetadataWrapperException $e) {
        watchdog_exception('search_api_views', $e, "%type while retrieving metadata for index %index: !message in %function (line %line of %file).", array('%index' => $index->name), WATCHDOG_WARNING);
        continue;
      }

      // Add field handlers and relationships provided by the Entity API.
      foreach ($wrapper as $key => $property) {
        $info = $property->info();
        if ($info) {
          entity_views_field_definition($key, $info, $table);
        }
      }

      try {
        $wrapper = $index->entityWrapper(NULL);
      }
      catch (EntityMetadataWrapperException $e) {
        watchdog_exception('search_api_views', $e, "%type while retrieving metadata for index %index: !message in %function (line %line of %file).", array('%index' => $index->name), WATCHDOG_WARNING);
        continue;
      }

      // Add handlers for all indexed fields.
      foreach ($index->getFields() as $key => $field) {
        $tmp = $wrapper;
        $group = '';
        $name = '';
        $parts = explode(':', $key);
        foreach ($parts as $i => $part) {
          if (!isset($tmp->$part)) {
            continue 2;
          }
          $tmp = $tmp->$part;
          $info = $tmp->info();
          $group = ($group ? $group . ' » ' . $name : ($name ? $name : ''));
          $name = $info['label'];
          if ($i < count($parts) - 1) {
            // Unwrap lists.
            $level = search_api_list_nesting_level($info['type']);
            for ($j = 0; $j < $level; ++$j) {
              $tmp = $tmp[0];
            }
          }
        }
        $id = _entity_views_field_identifier($key, $table);
        if ($group) {
          // @todo Entity type label instead of $group?
          $table[$id]['group'] = $group;
          $name = t('!field (indexed)', array('!field' => $name));
        }
        $table[$id]['title'] = $name;
        $table[$id]['help'] = empty($info['description']) ? t('(No information available)') : $info['description'];
        $table[$id]['type'] = $field['type'];
        if ($id != $key) {
          $table[$id]['real field'] = $key;
        }
        _search_api_views_add_handlers($key, $field, $tmp, $table);
      }

      // Special handlers
      $table['search_api_language']['filter']['handler'] = 'SearchApiViewsHandlerFilterLanguage';

      $table['search_api_id']['title'] = t('Entity ID');
      $table['search_api_id']['help'] = t("The entity's ID.");
      $table['search_api_id']['sort']['handler'] = 'SearchApiViewsHandlerSort';

      $table['search_api_relevance']['group'] = t('Search');
      $table['search_api_relevance']['title'] = t('Relevance');
      $table['search_api_relevance']['help'] = t('The relevance of this search result with respect to the query.');
      $table['search_api_relevance']['field']['type'] = 'decimal';
      $table['search_api_relevance']['field']['handler'] = 'entity_views_handler_field_numeric';
      $table['search_api_relevance']['field']['click sortable'] = TRUE;
      $table['search_api_relevance']['sort']['handler'] = 'SearchApiViewsHandlerSort';

      $table['search_api_excerpt']['group'] = t('Search');
      $table['search_api_excerpt']['title'] = t('Excerpt');
      $table['search_api_excerpt']['help'] = t('The search result excerpted to show found search terms.');
      $table['search_api_excerpt']['field']['type'] = 'text';
      $table['search_api_excerpt']['field']['handler'] = 'entity_views_handler_field_text';

      $table['search_api_views_fulltext']['group'] = t('Search');
      $table['search_api_views_fulltext']['title'] = t('Fulltext search');
      $table['search_api_views_fulltext']['help'] = t('Search several or all fulltext fields at once.');
      $table['search_api_views_fulltext']['filter']['handler'] = 'SearchApiViewsHandlerFilterFulltext';
      $table['search_api_views_fulltext']['argument']['handler'] = 'SearchApiViewsHandlerArgumentFulltext';

      $table['search_api_views_more_like_this']['group'] = t('Search');
      $table['search_api_views_more_like_this']['title'] = t('More like this');
      $table['search_api_views_more_like_this']['help'] = t('Find similar content.');
      $table['search_api_views_more_like_this']['argument']['handler'] = 'SearchApiViewsHandlerArgumentMoreLikeThis';

      // If there are taxonomy term references indexed in the index, include the
      // "Indexed taxonomy term fields" contextual filter. We also save for all
      // fields whether they contain only terms of a certain vocabulary, keying
      // that information by vocabulary for later ease of use.
      $vocabulary_fields = array();
      foreach ($index->getFields() as $key => $field) {
        if (isset($field['entity_type']) && $field['entity_type'] === 'taxonomy_term') {
          $field_id = ($pos = strrpos($key, ':')) ? substr($key, $pos + 1) : $key;
          $field_info = field_info_field($field_id);
          if ($vocabulary = _search_api_views_get_field_vocabulary($field_info)) {
            $vocabulary_fields[$vocabulary][] = $key;
          }
          else {
            $vocabulary_fields[''][] = $key;
          }
        }
      }
      if ($vocabulary_fields) {
        $table['search_api_views_taxonomy_term']['group'] = t('Search');
        $table['search_api_views_taxonomy_term']['title'] = t('Indexed taxonomy term fields');
        $table['search_api_views_taxonomy_term']['help'] = t('Search in all indexed taxonomy term fields.');
        $table['search_api_views_taxonomy_term']['argument']['handler'] = 'SearchApiViewsHandlerArgumentTaxonomyTerm';
        $table['search_api_views_taxonomy_term']['argument']['vocabulary_fields'] = $vocabulary_fields;
      }
    }
    return $data;
  }
  catch (Exception $e) {
    watchdog_exception('search_api_views', $e);
  }
}

/**
 * Adds handler definitions for a field to a Views data table definition.
 *
 * Helper method for search_api_views_views_data().
 *
 * @param $id
 *   The internal identifier of the field.
 * @param array $field
 *   Information about the field.
 * @param EntityMetadataWrapper $wrapper
 *   A wrapper providing further metadata about the field.
 * @param array $table
 *   The existing Views data table definition, as a reference.
 */
function _search_api_views_add_handlers($id, array $field, EntityMetadataWrapper $wrapper, array &$table) {
  $type = $field['type'];
  $inner_type = search_api_extract_inner_type($type);

  if (strpos($id, ':')) {
    entity_views_field_definition($id, $wrapper->info(), $table);
  }
  $id = _entity_views_field_identifier($id, $table);
  $table += array($id => array());

  if ($inner_type == 'text') {
    $table[$id] += array(
      'argument' => array(
        'handler' => 'SearchApiViewsHandlerArgumentString',
      ),
      'filter' => array(
        'handler' => 'SearchApiViewsHandlerFilterText',
      ),
    );
    return;
  }

  $info = $wrapper->info();
  if (isset($info['options list']) && is_callable($info['options list'])) {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilterOptions';
    $table[$id]['filter']['multi-valued'] = search_api_is_list_type($type);
  }
  elseif ($inner_type == 'boolean') {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilterBoolean';
  }
  elseif ($inner_type == 'date') {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilterDate';
  }
  elseif (isset($field['entity_type']) && $field['entity_type'] === 'user') {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilterUser';
  }
  elseif (isset($field['entity_type']) && $field['entity_type'] === 'taxonomy_term') {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilterTaxonomyTerm';
    $field_info = field_info_field($info['name']);
    // For the "Parent terms" and "All parent terms" properties, we can
    // extrapolate the vocabulary from the parent in the selector. (E.g.,
    // for "field_tags:parent" we can use the information of "field_tags".)
    // Otherwise, we can't include any vocabulary information.
    if (!$field_info && ($info['name'] == 'parent' || $info['name'] == 'parents_all')) {
      if (!empty($table[$id]['real field'])) {
        $parts = explode(':', $table[$id]['real field']);
        $field_info = field_info_field($parts[count($parts) - 2]);
      }
    }
    if ($vocabulary = _search_api_views_get_field_vocabulary($field_info)) {
      $table[$id]['filter']['vocabulary'] = $vocabulary;
    }
  }
  else {
    $table[$id]['filter']['handler'] = 'SearchApiViewsHandlerFilter';
  }

  if ($inner_type == 'string' || $inner_type == 'uri') {
    $table[$id]['argument']['handler'] = 'SearchApiViewsHandlerArgumentString';
  }
  elseif ($inner_type == 'date') {
    $table[$id]['argument']['handler'] = 'SearchApiViewsHandlerArgumentDate';
  }
  else {
    $table[$id]['argument']['handler'] = 'SearchApiViewsHandlerArgument';
  }

  // We can only sort according to single-valued fields.
  if ($type == $inner_type) {
    $table[$id]['sort']['handler'] = 'SearchApiViewsHandlerSort';
    if (isset($table[$id]['field'])) {
      $table[$id]['field']['click sortable'] = TRUE;
    }
  }
}

/**
 * Implements hook_views_plugins().
 */
function search_api_views_views_plugins() {
  // Collect all base tables provided by this module.
  $bases = array();
  foreach (search_api_index_load_multiple(FALSE) as $index) {
    $bases[] = 'search_api_index_' . $index->machine_name;
  }

  $ret = array(
    'query' => array(
      'search_api_views_query' => array(
        'title' => t('Search API Query'),
        'help' => t('Query will be generated and run using the Search API.'),
        'handler' => 'SearchApiViewsQuery',
      ),
    ),
    'cache' => array(
      'search_api_views_cache' => array(
        'title' => t('Search-specific'),
        'help' => t("Cache Search API views. (Other methods probably won't work with search views.)"),
        'base' => $bases,
        'handler' => 'SearchApiViewsCache',
        'uses options' => TRUE,
      ),
    ),
  );

  if (module_exists('search_api_facetapi')) {
    $ret['display']['search_api_views_facets_block'] = array(
      'title' => t('Facets block'),
      'help' => t('Display facets for this search as a block anywhere on the site.'),
      'handler' => 'SearchApiViewsFacetsBlockDisplay',
      'uses hook block' => TRUE,
      'use ajax' => FALSE,
      'use pager' => FALSE,
      'use more' => TRUE,
      'accept attachments' => TRUE,
      'admin' => t('Facets block'),
    );
  }

  return $ret;
}

/**
 * Returns the vocabulary machine name of a term field.
 *
 * @param array|null $field_info
 *   The field's field info array, or NULL if the field is not provided by the
 *   Field API. See the return value of field_info_field().
 *
 * @return string|null
 *   If the field contains taxonomy terms of a single vocabulary (which could be
 *   determined), that vocabulary's machine name; NULL otherwise.
 */
function _search_api_views_get_field_vocabulary($field_info) {
  // Test for "Term reference" fields.
  if (isset($field_info['settings']['allowed_values'][0]['vocabulary'])) {
    return $field_info['settings']['allowed_values'][0]['vocabulary'];
  }
  // Test for "Entity reference" fields.
  elseif (isset($field_info['settings']['handler']) && $field_info['settings']['handler'] === 'base') {
    if (!empty($field_info['settings']['handler_settings']['target_bundles'])) {
      $bundles = $field_info['settings']['handler_settings']['target_bundles'];
      if (count($bundles) == 1) {
        return key($bundles);
      }
    }
  }
  return NULL;
}
