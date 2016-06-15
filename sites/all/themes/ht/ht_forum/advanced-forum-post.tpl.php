<?php

/**
 * @file
 * Theme implementation: Template for each forum post whether node or comment.
 *
 * All variables available in node.tpl.php and comment.tpl.php for your theme
 * are available here. In addition, Advanced Forum makes available the following
 * variables:
 *
 * - $top_post: TRUE if we are formatting the main post (ie, not a comment)
 * - $reply_link: Text link / button to reply to topic.
 * - $total_posts: Number of posts in topic (not counting first post).
 * - $new_posts: Number of new posts in topic, and link to first new.
 * - $links_array: Unformatted array of links.
 * - $account: User object of the post author.
 * - $name: User name of post author.
 * - $author_pane: Entire contents of the Author Pane template.
 */
$name = format_username($account);
?>

<div id="<?php print $post_id; ?>" class="well <?php print $classes; ?>" <?php print $attributes; ?>>
  <div class="forum-post-info clearfix">
    <div class="post-meta">
      <div class="pull-left flip">
        <?php print theme('user_picture', ['account' => $account]); ?>
      </div>
      <div class="pull-left description flip">
        <b><?php print $name ;?></b><br/>
        <?php if ($top_post): ?>
        <span class="submit-time"><?php print t('started a topic @time ago', ['@time' => format_interval(REQUEST_TIME - $node->created)]); ?></span>
        <?php else: ?>
        <span class="submit-time"><?php print t('said @time ago', ['@time' => format_interval(REQUEST_TIME - $node->created)]); ?></span>
        <?php endif; ?>
      </div>
    </div>
    <?php /* End of posted on div. */ ?>


    <span class="forum-post-number"><?php print $permalink; ?></span>
  </div> <?php /* End of post info div. */ ?>

  <div class="forum-post-wrapper">


    <div class="forum-post-panel-main clearfix">

      <div class="forum-post-content">
        <?php
          // @codingStandardsIgnoreStart
          // We hide the comments and links now so that we can render them later.
          hide($content['taxonomy_forums']);
          hide($content['comments']);
          hide($content['links']);
          if (!$top_post)
            hide($content['body']);
          print render($content);
          // @codingStandardsIgnoreEnd
        ?>
      </div>

      <?php if (!empty($post_edited)): ?>
        <div class="post-edited">
          <?php print $post_edited ?>
        </div>
      <?php endif; ?>

      <?php if (!empty($signature)): ?>
        <div class="author-signature">
          <?php print $signature ?>
        </div>
      <?php endif; ?>
    </div>
  </div>
  <?php /* End of post wrapper div. */ ?>
</div>
<?php /* End of main wrapping div. */ ?>

<?php if ($top_post && !empty($content['comments'])): ?>
<div class="col-md-10 col-md-push-1">
  <?php print render($content['comments']); ?>
</div>
<?php endif;?>
