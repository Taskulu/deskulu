<?php

/**
 * @file
 * Theme implementation to display a single Wysiwyg (plugin) dialog page.
 */
?>

  <div id="page">

    <?php print $messages; ?>

    <div id="main-wrapper"><div id="main" class="clearfix">

      <div id="content" class="column"><div class="section">
        <a id="main-content"></a>
        <?php print render($title_prefix); ?>
        <?php if ($title): ?><h1 class="title" id="page-title"><?php print $title; ?></h1><?php endif; ?>
        <?php print render($title_suffix); ?>
        <?php if ($tabs): ?><div class="tabs"><?php print render($tabs); ?></div><?php endif; ?>
        <?php print render($page['help']); ?>
        <?php if ($action_links): ?><ul class="action-links"><?php print render($action_links); ?></ul><?php endif; ?>
        <?php print render($page['content']); ?>
      </div></div> <!-- /.section, /#content -->

    </div></div> <!-- /#main, /#main-wrapper -->

  </div> <!-- /#page -->
