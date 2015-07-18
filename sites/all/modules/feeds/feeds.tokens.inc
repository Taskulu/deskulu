<?php

/**
 * @file
 * Builds placeholder replacement tokens for feed-related data.
 */

/**
 * Implements hook_token_info().
 */
function feeds_token_info() {
  // @todo This token could be for any entity type.
  $info['tokens']['node']['feed-source'] = array(
    'name' => t('Feed source'),
    'description' => t('The node the feed item was sourced from.'),
    'type' => 'node',
  );

  return $info;
}

/**
 * Implements hook_tokens().
 */
function feeds_tokens($type, $tokens, array $data, array $options) {
  $replacements = array();

  if ($type == 'node' && !empty($data['node']) && !empty($data['node']->nid)) {

    $sanitize = !empty($options['sanitize']);

    $feed_nid = feeds_get_feed_nid($data['node']->nid, 'node');

    if ($feed_nid && $feed_source = node_load($feed_nid)) {
      foreach ($tokens as $name => $original) {
        switch ($name) {
          case 'feed-source':
            $replacements[$original] = $sanitize ? check_plain($feed_source->title) : $feed_source->title;
            break;
        }
      }

      // Chained node token relationships.
      if ($feed_source_tokens = token_find_with_prefix($tokens, 'feed-source')) {
        $replacements += token_generate('node', $feed_source_tokens, array('node' => $feed_source), $options);
      }
    }
  }

  return $replacements;
}
