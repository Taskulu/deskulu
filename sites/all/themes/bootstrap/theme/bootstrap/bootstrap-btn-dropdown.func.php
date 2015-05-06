<?php
/**
 * @file
 * bootstrap-btn-dropdown.func.php
 */

/**
 * Implements theme_bootstrap_btn_dropdown().
 */
function theme_bootstrap_btn_dropdown($variables) {
  $type_class = '';
  $sub_links = '';

  $variables['attributes']['class'][] = 'btn-group';
  // Type class.
  if (isset($variables['type'])) {
    $type_class .= ' btn-' . $variables['type'];
  }
  else {
    $type_class .= ' btn-default';
  }

  // Start markup.
  $output = '<div' . drupal_attributes($variables['attributes']) . '>';

  // Add as string if its not a link.
  if (is_array($variables['label'])) {
    $output .= l($variables['label']['title'], $$variables['label']['href'], $variables['label']);
  }
  $output .= '<a class="btn' . $type_class . ' dropdown-toggle" data-toggle="dropdown" href="#">';

  // It is a link, create one.
  if (is_string($variables['label'])) {
    $output .= check_plain($variables['label']);
  }
  if (is_array($variables['links'])) {
    $sub_links = theme('links', array(
      'links' => $variables['links'],
      'attributes' => array(
        'class' => array('dropdown-menu'),
      ),
    ));
  }
  // Finish markup.
  $output .= '<span class="caret"></span></a>' . $sub_links . '</div>';
  return $output;
}
