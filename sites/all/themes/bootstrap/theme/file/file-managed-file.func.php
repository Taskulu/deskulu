<?php
/**
 * @file
 * file-managed-file.func.php
 */

/**
 * Overrides theme_file_managed_file().
 */
function bootstrap_file_managed_file($variables) {
  $element = $variables['element'];

  $attributes = array();
  if (isset($element['#id'])) {
    $attributes['id'] = $element['#id'];
  }
  if (!empty($element['#attributes']['class'])) {
    $attributes['class'] = (array) $element['#attributes']['class'];
  }
  $attributes['class'][] = 'form-managed-file';
  $attributes['class'][] = 'input-group';

  $element['upload_button']['#prefix'] = '<span class="input-group-btn">';
  $element['upload_button']['#suffix'] = '</span>';
  $element['remove_button']['#prefix'] = '<span class="input-group-btn">';
  $element['remove_button']['#suffix'] = '</span>';

  if (!empty($element['filename'])) {
    $element['filename']['#prefix'] = '<div class="form-control">';
    $element['filename']['#suffix'] = '</div>';
  }

  $hidden_elements = array();
  foreach (element_children($element) as $child) {
    if ($element[$child]['#type'] === 'hidden') {
      $hidden_elements[$child] = $element[$child];
      unset($element[$child]);
    }
  }

  // This wrapper is required to apply JS behaviors and CSS styling.
  $output = '';
  $output .= '<div' . drupal_attributes($attributes) . '>';
  $output .= drupal_render_children($element);
  $output .= '</div>';
  $output .= render($hidden_elements);
  return $output;
}
