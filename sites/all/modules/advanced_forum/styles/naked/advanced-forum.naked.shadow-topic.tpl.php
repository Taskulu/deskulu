<?php

/**
 * @file
 * advanced-forum.naked.shadow-topic.tpl.php
 */
?>

<?php print $title; ?><br />
<?php print t('This topic has been moved to "!forum" ', array('!forum' => $new_forum)); ?>
(<a href="<?php print $new_forum_url; ?>"><?php print t('View topic'); ?></a>)
