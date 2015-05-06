<?php

/**
 * @file
 * Default theme implementation to format a simple string indicated when and
 * by whom a topic was submitted.
 *
 * Available variables:
 *
 * - $topic_link: On the forum overview page, this is the title of the last
 *   updated topic (node).
 * - $author: The author of the post.
 * - $time: How long ago the post was created.
 * - $topic: An object with the raw data of the thread. Unsafe, be sure
 *   to clean this data before printing.
 *
 * @see template_preprocess_forum_submitted()
 * @see advanced_forum_preprocess_forum_submitted()
 */
?>

<?php if ($time): ?>
  <?php if (!empty($topic_link)): ?>
    <?php if (isset($date_posted)): ?>
      <?php print t(
        '!title<br />by !author<br />@date_posted', array(
          '!title' => $topic_link,
          '@date_posted' => $date_posted,
          '!author' => $author,
        )); ?>
    <?php else: ?>
     <?php print t(
        '!title<br />by !author<br />@time ago', array(
          '!title' => $topic_link,
          '@time' => $time,
          '!author' => $author,
        )); ?>
    <?php endif; ?>
  <?php else: ?>
    <?php if (isset($date_posted)): ?>
      <?php print t(
        'by !author<br />@date_posted', array(
          '@date_posted' => $date_posted,
          '!author' => $author,
        )); ?>
    <?php else: ?>
     <?php print t(
        'by !author<br />@time ago', array(
          '@time' => $time,
          '!author' => $author,
        )); ?>
    <?php endif; ?>
  <?php endif; ?>
<?php else: ?>
  <?php print t('n/a'); ?>
<?php endif; ?>
