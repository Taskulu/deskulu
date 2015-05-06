<?php
/**
 * @file
 * table.vars.php
 */

/**
 * Implements hook_preprocess_table().
 */
function bootstrap_preprocess_table(&$variables) {
  if (isset($variables['attributes']['class']) && is_string($variables['attributes']['class'])) {
    // Convert classes to an array.
    $variables['attributes']['class'] = explode(' ', $variables['attributes']['class']);
  }
  $variables['attributes']['class'][] = 'table';
  if (!in_array('table-no-striping', $variables['attributes']['class'])) {
    $variables['attributes']['class'][] = 'table-striped';
  }
}
