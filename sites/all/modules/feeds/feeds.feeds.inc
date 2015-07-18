<?php

/**
 * @file
 * Feeds hooks.
 */

/**
 * Implements hook_feeds_processor_targets().
 */
function feeds_feeds_processor_targets($entity_type, $bundle) {
  // Record that we've been called.
  // @see _feeds_feeds_processor_targets_alter()
  $called = &drupal_static('feeds_feeds_processor_targets', FALSE);
  $called = TRUE;

  return array();
}

/**
 * Implements hook_feeds_processor_targets_alter().
 */
function feeds_feeds_processor_targets_alter(array &$targets, $entity_type, $bundle) {
  // This hook gets called last, so that we normalize the whole array.
  feeds_normalize_targets($targets);
}

/**
 * Normalizes the target array.
 *
 * @param array &$targets
 *   The Feeds target array.
 */
function feeds_normalize_targets(array &$targets) {
  static $defaults = array(
    'description' => '',
    'summary_callbacks' => array(),
    'form_callbacks' => array(),
    'preprocess_callbacks' => array(),
    'unique_callbacks' => array(),
  );

  foreach (array_keys($targets) as $target) {
    $targets[$target] += $defaults;

    // Filter out any uncallable keys.
    _feeds_filter_callback_arrays($targets[$target]);
  }
}

/**
 * Filters the callbacks of a single target array.
 *
 * @param array &$target
 *   The target arary.
 */
function _feeds_filter_callback_arrays(array &$target) {
  // Migrate keys summary_callback and form_callback to the new keys.
  if (isset($target['summary_callback'])) {
    $target['summary_callbacks'][] = $target['summary_callback'];
  }
  if (isset($target['form_callback'])) {
    $target['form_callbacks'][] = $target['form_callback'];
  }
  unset($target['summary_callback'], $target['form_callback']);

  static $callback_keys = array(
    'summary_callbacks',
    'form_callbacks',
    'preprocess_callbacks',
    'unique_callbacks',
  );

  // Filter out any incorrect callbacks. Do it here so it only has to be done
  // once.
  foreach ($callback_keys as $callback_key) {
    $target[$callback_key] = array_filter($target[$callback_key], 'is_callable');
  }

  // This makes checking in FeedsProcessor::mapToTarget() simpler.
  if (empty($target['callback']) || !is_callable($target['callback'])) {
    unset($target['callback']);
  }
}
