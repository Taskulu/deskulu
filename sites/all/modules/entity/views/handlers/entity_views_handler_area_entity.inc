<?php
/**
 * @file
 * Renders a full entity in a views area.
 */

class entity_views_handler_area_entity extends views_handler_area {
  public function option_definition() {
    $options = parent::option_definition();
    $options['entity_type'] = array('default' => 'node');
    $options['entity_id'] = array('default' => '');
    $options['view_mode'] = array('default' => 'full');
    $options['bypass_access'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $entity_type_options = array();
    foreach (entity_get_info() as $entity_type => $entity_info) {
      $entity_type_options[$entity_type] = $entity_info['label'];
    }

    $entity_type = $this->options['entity_type'];

    $form['entity_type'] = array(
      '#type' => 'select',
      '#title' => t('Entity type'),
      '#options' => $entity_type_options,
      '#description' => t('Choose the entity type you want to display in the area.'),
      '#default_value' => $entity_type,
      '#ajax' => array(
        'path' => views_ui_build_form_url($form_state),
      ),
      '#submit' => array('views_ui_config_item_form_submit_temporary'),
      '#executes_submit_callback' => TRUE,
    );

    $form['entity_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Entity id'),
      '#description' => t('Choose the entity you want to display in the area.'),
      '#default_value' => $this->options['entity_id'],
    );

    if ($entity_type) {
      $entity_info = entity_get_info($entity_type);
      $options = array();
      if (!empty($entity_info['view modes'])) {
        foreach ($entity_info['view modes'] as $mode => $settings) {
          $options[$mode] = $settings['label'];
        }
      }

      if (count($options) > 1) {
        $form['view_mode'] = array(
          '#type' => 'select',
          '#options' => $options,
          '#title' => t('View mode'),
          '#default_value' => $this->options['view_mode'],
        );
      }
      else {
        $form['view_mode_info'] = array(
          '#type' => 'item',
          '#title' => t('View mode'),
          '#description' => t('Only one view mode is available for this entity type.'),
          '#markup' => $options ? current($options) : t('Default'),
        );
        $form['view_mode'] = array(
          '#type' => 'value',
          '#value' => $options ? key($options) : 'default',
        );
      }
    }
    $form['bypass_access'] = array(
      '#type' => 'checkbox',
      '#title' => t('Bypass access checks'),
      '#description' => t('If enabled, access permissions for rendering the entity are not checked.'),
      '#default_value' => !empty($this->options['bypass_access']),
    );
    return $form;
  }

  public function admin_summary() {
    $label = parent::admin_summary();
    if (!empty($this->options['entity_id'])) {
      return t('@label @entity_type:@entity_id', array(
        '@label' => $label,
        '@entity_type' => $this->options['entity_type'],
        '@entity_id' => $this->options['entity_id'],
      ));
    }
  }

  public function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      return $this->render_entity($this->options['entity_type'], $this->options['entity_id'], $this->options['view_mode']);
    }
    return '';
  }

  /**
   * Render an entity using the view mode.
   */
  public function render_entity($entity_type, $entity_id, $view_mode) {
    if (!empty($entity_type) && !empty($entity_id) && !empty($view_mode)) {
      $entity = entity_load_single($entity_type, $entity_id);
      if (!empty($this->options['bypass_access']) || entity_access('view', $entity_type, $entity)) {
        $render = entity_view($entity_type, array($entity), $view_mode);
        $render_entity = reset($render);
        return drupal_render($render_entity);
      }
    }
    else {
      return '';
    }
  }
}
