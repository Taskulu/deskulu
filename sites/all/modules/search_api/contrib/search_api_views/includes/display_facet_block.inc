<?php

/**
 * @file
 * Display plugin for displaying the search facets in a block.
 */

/**
 * Plugin class for displaying search facets in a block.
 */
class SearchApiViewsFacetsBlockDisplay extends views_plugin_display_block {
  public function displays_exposed() {
    return FALSE;
  }
  public function uses_exposed() {
    return FALSE;
  }

  public function option_definition() {
    $options = parent::option_definition();

    $options['linked_path'] = array('default' => '');
    $options['facet_field'] = '';
    $options['hide_block'] = FALSE;

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    if (substr($this->view->base_table, 0, 17) != 'search_api_index_') {
      return;
    }

    switch ($form_state['section']) {
      case 'linked_path':
        $form['#title'] .= t('Search page path');
        $form['linked_path'] = array(
          '#type' => 'textfield',
          '#description' => t('The menu path to which search facets will link. Leave empty to use the current path.'),
          '#default_value' => $this->get_option('linked_path'),
        );
        break;
      case 'facet_field':
        $form['facet_field'] = array(
          '#type' => 'select',
          '#title' => t('Facet field'),
          '#options' => $this->getFieldOptions(),
          '#default_value' => $this->get_option('facet_field'),
        );
        break;
      case 'use_more':
        $form['use_more']['#description'] = t('This will add a more link to the bottom of this view, which will link to the base path for the facet links.');
        $form['use_more_always'] = array(
          '#type' => 'value',
          '#value' => $this->get_option('use_more_always'),
        );
        break;
      case 'hide_block':
        $form['hide_block'] = array(
          '#type' => 'checkbox',
          '#title' => t('Hide block'),
          '#description' => t('Hide this block, but still execute the search. ' .
              'Can be used to show native Facet API facet blocks linking to the search page specified above.'),
          '#default_value' => $this->get_option('hide_block'),
        );
        break;
    }
  }

  public function options_validate(&$form, &$form_state) {
    if (substr($this->view->base_table, 0, 17) != 'search_api_index_') {
      form_set_error('', t('The "Facets block" display can only be used with base tables based on Search API indexes.'));
    }
  }

  public function options_submit(&$form, &$form_state) {
    parent::options_submit($form, $form_state);

    switch ($form_state['section']) {
      case 'linked_path':
        $this->set_option('linked_path', $form_state['values']['linked_path']);
        break;
      case 'facet_field':
        $this->set_option('facet_field', $form_state['values']['facet_field']);
        break;
      case 'hide_block':
        $this->set_option('hide_block', $form_state['values']['hide_block']);
        break;
    }
  }

  public function options_summary(&$categories, &$options) {
    parent::options_summary($categories, $options);

    $options['linked_path'] = array(
      'category' => 'block',
      'title' => t('Search page path'),
      'value' => $this->get_option('linked_path') ? $this->get_option('linked_path') : t('Use current path'),
    );
    $field_options = $this->getFieldOptions();
    $options['facet_field'] = array(
      'category' => 'block',
      'title' => t('Facet field'),
      'value' => $this->get_option('facet_field') ? $field_options[$this->get_option('facet_field')] : t('None'),
    );
    $options['hide_block'] = array(
      'category' => 'block',
      'title' => t('Hide block'),
      'value' => $this->get_option('hide_block') ? t('Yes') : t('No'),
    );
  }

  protected $field_options = NULL;

  protected function getFieldOptions() {
    if (!isset($this->field_options)) {
      $index_id = substr($this->view->base_table, 17);
      if (!($index_id && ($index = search_api_index_load($index_id)))) {
        $table = views_fetch_data($this->view->base_table);
        $table = empty($table['table']['base']['title']) ? $this->view->base_table : $table['table']['base']['title'];
        throw new SearchApiException(t('The "Facets block" display cannot be used with a view for @basetable. ' .
            'Please only use this display with base tables representing search indexes.',
            array('@basetable' => $table)));
      }
      $this->field_options = array();
      if (!empty($index->options['fields'])) {
        foreach ($index->getFields() as $key => $field) {
          $this->field_options[$key] = $field['name'];
        }
      }
    }
    return $this->field_options;
  }

