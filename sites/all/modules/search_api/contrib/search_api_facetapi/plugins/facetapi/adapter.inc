<?php

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for the Search API module.
 */
class SearchApiFacetapiAdapter extends FacetapiAdapter {

  /**
   * Cached value for the current search for this searcher, if any.
   *
   * @see getCurrentSearch()
   *
   * @var array
   */
  protected $current_search;

  /**
   * The active facet fields for the current search.
   *
   * @var array
   */
  protected $fields = array();

  /**
   * Returns the path to the admin settings for a given realm.
   *
   * @param $realm_name
   *   The name of the realm.
   *
   * @return
   *   The path to the admin settings.
   */
  public function getPath($realm_name) {
    $base_path = 'admin/config/search/search_api';
    $index_id = $this->info['instance'];
    return $base_path . '/index/' . $index_id . '/facets/' . $realm_name;
  }

  /**
   * Overrides FacetapiAdapter::getSearchPath().
   */
  public function getSearchPath() {
    $search = $this->getCurrentSearch();
    if ($search && $search[0]->getOption('search_api_base_path')) {
      return $search[0]->getOption('search_api_base_path');
    }
    return $_GET['q'];
  }

  /**
   * Allows the backend to initialize its query object before adding the facet filters.
   *
   * @param mixed $query
   *   The backend's native object.
   */
  public function initActiveFilters($query) {
    $search_id = $query->getOption('search id');
    $index_id = $this->info['instance'];
    $facets = facetapi_get_enabled_facets($this->info['name']);
    $this->fields = array();

    // We statically store the current search per facet so that we can correctly
    // assign it when building the facets. See the build() method in the query
    // type plugin classes.
    $active = &drupal_static('search_api_facetapi_active_facets', array());
    foreach ($facets as $facet) {
      $options = $this->getFacet($facet)->getSettings()->settings;
      // The 'default_true' option is a choice between "show on all but the
      // selected searches" (TRUE) and "show for only the selected searches".
      $default_true = isset($options['default_true']) ? $options['default_true'] : TRUE;
      // The 'facet_search_ids' option is the list of selected searches that
      // will either be excluded or for which the facet will exclusively be
      // displayed.
      $facet_search_ids = isset($options['facet_search_ids']) ? $options['facet_search_ids'] : array();

      // Remember this search ID, if necessary.
      $this->rememberSearchId($index_id, $search_id);

      if (array_search($search_id, $facet_search_ids) === FALSE) {
        if (!$default_true) {
          continue; // We are only to show facets for explicitly named search ids.
        }
      }
      elseif ($default_true) {
        continue; // The 'facet_search_ids' in the settings are to be excluded.
      }
      $active[$facet['name']] = $search_id;
      $this->fields[$facet['name']] = array(
        'field'             => $facet['field'],
        'limit'             => $options['hard_limit'],
        'operator'          => $options['operator'],
        'min_count'         => $options['facet_mincount'],
        'missing'           => $options['facet_missing'],
      );
    }
  }

  /**
   * Adds a search ID to the list of known searches for an index.
   *
   * @param string $index_id
   *   The machine name of the search index.
   * @param string $search_id
   *   The identifier of the executed search.
   */
  protected function rememberSearchId($index_id, $search_id) {
    $search_ids = variable_get('search_api_facets_search_ids', array());
    if (empty($search_ids[$index_id][$search_id])) {
      $search_ids[$index_id][$search_id] = $search_id;
      asort($search_ids[$index_id]);
      variable_set('search_api_facets_search_ids', $search_ids);
    }
  }

