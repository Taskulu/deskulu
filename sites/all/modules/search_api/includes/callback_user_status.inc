<?php

/**
 * @file
 * Contains the SearchApiAlterUserStatus class.
 */

/**
 * Filters out blocked user accounts.
 */
class SearchApiAlterUserStatus extends SearchApiAbstractAlterCallback {

  /**
   * {@inheritdoc}
   */
  public function supportsIndex(SearchApiIndex $index) {
    return $index->getEntityType() == 'user';
  }

  /**
   * {@inheritdoc}
   */
  public function alterItems(array &$items) {
    foreach ($items as $id => $account) {
      if (empty($account->status)) {
        unset($items[$id]);
      }
    }
  }

}
