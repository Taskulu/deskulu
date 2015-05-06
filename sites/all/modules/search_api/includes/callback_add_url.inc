<?php

/**
 * @file
 * Contains SearchApiAlterAddUrl.
 */

/**
 * Search API data alteration callback that adds an URL field for all items.
 */
class SearchApiAlterAddUrl extends SearchApiAbstractAlterCallback {

  public function alterItems(array &$items) {
    foreach ($items as &$item) {
      $url = $this->index->datasource()->getItemUrl($item);
      if (!$url) {
        $item->search_api_url = NULL;
        continue;
      }
      $item->search_api_url = url($url['path'], array('absolute' => TRUE) + $url['options']);
    }
  }

  public function propertyInfo() {
    return array(
      'search_api_url' => array(
        'label' => t('URI'),
        'description' => t('An URI where the item can be accessed.'),
        'type' => 'uri',
      ),
    );
  }

}
