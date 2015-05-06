<?php

/**
 * @file
 * Contains the SearchApiAlterNodeStatus class.
 */

/**
 * Exclude unpublished nodes from node indexes.
 */
class SearchApiAlterNodeStatus extends SearchApiAbstractAlterCallback {

  /**
   * Check whether this data-alter callback is applicable for a certain index.
   *
   * Returns TRUE only for indexes on nodes.
   *
   * @param SearchApiIndex $index
   *   The index to check for.
   *
   * @return boolean
   *   TRUE if the callback can run on the given index; FALSE otherwise.
   */
  public function supportsIndex(SearchApiIndex $index) {
    return $index->getEntityType() === 'node';
  }

  /**
   * Alter items before indexing.
   *
   * Items which are removed from the array won't be indexed, but will be marked
   * as clean for future indexing.
   *
   * @param array $items
   *   An array of items to be altered, keyed by item IDs.
   */
  public function alterItems(array &$items) {
    foreach ($items as $nid => &$item) {
      if (empty($item->status)) {
        unset($items[$nid]);
      }
    }
  }

}
