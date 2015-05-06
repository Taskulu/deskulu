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

<div class="author-pane">
  <div class="author-pane-inner">
    <div class="author-pane-section author-pane-general">
      <?php if (!empty($picture)): ?>
        <?php print $picture; ?>
      <?php endif; ?>
    </div>
  </div>
</div>
