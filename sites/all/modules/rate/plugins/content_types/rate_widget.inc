<?php

$plugin = array(
  'title' => t('Rate widget'),
  'description' => t('A rate widget for this node.'),
  'category' => t('Node'),
  'required context' => new ctools_context_required(t('Node'), 'node'), // you can also define your required context for this widget to function
  'render callback' => 'rate_panels_rate_widget_render',
  'edit form' => 'rate_panels_rate_widget_form',
  'defaults' => array(
    'build_mode' => 'RATE_FULL',
    'widget_name' => '',
  ),
);

// Plugins form
function rate_panels_rate_widget_form($form, &$form_state) {
  $conf = $form_state['conf'];
  // General options, that depends on the type of options your widget or plugin must have as a configuration
  $form['build_mode'] = array(
    '#title' => t('Build mode'),
    '#type' => 'select',
    '#options' => array(
      RATE_FULL => t('RATE_FULL'),
      RATE_COMPACT => t('RATE_COMPACT'),
      RATE_DISABLED => t('RATE_DISABLED'),
      RATE_CLOSED => t('RATE_CLOSED'),
      RATE_COMPACT_DISABLED => t('RATE_COMPACT_DISABLED'),
    ),
    '#default_value' => $conf['build_mode'],
  );

  $widgets = variable_get(RATE_VAR_WIDGETS, array());
  $widget_options = array();
  foreach ($widgets as $id => $widget) {
    $widget_options[$id] = $widget->title;
  }

  $form['widget_name'] = array(
    '#title' => t('Widget'),
    '#type' => 'select',
    '#options' => $widget_options,
    '#default_value' => $conf['widget_name'],
  );

  return $form;
}

/**
 * Submit handler for the custom type settings form.
 */
function rate_panels_rate_widget_form_submit($form, &$form_state) {
  // Copy everything from our defaults.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

function rate_panels_rate_widget_render($subtype, $conf, $panel_args, $context) {
  if (empty($context) || empty($context->data)) {
    return;
  }

  // Get a shortcut to the node.
  $node = $context->data;

  // Load information about the node type.
  $type = node_type_get_type($node);

  $widget_code = array(
    '#markup' => rate_generate_widget($conf['widget_name'], 'node', $node->nid, $conf['build_mode']),
  );

  // Build the content type block.
  $block = new stdClass();
  $block->module = 'rate_extras';
  $block->title = $type->title_label;
  $block->content = $widget_code;
  $block->delta = $node->nid;

  return $block;
}