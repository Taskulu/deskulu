<?php
/**
 * @file
 * services-documentation-method-error.tpl.php
 *
 * Template file for theming errors for a given Services method.
 *
 * Available custom variables:
 * - $question:
 * - $description:
 * - $response:
 * - $anchor:
 */
?>
<!-- services-documentation-method-error -->
<div class="method-error">
  <a name="<?php print $anchor; ?>"></a>
  <div class="method-error-detail">
    <?php if($question): ?>
      <div class="method-error-question">
        <h6><?php print $question; ?></h6>
      </div>
    <?php endif; ?>
    <?php if($description): ?>
      <div class="method-error-description">
        <?php print $description; ?>
      </div>
    <?php endif; ?>
  </div>
  <?php if($response): ?>
    <pre class="method-error-response">
<?php print $response; ?>
    </pre>
  <?php endif; ?>
</div>
<!-- /services-documentation-method-error -->
