<?php

/**
 * @file
 * Theme implementation to display information about the most active poster for a forum.
 *
 * See active-poster-pane.tpl.php in active-poster Pane module for a full list of variables.
 */
?>

<div class="active-poster">
    <div class="active-poster-name">
      <?php print $account_name; ?>
    </div>

    <?php /* Avatar (has div in variable) */ ?>
    <?php if (!empty($picture)): ?>
      <?php print $picture; ?>
    <?php endif; ?>

    <?php /* Posts */ ?>
    <div class="active-poster-posts">
      <span class="active-poster-label"><?php print t('Posts'); ?>:</span> <?php print $posts; ?>
    </div>

    <?php /* Posts */ ?>
    <div class="active-poster-topics">
      <span class="active-poster-label"><?php print t('Topics'); ?>:</span> <?php print $topics; ?>
    </div>

    <div class="last-post">
      <div class="active-poster-label"><?php print t('Last post'); ?></div>
      <div class="active-poster-title">
        <?php print $last_post_title; ?>
      </div>
      <div class="active-poster-date">
        <span class="active-poster-label"><?php print t('On'); ?>:</span> <?php print $last_post_date; ?>
      </div>

    </div>
</div>
