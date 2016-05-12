<?php

/**
 * @file
 * Contains the SearchApiDataSourceControllerInterface as well as a default base class.
 */

/**
 * Interface for all data source controllers for Search API indexes.
 *
 * Data source controllers encapsulate all operations specific to an item type.
 * They are used for loading items, extracting item data, keeping track of the
 * item status, etc.
 *
 * Modules providing implementations of this interface that use a different way
 * (either different table or different method altogether) of keeping track of
 * indexed/dirty items than SearchApiAbstractDataSourceController should be
 * aware that indexes' numerical IDs can change due to feature reverts. It is
 * therefore recommended to use search_api_index_update_datasource(), or similar
 * code, in a hook_search_api_index_update() implementation.
 */
interface SearchApiDataSourceControllerInterface {

  /**
   * Constructs an SearchApiDataSourceControllerInterface object.
   *
   * @param string $type
   *   The item type for which this controller is created.
   */
  public function __construct($type);

  /**
   * Returns information on the ID field for this controller's type.
   *
   * @return array
   *   An associative array containing the following keys:
   *   - key: The property key for the ID field, as used in the item wrapper.
   *   - type: The type of the ID field. Has to be one of the types from
   *     search_api_field_types(). List types ("list<*>") are not allowed.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getIdFieldInfo();

  /**
   * Loads items of the type of this data source controller.
   *
   * @param array $ids
   *   The IDs of the items to load.
   *
   * @return array
   *   The loaded items, keyed by ID.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function loadItems(array $ids);

  /**
   * Creates a metadata wrapper for this datasource controller's type.
   *
   * @param mixed $item
   *   Unless NULL, an item of the item type for this controller to be wrapped.
   * @param array $info
   *   Optionally, additional information that should be used for creating the
   *   wrapper. Uses the same format as entity_metadata_wrapper().
   *
   * @return EntityMetadataWrapper
   *   A wrapper for the item type of this data source controller, according to
   *   the info array, and optionally loaded with the given data.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   *
   * @see entity_metadata_wrapper()
   */
  public function getMetadataWrapper($item = NULL, array $info = array());

  /**
   * Retrieves the unique ID of an item.
   *
   * @param mixed $item
   *   An item of this controller's type.
   *
   * @return mixed
   *   Either the unique ID of the item, or NULL if none is available.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getItemId($item);

  /**
   * Retrieves a human-readable label for an item.
   *
   * @param mixed $item
   *   An item of this controller's type.
   *
   * @return string|null
   *   Either a human-readable label for the item, or NULL if none is available.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getItemLabel($item);

  /**
   * Retrieves a URL at which the item can be viewed on the web.
   *
   * @param mixed $item
   *   An item of this controller's type.
   *
   * @return array|null
   *   Either an array containing the 'path' and 'options' keys used to build
   *   the URL of the item, and matching the signature of url(), or NULL if the
   *   item has no URL of its own.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getItemUrl($item);

  /**
   * Initializes tracking of the index status of items for the given indexes.
   *
   * All currently known items of this data source's type should be inserted
   * into the tracking table for the given indexes, with status "changed". If
   * items were already present, these should also be set to "changed" and not
   * be inserted again.
   *
   * @param SearchApiIndex[] $indexes
   *   The SearchApiIndex objects for which item tracking should be initialized.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function startTracking(array $indexes);

  /**
   * Stops tracking of the index status of items for the given indexes.
   *
   * The tracking tables of the given indexes should be completely cleared.
   *
   * @param SearchApiIndex[] $indexes
   *   The SearchApiIndex objects for which item tracking should be stopped.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function stopTracking(array $indexes);

  /**
   * Starts tracking the index status for the given items on the given indexes.
   *
   * @param array $item_ids
   *   The IDs of new items to track.
   * @param SearchApiIndex[] $indexes
   *   The indexes for which items should be tracked.
   *
   * @return SearchApiIndex[]|null
   *   All indexes for which any items were added; or NULL if items were added
   *   for all of them.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function trackItemInsert(array $item_ids, array $indexes);

  /**
   * Sets the tracking status of the given items to "changed"/"dirty".
   *
   * Unless $dequeue is set to TRUE, this operation is ignored for items whose
   * status is not "indexed".
   *
   * @param array|false $item_ids
   *   Either an array with the IDs of the changed items. Or FALSE to mark all
   *   items as changed for the given indexes.
   * @param SearchApiIndex[] $indexes
   *   The indexes for which the change should be tracked.
   * @param bool $dequeue
   *   (deprecated) If set to TRUE, also change the status of queued items.
   *   The concept of queued items will be removed in the Drupal 8 version of
   *   this module.
   *
   * @return SearchApiIndex[]|null
   *   All indexes for which any items were updated; or NULL if items were
   *   updated for all of them.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function trackItemChange($item_ids, array $indexes, $dequeue = FALSE);

  /**
   * Sets the tracking status of the given items to "queued".
   *
   * Queued items are not marked as "dirty" even when they are changed, and they
   * are not returned by the getChangedItems() method.
   *
   * @param array|false $item_ids
   *   Either an array with the IDs of the queued items. Or FALSE to mark all
   *   items as queued for the given indexes.
   * @param SearchApiIndex $index
   *   The index for which the items were queued.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   *
   * @deprecated
   *   As of Search API 1.10, the cron queue is not used for indexing anymore,
   *   therefore this method has become useless. It will be removed in the
   *   Drupal 8 version of this module.
   */
  public function trackItemQueued($item_ids, SearchApiIndex $index);

