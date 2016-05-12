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
   * Entity type info for this type.
   *
   * @var array
   */
  protected $entityInfo;

  /**
   * The ID key of this entity type, if any.
   *
   * @var string|null
   */
  protected $idKey;

  /**
   * The bundle key of this entity type, if any.
   *
   * @var string|null
   */
  protected $bundleKey;

  /**
   * Cached return values for getBundles(), keyed by index machine name.
   *
   * @var array
   */
  protected $bundles = array();

  /**
   * {@inheritdoc}
   */
  public function __construct($type) {
    parent::__construct($type);

    $this->entityInfo = entity_get_info($this->entityType);
    if (!empty($this->entityInfo['entity keys']['id'])) {
      $this->idKey = $this->entityInfo['entity keys']['id'];
    }
    if (!empty($this->entityInfo['entity keys']['bundle'])) {
      $this->bundleKey = $this->entityInfo['entity keys']['bundle'];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getIdFieldInfo() {
    $properties = entity_get_property_info($this->entityType);
    if (!$this->idKey) {
      throw new SearchApiDataSourceException(t("Entity type @type doesn't specify an ID key.", array('@type' => $this->entityInfo['label'])));
    }
    if (empty($properties['properties'][$this->idKey]['type'])) {
      throw new SearchApiDataSourceException(t("Entity type @type doesn't specify a type for the @prop property.", array('@type' => $this->entityInfo['label'], '@prop' => $this->idKey)));
    }
    $type = $properties['properties'][$this->idKey]['type'];
    if (search_api_is_list_type($type)) {
      throw new SearchApiDataSourceException(t("Entity type @type uses list field @prop as its ID.", array('@type' => $this->entityInfo['label'], '@prop' => $this->idKey)));
    }
    if ($type == 'token') {
      $type = 'string';
    }
    return array(
      'key' => $this->idKey,
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

    if (!empty($this->entityInfo['base table']) && $this->idKey) {
      // Use a subselect, which will probably be much faster than entity_load().

      // Assumes that all entities use the "base table" property and the
      // "entity keys[id]" in the same way as the default controller.
      $table = $this->entityInfo['base table'];

      // We could also use a single insert (with a UNION in the nested query),
      // but this method will be mostly called with a single index, anyways.
      foreach ($indexes as $index) {
        // Select all entity ids.
        $query = db_select($table, 't');
        $query->addField('t', $this->idKey, 'item_id');
        $query->addExpression(':index_id', 'index_id', array(':index_id' => $index->id));
        $query->addExpression('1', 'changed');
        if ($bundles = $this->getIndexBundles($index)) {
          $bundle_column = $this->bundleKey;
          if (!db_field_exists($table, $bundle_column)) {
            if ($this->entityType == 'taxonomy_term') {
              $bundle_column = 'vid';
              $bundles = db_query('SELECT vid FROM {taxonomy_vocabulary} WHERE machine_name IN (:bundles)', array(':bundles' => $bundles))->fetchCol();
            }
            elseif ($this->entityType == 'comment') {
              // Comments are significantly more complicated, since they don't
              // store their bundle explicitly in their database table. Instead,
              // we need to get all the nodes from the enabled types and filter
              // by those.
              $bundle_column = 'nid';
              $node_types = array();
              foreach ($bundles as $bundle) {
                if (substr($bundle, 0, 13) === 'comment_node_') {
                  $node_types[] = substr($bundle, 13);
                }
              }
              if ($node_types) {
                $bundles = db_query('SELECT nid FROM {node} WHERE type IN (:bundles)', array(':bundles' => $node_types))->fetchCol();
              }
              else {
                return;
              }
            }
            else {
              $this->startTrackingFallback(array($index->machine_name => $index));
            }
          }
          $query->condition($bundle_column, $bundles);
        }

        // INSERT ... SELECT ...
        db_insert($this->table)
          ->from($query)
          ->execute();
      }
    }
    else {
      $this->startTrackingFallback($indexes);
    }
  }

  /**
   * Initializes tracking of the index status of items for the given indexes.
   *
   * Fallback for when the items cannot directly be loaded into
   * {search_api_item} via "INSERT INTO … SELECT …".
   *
   * @param SearchApiIndex[] $indexes
   *   The indexes for which item tracking should be initialized.
   *
   * @throws SearchApiDataSourceException
   *   Thrown if any error state was encountered.
   *
   * @see SearchApiEntityDataSourceController::startTracking()
   */
  protected function startTrackingFallback(array $indexes) {
    // In the absence of a 'base table', use the slower way of retrieving the
    // items and inserting them "manually". For each index we get the item IDs
    // (since selected bundles might differ) and insert all of them as new.
    foreach ($indexes as $index) {
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', $this->entityType);
      if ($bundles = $this->getIndexBundles($index)) {
        $query->entityCondition('bundle', $bundles);
      }
      $result = $query->execute();
      $ids = !empty($result[$this->entityType]) ? array_keys($result[$this->entityType]) : array();
      if ($ids) {
        $this->trackItemInsert($ids, array($index));
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function trackItemInsert(array $item_ids, array $indexes) {
    $ret = array();

    foreach ($indexes as $index_id => $index) {
      $ids = $item_ids;
      if ($bundles = $this->getIndexBundles($index)) {
        $ids = drupal_map_assoc($ids);
        foreach (entity_load($this->entityType, $ids) as $id => $entity) {
          if (empty($bundles[$entity->{$this->bundleKey}])) {
            unset($ids[$id]);
          }
        }
      }
      if ($ids) {
        parent::trackItemInsert($ids, array($index));
        $ret[$index_id] = $index;
      }
    }

    return $ret;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm(array $form, array &$form_state) {
    $options = $this->getAvailableBundles();
    if (!$options) {
      return FALSE;
    }
    $form['bundles'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Bundles'),
      '#description' => t('Restrict the entity bundles that will be included in this index. Leave blank to include all bundles. This setting cannot be changed for existing indexes.'),
      '#options' => array_map('check_plain', $options),
      '#attributes' => array('class' => array('search-api-checkboxes-list')),
      '#disabled' => !empty($form_state['index']),
    );
    if (!empty($form_state['index']->options['datasource'])) {
      $form['bundles']['#default_value'] = drupal_map_assoc($form_state['index']->options['datasource']['bundles']);
    }
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    if (!empty($values['bundles'])) {
      $values['bundles'] = array_keys(array_filter($values['bundles']));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigurationSummary(SearchApiIndex $index) {
    if ($bundles = $this->getIndexBundles($index)) {
      $args['!bundles'] = implode(', ', array_intersect_key($this->getAvailableBundles(), $bundles));
      return format_plural(count($bundles), 'Indexed bundle: !bundles.', 'Indexed bundles: !bundles.', $args);
    }
    return NULL;
  }

  /**
   * Retrieves the available bundles for this entity type.
   *
   * @return array
   *   An array (which might be empty) mapping this entity type's bundle keys to
   *   their labels.
   */
  protected function getAvailableBundles() {
    if (!$this->bundleKey || empty($this->entityInfo['bundles'])) {
      return array();
    }
    $bundles = array();
    foreach ($this->entityInfo['bundles'] as $bundle => $bundle_info) {
      $bundles[$bundle] = isset($bundle_info['label']) ? $bundle_info['label'] : $bundle;
    }
    return $bundles;
  }

  /**
   * Computes the bundles that should be indexed for an index.
   *
   * @param SearchApiIndex $index
   *   The index for which to check.
   *
   * @return array
   *   An array containing all bundles that should be included in this index, as
   *   both the keys and values. An empty array means all current bundles should
   *   be included.
   *
   * @throws SearchApiException
   *   If the index doesn't belong to this datasource controller.
   */
  protected function getIndexBundles(SearchApiIndex $index) {
    $this->checkIndex($index);

    if (!isset($this->bundles[$index->machine_name])) {
      $this->bundles[$index->machine_name] = array();
      if (!empty($index->options['datasource']['bundles'])) {
        // We retrieve the available bundles here to check whether all of them
        // are included by the index's setting. In this case, we return an empty
        // array, too, to save on complexity.
        // On the other hand, we still want to return deleted bundles since we
        // do not want to suddenly include all bundles when all selected bundles
        // were deleted.
        $available = $this->getAvailableBundles();
        foreach ($index->options['datasource']['bundles'] as $bundle) {
          $this->bundles[$index->machine_name][$bundle] = $bundle;
          unset($available[$bundle]);
        }
        if (!$available) {
          $this->bundles[$index->machine_name] = array();
        }
      }
    }

    return $this->bundles[$index->machine_name];
  }

}
