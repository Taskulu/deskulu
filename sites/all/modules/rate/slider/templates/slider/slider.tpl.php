<?php
/**
 * @file
 * Rate widget theme
 */

print '<div class="rate-slider rate-value-' . $value . '">';
print theme('item_list', array('items' => $buttons));
print '</div>';

if ($info) {
  print '<div class="rate-info">' . $info . '</div>';
}

if ($display_options['description']) {
  print '<div class="rate-description">' . $display_options['description'] . '</div>';
}
