<?php

/**
 * @file
 * Render an importer name.
 */

class feeds_views_handler_field_importer_name extends views_handler_field {

  /**
   * Overrides parent::option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['link'] = array('default' => 0);
    return $options;
  }

  /**
   * Overrides parent::options_form().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = array(
      0 => t('Do not link'),
      1 => t('Link to configuration form'),
      2 => t('Link to standalone form'),
    );
    $form['link'] = array(
      '#type' => 'radios',
      '#title' => t('Link importer name'),
      '#description' => t('If "Link to standalone form" is used, only importers that use a standalone form will be linked.'),
      '#default_value' => isset($this->options['link']) ? $this->options['link'] : FALSE,
      '#options' => $options,
    );
  }

  /**
   * Overrides parent::render().
   */
  function render($values) {
    try {
      $importer = feeds_importer($values->{$this->field_alias})->existing();
      if ($this->options['link'] == 1) {
        return l($importer->config['name'], 'admin/structure/feeds/' . $importer->id);
      }
      elseif ($this->options['link'] == 2 && empty($importer->config['content_type'])) {
        return l($importer->config['name'], 'import/' . $importer->id);
      }
      return check_plain($importer->config['name']);
    }
    catch (Exception $e) {
      return t('Missing importer');
    }
  }

  /**
   * Disallows advanced rendering.
   */
  function allow_advanced_render() {
    return FALSE;
  }
}
