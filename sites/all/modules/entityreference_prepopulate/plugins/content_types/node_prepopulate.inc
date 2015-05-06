<?php

/**
 * Plugin definition.
 */
$plugin = array(
  'title' => t('Content prepopulate links'),
  'description' => t('Crafted links to create content (nodes) for an entity reference field.'),
  'required context' => new ctools_context_required(t('Node'), 'node'),
  'category' => t('Entity reference'),
  'defaults' => array(
    'types' => array(),
    'field_name' => '',
  ),
);

/**
 * Render callback.
 */
function entityreference_prepopulate_node_prepopulate_content_type_render($subtype, $conf, $args, $context) {
  if (empty($context->data)) {
    return;
  }

  $node = $context->data;
  $links = entityreference_prepopulate_create_node_links('node', $node->nid, $conf['field_name'], NULL, !empty($conf['types']) ? $conf['types'] : NULL);
  if (!$links) {
    return;
  }

  $module = 'entityreference_prepopulate';
  $block = new stdClass();
  $block->module = $module;
  $block->title = t('Content create links');
  $block->content = $links;
  return $block;
}

/**
 * Edit form.
 */
function entityreference_prepopulate_node_prepopulate_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $options = array();
  $bundles = array();
  // Use CTools to get the best matching field name.
  ctools_include('fields');
  foreach (field_info_instances('node') as $node_type => $instances) {
    foreach ($instances as $field_name => $instance) {
      if (empty($instance['settings']['behaviors']['prepopulate']['status'])) {
        continue;
      }

      $field = field_info_field($field_name);

      if ($field['settings']['target_type'] != 'node') {
        // Field doesn't reference a node.
        continue;
      }

      $bundles[] = $instance['bundle'];
      if (empty($options[$field_name])) {
        $options[$field_name] = ctools_field_label($field_name) . ' (' . $field_name . ')';
      }
    }
  }

  $form['field_name'] = array(
    '#title' => t('Field name'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $conf['field_name'],
    '#description' => $options ? t('The entity reference field to prepopulate.') : t('There are no entity reference fields, with prepopulate enabled'),
    '#required' => TRUE,
  );

  $options = array();
  foreach (node_type_get_types() as $type) {
    if (in_array($type->type, $bundles)) {
      $options[$type->type] = check_plain($type->name);
    }
  }
  $form['types'] = array(
    '#title' => t('Restrict to content types'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $conf['types'],
    '#description' => t('If left empty, all possible content types are shown.'),
  );
  return $form;
}

/**
 * Edit form submit callback.
 */
function entityreference_prepopulate_node_prepopulate_content_type_edit_form_submit($form, &$form_state) {
  $form_state['conf']['field_name'] = $form_state['values']['field_name'];
  $form_state['conf']['types'] = array_filter($form_state['values']['types']);
}