  /**
   * Add the given facet to the query.
   */
  public function addFacet(array $facet, SearchApiQueryInterface $query) {
    if (isset($this->fields[$facet['name']])) {
      $options = &$query->getOptions();
      $facet_info = $this->fields[$facet['name']];
      if (!empty($facet['query_options'])) {
        // Let facet-specific query options override the set options.
        $facet_info = $facet['query_options'] + $facet_info;
      }
      $options['search_api_facets'][$facet['name']] = $facet_info;
    }
  }

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return (bool) $this->getCurrentSearch();
  }

  /**
   * Helper method for getting a current search for this searcher.
   *
   * @return array
   *   The first matching current search, in the form specified by
   *   search_api_current_search(). Or NULL, if no match was found.
   */
  public function getCurrentSearch() {
    // Even if this fails once, there might be a search query later in the page
    // request. We therefore don't store anything in $this->current_search in
    // case of failure, but just try again if the method is called again.
    if (!isset($this->current_search)) {
      $index_id = $this->info['instance'];
      // There is currently no way to configure the "current search" block to
      // show on a per-searcher basis as we do with the facets. Therefore we
      // cannot match it up to the correct "current search".
      // I suspect that http://drupal.org/node/593658 would help.
      // For now, just taking the first current search for this index. :-/
      foreach (search_api_current_search() as $search) {
        list($query) = $search;
        if ($query->getIndex()->machine_name == $index_id) {
          $this->current_search = $search;
        }
      }
    }
    return $this->current_search;
  }

  /**
   * Returns a boolean flagging whether facets in a realm shoud be displayed.
   *
   * Useful, for example, for suppressing sidebar blocks in some cases.
   *
   * @return
   *   A boolean flagging whether to display a given realm.
   */
  public function suppressOutput($realm_name) {
    // Not sure under what circumstances the output will need to be suppressed?
    return FALSE;
  }

  /**
   * Returns the search keys.
   */
  public function getSearchKeys() {
    $search = $this->getCurrentSearch();
    $keys = $search[0]->getOriginalKeys();
    if (is_array($keys)) {
      // This will happen nearly never when displaying the search keys to the
      // user, so go with a simple work-around.
      // If someone complains, we can easily add a method for printing them
      // properly.
      $keys = '[' . t('complex query') . ']';
    }
    drupal_alter('search_api_facetapi_keys', $keys, $search[0]);
    return $keys;
  }

  /**
   * Returns the number of total results found for the current search.
   */
  public function getResultCount() {
    $search = $this->getCurrentSearch();
    // Each search is an array with the query as the first element and the results
    // array as the second.
    if (isset($search[1])) {
      return $search[1]['result count'];
    }
    return 0;
  }

  /**
   * Allows for backend specific overrides to the settings form.
   */
  public function settingsForm(&$form, &$form_state) {
    $facet = $form['#facetapi']['facet'];
    $facet_settings = $this->getFacet($facet)->getSettings();
    $options = $facet_settings->settings;
    $search_ids = variable_get('search_api_facets_search_ids', array());
    $search_ids = isset($search_ids[$this->info['instance']]) ? $search_ids[$this->info['instance']] : array();
    if (count($search_ids) > 1) {
      $form['global']['default_true'] = array(
        '#type' => 'select',
        '#title' => t('Display for searches'),
        '#prefix' => '<div class="facetapi-global-setting">',
        '#options' => array(
          TRUE => t('For all except the selected'),
          FALSE => t('Only for the selected'),
        ),
        '#default_value' => isset($options['default_true']) ? $options['default_true'] : TRUE,
      );
      $form['global']['facet_search_ids'] = array(
        '#type' => 'select',
        '#title' => t('Search IDs'),
        '#suffix' => '</div>',
        '#options' => $search_ids,
        '#size' => min(4, count($search_ids)),
        '#multiple' => TRUE,
        '#default_value' => isset($options['facet_search_ids']) ? $options['facet_search_ids'] : array(),
      );
    }
    else {
      $form['global']['default_true'] = array(
        '#type' => 'value',
        '#value' => TRUE,
      );
      $form['global']['facet_search_ids'] = array(
        '#type' => 'value',
        '#value' => array(),
      );
    }

    // Add a granularity option to date query types.
    if (isset($facet['query type']) && $facet['query type'] == 'date') {
      $granularity_options = array(
        FACETAPI_DATE_YEAR => t('Years'),
        FACETAPI_DATE_MONTH => t('Months'),
        FACETAPI_DATE_DAY => t('Days'),
        FACETAPI_DATE_HOUR => t('Hours'),
        FACETAPI_DATE_MINUTE => t('Minutes'),
        FACETAPI_DATE_SECOND => t('Seconds'),
      );

      $form['global']['date_granularity'] = array(
        '#type' => 'select',
        '#title' => t('Granularity'),
        '#description' => t('Determine the maximum drill-down level'),
        '#prefix' => '<div class="facetapi-global-setting">',
        '#suffix' => '</div>',
        '#options' => $granularity_options,
        '#default_value' => isset($options['date_granularity']) ? $options['date_granularity'] : FACETAPI_DATE_MINUTE,
      );

      // Date facets don't support the "OR" operator (for now).
      $form['global']['operator']['#access'] = FALSE;
    }

    // Add an "Exclude" option for terms.
    if(!empty($facet['query types']) && in_array('term', $facet['query types'])) {
      $form['global']['operator']['#weight'] = -2;
      unset($form['global']['operator']['#suffix']);
      $form['global']['exclude'] = array(
        '#type' => 'checkbox',
        '#title' => t('Exclude'),
        '#description' => t('Make the search exclude selected facets, instead of restricting it to them.'),
        '#suffix' => '</div>',
        '#weight' => -1,
        '#default_value' => !empty($options['exclude']),
      );
    }
  }
}
