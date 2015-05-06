<?php

/**
 * @file
 * Provide views handler for votingapi joins.
 */

/**
 * A custom join handler that connects arbitrary base tables to VotingAPI's data.
 *
 * The base relationship handler can only handle a single join. Some relationships
 * are more complex and might require chains of joins; for those, you must
 * utilize a custom relationship handler.
 *
 * Definition items:
 * - base: The new base table this relationship will be adding. This does not
 *   have to be a declared base table, but if there are no tables that
 *   utilize this base table, it won't be very effective.
 * - relationship table: The actual table this relationship operates against.
 *   This is analogous to using a 'table' override.
 * - relationship field: The actual field this relationsihp operates against.
 *   This is analogous to using a 'real field' override.
 * - label: The default label to provide for this relationship, which is
 *   shown in parentheses next to any field/sort/filter/argument that uses
 *   the relationship.
 */
class votingapi_views_handler_relationship extends views_handler_relationship {
  function option_definition() {
    $options = parent::option_definition();

    $label = !empty($this->definition['label']) ? $this->definition['label'] : $this->definition['title'];
    $options['label'] = array('default' => $label, 'translatable' => TRUE);

    $options['votingapi']['value_type'] = array('default' => NULL);
    $options['votingapi']['tag'] = array('default' => NULL);
    if ($this->definition['base'] == 'votingapi_cache') {
      $options['votingapi']['function'] = array('default' => NULL);
    }
    elseif ($this->definition['base'] == 'votingapi_vote') {
      $options['current_user'] = array('default' => FALSE);
    }

    return $options;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $options['value_types'][''] = t('No filtering');
    $options['tags'][''] = t('No filtering');
    $options['functions'][''] = t('No filtering');
    foreach (votingapi_metadata() as $bin => $bin_data) {
      foreach ($bin_data as $key => $data) {
        $options[$bin][$key] = $data['name'];
      }
    }

    $form['votingapi'] = array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#title' => t('Data filters'),
      '#description' => t('For each piece of content, many pieces of voting data may be saved. Use these options to specify exactly which types should be available via this relationship. <strong>Warning!</strong> Leaving any of these filters empty may result in multiple copies of each piece of content being displayed in listings.'),
      '#tree' => TRUE,
    );
    $form['votingapi']['value_type'] = array(
      '#title' => t('Value type'),
      '#type' => 'select',
      '#options' => $options['value_types'],
      '#default_value' => $this->options['votingapi']['value_type'],
    );
    $form['votingapi']['tag'] = array(
      '#title' => t('Vote tag'),
      '#type' => 'select',
      '#options' => $options['tags'],
      '#default_value' => $this->options['votingapi']['tag'],
    );

    if ($this->definition['base'] == 'votingapi_cache') {
      $form['votingapi']['function'] = array(
        '#title' => t('Aggregation function'),
        '#type' => 'select',
        '#options' => $options['functions'],
        '#default_value' => $this->options['votingapi']['function'],
      );
    }
    else {
      $form['current_user'] = array(
        '#title' => t('Restrict to current user'),
        '#type' => 'checkbox',
        '#return_value' => TRUE,
        '#default_value' => $this->options['current_user'],
      );
    }
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    // Figure out what base table this relationship brings to the party.
    $table_data = views_fetch_data($this->definition['base']);

    $def = $this->definition;
    $def['table'] = $this->definition['base'];
    $def['field'] = 'entity_id';
    $def['left_table'] = $this->table;
    $def['left_field'] = $this->field;
    if (!empty($this->options['required'])) {
      $def['type'] = 'INNER';
    }

    if (!empty($def['join_handler']) && class_exists($def['join_handler'])) {
      $join = new $def['join_handler'];
    }
    else {
      $join = new views_join();
    }

    // use a short alias for this:
    $alias = $def['table'] . '_' . $def['left_table'];

    if (!empty($this->options['votingapi'])) {
      foreach ($this->options['votingapi'] as $field => $value) {
        if (!empty($value)) {
          $def['extra'][] = array(
            'field' => $field,
            'value' => $value,
            'numeric' => FALSE
          );
          $alias .= '_' . str_replace(array(' ', '-', '.'), '_', $value);
        }
      }
    }

    if (!empty($this->options['current_user'])) {
      $def['extra'][] = array(
        'field' => 'uid',
        'value' => '***CURRENT_USER***',
        'numeric' => FALSE
      );
      $alias .= '_curuser';
    }

    $join->definition = $def;
    $join->construct();

    $this->ensure_my_table();

    $this->alias = $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
  }
}
