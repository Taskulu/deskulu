<?php

/**
 * @file
 * Content type plugin to expose rendered entities, view mode configuration
 * still available.
 */

$plugin = array(
  'title' => t('Rendered entity'),
  'defaults' => array('view_mode' => 'full'),
  'content type' => 'entity_entity_view_content_type_info',
);

/**
 * Get the entity content type info.
 */
function entity_entity_view_content_type_info($entity_type) {
  $types = entity_entity_view_content_type_content_types();
  if (isset($types[$entity_type])) {
    return $types[$entity_type];
  }
}

/**
 * Implements hook_PLUGIN_content_type_content_types().
 *
 * Rendered entity use entity types machine name as subtype name.
 */
function entity_entity_view_content_type_content_types() {
  $types = array();
  $entities = entity_get_info();

  foreach ($entities as $entity_type => $info) {
    if (entity_type_supports($entity_type, 'view')) {
      $types[$entity_type] = array(
        'title' => t('Rendered @entity_type', array('@entity_type' => $info['label'])),
        'category' => t('Entity'),
        'required context' => new ctools_context_required(t('Entity'), $entity_type),
      );
    }
  }

  return $types;
}

/**
 * Returns an edit form for a entity.
 *
 * Rendered entity use entity types machine name as subtype name.
 *
 * @see entity_entity_view_get_content_types()
 */
function entity_entity_view_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $entity_type = $form_state['subtype_name'];
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
      '#default_value' => $conf['view_mode'],
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

  return $form;
}

/**
 * Save selected view mode.
 */
function entity_entity_view_content_type_edit_form_submit(&$form, &$form_state) {
  if (isset($form_state['values']['view_mode'])) {
    $form_state['conf']['view_mode'] = $form_state['values']['view_mode'];
  }
}

/**
 * Implements hook_PLUGIN_content_type_render().
 *
 * Ctools requires us to return a block.
 *
 * @see ctools_content_render()
 */
function entity_entity_view_content_type_render($entity_type, $conf, $panel_args, $context) {
  if ($context->empty) {
    return;
  }
  $block = new stdClass();
  $block->module = 'entity';
  $block->delta = $entity_type . '-' . str_replace('-', '_', $conf['view_mode']);

  $entity_id = $context->argument;
  $entity = entity_load_single($entity_type, $entity_id);
  $block->content = entity_view($entity_type, array($entity_id => $entity), $conf['view_mode']);
  return $block;
}

/**
 * Implements hook_PLUGIN_content_type_admin_title().
 *
 * Returns the administrative title for a type.
 */
function entity_entity_view_content_type_admin_title($entity_type, $conf, $contexts) {
  $entity_info = entity_get_info($entity_type);
  $view_mode = $conf['view_mode'];
  if (isset($entity_info['view modes'][$view_mode])) {
    $view_mode = $entity_info['view modes'][$view_mode]['label'];
  }
  return t('Rendered @entity_type using view mode "@view_mode"', array('@entity_type' => $entity_info['label'], '@view_mode' => $view_mode));
}
