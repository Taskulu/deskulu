<?php
/**
 * @file
 * views-view-table.tpl.php
 * Template to display a view as a table.
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $header: An array of header labels keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $class: A class or classes to apply to the table, based on settings.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 * - $rows: An array of row items. Each row is an array of content.
 *   $rows are keyed by row number, fields within rows are keyed by field ID.
 * @ingroup views_templates
 */
?>
<div id="forum-topic-list">
  <div class="row topic-list-wrapper">
    <div class="topic-list clearfix">
      <?php foreach ($rows as $count => $row): ?>

      <div class="col-md-12 topic-row <?php print implode(' ', $row_classes[$count]); ?> <?php echo isset($row['field_feature_request_status']) ? $row['field_feature_request_status'] : ''; ?>">
        <div class="topic-row-inner clearfix">
          <div class="user-picture hidden-sm pull-left flip">
            <?php print $row['picture']; ?>
          </div>
          <div class="topic-info col-md-8 pull-left flip">
            <?php print $row['title']; ?><?php print $row['cid_1']; ?><?php print $row['value_1']; ?>
          </div>
          <div class="topic-votes pull-right flip">
            <?php print $row['value']; ?>
          </div>
        </div>
      </div>
      <?php endforeach; ?>
    </div>
  </div>
</div>
