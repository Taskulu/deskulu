<?php

/**
 * @file
 * Rules Scheduler Drush integration.
 */

/**
 * Implements hook_drush_command().
 */
function rules_scheduler_drush_command() {
  $items = array();

  $items['rules-scheduler-tasks'] = array(
    'description' => 'Checks for scheduled tasks to be added to the queue.',
    'options' => array(
      'claim' => 'Optionally claim tasks from the queue to work on. Any value set will override the default time spent on this queue.',
    ),
    'drupal dependencies' => array('rules', 'rules_scheduler'),
    'aliases' => array('rusch'),
    'examples' => array(
      'drush rusch' => 'Add scheduled tasks to the queue.',
      'drush rusch --claim' => 'Add scheduled tasks to the queue and claim items for the default amount of time.',
      'drush rusch --claim=30' => 'Add schedules tasks to the queue and claim items for 30 seconds.',
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function rules_scheduler_drush_help($section) {
  switch ($section) {
    case 'drush:rules-scheduler-tasks':
      return dt('Checks for scheduled tasks to be added the queue. Can optionally claim tasks from the queue to work on.');
  }
}

/**
 * Command callback for processing the rules_scheduler_tasks queue.
 *
 * @see rules_scheduler_cron_queue_info().
 * @see rules_scheduler_cron().
 */
function drush_rules_scheduler_tasks() {
  if (rules_scheduler_queue_tasks()) {
    // hook_exit() is not invoked for drush runs, so register it as shutdown
    // callback for logging the rules log to the watchdog.
    drupal_register_shutdown_function('rules_exit');
    // Clear the log before running tasks via the queue to avoid logging
    // unrelated logs from previous operations.
    RulesLog::logger()->clear();
    drush_log(dt('Added scheduled tasks to the queue.'), 'success');
  }

  $claim = drush_get_option('claim', FALSE);
  if ($claim) {
    // Fetch the queue information and let other modules alter it.
    $queue_name = 'rules_scheduler_tasks';
    $info = module_invoke('rules_scheduler', 'cron_queue_info');
    drupal_alter('cron_queue_info', $info);

    $function = $info[$queue_name]['worker callback'];
    // The drush option can override the default process time.
    $time = is_numeric($claim) ? (int) $claim : $info[$queue_name]['time'];
    $end = time() + $time;
    // Claim items and process the queue.
    $queue = DrupalQueue::get($queue_name);
    $claimed = 0;
    while (time() < $end && ($item = $queue->claimItem())) {
      $function($item->data);
      $queue->deleteItem($item);
      $claimed++;
    }
    if ($claimed) {
      drush_log(dt('Claimed and worked on !claimed scheduled tasks for up to !time seconds.', array('!claimed' => $claimed, '!time' => $time)), 'success');
    }
  }
}
