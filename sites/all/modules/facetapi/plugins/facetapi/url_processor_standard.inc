<?php

/**
 * @file
 * The standard url processor class.
 */

/**
 * Url processor plugin that retrieves facet data from the query string.
 *
 * This plugin retrieves facet data from $_GET, and stored all information in
 * the "f" query string variable by default.
 */
class FacetapiUrlProcessorStandard extends FacetapiUrlProcessor {

  /**
   * Stores the "limit_active_items" settings for each facet.
   *
   * @var array
   */
  protected $limitActiveItems = array();

  /**
   * Implements FacetapiUrlProcessor::fetchParams().
   *
   * Use $_GET as the source for facet data.
   */
  public function fetchParams() {
    return $_GET;
  }

  /**
   * Implements FacetapiUrlProcessor::normalizeParams().
   *
   * Strips the "q" and "page" variables from the params array.
   */
  public function normalizeParams(array $params, $filter_key = 'f') {
    return drupal_get_query_parameters($params, array('q', 'page'));
  }

  /**
   * Implements FacetapiUrlProcessor::getQueryString().
   */
  public function getQueryString(array $facet, array $values, $active) {
    $qstring = $this->params;
    $active_items = $this->adapter->getActiveItems($facet);

    // Appends to qstring if inactive, removes if active.
    foreach ($values as $value) {
      if ($active && isset($active_items[$value])) {
        unset($qstring[$this->filterKey][$active_items[$value]['pos']]);
      }
      elseif (!$active) {
        $field_alias = rawurlencode($facet['field alias']);

        // Strips all other filters for this facet if limit option is set.
        if ($this->limitActiveItems($facet)) {
          foreach ($qstring[$this->filterKey] as $pos => $filter) {
            if (0 === strpos($filter, $field_alias)) {
              unset($qstring[$this->filterKey][$pos]);
            }
          }
        }

        // Adds the filter to the query string.
        $qstring[$this->filterKey][] = $field_alias . ':' . $value;
      }
    }

    // Removes duplicates, resets array keys and returns query string.
    // @see http://drupal.org/node/1340528
    $qstring[$this->filterKey] = array_values(array_unique($qstring[$this->filterKey]));
    return array_filter($qstring);
  }

  /**
   * Checks the facet's global "limit_active_items" settings.
   *
   * @return int
   *   Whether or not to limit active items to one per facet.
   */
  public function limitActiveItems(array $facet) {
    if (!isset($this->limitActiveItems[$facet['name']])) {
      $settings = $this->adapter->getFacetSettingsGlobal($facet);
      $this->limitActiveItems[$facet['name']] = $settings->settings['limit_active_items'];
    }
    return $this->limitActiveItems[$facet['name']];
  }

  /**
   * Implements FacetapiUrlProcessor::setBreadcrumb().
   */
  public function setBreadcrumb() {
    $breadcrumb = drupal_get_breadcrumb();

    // Gets search keys and active items form the adapter.
    $keys = $this->adapter->getSearchKeys();
    $active_items = $this->adapter->getAllActiveItems();

    $item = menu_get_item();

    // Initializes base breadcrumb query.
    $query = $this->params;
    unset($query[$this->filterKey]);

    // Adds the current search to the query.
    if ($keys) {
      // The last item should be text, not a link.
      $breadcrumb[] = $active_items ? l($keys, current_path(), array('query' => $query)) : check_plain($keys);
    }

    // Adds filters to the breadcrumb trail.
    $last = end($active_items);
    foreach ($active_items as $item) {
      $query[$this->filterKey][] = rawurlencode($item['field alias']) . ':' . $item['value'];

      // Replaces with the mapped value.
      $value = $this->adapter->getMappedValue($item['facets'][0], $item['value']);

      // The last item should be text, not a link.
      if ($last == $item) {
        $breadcrumb[] = !empty($value['#html']) ? $value['#markup'] : check_plain($value['#markup']);
      }
      else {
        // Appends the filter to the breadcrumb trail.
        $breadcrumb[] = l($value['#markup'], current_path(), array('query' => $query, 'html' => !empty($value['#html'])));
      }
    }

    // Sets the breadcrumb trail with the keys and filters.
    drupal_set_breadcrumb($breadcrumb);
  }
  /**
   * Allows for processor specific overrides to the settings form.
   */
  public function settingsForm(&$form, &$form_state) {
    $facet = $form['#facetapi']['facet'];
    $settings = $this->adapter->getFacetSettingsGlobal($facet);

    $form['global']['limit_active_items'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit to one active item'),
      '#prefix' => '<div class="facetapi-global-setting">',
      '#suffix' => '</div>',
      '#default_value' => !empty($settings->settings['limit_active_items']),
      '#description' => t('Enabling this option allows only one item to be active at a time.'),
    );
  }

  /**
   * Provides default values for the backend specific settings.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array(
      'limit_active_items' => 0
    );
  }
}
