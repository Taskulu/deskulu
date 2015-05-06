<?php

/**
 * @file
 * Rate widget theme
 */
?>

<?php
  /*
  <div class="rate-label">
    <?php print $display_options['title']; ?>
  </div>
  */
?>

<ul>
  <li class="thumb-up">
    <?php print $up_button; ?>
    <div class="percent"><?php print $results['up_percent'] . '%'; ?></div>
  </li>
  <li class="thumb-down">
    <?php print $down_button; ?>
    <div class="percent"><?php print $results['down_percent'] . '%'; ?></div>
  </li>
</ul>
<?php

if ($info) {
  print '<div class="rate-info">' . $info . '</div>';
}

if ($display_options['description']) {
  print '<div class="rate-description">' . $display_options['description'] . '</div>';
}
