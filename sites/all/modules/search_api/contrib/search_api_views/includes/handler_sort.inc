<?php

/**
 * @file
 * Contains SearchApiViewsHandlerSort.
 */

/**
 * Class for sorting results according to a specified field.
 */
class SearchApiViewsHandlerSort extends views_handler_sort {

  /**
   * The associated views query object.
   *
   * @var SearchApiViewsQuery
   */
  public $query;

  /**
   * Called to add the sort to a query.
   */
  public function query() {
    // When there are exposed sorts, the "exposed form" plugin will set
    // $query->orderby to an empty array. Therefore, if that property is set,
    // we here remove all previous sorts.
    if (isset($this->query->orderby)) {
      unset($this->query->orderby);
      $sort = &$this->query->getSort();
      $sort = array();
      unset($sort);
    }

    // If two of the same fields are used for sort, ignore the latter in order
    // for the prior to take precedence. (Temporary workaround until
    // https://www.drupal.org/node/2145547 is fixed in Views.)
    $alreadySorted = $this->query->getSort();
    if (is_array($alreadySorted) && isset($alreadySorted[$this->real_field])) {
      return;
    }

    try {
      $this->query->sort($this->real_field, $this->options['order']);
    }
    catch (SearchApiException $e) {
      $this->query->abort($e->getMessage());
    }
  }

}
