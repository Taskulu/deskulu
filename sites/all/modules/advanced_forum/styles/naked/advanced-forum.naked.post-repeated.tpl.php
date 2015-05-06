<?php

/**
 * @file
 * Used for the repeated node on the top of each page of a paginated forum
 * thread. By default, it contains only the "header" information for the thread
 * and the rest is empty.
 *
 * If you leave it empty, subsequent pages will start with the next comment
 * like you typically find in forum software. You could also put a specially
 * formatted teaser to remind people what post they are reading. If you like
 * having the entire node repeated, simply copy the entire contents of
 * advanced_forum-post.tpl.php into this file. All the same variables are available.
 */
?>

<?php print $topic_header ?>
<?php print render($content['comments']); ?>
