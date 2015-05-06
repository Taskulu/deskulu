<?php

/**
 * Wrapper function for update_status().
 *
 * @see update_status().
 */
function module_filter_update_status() {
  module_load_include('inc', 'update', 'update.report');
  $update_report = update_status();

  return array(
    'module_filter' => drupal_get_form('module_filter_update_status_form'),
    'update_report' => array(
      '#markup' => $update_report
    )
  );
}

function module_filter_update_status_form($form, &$form_state) {
  $form['module_filter'] = array(
    '#type' => 'module_filter',
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'module_filter') . '/css/update_status.css'
      ),
      'js' => array(
        drupal_get_path('module', 'module_filter') . '/js/update_status.js'
      ),
    ),
  );
  $form['module_filter']['show'] = array(
    '#type' => 'radios',
    '#default_value' => (isset($_GET['show']) && in_array($_GET['show'], array('all', 'updates', 'security', 'unknown'))) ? $_GET['show'] : 'all',
    '#options' => array('all' => t('All'), 'updates' => t('Update available'), 'security' => t('Security update'), 'unknown' => t('Unknown')),
    '#prefix' => '<div id="module-filter-show-wrapper">',
    '#suffix' => '</div>'
  );
  if (module_exists('update_advanced')) {
    $options = $form['module_filter']['show']['#options'];
    $form['module_filter']['show']['#options'] = array_slice($options, 0, 2);
    $form['module_filter']['show']['#options']['ignore'] = t('Ignored from settings');
    $form['module_filter']['show']['#options'] = array_merge($form['module_filter']['show']['#options'], array_slice($options, 2));
  }
  return $form;
}
