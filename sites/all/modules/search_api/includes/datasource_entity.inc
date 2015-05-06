<?php

/**
 * @file
 * Contains the SearchApiEntityDataSourceController class.
 */

/**
 * Represents a datasource for all entities known to the Entity API.
 */
class SearchApiEntityDataSourceController extends SearchApiAbstractDataSourceController {

  /**
   * {@inheritdoc}
   */
  public function getIdFieldInfo() {
    $info = entity_get_info($this->entityType);
    $properties = entity_get_property_info($this->entityType);
    if (empty($info['entity keys']['id'])) {
      throw new SearchApiDataSourceException(t("Entity type @type doesn't specify an ID key.", array('@type' => $info['label'])));
    }
    $field = $info['entity keys']['id'];
    if (empty($properties['properties'][$field]['type'])) {
      throw new SearchApiDataSourceException(t("Entity type @type doesn't specify a type for the @prop property.", array('@type' => $info['label'], '@prop' => $field)));
    }
    $type = $properties['properties'][$field]['type'];
    if (search_api_is_list_type($type)) {
      throw new SearchApiDataSourceException(t("Entity type @type uses list field @prop as its ID.", array('@type' => $info['label'], '@prop' => $field)));
    }
    if ($type == 'token') {
      $type = 'string';
    }
    return array(
      'key' => $field,
      'type' => $type,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function loadItems(array $ids) {
    $items = entity_load($this->entityType, $ids);
    // If some items couldn't be loaded, remove them from tracking.
    if (count($items) != count($ids)) {
      $ids = array_flip($ids);
      $unknown = array_keys(array_diff_key($ids, $items));
      if ($unknown) {
        search_api_track_item_delete($this->type, $unknown);
      }
    }
    return $items;
  }

  /**
   * {@inheritdoc}
   */
  public function getMetadataWrapper($item = NULL, array $info = array()) {
    return entity_metadata_wrapper($this->entityType, $item, $info);
  }

  /**
   * {@inheritdoc}
   */
  public function getItemId($item) {
    $id = entity_id($this->entityType, $item);
    return $id ? $id : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getItemLabel($item) {
    $label = entity_label($this->entityType, $item);
    return $label ? $label : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getItemUrl($item) {
    if ($this->entityType == 'file') {
      return array(
        'path' => file_create_url($item->uri),
        'options' => array(
          'entity_type' => 'file',
          'entity' => $item,
        ),
      );
    }
    $url = entity_uri($this->entityType, $item);
    return $url ? $url : NULL;
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

    $entity_info = entity_get_info($this->entityType);

    if (!empty($entity_info['base table'])) {
      // Use a subselect, which will probably be much faster than entity_load().

      // Assumes that all entities use the "base table" property and the
      // "entity keys[id]" in the same way as the default controller.
      $id_field = $entity_info['entity keys']['id'];
      $table = $entity_info['base table'];

      // We could also use a single insert (with a JOIN in the nested query),
      // but this method will be mostly called with a single index, anyways.
      foreach ($indexes as $index) {
        // Select all entity ids.
        $query = db_select($table, 't');
        $query->addField('t', $id_field, 'item_id');
        $query->addExpression(':index_id', 'index_id', array(':index_id' => $index->id));
        $query->addExpression('1', 'changed');

        // INSERT ... SELECT ...
        db_insert($this->table)
          ->from($query)
          ->execute();
      }
    }
    else {
      // In the absence of a 'base table', use the slow entity_load().
      parent::startTracking($indexes);
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function getAllItemIds() {
    return array_keys(entity_load($this->entityType));
  }

}
