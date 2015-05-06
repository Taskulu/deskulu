<?php

/**
 * @file
 * Views handler.
 */

/**
 * Filter by tag id.
 * Largely copied from views_handler_filter_term_node_tid.inc
 */
class views_handler_filter_tag_font_tid extends views_handler_filter_many_to_one {

  function has_extra_options() {
    return TRUE;
  } // has_extra_options

  function get_value_options() { }

  function option_definition() {

    $options = parent::option_definition();

    $options['type'] = array('default' => 'textfield');
    $options['tag_type'] = array('default' => '*');

    return $options;

  } // option_definition

  function extra_options_form(&$form, &$form_state) {

    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Selection type'),
      '#options' => array('select' => t('Dropdown'), 'textfield' => t('Autocomplete')),
      '#default_value' => $this->options['type'],
    );

    $type_options = array(
      '*' =>  '-- Any --'
    );
    $type_results = db_query('SELECT DISTINCT type FROM {fontyourface_tag}');

    foreach ($type_results as $type_result) {
      $type_options[$type_result->type] = $type_result->type;
    } // foreach

    $form['tag_type'] = array(
      '#type' => 'radios',
      '#title' => t('Tag Type'),
      '#options' => $type_options,
      '#description' => t('Limit which type of tags to show options.'),
      '#default_value' => $this->options['tag_type'],
    );

  } // extra_options_form

  function value_form(&$form, &$form_state) {

    if ($this->options['type'] == 'textfield') {

      $default = '';

      if ($this->value) {

        $result = db_select('fontyourface_tag', 't')
          ->condition('t.tid', $this->value, 'IN')
          ->execute();

        foreach ($result as $tag) {

          if ($default) {
            $default .= ', ';
          } // if

          $default .= $tag->name;

        } // foreach

      } // if

      $form['value'] = array(
        '#title' => t('Select tags'),
        '#type' => 'textfield',
        '#default_value' => $default,
      );

    } // if
    else {

      $options = array();

      $query = db_select('fontyourface_tag', 't');
      $query
        ->fields('t')
        ->orderBy('t.name');

      if (isset($this->options['tag_type']) && $this->options['tag_type'] != '*') {
        $query->condition('t.type', $this->options['tag_type']);
      } // if

      $result = $query->execute();

      while ($tag = $result->fetchObject()) {
        $options[$tag->tid] = $tag->name;
      } // while

      $default_value = (array) $this->value;

      if (!empty($form_state['exposed'])) {

        $identifier = $this->options['expose']['identifier'];

        if (!empty($this->options['expose']['reduce'])) {

          $options = $this->reduce_value_options($options);

          if (empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
            $default_value = array();
          } // if

        } // if

        if (!empty($this->options['expose']['single'])) {

          if (!empty($this->options['expose']['optional']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
            $default_value = 'All';
          } // if
          elseif (empty($default_value)) {
            $keys = array_keys($options);
            $default_value = array_shift($keys);
          } // elseif
          else {
            $copy = $default_value;
            $default_value = array_shift($copy);
          } // else

        } // if

      } // if

      $form['value'] = array(
        '#type' => 'select',
        '#title' => t('Select tags'),
        '#multiple' => TRUE,
        '#options' => $options,
        '#size' => min(9, count($options)),
        '#default_value' => $default_value,
      );

      if (!empty($form_state['exposed']) && !isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $default_value;
      } // if

    } // else

    if (empty($form_state['exposed'])) {
      $this->helper->options_form($form, $form_state);
    } // if

  } // value_form

  function value_validate($form, &$form_state) {

    if ($this->options['type'] != 'textfield') {
      return;
    } // if

    $values = drupal_explode_tags($form_state['values']['options']['value']);
    $tids = $this->validate_tag_strings($form['value'], $values);

    if ($tids) {
      $form_state['values']['options']['value'] = $tids;
    } // if

  } // value_validate

  function accept_exposed_input($input) {

    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    // If it's optional and there's no value don't bother filtering.
    if (isset($this->options['expose']['optional']) && $this->options['expose']['optional'] && empty($this->validated_exposed_input)) {
      return FALSE;
    }

    $rc = parent::accept_exposed_input($input);

    if ($rc) {

      // If we have previously validated input, override.
      if (isset($this->validated_exposed_input)) {
        $this->value = $this->validated_exposed_input;
      } // if

    } // if

    return $rc;

  } // value_validate

  function exposed_validate(&$form, &$form_state) {

    if (empty($this->options['exposed'])) {
      return;
    } // if

    $identifier = $this->options['expose']['identifier'];

    if ($this->options['type'] != 'textfield') {

      if ($form_state['values'][$identifier] != 'All') {
        $this->validated_exposed_input = (array) $form_state['values'][$identifier];
      } // if

      return;

    } // if

    if (empty($this->options['expose']['identifier'])) {
      return;
    } // if

    $values = drupal_explode_tags($form_state['values'][$identifier]);

    $tids = $this->validate_tag_strings($form[$identifier], $values);

    if ($tids) {
      $this->validated_exposed_input = $tids;
    } // if

  } // exposed_validate

  /**
   * Validate the user string. Since this can come from either the form
   * or the exposed filter, this is abstracted out a bit so it can
   * handle the multiple input sources.
   */
  function validate_tag_strings(&$form, $values) {

    if (empty($values)) {
      return array();
    } // if

    $tids = array();
    $args = array();
    $results = array();

    foreach ($values as $value) {
      $missing[strtolower($value)] = TRUE;
      $args[] = $value;
    } // foreach

    if (!$args) {
      return;
    } // if

    $result = db_select('fontyourface_tag', 't')
      ->condition('t.name', $args, 'IN')
      ->execute();

    foreach ($result as $tag) {
      unset($missing[strtolower($tag->name)]);
      $tids[] = $tag->tid;
    } // foreach

    if ($missing) {
      form_error($form, format_plural(count($missing), 'Unable to find tag: @tags', 'Unable to find tags: @tags', array('@tags' => implode(', ', array_keys($missing)))));
    } // if

    return $tids;

  } // validate_tag_strings

  function value_submit($form, &$form_state) { }

  function expose_form_right(&$form, &$form_state) {

    parent::expose_form_right($form, $form_state);

    if ($this->options['type'] != 'select') {
      unset($form['expose']['reduce']);
    } // if

  } // expose_form_right

  function admin_summary() {

    $this->value_options = array();

    if ($this->value) {

      $result = db_select('fontyourface_tag', 't')
        ->condition('t.tid', $this->value, 'IN')
        ->execute();

      foreach ($result as $tag) {
        $this->value_options[$tag->tid] = $tag->name;
      } // foreach

    } // if

    return parent::admin_summary();

  } // admin_summary

} // views_handler_filter_tag_font_tid
