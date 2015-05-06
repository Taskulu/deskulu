<?php
/**
 * @file
 * Display an appropriate icon for a forum post.
 *
 * Available variables:
 * - $new_posts: Indicates whether or not the topic contains new posts.
 * - $icon: The icon to display. May be one of 'hot', 'hot-new', 'new',
 *   'default', 'closed', or 'sticky'.
 * - $node_type: The type of the node that is displayed
 *
 * @see template_preprocess_forum_icon()
 * @see advanced_forum_preprocess_forum_icon()
 */
?>
<?php if ($new_posts): ?>
  <a name="new">
  <?php endif; ?>

  <?php if (!empty($icon_class)): ?>
    <span class="<?php print "topic-icon topic-icon-$icon_class topic-icon-node-type-$node_type"; ?>"><?php print "$icon_title"; ?></span>
  <?php endif; ?>

  <?php if ($new_posts): ?>
  </a>
<?php endif; ?>
