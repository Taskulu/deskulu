<?php
/**
 * @file
 * Translate link plugin.
 */

/**
 * This handler adds translate link for all translatable entities.
 */
class entity_translation_handler_field_translate_link extends views_handler_field {

  function construct() {
    parent::construct();
    $this->additional_fields['entity_id'] = 'entity_id';
    $this->additional_fields['entity_type'] = 'entity_type';
    $this->additional_fields['language'] = 'language';
  }

  /**
   * Add required additional fields.
   */
  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  /**
   * Add the text option.
   * @see views_handler_field::option_definition()
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  /**
   * Add the option to set the title of the translate link.
   * @see views_handler_field::options_form()
   */
  function options_form(&$form, &$form_state) {
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow setting it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
  }

  /**
   * Load all entities based on the data we have.
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

  /**
   * @see views_handler_field::render()
   */
  function render($values) {
    $type = $this->get_value($values, 'entity_type');
    $entity_id = $this->get_value($values, 'entity_id');
    // Check if entity is not empty
    if (!$entity_id || !$type) {
      return NULL;
    }
    $language = $this->get_value($values, 'language');
    $entity = $this->entities[$type][$entity_id];
    return $this->render_link($type, $entity_id, $entity, $language);
  }

  /**
   * Render the link to the translation overview page of the entity.
   */
  function render_link($entity_type, $entity_id, $entity, $language) {
    if (!entity_translation_tab_access($entity_type, $entity)) {
      return;
    }
    // We use the entity info here to avoid having to call entity_load() for all
    // the entities.
    $info = entity_get_info($entity_type);
    $path = $info['translation']['entity_translation']['path schemes']['default']['translate path'];
    $path = str_replace($info['translation']['entity_translation']['path schemes']['default']['path wildcard'], $entity_id, $path);
    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = $path;
    $this->options['alter']['query'] = drupal_get_destination();
    $text = !empty($this->options['text']) ? $this->options['text'] : t('translate');
    return $text;
  }
}
