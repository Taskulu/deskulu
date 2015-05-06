<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterText.
 */

/**
 * Views filter handler class for handling fulltext fields.
 */
class SearchApiViewsHandlerFilterText extends SearchApiViewsHandlerFilter {

  /**
   * Provide a list of options for the operator form.
   */
  public function operator_options() {
    return array('=' => t('contains'), '<>' => t("doesn't contain"));
  }

}
