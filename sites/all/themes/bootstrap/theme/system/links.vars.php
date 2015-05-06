<?php
/**
 * @file
 * links.vars.php
 */

/**
 * Implements hook_preprocess_links().
 */
function bootstrap_preprocess_links(&$variables) {
  if (isset($variables['attributes']) && isset($variables['attributes']['class'])) {
    $string = is_string($variables['attributes']['class']);
    if ($string) {
      $variables['attributes']['class'] = explode(' ', $variables['attributes']['class']);
    }

    if ($key = array_search('inline', $variables['attributes']['class'])) {
      $variables['attributes']['class'][$key] = 'list-inline';
    }

    if ($string) {
      $variables['attributes']['class'] = implode(' ', $variables['attributes']['class']);
    }
  }
}
