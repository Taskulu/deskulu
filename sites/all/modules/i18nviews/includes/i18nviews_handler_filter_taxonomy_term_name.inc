<?php

/**
 * @todo:
 * In general a string filter for i18n currently only supports equalility,
 * because joining tables is hard.
 */
class i18nviews_handler_filter_taxonomy_term_name extends views_handler_filter_string {
  function op_equal($field) {
    // Change the input of the word to the english source term.
    $source = i18nviews_locale_source($this->value, 'taxonomy');
    if (isset($source->source)) {
      $this->value = $source->source;
    }
    parent::op_equal($field);
  }

  /**
   * @todo: We could remove all operators which aren't full supported at the moment.
   */
}