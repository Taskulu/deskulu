<?php

/**
 * @file
 * Contains the SearchApiExternalDataSourceController class.
 */

/**
 * Base class for data source controllers for external data sources.
 *
 * This data source controller is a base implementation for item types that
 * represent external data, not directly accessible in Drupal. You can use this
 * controller as a base class when you don't want to index items of the type via
 * Drupal, but only want the search capabilities of the Search API. In addition
 * you most probably also have to create a fitting service class for executing
 * the actual searches.
 *
 * To use most of the functionality of the Search API and related modules, you
 * will only have to specify some property information in getPropertyInfo(). If
 * you have a custom service class which already returns the extracted fields
 * with the search results, you will only have to provide a label and a type for
 * each field.
 */
class SearchApiExternalDataSourceController extends SearchApiAbstractDataSourceController {

  /**
   * Return information on the ID field for this controller's type.
   *
   * This implementation will return a field named "id" of type "string". This
   * can also be used if the item type in question has no IDs.
   *
   * @return array
   *   An associative array containing the following keys:
   *   - key: The property key for the ID field, as used in the item wrapper.
   *   - type: The type of the ID field. Has to be one of the types from
   *     search_api_field_types(). List types ("list<*>") are not allowed.
   */
  public function getIdFieldInfo() {
    return array(
      'key' => 'id',
      'type' => 'string',
    );
  }

  /**
   * Load items of the type of this data source controller.
   *
   * Always returns an empty array. If you want the items of your type to be
   * loadable, specify a function here.
   *
   * @param array $ids
   *   The IDs of the items to laod.
   *
   * @return array
   *   The loaded items, keyed by ID.
   */
  public function loadItems(array $ids) {
    return array();
  }

  /**
   * Overrides SearchApiAbstractDataSourceController::getPropertyInfo().
   *
   * Only returns a single string ID field.
   */
  protected function getPropertyInfo() {
    $info['property info']['id'] = array(
      'label' => t('ID'),
      'type' => 'string',
    );

    return $info;
  }

  /**
   * Get the unique ID of an item.
   *
   * Always returns 1.
   *
   * @param $item
   *   An item of this controller's type.
   *
   * @return
   *   Either the unique ID of the item, or NULL if none is available.
   */
  public function getItemId($item) {
    return 1;
  }

  /**
   * Get a human-readable label for an item.
   *
   * Always returns NULL.
   *
   * @param $item
   *   An item of this controller's type.
   *
   * @return
   *   Either a human-readable label for the item, or NULL if none is available.
   */
  public function getItemLabel($item) {
    return NULL;
  }

  /**
   * Get a URL at which the item can be viewed on the web.
   *
   * Always returns NULL.
   *
   * @param $item
   *   An item of this controller's type.
   *
   * @return
   *   Either an array containing the 'path' and 'options' keys used to build
   *   the URL of the item, and matching the signature of url(), or NULL if the
   *   item has no URL of its own.
   */
  public function getItemUrl($item) {
    return NULL;
  }

  /**
   * Initialize tracking of the index status of items for the given indexes.
   *
   * All currently known items of this data source's type should be inserted
   * into the tracking table for the given indexes, with status "changed". If
   * items were already present, these should also be set to "changed" and not
   * be inserted again.
   *
   * @param array $indexes
   *   The SearchApiIndex objects for which item tracking should be initialized.
   *
   * @throws SearchApiDataSourceException
   *   If any of the indexes doesn't use the same item type as this controller.
   */
  public function startTracking(array $indexes) {
    return;
  }

  /**
   * Stop tracking of the index status of items for the given indexes.
   *
   * The tracking tables of the given indexes should be completely cleared.
   *
   * @param array $indexes
   *   The SearchApiIndex objects for which item tracking should be stopped.
   *
   * @throws SearchApiDataSourceException
   *   If any of the indexes doesn't use the same item type as this controller.
   */
  public function stopTracking(array $indexes) {
    return;
  }

  /**
   * Start tracking the index status for the given items on the given indexes.
   *
   * @param array $item_ids
   *   The IDs of new items to track.
   * @param array $indexes
   *   The indexes for which items should be tracked.
   *
   * @throws SearchApiDataSourceException
   *   If any of the indexes doesn't use the same item type as this controller.
   */
  public function trackItemInsert(array $item_ids, array $indexes) {
    return;
  }

  /**
   * Set the tracking status of the given items to "changed"/"dirty".
   *
   * @param $item_ids
   *   Either an array with the IDs of the changed items. Or FALSE to mark all
   *   items as changed for the given indexes.
   * @param array $indexes
   *   The indexes for which the change should be tracked.
   * @param $dequeue
   *   If set to TRUE, also change the status of queued items.
   *
   * @throws SearchApiDataSourceException
   *   If any of the indexes doesn't use the same item type as this controller.
   */
  public function trackItemChange($item_ids, array $indexes, $dequeue = FALSE) {
    return;
  }

  /**
   * Set the tracking status of the given items to "indexed".
   *
   * @param array $item_ids
   *   The IDs of the indexed items.
   * @param SearchApiIndex $indexes
   *   The index on which the items were indexed.
   *
   * @throws SearchApiDataSourceException
   *   If the index doesn't use the same item type as this controller.
   */
  public function trackItemIndexed(array $item_ids, SearchApiIndex $index) {
    return;
  }

  /**
   * Stop tracking the index status for the given items on the given indexes.
   *
   * @param array $item_ids
   *   The IDs of the removed items.
   * @param array $indexes
   *   The indexes for which the deletions should be tracked.
   *
   * @throws SearchApiDataSourceException
   *   If any of the indexes doesn't use the same item type as this controller.
   */
  public function trackItemDelete(array $item_ids, array $indexes) {
    return;
  }

  /**
   * Get a list of items that need to be indexed.
   *
   * If possible, completely unindexed items should be returned before items
   * that were indexed but later changed. Also, items that were changed longer
   * ago should be favored.
   *
   * @param SearchApiIndex $index
   *   The index for which changed items should be returned.
   * @param $limit
   *   The maximum number of items to return. Negative values mean "unlimited".
   *
   * @return array
   *   The IDs of items that need to be indexed for the given index.
   */
  public function getChangedItems(SearchApiIndex $index, $limit = -1) {
    return array();
  }

  /**
   * Get information on how many items have been indexed for a certain index.
   *
   * @param SearchApiIndex $index
   *   The index whose index status should be returned.
   *
   * @return array
   *   An associative array containing two keys (in this order):
   *   - indexed: The number of items already indexed in their latest version.
   *   - total: The total number of items that have to be indexed for this
   *     index.
   *
   * @throws SearchApiDataSourceException
   *   If the index doesn't use the same item type as this controller.
   */
  public function getIndexStatus(SearchApiIndex $index) {
    return array(
      'indexed' => 0,
      'total' => 0,
    );
  }

}
