<?php
/**
 * @file
 * Template file for theming a given Services resource.
 *
 * A given Services resource contains the following nested elements:
 * - Method bundles. E.g., operations, actions, targeted actions.
 * - Methods. E.g., create, update, index, etc.
 *
 * Available custom variables:
 * - $name: The name of the resource.
 * - $description: The description of the resource.
 * - $limit: Limits placed on consumers of the resource.
 * - $method_bundles: An array of methods for this Services resource.
 */
?>
<!-- services-documentation-resource -->
<div class="services-documentation-resource">
  <h2 class="resource-title">
    <a name="<?php print $name; ?>"></a><?php print $name; ?>
  </h2>

  <?php if ($description): ?>
    <p class="resource-description"><?php print $description; ?></p>
  <?php endif; ?>

  <?php if ($limit): ?>
    <span class="resource-limit">Limits: <?php print $limit; ?></span>
  <?php endif; ?>

  <?php print render($method_bundles); ?>
</div>
<!-- /services-documentation-resource -->