  /**
   * Render the 'more' link
   */
  public function render_more_link() {
    if ($this->use_more()) {
      $path = $this->get_option('linked_path');
      $theme = views_theme_functions('views_more', $this->view, $this->display);
      $path = check_url(url($path, array()));

      return array(
        '#theme' => $theme,
        '#more_url' => $path,
        '#link_text' => check_plain($this->use_more_text()),
      );
    }
  }

  public function query(){
    parent::query();

    $facet_field = $this->get_option('facet_field');
    if (!$facet_field) {
      return NULL;
    }

    $base_path = $this->get_option('linked_path');
    if (!$base_path) {
      $base_path = $_GET['q'];
    }

    $limit = empty($this->view->query->pager->options['items_per_page']) ? 10 : $this->view->query->pager->options['items_per_page'];
    $query_options = &$this->view->query->getOptions();
    if (!$this->get_option('hide_block')) {
      // If we hide the block, we don't need this extra facet.
      $query_options['search_api_facets']['search_api_views_facets_block'] = array(
        'field' => $facet_field,
        'limit' => $limit,
        'missing' => FALSE,
        'min_count' => 1,
      );
    }
    $query_options['search_api_base_path'] = $base_path;
    $this->view->query->range(0, 0);
  }

  public function render() {
    if (substr($this->view->base_table, 0, 17) != 'search_api_index_') {
      form_set_error('', t('The "Facets block" display can only be used with base tables based on Search API indexes.'));
      return NULL;
    }
    $facet_field = $this->get_option('facet_field');
    if (!$facet_field) {
      return NULL;
    }

    $this->view->execute();

    if ($this->get_option('hide_block')) {
      return NULL;
    }

    $results = $this->view->query->getSearchApiResults();

    if (empty($results['search_api_facets']['search_api_views_facets_block'])) {
      return NULL;
    }
    $terms = $results['search_api_facets']['search_api_views_facets_block'];

    $filters = array();
    foreach ($terms as $term) {
      $filter = $term['filter'];
      if ($filter[0] == '"') {
        $filter = substr($filter, 1, -1);
      }
      elseif ($filter != '!') {
        // This is a range filter.
        $filter = substr($filter, 1, -1);
        $pos = strpos($filter, ' ');
        if ($pos !== FALSE) {
          $filter = '[' . substr($filter, 0, $pos) . ' TO ' . substr($filter, $pos + 1) . ']';
        }
      }
      $filters[$term['filter']] = $filter;
    }

    $index = $this->view->query->getIndex();
    $options['field'] = $index->options['fields'][$facet_field];
    $options['field']['key'] = $facet_field;
    $options['index id'] = $index->machine_name;
    $options['value callback'] = '_search_api_facetapi_facet_create_label';
    $map = search_api_facetapi_facet_map_callback($filters, $options);

    $facets = array();
    $prefix = rawurlencode($facet_field) . ':';
    foreach ($terms as $term) {
      $name = $filter = $filters[$term['filter']];
      if (isset($map[$filter])) {
        $name = $map[$filter];
      }
      $query['f'][0] = $prefix . $filter;

      // Initializes variables passed to theme hook.
      $variables = array(
        'text' => $name,
        'path' => $this->view->query->getOption('search_api_base_path'),
        'count' => $term['count'],
        'options' => array(
          'attributes' => array('class' => 'facetapi-inactive'),
          'html' => FALSE,
          'query' => $query,
        ),
      );

      // Themes the link, adds row to facets.
      $facets[] = array(
        'class' => array('leaf'),
        'data' => theme('facetapi_link_inactive', $variables),
      );
    }

    if (!$facets) {
      return NULL;
    }

    return array(
      'facets' => array(
      '#theme'  => 'item_list',
      '#items'  => $facets,
      )
    );
  }

  public function execute(){
    $info['content'] = $this->render();
    $info['content']['more'] = $this->render_more_link();
    $info['subject'] = filter_xss_admin($this->view->get_title());
    return $info;
  }

}
