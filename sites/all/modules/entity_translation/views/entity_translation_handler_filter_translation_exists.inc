<?php
/**
 * @file
 * Contains an entity type filter handler.
 */

/**
 * This handler determines if a translation exists for a particular translation.
 */
class entity_translation_handler_filter_translation_exists extends views_handler_filter_locale_language {

  /**
   * Add a 'entity_type' option definition.
   * @see views_handler_field::option:definition()
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['entity_type'] = array('default' => '', 'translatable' => FALSE);
    $options['use_filter'] = array('default' => '', 'translatable' => FALSE);
    $options['filter'] = array('default' => '', 'translatable' => FALSE);
    return $options;
  }

  /**
   * Override the default title for the operators.
   */
  function operators() {
    $operators = parent::operators();
    $operators['in']['title'] = t('Translation exists');
    $operators['not in']['title'] = t('Translation doesn\'t exist');
    return $operators;
  }

  /**
   * Add option for setting entity type either directly or through a filter.
   * @see views_handler_field::options_form()
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $filters = $this->get_entity_type_filters();
    if (!empty($filters)) {
      $form['use_filter'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use an entity type filter.'),
        '#default_value' => $this->options['use_filter'],
      );
      $form['filter'] = array(
        '#type' => 'select',
        '#title' => t('Filter'),
        '#options' => $filters,
        '#dependency' => array(
          'edit-options-use-filter' => array(1)
        ),
        '#default_value' => $this->options['filter'],
      );
    }
    $form['entity_type'] = array(
      '#title' => t('Entity type'),
      '#type' => 'select',
      '#options' => $this->get_allowed_types(),
      '#dependency' => array(
        'edit-options-use-filter' => array(0)
      ),
      '#default_value' => $this->options['entity_type'],
      '#description' => t('You have to filter on a particular entity type when you use this filter'),
    );
  }

  /**
   * Get all available entity type filters that can be used to build the query.
   */
  function get_entity_type_filters() {
    // We need to build the query to know about the available fields.
    $this->view->build();
    $filters = array();
     foreach ($this->view->filter as $key => $filter) {
       // Break if we encounter our filter, the filter must be before this one.
       if ($filter == $this) {
         break;
       }
       if ($filter instanceof entity_translation_handler_filter_entity_type && count($filter->value) == 1 && empty($filter->options['expose']['multiple'])) {
        $filters[$key] = $filter->value_title;
      }
    }
    return $filters;
  }

  /**
   * Get entity types managed by entity translation.
   */
  function get_allowed_types() {
    $allowed_types_options = variable_get('entity_translation_entity_types');
    $allowed_types = array();
    $entity_info = entity_get_info();
    foreach ($allowed_types_options as $key => $allowed) {
      if ($allowed) {
        $allowed_types[$key] = $entity_info[$key]['label'];
      }
    }
    return $allowed_types;
  }

  /**
   * Override the default behaviour of the handler.
   */
  function query() {
    $this->ensure_my_table();
    // We need a subquery to determine not in.
    if ($this->operator == 'not in') {
      $entity_type = 'node';
      if ($this->options['use_filter'] && isset($this->view->filter[$this->options['filter']])) {
        $filter = $this->view->filter[$this->options['filter']];
        $entity_type = current($filter->value);
      }
      else {
        $this->query->add_where($this->options['group'], "$this->table_alias.entity_type", $this->options['entity_type'], '=');
        $entity_type = $this->options['entity_type'];
      }
      $query = db_select('entity_translation', 'es')
        ->condition('entity_type', $entity_type)
        ->condition('language', $this->value);
      $query->addField('es', 'entity_id');
      $this->query->add_where($this->options['group'], "$this->table_alias.entity_id", $query, $this->operator);
    }
    // We can determine if a translation exists without a subquery.
    else {
      $value = array_keys($this->value);
      $this->query->add_where($this->options['group'], "$this->table_alias.source", '', '<>');
      $this->query->add_where($this->options['group'], "$this->table_alias.language", array_values($this->value), $this->operator);
    }
  }
}
