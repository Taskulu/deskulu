<?php

/**
 * @file
 * Passes selected ids as arguments to a page.
 * The ids might be entity ids or revision ids, depending on the type of the
 * VBO field.
 */

/**
 * Implementation of hook_action_info().
 */
function views_bulk_operations_argument_selector_action_info() {
  return array(
    'views_bulk_operations_argument_selector_action' => array(
      'label' => t('Pass ids as arguments to a page'),
      'type' => 'entity',
      'aggregate' => TRUE,
      'configurable' => FALSE,
      'hooks' => array(),
      'triggers' => array('any'),
    ),
  );
}

/**
* Implementation of a Drupal action.
* Passes selected ids as arguments to a view.
*/
function views_bulk_operations_argument_selector_action($entities, $context = array()) {
  $base_url = $context['settings']['url'];
  $arguments = implode(',', array_keys($entities));
  // Add a trailing slash if missing.
  if (substr($base_url, -1, 1) != '/') {
    $base_url .= '/';
  }
  drupal_goto($base_url . $arguments);
}

function views_bulk_operations_argument_selector_action_views_bulk_operations_form($options) {
  $form['url'] = array(
    '#title' => t('URL'),
    '#type' => 'textfield',
    '#description' => t('Enter a URL that the user will be sent to. A comma-separated list of selected ids will be appended.'),
    '#default_value' => isset($options['url']) ? $options['url'] : '',
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
  );
  return $form;
}
