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

<?php foreach ($tables as $table_id => $table): ?>
  <?php $table_info = $table['table_info']; ?>

  <div class="forum-table-wrap">
    <div class="forum-table-superheader">
      <div class="forum-table-name">
        <?php if (empty($table_info->link)): ?>
          <?php print $table_info->name; ?>
        <?php else: ?>
          <a href="<?php print $table_info->link; ?>"><?php print $table_info->name; ?></a>
        <?php endif; ?>
      </div>
      <?php if ($collapsible): ?>
        <span id="forum-collapsible-<?php print $table_info->tid; ?>" class="forum-collapsible" >&nbsp;</span>
      <?php endif; ?>
      <div class="forum-table-description"><?php print $table_info->description; ?></div>

    </div>
    <div id="forum-table-<?php print $table_info->tid; ?>">
      <table class="forum-table forum-table-forums">
        <thead class="forum-header">
          <tr>
            <th class="forum-icon">&nbsp;</th>
            <th class="forum-name"><?php print t('Forum'); ?></th>
            <th class="forum-topics"><?php print t('Topics'); ?></th>
            <th class="forum-posts"><?php print t('Posts'); ?></th>
            <th class="forum-last-post"><?php print t('Last post'); ?></th>
          </tr>
        </thead>

        <tbody id="forum-table-<?php print $table_info->tid; ?>-content">
          <?php foreach ($table['items'] as $item_id => $item): ?>
            <?php if ($item->is_container): ?>
              <tr id="subcontainer-<?php print $item_id; ?>" class="forum-row <?php print $item->zebra; ?> container-<?php print $item_id; ?>-child">
              <?php else: ?>
              <tr id="forum-<?php print $item_id; ?>" class="forum-row <?php print $item->zebra; ?> container-<?php print $item_id; ?>-child">
              <?php endif; ?>

              <?php if (!empty($item->forum_image)): ?>
                <td class="forum-image forum-image-<?php print $item_id; ?>">
                  <?php print $item->forum_image; ?>
                </td>
              <?php else: ?>
                <td class="<?php print $item->icon_classes ?>">
                  <span class="forum-list-icon-wrapper"><span><?php print $item->icon_text ?></span></span>
                </td>
              <?php endif; ?>

              <?php $colspan = ($item->is_container) ? 4 : 1 ?>
              <td class="forum-details" colspan="<?php print $colspan ?>">
                <div class="forum-name">
                  <a href="<?php print $item->link; ?>"><?php print $item->name; ?></a>
                </div>
                <?php if (!empty($item->description)): ?>
                  <div class="forum-description">
                    <?php print $item->description; ?>
                  </div>
                <?php endif; ?>

                <?php if (!empty($item->subcontainers)): ?>
                  <div class="forum-subcontainers">
                    <span class="forum-subcontainers-label"><?php print t("Subcontainers") ?>:</span> <?php print $item->subcontainers; ?>
                  </div>
                <?php endif; ?>

                <?php if (!empty($item->subforums)): ?>
                  <div class="forum-subforums">
                    <span class="forum-subforums-label"><?php print t("Subforums") ?>:</span> <?php print $item->subforums; ?>
                  </div>
                <?php endif; ?>
              </td>
              <?php if (!$item->is_container): ?>
                <td class="forum-number-topics">
                  <div class="forum-number-topics"><?php print $item->total_topics ?>
                    <?php if ($item->new_topics): ?>
                      <div class="forum-number-new-topics">
                        <a href="<?php print $item->new_topics_link; ?>"><?php print $item->new_topics_text; ?></a>
                      </div>
                    <?php endif; ?>
                  </div>
                </td>

                <td class="forum-number-posts">
                  <?php print $item->total_posts ?>

                  <?php if ($item->new_posts): ?>
                    <br />
                    <a href="<?php print $item->new_posts_link; ?>"><?php print $item->new_posts_text; ?></a>
                  <?php endif; ?>
                </td>
                <td class="forum-last-reply">
                  <?php print $item->last_post ?>
                </td>
              <?php endif; ?>

            </tr>

          <?php endforeach; ?>
        </tbody>
      </table>
    </div>
  </div>
<?php endforeach; ?>
