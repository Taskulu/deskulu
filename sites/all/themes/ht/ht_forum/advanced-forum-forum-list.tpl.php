<?php
/**
 * @file
 * Default theme implementation to display a list of forums and containers.
 *
 * Available variables:
 * - $forums: An array of forums and containers to display. It is keyed to the
 *   numeric id's of all child forums and containers.
 * - $forum_id: Forum id for the current forum. Parent to all items within
 *   the $forums array.
 *
 * Each $forum in $forums contains:
 * - $forum->is_container: Is TRUE if the forum can contain other forums. Is
 *   FALSE if the forum can contain only topics.
 * - $forum->depth: How deep the forum is in the current hierarchy.
 * - $forum->zebra: 'even' or 'odd' string used for row class.
 * - $forum->name: The name of the forum.
 * - $forum->link: The URL to link to this forum.
 * - $forum->description: The description of this forum.
 * - $forum->new_topics: True if the forum contains unread posts.
 * - $forum->new_url: A URL to the forum's unread posts.
 * - $forum->new_text: Text for the above URL which tells how many new posts.
 * - $forum->old_topics: A count of posts that have already been read.
 * - $forum->total_posts: The total number of posts in the forum.
 * - $forum->last_reply: Text representing the last time a forum was posted or
 *   commented in.
 * - $forum->forum_image: If used, contains an image to display for the forum.
 *
 * @see template_preprocess_forum_list()
 * @see theme_forum_list()
 */
?>

<?php
/*
  The $tables variable holds the individual tables to be shown. A table is
  either created from a root level container or added as needed to hold root
  level forums. The following code will loop through each of the tables.
  In each table, it loops through the items in the table. These items may be
  subcontainers or forums. Subcontainers are printed simply with the name
  spanning the entire table. Forums are printed out in more detail. Subforums
  have already been attached to their parent forums in the preprocessing code
  and will display under their parents.
 */

?>
<div class="row">
<div class="col-md-12">
<?php foreach ($tables as $table_id => $table): ?>
  <?php $table_info = $table['table_info']; ?>
  <?php if (empty($table_info->link)): ?>
    <h3 class="section">
      <?php print $table_info->name; ?>
    </h3>
    <div class="pull-right flip small actions">
      <ul class="list-inline">
        <li><a href="<?php echo url('node/add/forum'); ?>"><?php echo t('Start a new topic'); ?></a></li>
      </ul>
    </div>
  <?php else: ?>
    <h3 class="section"><a href="<?php print $table_info->link; ?>"><?php print $table_info->name; ?></a></h3>
  <?php endif;
  $i = 0;
  ?>
  <?php foreach ($table['items'] as $item_id => $item): ?>
    <?php if ($i % 2 == 0): ?>
      <div class="row">
    <?php endif; ?>
    <div class="col-md-6 subsection-container">
      <span class="glyphicon <?php echo $item->icon; ?> pull-left flip large"></span>
      <div class="subsection-inner pull-left flip">
        <h4 class="subsection">
          <a href="<?php print $item->link; ?>"><?php print $item->name; ?> <span class="count">(<?php print $item->total_topics; ?>)</span></a>
        </h4>
        <?php if (!empty($item->description)): ?>
          <span class="description"><?php print $item->description; ?></span>
          <ul class="list-unstyled clearfix">

          <?php foreach ($item->recent_nodes as $node): ?>
            <li>
              <span class="glyphicon glyphicon-comment pull-left flip"></span>
              <div class="last-reply pull-left flip">
                <a class="discussion-title" href="<?php echo url('node/' . $node->nid); ?>"><?php echo check_plain($node->title); ?></a>
                <p class="submitted">
                  <?php
                  $account = user_load($node->uid);
                  echo t('Posted by <b>!name</b>, @time ago', ['!name' => theme('username', ['account' => $account]), '@time' => format_interval(REQUEST_TIME - $node->created)]);
                  ?>
                </p>
                <p class="last-reply">
                  <?php
                  $last_commenter = user_load($node->last_comment_uid);
                  echo t('!last_reply by !name @time ago', ['!last_reply' => l(t('Last reply'), 'comment/' . $node->cid, ['fragment' => 'comment-' . $node->cid]), '!name' => theme('username', ['account' => $last_commenter]), '@time' => format_interval(REQUEST_TIME - $node->last_comment_timestamp)]);
                  ?>
                </p>
              </div>
            </li>
          <?php endforeach; ?>
          </ul>
        <?php endif;?>
      </div>
    </div>
    <?php if ($i % 2 == 1): ?>
      </div>
    <?php endif;
    $i++;
    ?>
  <?php endforeach; ?>
<?php endforeach; ?>
</div>
</div>