<?php
/**
 * @file
 * bootstrap-modal.tpl.php
 *
 * Markup for Bootstrap modals.
 *
 * Variables:
 * - $attributes: Attributes for the outer modal div.
 * - $heading: Modal title.
 * - $body: The rendered body of the modal.
 * - $footer: The rendered footer of the modal.
 */
?>
<div<?php print $attributes; ?>>
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
        <h4 class="modal-title"><?php print $heading; ?></h4>
      </div>
      <div class="modal-body"><?php print $body; ?></div>
      <div class="modal-footer"><?php print $footer; ?></div>
    </div>
  </div>
</div>
