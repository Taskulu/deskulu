<?php

/**
 * @file
 * Contains the 'Feeds node id' argument validator plugin.
 */

/**
 * Validates whether an argument is a valid Feed node.
 */
class feeds_views_plugin_argument_validate_feed_nid extends views_plugin_argument_validate {
  /**
   * Validates if argument is a valid Feed node.
   */
  public function validate_argument($argument) {
    $node = node_load($argument);
    if ($node) {
      $importer_id = feeds_get_importer_id($node->type);
      if ($importer_id) {
        return TRUE;
      }
    }
    return FALSE;
  }
}
