<?php

/**
 * @file
 * Contains SearchApiAlterBundleFilter.
 */

/**
 * Represents a data alteration that restricts entity indexes to some bundles.
 */
class SearchApiAlterBundleFilter extends SearchApiAbstractAlterCallback {

  /**
   * {@inheritdoc}
   */
  public function supportsIndex(SearchApiIndex $index) {
    if ($this->isMultiEntityIndex($index)) {
      $info = entity_get_info();
      foreach ($index->options['datasource']['types'] as $type) {
        if (isset($info[$type]) && self::hasBundles($info[$type])) {
          return TRUE;
        }
      }
      return FALSE;
    }
    return $index->getEntityType() && ($info = entity_get_info($index->getEntityType())) && self::hasBundles($info);
  }

  /**
   * {@inheritdoc}
   */
  public function alterItems(array &$items) {
    if (!$this->supportsIndex($this->index) || !isset($this->options['bundles'])) {
      return;
    }

    if ($this->isMultiEntityIndex()) {
      $bundle_prop = 'item_bundle';
    }
    else {
      $info = entity_get_info($this->index->getEntityType());
      $bundle_prop = $info['entity keys']['bundle'];
    }

    $bundles = array_flip($this->options['bundles']);
    $default = (bool) $this->options['default'];

    foreach ($items as $id => $item) {
      if (isset($bundles[$item->$bundle_prop]) == $default) {
        unset($items[$id]);
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm() {
    if ($this->supportsIndex($this->index)) {
      $options = array();
      if ($this->isMultiEntityIndex()) {
        $info = entity_get_info();
        $unsupported_types = array();
        foreach ($this->index->options['datasource']['types'] as $type) {
          if (isset($info[$type]) && self::hasBundles($info[$type])) {
            foreach ($info[$type]['bundles'] as $bundle => $bundle_info) {
              $options["$type:$bundle"] = $info[$type]['label'] . ' » ' . $bundle_info['label'];
            }
          }
          else {
            $unsupported_types[] = isset($info[$type]['label']) ? $info[$type]['label'] : $type;
          }
        }
        if ($unsupported_types) {
          $form['unsupported_types']['#markup'] = '<p>' . t('The following entity types do not contain any bundles: @types. All items of those types will therefore be included in the index.', array('@types' => implode(', ', $unsupported_types))) . '</p>';
        }
      }
      else {
        $info = entity_get_info($this->index->getEntityType());
        foreach ($info['bundles'] as $bundle => $bundle_info) {
          $options[$bundle] = isset($bundle_info['label']) ? $bundle_info['label'] : $bundle;
        }
      }
      if (!empty($this->index->options['datasource']['bundles'])) {
        $form['message']['#markup'] = '<p>' . t("<strong>Note:</strong> This index is already restricted to certain bundles. If you use this data alteration, those will be reduced further. However, the index setting is better supported in the user interface and should therefore be prefered. For example, using this data alteration will not reduce the displayed total number of items to index (even though some of them will not be indexed). Consider creating a new index with appropriate bundle settings instead.") . '</p>';
        $included_bundles = array_flip($this->index->options['datasource']['bundles']);
        $options = array_intersect_key($options, $included_bundles);
      }
      $form['default'] = array(
        '#type' => 'radios',
        '#title' => t('Which items should be indexed?'),
        '#default_value' => isset($this->options['default']) ? $this->options['default'] : 1,
        '#options' => array(
          1 => t('All but those from one of the selected bundles'),
          0 => t('Only those from the selected bundles'),
        ),
      );
      $form['bundles'] = array(
        '#type' => 'select',
        '#title' => t('Bundles'),
        '#default_value' => isset($this->options['bundles']) ? $this->options['bundles'] : array(),
        '#options' => $options,
        '#size' => min(4, count($options)),
        '#multiple' => TRUE,
      );
    }
    else {
      $form = array(
        'forbidden' => array(
          '#markup' => '<p>' . t("Items indexed by this index don't have bundles and therefore cannot be filtered here.") . '</p>',
        ),
      );
    }
    return $form;
  }

  /**
   * Determines whether a certain entity type has any bundles.
   *
   * @param array $entity_info
   *   The entity type's entity_get_info() array.
   *
   * @return bool
   *   TRUE if the entity type has bundles, FALSE otherwise.
   */
  protected static function hasBundles(array $entity_info) {
    return !empty($entity_info['entity keys']['bundle']) && !empty($entity_info['bundles']);
  }

  /**
   * Determines whether the given index contains multiple types of entities.
   *
   * @param SearchApiIndex|null $index
   *   (optional) The index to examine. Defaults to the index set for this
   *   plugin.
   *
   * @return bool
   *   TRUE if the index is a multi-entity index, FALSE otherwise.
   */
  protected function isMultiEntityIndex(SearchApiIndex $index = NULL) {
    $index = $index ? $index : $this->index;
    return $index->datasource() instanceof SearchApiCombinedEntityDataSourceController;
  }

}
