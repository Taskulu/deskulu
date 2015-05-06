<?php
/**
 * @file
 * file-widget.func.php
 */

/**
 * Overrides theme_file_widget().
 */
function bootstrap_file_widget($variables) {
  $element = $variables['element'];
  $output = '';

  $hidden_elements = array();
  foreach (element_children($element) as $child) {
    if ($element[$child]['#type'] === 'hidden') {
      $hidden_elements[$child] = $element[$child];
      unset($element[$child]);
    }
  }

  $element['upload_button']['#prefix'] = '<span class="input-group-btn">';
  $element['upload_button']['#suffix'] = '</span>';

  // The "form-managed-file" class is required for proper Ajax functionality.
  $output .= '<div class="file-widget form-managed-file clearfix input-group">';
  if (!empty($element['fid']['#value'])) {
    // Add the file size after the file name.
    $element['filename']['#markup'] .= ' <span class="file-size">(' . format_size($element['#file']->filesize) . ')</span> ';
  }
  $output .= drupal_render_children($element);
  $output .= '</div>';
  $output .= render($hidden_elements);
  return $output;
}
