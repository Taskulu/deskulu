<?php

/**
 * @file
 * Theme implementation to display information about the post/profile author.
 *
 * See author-pane.tpl.php in Author Pane module for a full list of variables.
 */
?>

<?php
  // This bit of debugging info will show the full path to and name of this
  // template file to make it easier to figure out which template is
  // controlling which author pane.
// @codingStandardsIgnoreStart
  if (!empty($show_template_location)) {
    print __FILE__;
  }
// @codingStandardsIgnoreEnd
?>

<div class="author-pane clearfix">
 <div class="author-pane-inner">
    <?php /* User picture / avatar (has div in variable) */ ?>
    <?php if (!empty($picture)): ?>
      <?php print $picture; ?>
    <?php endif; ?>

    <?php /* General section */ ?>
    <div class="author-pane-section author-pane-section-1">
      <?php /* Account name */ ?>
      <div class="author-pane-line author-name">
        <?php print $account_name; ?>
      </div>

      <?php /* Online status */ ?>
      <?php if (!empty($online_status)): ?>
        <div class="author-pane-line <?php print $online_status_class ?>">
           <?php print $online_status; ?>
        </div>
      <?php endif; ?>

      <?php /* User title */ ?>
      <?php if (!empty($user_title)): ?>
        <div class="author-pane-line author-title">
          <?php print $user_title; ?>
        </div>
      <?php endif; ?>

      <?php /* User badges */ ?>
      <?php if (!empty($user_badges)): ?>
        <div class="author-pane-line author-badges">
          <?php print $user_badges; ?>
        </div>
      <?php endif; ?>
    </div>

    <div class="author-pane-section author-pane-section-2">
      <?php /* Location */ ?>
      <?php if (!empty($location_user_location)): ?>
        <div class="author-pane-line author-location">
          <?php print $location_user_location;  ?>
        </div>
      <?php endif; ?>

      <?php /* Joined */ ?>
      <?php if (!empty($joined)): ?>
        <div class="author-pane-line author-joined">
          <span class="author-pane-label"><?php print t('Joined'); ?>:</span> <?php print $joined; ?>
        </div>
      <?php endif; ?>

      <?php /* Posts */ ?>
      <?php if (isset($user_stats_posts)): ?>
        <div class="author-pane-line author-posts">
          <span class="author-pane-label"><?php print t('Posts'); ?>:</span> <?php print $user_stats_posts; ?>
        </div>
      <?php endif; ?>

      <?php /* Points */ ?>
      <?php if (isset($userpoints_points)): ?>
        <div class="author-pane-line author-points">
          <span class="author-pane-label"><?php print t('!Points', userpoints_translation()); ?></span>: <?php print $userpoints_points; ?>
        </div>
      <?php endif; ?>
    </div>

    <?php /* Contact section */ ?>
    <div class="author-pane-section author-pane-contact">
      <?php /* Contact / Email */ ?>
      <?php if (!empty($contact)): ?>
        <div class="author-pane-line author-pane-link-line author-contact">
          <?php print $contact; ?>
        </div>
      <?php endif; ?>

      <?php /* Private message */ ?>
      <?php if (!empty($privatemsg)): ?>
        <div class="author-pane-line author-pane-link-line author-privatemsg">
          <?php print $privatemsg; ?>
        </div>
      <?php endif; ?>

      <?php /* User relationships */ ?>
      <?php if (!empty($user_relationships_api)): ?>
        <div class="author-pane-line author-pane-link-line author-user-relationship">
          <?php print $user_relationships_api; ?>
        </div>
      <?php endif; ?>

      <?php /* Flag friend */ ?>
      <?php if (!empty($flag_friend)): ?>
        <div class="author-pane-line author-pane-link-line author-flag-friend">
          <?php print $flag_friend; ?>
        </div>
      <?php endif; ?>
    </div>

    <?php /* Admin section */ ?>
    <div class="author-pane-section author-pane-admin">
      <?php /* IP */ ?>
      <?php if (!empty($user_stats_ip)): ?>
        <div class="author-pane-line author-ip">
          <span class="author-pane-label"><?php print t('IP'); ?>:</span> <?php print $user_stats_ip; ?>
        </div>
      <?php endif; ?>

     <?php /* Fasttoggle block */ ?>
     <?php if (!empty($fasttoggle_block_author)): ?>
        <div class="author-fasttoggle-block"><?php print $fasttoggle_block_author; ?></div>
      <?php endif; ?>

     <?php /* Troll ban */ ?>
      <?php if (!empty($troll_ban_author)): ?>
        <div class="author-pane-line author-troll-ban"><?php print $troll_ban_author; ?></div>
      <?php endif; ?>
    </div>
  </div>
</div>
