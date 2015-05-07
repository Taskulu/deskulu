<?php

/**
 * Field handler to provide simple renderer that allows linking to a taxonomy
 * term.
 */
class i18nviews_handler_field_taxonomy_term_name extends views_handler_field_taxonomy {

  /**
   * Get the value that's supposed to be rendered.
   *
   * @param $values
   *   An object containing all retrieved values.
   * @param $field
   *   Optional name of the field where the value is stored.
   */
  function get_value($values, $field = NULL) {
    $alias = isset($field) ? $this->aliases[$field] : $this->field_alias;
    $alias_localized = $alias . '_i18n';
    if (isset($values->{$alias_localized})) {
      return $values->{$alias_localized};
    }
    return parent::get_value($values, $field);
  }

  /**
   * Translate the taxonomy term name.
   *
   * @param $values
   */
  function pre_render(&$values) {
    foreach ($values as $key => $value) {
      if (isset($value->{$this->field_alias})) {
        $term = new stdClass();
        $term->tid = $this->get_value($value, 'tid');
        $term->vid = $this->get_value($value, 'vid');
        $term->name = $this->get_value($value);
        $values[$key]->{$this->field_alias . '_i18n'} = i18n_taxonomy_term_name($term);
      }
    }
  }
}