  /**
   * Sets the tracking status of the given items to "indexed".
   *
   * @param array $item_ids
   *   The IDs of the indexed items.
   * @param SearchApiIndex $index
   *   The index on which the items were indexed.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function trackItemIndexed(array $item_ids, SearchApiIndex $index);

  /**
   * Stops tracking the index status for the given items on the given indexes.
   *
   * @param array $item_ids
   *   The IDs of the removed items.
   * @param SearchApiIndex[] $indexes
   *   The indexes for which the deletions should be tracked.
   *
   * @return SearchApiIndex[]|null
   *   All indexes for which any items were deleted; or NULL if items were
   *   deleted for all of them.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function trackItemDelete(array $item_ids, array $indexes);

  /**
   * Retrieves a list of items that need to be indexed.
   *
   * If possible, completely unindexed items should be returned before items
   * that were indexed but later changed. Also, items that were changed longer
   * ago should be favored.
   *
   * @param SearchApiIndex $index
   *   The index for which changed items should be returned.
   * @param int $limit
   *   The maximum number of items to return. Negative values mean "unlimited".
   *
   * @return array
   *   The IDs of items that need to be indexed for the given index.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getChangedItems(SearchApiIndex $index, $limit = -1);

  /**
   * Retrieves information on how many items have been indexed for a certain index.
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
   *   If any error state was encountered.
   */
  public function getIndexStatus(SearchApiIndex $index);

  /**
   * Retrieves the entity type of items from this datasource.
   *
   * @return string|null
   *   An entity type string if the items provided by this datasource are
   *   entities; NULL otherwise.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function getEntityType();

  /**
   * Form constructor for configuring the datasource for a given index.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $form_state
   *   The form state. $form_state['index'] will contain the edited index. If
   *   this key is empty, then a new index is being created. In case of an edit,
   *   $form_state['index']->options['datasource'] contains the previous
   *   settings for the datasource.
   *
   * @return array|false
   *   A form array for configuring this callback, or FALSE if no configuration
   *   is possible.
   */
  public function configurationForm(array $form, array &$form_state);

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * This method will only be called if that form was non-empty.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state);

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * This method will only be called if that form was non-empty.
   *
   * Any necessary changes to the submitted values should be made, afterwards
   * they will automatically be stored as the index's "datasource" options. The
   * method can also be used by the datasource controller to react to the
   * possible change in its settings.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state);

  /**
   * Returns a summary of an index's current datasource configuration.
   *
   * @param SearchApiIndex $index
   *   The index whose datasource configuration should be summarized.
   *
   * @return string|null
   *   A translated string describing the index's current datasource
   *   configuration. Or NULL, if there is no configuration (or no description
   *   is available).
   */
  public function getConfigurationSummary(SearchApiIndex $index);

}

/**
 * Provides a default base class for datasource controllers.
 *
 * Contains default implementations for a number of methods which will be
 * similar for most data sources. Concrete data sources can decide to extend
 * this base class to save time, but can also implement the interface directly.
 *
 * A subclass will still have to provide implementations for the following
 * methods:
 * - getIdFieldInfo()
 * - loadItems()
 * - getMetadataWrapper() or getPropertyInfo()
 * - startTracking() or getAllItemIds()
 *
 * The table used by default for tracking the index status of items is
 * {search_api_item}. This can easily be changed, for example when an item type
 * has non-integer IDs, by changing the $table property.
 */
abstract class SearchApiAbstractDataSourceController implements SearchApiDataSourceControllerInterface {

  /**
   * The item type for this controller instance.
   */
  protected $type;

  /**
   * The entity type for this controller instance.
   *
   * @var string|null
   *
   * @see getEntityType()
   */
  protected $entityType = NULL;

  /**
   * The info array for the item type, as specified via
   * hook_search_api_item_type_info().
   *
   * @var array
   */
  protected $info;

