<?php
/**
 * @file
 * exposed-filters.func.php
 */

/**
 * Overrides theme_exposed_filters().
 */
function bootstrap_exposed_filters($variables) {
  $form = $variables['form'];
  $output = '';

  foreach (element_children($form['status']['filters']) as $key) {
    $form['status']['filters'][$key]['#field_prefix'] = '<div class="col-sm-10">';
    $form['status']['filters'][$key]['#field_suffix'] = '</div>';
  }
  $form['status']['actions']['#attributes']['class'][] = 'col-sm-offset-2';
  $form['status']['actions']['#attributes']['class'][] = 'col-sm-10';
  $form['status']['actions']['#prefix'] = '<div class="form-group">';
  $form['status']['actions']['#suffix'] = '</div>';

  if (isset($form['current'])) {
    $items = array();
    foreach (element_children($form['current']) as $key) {
      $items[] = drupal_render($form['current'][$key]);
    }
    $output .= theme('item_list', array(
      'items' => $items,
      'attributes' => array(
        'class' => array(
          'clearfix',
          'current-filters',
        ),
      ),
    ));
  }
  $output .= drupal_render_children($form);
  return '<div class="form-horizontal">' . $output . '</div>';
}
