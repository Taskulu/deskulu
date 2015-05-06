<?php
/**
 * @file
 * bootstrap-panel.tpl.php
 *
 * Markup for Bootstrap panels ([collapsible] fieldsets).
 */

?>
<?php if ($prefix): ?>
  <?php print $prefix; ?>
<?php endif; ?>
<fieldset <?php print $attributes; ?>>
  <?php if ($title): ?>
    <?php if ($collapsible): ?>
      <legend class="panel-heading">
        <a href="#<?php print $id; ?>" class="panel-title fieldset-legend" data-toggle="collapse">
          <?php print $title; ?>
        </a>
      </legend>
    <?php else: ?>
      <legend class="panel-heading">
        <div class="panel-title fieldset-legend">
          <?php print $title; ?>
        </div>
      </legend>
    <?php endif; ?>
  <?php endif; ?>
  <?php if ($collapsible): ?>
    <div id="<?php print $id; ?>" class="panel-collapse collapse fade<?php print (!$collapsed ? ' in' : ''); ?>">
  <?php endif; ?>
  <div class="panel-body">
    <?php if ($description): ?>
      <p class="help-block">
        <?php print $description; ?>
      </p>
    <?php endif; ?>
    <?php print $content; ?>
  </div>
  <?php if ($collapsible): ?>
    </div>
  <?php endif; ?>
</fieldset>
<?php if ($suffix): ?>
  <?php print $suffix; ?>
<?php endif; ?>
