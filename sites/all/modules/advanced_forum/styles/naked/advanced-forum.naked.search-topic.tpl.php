<?php

/**
 * @file
 * Display the topic forum widget
 *
 * The real widget is part of a view, but this widget actually leads to the
 * view, and can be redone. It does not need to use FAPI because it
 * is just a simple get form, which allows us to style it however we
 * like.
 *
 * Variables:
 * - $node: The node to be searched.
 * - $path: The path to the search widget for the form action.
 */
?>
<div class="search-topic">
  <form action="<?php print $path ?>" accept-charset="UTF-8" method="get" id="advanced-forum-search-topic">
  <div class="container-inline">
    <div class="form-item" id="edit-keys-wrapper">
      <input type="text" maxlength="128" name="keys" id="edit-keys" value="" title="<?php print t('Enter the terms you wish to search for.'); ?>" class="form-text" />
    </div>
    <input type="submit" id="edit-submit" value="<?php print t('Search'); ?>"  class="form-submit" />
  </div>
  </form>
</div>
