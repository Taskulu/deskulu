<div class="row">
  <div class="col-md-12">

  <?php
  foreach ($term_tree as $item) {
    ?>
    <h3 class="section"><?php echo $item['title'];?></h3>
  <?php
    foreach ($item['children'] as $row) {
      ?>
      <div class="row">
        <?php foreach ($row as $cell) {
          ?>
          <div class="col-md-6 subsection-container">
          <h4 class="subsection"><?php echo $cell['title']; ?> <span class="count">(<?php echo count($cell['content']); ?>)</span></h4>
            <ul class="list-unstyled">
          <?php foreach ($cell['content'] as $nid => $title) {
            ?>
              <li><span class="mdi-editor-insert-comment"></span><?php echo l($title, 'node/' . $nid); ?></li>
        <?php
          }
          ?>
            </ul>
          </div>
        <?php
        } ?>
      </div>
  <?php
    }
  }
  ?>
  </div>
</div>