  /**
   * The table used for tracking items. Set to NULL on subclasses to disable
   * the default tracking for an item type, or change the property to use a
   * different table for tracking.
   *
   * @var string
   */
  protected $table = 'search_api_item';

  /**
   * When using the default tracking mechanism: the name of the column on
   * $this->table containing the item ID.
   *
   * @var string
   */
  protected $itemIdColumn = 'item_id';

  /**
   * When using the default tracking mechanism: the name of the column on
   * $this->table containing the index ID.
   *
   * @var string
   */
  protected $indexIdColumn = 'index_id';

  /**
   * When using the default tracking mechanism: the name of the column on
   * $this->table containing the indexing status.
   *
   * @var string
   */
  protected $changedColumn = 'changed';

  /**
   * {@inheritdoc}
   */
  public function __construct($type) {
    $this->type = $type;
    $this->info = search_api_get_item_type_info($type);

    if (!empty($this->info['entity_type'])) {
      $this->entityType = $this->info['entity_type'];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getEntityType() {
    return $this->entityType;
  }

  /**
   * {@inheritdoc}
   */
  public function getMetadataWrapper($item = NULL, array $info = array()) {
    $info += $this->getPropertyInfo();
    return entity_metadata_wrapper($this->entityType ? $this->entityType : $this->type, $item, $info);
  }

  /**
   * Retrieves the property info for this item type.
   *
   * This is a helper method for getMetadataWrapper() that can be used by
   * subclasses to specify the property information to use when creating a
   * metadata wrapper.
   *
   * The data structure uses largely the format specified in
   * hook_entity_property_info(). However, the first level of keys (containing
   * the entity types) is omitted, and the "properties" key is called
   * "property info" instead. So, an example return value would look like this:
   *
   * @code
   * return array(
   *   'property info' => array(
   *     'foo' => array(
   *       'label' => t('Foo'),
   *       'type' => 'text',
   *     ),
   *     'bar' => array(
   *       'label' => t('Bar'),
   *       'type' => 'list<integer>',
   *     ),
   *   ),
   * );
   * @endcode
   *
   * SearchApiExternalDataSourceController::getPropertyInfo() contains a working
   * example of this method.
   *
   * If the item type is an entity type, no additional property information is
   * required, the method will thus just return an empty array. You can still
   * use this to append additional properties to the entities, or the like,
   * though.
   *
   * @return array
   *   Property information as specified by entity_metadata_wrapper().
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   *
   * @see getMetadataWrapper()
   * @see hook_entity_property_info()
   */
  protected function getPropertyInfo() {
    // If this is an entity type, no additional property info is needed.
    if ($this->entityType) {
      return array();
    }
    throw new SearchApiDataSourceException(t('No known property information for type @type.', array('@type' => $this->type)));
  }

  /**
   * {@inheritdoc}
   */
  public function getItemId($item) {
    $id_info = $this->getIdFieldInfo();
    $field = $id_info['key'];
    $wrapper = $this->getMetadataWrapper($item);
    if (!isset($wrapper->$field)) {
      return NULL;
    }
    $id = $wrapper->$field->value();
    return $id ? $id : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getItemLabel($item) {
    $label = $this->getMetadataWrapper($item)->label();
    return $label ? $label : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getItemUrl($item) {
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function startTracking(array $indexes) {
    if (!$this->table) {
      return;
    }
    // We first clear the tracking table for all indexes, so we can just insert
    // all items again without any key conflicts.
    $this->stopTracking($indexes);
    // Insert all items as new.
    $this->trackItemInsert($this->getAllItemIds(), $indexes);
  }

  /**
   * Returns the IDs of all items that are known for this controller's type.
   *
   * Helper method that can be used by subclasses instead of implementing
   * startTracking().
   *
   * @return array
   *   An array containing all item IDs for this type.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  protected function getAllItemIds() {
    throw new SearchApiDataSourceException(t('Items not known for type @type.', array('@type' => $this->type)));
  }

  /**
   * {@inheritdoc}
   */
  public function stopTracking(array $indexes) {
    if (!$this->table) {
      return;
    }
    // We could also use a single query with "IN" operator, but this method
    // will mostly be called with only one index.
    foreach ($indexes as $index) {
      $this->checkIndex($index);
      db_delete($this->table)
        ->condition($this->indexIdColumn, $index->id)
        ->execute();
    }
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemInsert(array $item_ids, array $indexes) {
    if (!$this->table || $item_ids === array()) {
      return;
    }

    foreach ($indexes as $index) {
      $this->checkIndex($index);
    }

    // Since large amounts of items can overstrain the database, only add items
    // in chunks.
    foreach (array_chunk($item_ids, 1000) as $chunk) {
      $insert = db_insert($this->table)
        ->fields(array($this->itemIdColumn, $this->indexIdColumn, $this->changedColumn));
      foreach ($chunk as $item_id) {
        foreach ($indexes as $index) {
          $insert->values(array(
            $this->itemIdColumn => $item_id,
            $this->indexIdColumn => $index->id,
            $this->changedColumn => 1,
          ));
        }
      }
      $insert->execute();
    }
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemChange($item_ids, array $indexes, $dequeue = FALSE) {
    if (!$this->table || $item_ids === array()) {
      return NULL;
    }

    $ret = array();

    foreach ($indexes as $index) {
      $this->checkIndex($index);
      $update = db_update($this->table)
        ->fields(array(
          $this->changedColumn => REQUEST_TIME,
        ))
        ->condition($this->indexIdColumn, $index->id)
        ->condition($this->changedColumn, 0, $dequeue ? '<=' : '=');
      if ($item_ids !== FALSE) {
        $update->condition($this->itemIdColumn, $item_ids, 'IN');
      }
      if ($update->execute()) {
        $ret[] = $index;
      }
    }

    return $ret;
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemQueued($item_ids, SearchApiIndex $index) {
    $this->checkIndex($index);
    if (!$this->table || $item_ids === array()) {
      return;
    }
    $update = db_update($this->table)
      ->fields(array(
        $this->changedColumn => -1,
      ))
      ->condition($this->indexIdColumn, $index->id);
    if ($item_ids !== FALSE) {
      $update->condition($this->itemIdColumn, $item_ids, 'IN');
    }
    $update->execute();
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemIndexed(array $item_ids, SearchApiIndex $index) {
    if (!$this->table || $item_ids === array()) {
      return;
    }
    $this->checkIndex($index);
    db_update($this->table)
      ->fields(array(
        $this->changedColumn => 0,
      ))
      ->condition($this->itemIdColumn, $item_ids, 'IN')
      ->condition($this->indexIdColumn, $index->id)
      ->execute();
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemDelete(array $item_ids, array $indexes) {
    if (!$this->table || $item_ids === array()) {
      return NULL;
    }

    $ret = array();

    foreach ($indexes as $index) {
      $this->checkIndex($index);
      $delete = db_delete($this->table)
        ->condition($this->indexIdColumn, $index->id)
        ->condition($this->itemIdColumn, $item_ids, 'IN');
      if ($delete->execute()) {
        $ret[] = $index;
      }
    }

    return $ret;
  }

  /**
   * {@inheritdoc}
   */
  public function getChangedItems(SearchApiIndex $index, $limit = -1) {
    if ($limit == 0) {
      return array();
    }
    $this->checkIndex($index);
    $select = db_select($this->table, 't');
    $select->addField('t', 'item_id');
    $select->condition($this->indexIdColumn, $index->id);
    $select->condition($this->changedColumn, 0, '>');
    $select->orderBy($this->changedColumn, 'ASC');
    if ($limit > 0) {
      $select->range(0, $limit);
    }
    return $select->execute()->fetchCol();
  }

  /**
   * {@inheritdoc}
   */
  public function getIndexStatus(SearchApiIndex $index) {
    if (!$this->table) {
      return array('indexed' => 0, 'total' => 0);
    }
    $this->checkIndex($index);
    $indexed = db_select($this->table, 'i')
      ->condition($this->indexIdColumn, $index->id)
      ->condition($this->changedColumn, 0)
      ->countQuery()
      ->execute()
      ->fetchField();
    $total = db_select($this->table, 'i')
      ->condition($this->indexIdColumn, $index->id)
      ->countQuery()
      ->execute()
      ->fetchField();
    return array('indexed' => $indexed, 'total' => $total);
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm(array $form, array &$form_state) {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigurationSummary(SearchApiIndex $index) {
    return NULL;
  }

  /**
   * Checks whether the given index is valid for this datasource controller.
   *
   * Helper method used by various methods in this class. By default only checks
   * whether the types match.
   *
   * @param SearchApiIndex $index
   *   The index to check.
   *
   * @throws SearchApiDataSourceException
   *   If the index doesn't fit to this datasource controller.
   */
  protected function checkIndex(SearchApiIndex $index) {
    if ($index->item_type != $this->type) {
      $index_type = search_api_get_item_type_info($index->item_type);
      $index_type = empty($index_type['name']) ? $index->item_type : $index_type['name'];
      $msg = t(
        'Invalid index @index of type @index_type passed to data source controller for type @this_type.',
        array('@index' => $index->name, '@index_type' => $index_type, '@this_type' => $this->info['name'])
      );
      throw new SearchApiDataSourceException($msg);
    }
  }

}
