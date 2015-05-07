<?php

/**
 * Field handler to provide simple renderer that shows term language.
 */
class i18nviews_handler_field_taxonomy_term_language extends views_handler_field_taxonomy {
  function render($values) {
    $languages = locale_language_list();
    $value = isset($languages[$values->{$this->field_alias}]) ? $languages[$values->{$this->field_alias}] : '';
    $value = $this->get_value($values);
    return $this->render_link($this->sanitize_value($value), $values);
  }
}
