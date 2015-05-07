<?php

/**
 * Field handler to provide simple renderer that allows linking to a taxonomy
 * term.
 */
class i18nviews_handler_field_taxonomy_term_description extends views_handler_field_markup {
  function construct() {
    parent::construct();
    $this->additional_fields['tid'] = 'tid';
  }

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
   * @param $values
   */
  function pre_render(&$values) {
    $tids = array();
    foreach ($values as $key => $value) {
      if (isset($value->{$this->field_alias})) {
        $tids[] = $this->get_value($value, 'tid');
      }
    }

    $terms = taxonomy_term_load_multiple($tids);
    $terms = i18n_taxonomy_localize_terms($terms);
    foreach ($values as $key => $value) {
      if (isset($value->{$this->field_alias})) {
        $tid = $this->get_value($value, 'tid');
        $values[$key]->{$this->field_alias . '_i18n'} = $terms[$tid]->description;
      }
    }
  }
}
