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
    return $index->getEntityType() && ($info = entity_get_info($index->getEntityType())) && self::hasBundles($info);
  }

  /**
   * {@inheritdoc}
   */
  public function alterItems(array &$items) {
    $info = entity_get_info($this->index->getEntityType());
    if (self::hasBundles($info) && isset($this->options['bundles'])) {
      $bundles = array_flip($this->options['bundles']);
      $default = (bool) $this->options['default'];
      $bundle_prop = $info['entity keys']['bundle'];
      foreach ($items as $id => $item) {
        if (isset($bundles[$item->$bundle_prop]) == $default) {
          unset($items[$id]);
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm() {
    $info = entity_get_info($this->index->getEntityType());
    if (self::hasBundles($info)) {
      $options = array();
      foreach ($info['bundles'] as $bundle => $bundle_info) {
        $options[$bundle] = isset($bundle_info['label']) ? $bundle_info['label'] : $bundle;
      }
      $form = array(
        'default' => array(
          '#type' => 'radios',
          '#title' => t('Which items should be indexed?'),
          '#default_value' => isset($this->options['default']) ? $this->options['default'] : 1,
          '#options' => array(
            1 => t('All but those from one of the selected bundles'),
            0 => t('Only those from the selected bundles'),
          ),
        ),
        'bundles' => array(
          '#type' => 'select',
          '#title' => t('Bundles'),
          '#default_value' => isset($this->options['bundles']) ? $this->options['bundles'] : array(),
          '#options' => $options,
          '#size' => min(4, count($options)),
          '#multiple' => TRUE,
        ),
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
   *   TRUE if the entity type has bundles, FASLE otherwise.
   */
  protected static function hasBundles(array $entity_info) {
    return !empty($entity_info['entity keys']['bundle']) && !empty($entity_info['bundles']);
  }

}
