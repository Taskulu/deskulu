<?php
/**
 * Field handler to present field title with link to the entity.
 *
 * @ingroup views_field_handlers
 */
class views_handler_title_field extends views_handler_field_field {

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_entity'] = array('default' => isset($this->definition['link_to_entity default']) ? $this->definition['link_to_entity default'] : FALSE);
    return $options;

  }

  /**
   * Provide link to entity option.
   */
  function options_form(&$form, &$form_state) {
    $form['link_to_entity'] = array(
      '#title' => t('Link this field to the original entity'),
      '#description' => t("Enable to override this field's links."),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_entity']),
    );

    parent::options_form($form, $form_state);
  }

  function advanced_render($values) {
    $this->original_values = $values;
    return parent::advanced_render($values);
  }

  function render_item($count, $item) {
    if (!empty($this->options['link_to_entity'])) {
      $values = $this->original_values;
      $entity_type = $this->definition['entity_tables'][$this->base_table];
      $entity_info = entity_get_info($entity_type);
      $key = $entity_info['entity keys']['id'];
      if (!empty($values->_field_data[$key]['entity'])) {
        $entity = $values->_field_data[$key]['entity'];
        $uri = entity_uri($entity_type, $entity);
        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = $uri['path'];
      }
    }
    return parent::render_item($count, $item);
  }
}
