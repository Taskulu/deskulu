<?php

/**
 * @file
 * Default theme implementation to display a forum which may contain forum
 * containers as well as forum topics.
 *
 * Variables available:
 * - $forum_links: An array of links that allow a user to post new forum topics.
 *   It may also contain a string telling a user they must log in in order
 *   to post. Empty if there are no topics on the page. (ie: forum overview)
 *   This is no longer printed in the template by default because it was moved
 *   to the topic list section. The variable is still available for customizations.
 * - $forums: The forums to display (as processed by forum-list.tpl.php)
 * - $topics: The topics to display (as processed by forum-topic-list.tpl.php)
 * - $forums_defined: A flag to indicate that the forums are configured.
 * - $forum_legend: Legend to go with the forum graphics.
 * - $topic_legend: Legend to go with the topic graphics.
 * - $forum_tools: Drop down menu for various forum actions.
 * - $forum_description: Description that goes with forum term. Not printed by default.
 *
 * @see template_preprocess_forums()
 * @see advanced_forum_preprocess_forums()
 */
?>

<?php if ($forums_defined): ?>
  <div id="forum">

    <?php print $forums; ?>

    <?php if (!empty($forum_tools)): ?>
      <div class="forum-tools"><?php print $forum_tools; ?></div>
    <?php endif; ?>

    <?php print $topics; ?>

    <?php if (!empty($topics)): ?>
      <?php print $topic_legend; ?>
    <?php endif; ?>

    <?php if (!empty($forum_legend)): ?>
      <?php print $forum_legend; ?>
    <?php endif; ?>

     <?php if (!empty($forum_statistics)): ?>
       <?php print $forum_statistics; ?>
     <?php endif; ?>
  </div>
<?php endif; ?>
