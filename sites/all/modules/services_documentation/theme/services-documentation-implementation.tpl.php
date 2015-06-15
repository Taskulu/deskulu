<?php
/**
 * @file
 * services-documentation-implementation.tpl.php
 *
 * Template file for theming an example implementation for a given method.
 * method.
 *
 * Available custom variables:
 * - $name:
 * - $description:
 * - $location:
 * - $files:
 * - $download_link:
 * - $uses_sdk:
 */
?>
<!-- services-documentation-implementation -->
<div class="implementation">
  <div class="implementation-name"><?php print $name; ?></div>

  <?php if ($uses_sdk): ?>
    <div class="implementation-uses-sdk"><strong>Uses SDK</strong></div>
  <?php endif; ?>

  <?php if ($description): ?>
    <div class="implementation-description"><?php print $description; ?></div>
  <?php endif; ?>

  <?php if ($files): ?>
    <div class="implementation-files"><ul><?php print render($files); ?></ul></div>
  <?php endif; ?>

  <?php if ($download_link): ?>
    <div class="implementation-download">
      <?php print l(t('Download'), $download_link, array('absolute' => TRUE)); ?>
    </div>
  <?php endif; ?>

</div>
<!-- /services-documentation-implementation -->
