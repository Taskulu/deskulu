<?php

/**
 * @file
 * Contains SearchApiAlterAddHierarchy.
 */

/**
 * Adds all ancestors for hierarchical fields.
 */
class SearchApiAlterAddHierarchy extends SearchApiAbstractAlterCallback {

  /**
   * Cached value for the hierarchical field options.
   *
   * @var array
   *
   * @see getHierarchicalFields()
   */
  protected $field_options;

  /**
   * Overrides SearchApiAbstractAlterCallback::supportsIndex().
   *
   * Returns TRUE only if any hierarchical fields are available.
   */
  public function supportsIndex(SearchApiIndex $index) {
    return (bool) $this->getHierarchicalFields();
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm() {
    $options = $this->getHierarchicalFields();
    $this->options += array('fields' => array());
    $form['fields'] = array(
      '#title' => t('Hierarchical fields'),
      '#description' => t('Select the fields which should be supplemented with their ancestors. ' .
          'Each field is listed along with its children of the same type. ' .
          'When selecting several child properties of a field, all those properties will be recursively added to that field. ' .
          'Please note that you should de-select all fields before disabling this data alteration.'),
      '#type' => 'select',
      '#multiple' => TRUE,
      '#size' => min(6, count($options, COUNT_RECURSIVE)),
      '#options' => $options,
      '#default_value' => $this->options['fields'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    // Change the saved type of fields in the index, if necessary.
    if (!empty($this->index->options['fields'])) {
      $fields = &$this->index->options['fields'];
      $previous = drupal_map_assoc($this->options['fields']);
      foreach ($values['fields'] as $field) {
        list($key) = explode(':', $field);
        if (empty($previous[$field]) && isset($fields[$key]['type'])) {
          $fields[$key]['type'] = 'list<' . search_api_extract_inner_type($fields[$key]['type']) . '>';
          $change = TRUE;
        }
      }
      $new = drupal_map_assoc($values['fields']);
      foreach ($previous as $field) {
        list($key) = explode(':', $field);
        if (empty($new[$field]) && isset($fields[$key]['type'])) {
          $w = $this->index->entityWrapper(NULL, FALSE);
          if (isset($w->$key)) {
            $type = $w->$key->type();
            $inner = search_api_extract_inner_type($fields[$key]['type']);
            $fields[$key]['type'] = search_api_nest_type($inner, $type);
            $change = TRUE;
          }
        }
      }
      if (isset($change)) {
        $this->index->save();
      }
    }

    return parent::configurationFormSubmit($form, $values, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function alterItems(array &$items) {
    if (empty($this->options['fields'])) {
      return;
    }
    foreach ($items as $item) {
      $wrapper = $this->index->entityWrapper($item, FALSE);

      $values = array();
      foreach ($this->options['fields'] as $field) {
        list($key, $prop) = explode(':', $field);
        if (!isset($wrapper->$key)) {
          continue;
        }
        $child = $wrapper->$key;

        $values += array($key => array());
        $this->extractHierarchy($child, $prop, $values[$key]);
      }
      foreach ($values as $key => $value) {
        $item->$key = $value;
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function propertyInfo() {
    if (empty($this->options['fields'])) {
      return array();
    }

    $ret = array();
    $wrapper = $this->index->entityWrapper(NULL, FALSE);
    foreach ($this->options['fields'] as $field) {
      list($key, $prop) = explode(':', $field);
      if (!isset($wrapper->$key)) {
        continue;
      }
      $child = $wrapper->$key;
      while (search_api_is_list_type($child->type())) {
        $child = $child[0];
      }
      if (!isset($child->$prop)) {
        continue;
      }
      if (!isset($ret[$key])) {
        $ret[$key] = $child->info();
        $type = search_api_extract_inner_type($ret[$key]['type']);
        $ret[$key]['type'] = "list<$type>";
        $ret[$key]['getter callback'] = 'entity_property_verbatim_get';
        // The return value of info() has some additional internal values set,
        // which we have to unset for the use here.
        unset($ret[$key]['name'], $ret[$key]['parent'], $ret[$key]['langcode'], $ret[$key]['clear'],
            $ret[$key]['property info alter'], $ret[$key]['property defaults']);
      }
      if (isset($ret[$key]['bundle'])) {
        $info = $child->$prop->info();
        if (empty($info['bundle']) || $ret[$key]['bundle'] != $info['bundle']) {
          unset($ret[$key]['bundle']);
        }
      }
    }
    return $ret;
  }

  /**
   * Finds all hierarchical fields for the current index.
   *
   * @return array
   *   An array containing all hierarchical fields of the index, structured as
   *   an options array grouped by primary field.
   */
  protected function getHierarchicalFields() {
    if (!isset($this->field_options)) {
      $this->field_options = array();
      $wrapper = $this->index->entityWrapper(NULL, FALSE);
      // Only entities can be indexed in hierarchies, as other properties don't
      // have IDs that we can extract and store.
      $entity_info = entity_get_info();
      foreach ($wrapper as $key1 => $child) {
        while (search_api_is_list_type($child->type())) {
          $child = $child[0];
        }
        $info = $child->info();
        $type = $child->type();
        if (empty($entity_info[$type])) {
          continue;
        }
        foreach ($child as $key2 => $prop) {
          if (search_api_extract_inner_type($prop->type()) == $type) {
            $prop_info = $prop->info();
            $this->field_options[$info['label']]["$key1:$key2"] = $prop_info['label'];
          }
        }
      }
    }
    return $this->field_options;
  }

  /**
   * Extracts a hierarchy from a metadata wrapper by modifying $values.
   */
  public function extractHierarchy(EntityMetadataWrapper $wrapper, $property, array &$values) {
    if (search_api_is_list_type($wrapper->type())) {
      foreach ($wrapper as $w) {
        $this->extractHierarchy($w, $property, $values);
      }
      return;
    }
    try {
      $v = $wrapper->value(array('identifier' => TRUE));
      if ($v && !isset($values[$v])) {
        $values[$v] = $v;
        if (isset($wrapper->$property) && $wrapper->value() && $wrapper->$property->value()) {
          $this->extractHierarchy($wrapper->$property, $property, $values);
        }
      }
    }
    catch (EntityMetadataWrapperException $e) {
      // Some properties like entity_metadata_book_get_properties() throw
      // exceptions, so we catch them here and ignore the property.
    }
  }

}
