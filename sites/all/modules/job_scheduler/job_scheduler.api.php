<?php

/**
 * @file
 * API documentation for hooks.
 */

/**
 * Declare job scheduling holding items that need to be run periodically.
 *
 * @return
 *   An associative array where the key is the queue name and the value is
 *   again an associative array. Possible keys are:
 *   - 'worker callback': The name of the function to call. It will be called
 *     at schedule time.
 *   - 'queue name': The name of the queue to use to queue this task. Must
 *     contain a valid queue name, declared by hook_cron_queue_info().
 *   If queue name is given, worker callback will be ignored.
 *
 * @see hook_cron_job_scheduler_info_alter()
 * @see hook_cron_queue_info()
 * @see hook_cron_queue_info_alter()
 */
function hook_cron_job_scheduler_info() {
  $info = array();
  $info['example_reset'] = array(
    'worker callback' => 'example_cache_clear_worker',
  );
  $info['example_import'] = array(
    'queue name' => 'example_import_queue',
  );
  return $info;
}

/**
 * Alter cron queue information before cron runs.
 *
 * Called by drupal_cron_run() to allow modules to alter cron queue settings
 * before any jobs are processesed.
 *
 * @param array $info
 *   An array of cron schedule information.
 *
 * @see hook_cron_queue_info()
 * @see drupal_cron_run()
 */
function hook_cron_job_scheduler_info_alter(&$info) {
  // Replace the default callback 'example_cache_clear_worker'
  $info['example_reset']['worker callback'] = 'my_custom_reset';
}
