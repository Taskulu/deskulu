<?php

/**
 * @file
 * Rate widget theme
 */

print theme('item_list', array(
  'items' => $stars,
  //'title' => $display_options['title'],
  ));

if ($info) {
  print '<div class="rate-info">' . $info . '</div>';
}

if ($display_options['description']) {
  print '<div class="rate-description">' . $display_options['description'] . '</div>';
}
