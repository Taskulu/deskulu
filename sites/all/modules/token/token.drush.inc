<?php

/**
 * @file
 * Drush integration for the Token module.
 */

/**
 * Implements hook_drush_cache_clear().
 */
function token_drush_cache_clear(&$types) {
  if (function_exists('module_exists') && module_exists('token')) {
    $types['token'] = 'drush_token_cache_clear_token_info';
  }
}

/**
 * Clear caches internal to Token module.
 */
function drush_token_cache_clear_token_info() {
  token_clear_cache();
}
