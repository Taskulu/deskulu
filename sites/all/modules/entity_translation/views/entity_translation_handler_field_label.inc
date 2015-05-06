<?php
/**
 * @file
 * This file contains a label field handler for entity translation.
 */

/**
 * This handler shows the entity label for entities in the entity_translation table.
 */
class entity_translation_handler_field_label extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['entity_id'] = 'entity_id';
    $this->additional_fields['entity_type'] = 'entity_type';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }
  /**
   * Add a 'link to entity' option definition.
   * @see views_handler_field::option:definition()
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_entity'] = array('default' => '', 'translatable' => FALSE);
    return $options;
  }

  /**
   * Add a 'link to entity' option.
   * @see views_handler_field::options_form()
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_entity'] = array(
      '#title' => t('Link this field to it\'s entity'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_entity']
    );
  }

  /**
   * Load all entities, so that we can get the label.
   */
  function post_execute(&$values) {
    $ids = array();
    $ids_by_type = array();
    foreach ($values as $row) {
      if ($entity_type = $this->get_value($row, 'entity_type')) {
        $ids_by_type[$entity_type][] = $this->get_value($row, 'entity_id');
      }
    }
    foreach ($ids_by_type as $type => $ids) {
      $this->entities[$type] = entity_load($type, $ids);
    }
  }

  function render($values) {
    $entity_type = $this->get_value($values, 'entity_type');
    $entity_id = $this->get_value($values, 'entity_id');
    // Check if entity is not empty
    if (!$entity_id || !$entity_type) {
      return NULL;
    }
    $entity = $this->entities[$entity_type][$entity_id];
    // We could also use entity_label(), but since this we might want to let
    // the handler decide what's best to show.
    $handler = entity_translation_get_handler($entity_type, $entity);
    $label = $handler->getLabel();
    if ($this->options['link_to_entity']) {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = $handler->getViewPath();
    }
    return $label;
  }
}
