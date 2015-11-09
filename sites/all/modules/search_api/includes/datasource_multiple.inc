<?php

/**
 * @file
 * Contains SearchApiCombinedEntityDataSourceController.
 */

/**
 * Provides a datasource for indexing multiple types of entities.
 */
class SearchApiCombinedEntityDataSourceController extends SearchApiAbstractDataSourceController {

  /**
   * {@inheritdoc}
   */
  protected $table = 'search_api_item_string_id';

  /**
   * {@inheritdoc}
   */
  public function getIdFieldInfo() {
    return array(
      'key' => 'item_id',
      'type' => 'string',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function loadItems(array $ids) {
    $ids_by_type = array();
    foreach ($ids as $id) {
      list($type, $entity_id) = explode('/', $id);
      $ids_by_type[$type][$entity_id] = $id;
    }

    $items = array();
    foreach ($ids_by_type as $type => $type_ids) {
      foreach (entity_load($type, array_keys($type_ids)) as $entity_id => $entity) {
        $id = $type_ids[$entity_id];
        $item = (object) array($type => $entity);
        $item->item_id = $id;
        $item->item_type = $type;
        $item->item_entity_id = $entity_id;
        $item->item_bundle = NULL;
        try {
          list(, , $bundle) = entity_extract_ids($type, $entity);
          $item->item_bundle = $bundle ? "$type:$bundle" : NULL;
        }
        catch (EntityMalformedException $e) {
          // Will probably make problems at some other place, but for extracting
          // the bundle it is really not critical enough to fail on – just
          // ignore this exception.
        }
        $items[$id] = $item;
        unset($type_ids[$entity_id]);
      }
      if ($type_ids) {
        search_api_track_item_delete($type, array_keys($type_ids));
      }
    }

    return $items;
  }

  /**
   * {@inheritdoc}
   */
  protected function getPropertyInfo() {
    $info = array(
      'item_id' => array(
        'label' => t('ID'),
        'description' => t('The combined ID of the item, containing both entity type and entity ID.'),
        'type' => 'token',
      ),
      'item_type' => array(
        'label' => t('Entity type'),
        'description' => t('The entity type of the item.'),
        'type' => 'token',
        'options list' => 'search_api_entity_type_options_list',
      ),
      'item_entity_id' => array(
        'label' => t('Entity ID'),
        'description' => t('The entity ID of the item.'),
        'type' => 'token',
      ),
      'item_bundle' => array(
        'label' => t('Bundle'),
        'description' => t('The bundle of the item, if applicable.'),
        'type' => 'token',
        'options list' => 'search_api_combined_bundle_options_list',
      ),
      'item_label' => array(
        'label' => t('Label'),
        'description' => t('The label of the item.'),
        'type' => 'text',
        // Since this needs a bit more computation than the others, we don't
        // include it always when loading the item but use a getter callback.
        'getter callback' => 'search_api_get_multi_type_item_label',
      ),
    );

    foreach ($this->getSelectedEntityTypeOptions() as $type => $label) {
      $info[$type] = array(
        'label' => $label,
        'description' => t('The indexed entity, if it is of type %type.', array('%type' => $label)),
        'type' => $type,
      );
    }

    return array('property info' => $info);
  }

  /**
   * {@inheritdoc}
   */
  public function getItemId($item) {
    return isset($item->item_id) ? $item->item_id : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getItemLabel($item) {
    return search_api_get_multi_type_item_label($item);
  }

  /**
   * {@inheritdoc}
   */
  public function getItemUrl($item) {
    if ($item->item_type == 'file') {
      return array(
        'path' => file_create_url($item->file->uri),
        'options' => array(
          'entity_type' => 'file',
          'entity' => $item,
        ),
      );
    }
    $url = entity_uri($item->item_type, $item->{$item->item_type});
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

    foreach ($indexes as $index) {
      $types = $this->getEntityTypes($index);

      // Wherever possible, use a sub-select instead of the much slower
      // entity_load().
      foreach ($types as $type) {
        $entity_info = entity_get_info($type);

        if (!empty($entity_info['base table'])) {
          // Assumes that all entities use the "base table" property and the
          // "entity keys[id]" in the same way as the default controller.
          $id_field = $entity_info['entity keys']['id'];
          $table = $entity_info['base table'];

          // Select all entity ids.
          $query = db_select($table, 't');
          $query->addExpression("CONCAT(:prefix, t.$id_field)", 'item_id', array(':prefix' => $type . '/'));
          $query->addExpression(':index_id', 'index_id', array(':index_id' => $index->id));
          $query->addExpression('1', 'changed');

          // INSERT ... SELECT ...
          db_insert($this->table)
            ->from($query)
            ->execute();

          unset($types[$type]);
        }
      }

      // In the absence of a "base table", use the slow entity_load().
      if ($types) {
        foreach ($types as $type) {
          $query = new EntityFieldQuery();
          $query->entityCondition('entity_type', $type);
          $result = $query->execute();
          $ids = !empty($result[$type]) ? array_keys($result[$type]) : array();
          if ($ids) {
            foreach ($ids as $i => $id) {
              $ids[$i] = $type . '/' . $id;
            }
            $this->trackItemInsert($ids, array($index), TRUE);
          }
        }
      }
    }
  }

  /**
   * Starts tracking the index status for the given items on the given indexes.
   *
   * @param array $item_ids
   *   The IDs of new items to track.
   * @param SearchApiIndex[] $indexes
   *   The indexes for which items should be tracked.
   * @param bool $skip_type_check
   *   (optional) If TRUE, don't check whether the type matches the index's
   *   datasource configuration. Internal use only.
   *
   * @return SearchApiIndex[]|null
   *   All indexes for which any items were added; or NULL if items were added
   *   for all of them.
   *
   * @throws SearchApiDataSourceException
   *   If any error state was encountered.
   */
  public function trackItemInsert(array $item_ids, array $indexes, $skip_type_check = FALSE) {
    $ret = array();

    foreach ($indexes as $index_id => $index) {
      $ids = drupal_map_assoc($item_ids);

      if (!$skip_type_check) {
        $types = $this->getEntityTypes($index);
        foreach ($ids as $id) {
          list($type) = explode('/', $id);
          if (!isset($types[$type])) {
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
    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Entity types'),
      '#description' => t('Select the entity types which should be included in this index.'),
      '#options' => search_api_entity_type_options_list(),
      '#attributes' => array('class' => array('search-api-checkboxes-list')),
      '#disabled' => !empty($form_state['index']),
      '#required' => TRUE,
    );
    if (!empty($form_state['index']->options['datasource']['types'])) {
      $form['types']['#default_value'] = $this->getEntityTypes($form_state['index']);
    }
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    if (!empty($values['types'])) {
      $values['types'] = array_keys(array_filter($values['types']));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigurationSummary(SearchApiIndex $index) {
    if ($type_labels = $this->getSelectedEntityTypeOptions($index)) {
      $args['!types'] = implode(', ', $type_labels);
      return format_plural(count($type_labels), 'Indexed entity types: !types.', 'Indexed entity types: !types.', $args);
    }
    return NULL;
  }

  /**
   * Retrieves the index for which the current method was called.
   *
   * Very ugly method which uses the stack trace to find the right object.
   *
   * @return SearchApiIndex
   *   The active index.
   *
   * @throws SearchApiException
   *   Thrown if the active index could not be determined.
   */
  protected function getCallingIndex() {
    foreach (debug_backtrace() as $trace) {
      if (isset($trace['object']) && $trace['object'] instanceof SearchApiIndex) {
        return $trace['object'];
      }
    }
    // If there's only a single index on the site, it's also easy.
    $indexes = search_api_index_load_multiple(FALSE);
    if (count($indexes) === 1) {
      return reset($indexes);
    }
    throw new SearchApiException('Could not determine the active index of the datasource.');
  }

  /**
   * Returns the entity types for which this datasource is configured.
   *
   * Depends on the index from which this method is (indirectly) called.
   *
   * @param SearchApiIndex $index
   *   (optional) The index for which to get the enabled entity types. If not
   *   given, will be determined automatically.
   *
   * @return string[]
   *   The machine names of the datasource's enabled entity types, as both keys
   *   and values.
   *
   * @throws SearchApiException
   *   Thrown if the active index could not be determined.
   */
  protected function getEntityTypes(SearchApiIndex $index = NULL) {
    if (!$index) {
      $index = $this->getCallingIndex();
    }
    if (isset($index->options['datasource']['types'])) {
      return drupal_map_assoc($index->options['datasource']['types']);
    }
    return array();
  }

  /**
   * Returns the selected entity type options for this datasource.
   *
   * Depends on the index from which this method is (indirectly) called.
   *
   * @param SearchApiIndex $index
   *   (optional) The index for which to get the enabled entity types. If not
   *   given, will be determined automatically.
   *
   * @return string[]
   *   An associative array, mapping the machine names of the enabled entity
   *   types to their labels.
   *
   * @throws SearchApiException
   *   Thrown if the active index could not be determined.
   */
  protected function getSelectedEntityTypeOptions(SearchApiIndex $index = NULL) {
    return array_intersect_key(search_api_entity_type_options_list(), $this->getEntityTypes($index));
  }

}
