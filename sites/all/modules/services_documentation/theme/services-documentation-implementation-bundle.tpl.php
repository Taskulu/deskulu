<?php
/**
 * @file
 * services-documentation-method-example-implementation-bundle.tpl.php
 *
 * Template file for theming an example implementation bundle for a given
 * Services method.
 *
 * Available custom variables:
 * - $language:
 * - $examples:
 */
?>
<!-- implementation-bundle -->
<div class="implementation-bundle">
  <h6 class="implementation-bundle-language"><?php print $language; ?></h6>
  <?php foreach ($examples as $example): ?>
    <?php print render($example); ?>
  <?php endforeach; ?>
</div>
<!-- /implementation-bundle -->
