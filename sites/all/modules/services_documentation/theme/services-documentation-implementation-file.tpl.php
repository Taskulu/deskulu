<?php
/**
 * @file
 * services-documentation-implementation-file.tpl.php
 *
 * Template file for theming an example implementation file for a given Services
 * method.
 *
 * Available custom variables:
 * - $name:
 * - $path:
 * - $type:
 * - $contents:
 * - $children
 */
?>
<!-- services-documentation-implementation-file -->
<li class="implementation-file">
  <div class="file-name"><em><?php print $name; ?></em></div>
  <?php if ($type && $type == 'directory' && $children): ?>
    <ul class="file-children"><?php print render($children); ?></ul>
  <?php elseif ($type && $type == 'file' && $contents): ?>
    <pre class="file-contents"><?php print $contents; ?></pre>
  <?php endif; ?>
</li>
<!-- /services-documentation-implementation-file -->
