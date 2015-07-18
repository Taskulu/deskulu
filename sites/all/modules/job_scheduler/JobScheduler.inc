<?php

/**
 * @file
 * JobScheduler class.
 */

/**
 * Use to make Job Scheduler exceptions identifiable by type.
 */
class JobSchedulerException extends Exception {}

/**
 * Manage scheduled jobs.
 */
class JobScheduler {
  /**
   * The name of this scheduler.
   */
  protected $name;

  /**
   * Produces a single instance of JobScheduler for a schedule name.
   */
  public static function get($name) {
    static $schedulers;
    // Instantiante a new scheduler for $name if we haven't done so yet.
    if (!isset($schedulers[$name])) {
      $class = variable_get('job_scheduler_class_' . $name, 'JobScheduler');
      $schedulers[$name] = new $class($name);
    }
    return $schedulers[$name];
  }

  /**
   * Creates a JobScheduler object.
   */
  protected function __construct($name) {
    $this->name = $name;
  }

  /**
   * Returns scheduler info.
   *
   * @see hook_cron_job_scheduler_info()
   *
   * @throws JobSchedulerException.
   */
  public function info() {
    if ($info = job_scheduler_info($this->name)) {
      return $info;
    }
    throw new JobSchedulerException('Could not find Job Scheduler cron information for ' . check_plain($this->name));
  }

  /**
   * Add a job to the schedule, replace any existing job.
   *
   * A job is uniquely identified by $job = array(type, id).
   *
   * @param $job
   *   An array that must contain the following keys:
   *   'type'     - A string identifier of the type of job.
   *   'id'       - A numeric identifier of the job.
   *   'period'   - The time when the task should be executed.
   *   'periodic' - True if the task should be repeated periodically.
   *
   * @code
   * function worker_callback($job) {
   *   // Work off job.
   *   // Set next time to be called. If this portion of the code is not
   *   // reached for some reason, the scheduler will keep periodically invoking
   *   // the callback() with the period value initially specified.
   *   $scheduler->set($job);
   * }
   * @endcode
   */
  public function set($job) {
    $job['name'] = $this->name;
    $job['last'] = REQUEST_TIME;
    if (!empty($job['crontab'])) {
      $crontab = new JobSchedulerCronTab($job['crontab']);
      $job['next'] = $crontab->nextTime(REQUEST_TIME);
    }
    else {
      $job['next'] = REQUEST_TIME + $job['period'];
    }
    $job['scheduled'] = 0;
    $this->remove($job);
    drupal_write_record('job_schedule', $job);
  }

  /**
   * Reserve a job.
   */
  protected function reserve($job) {
    $job['name'] = $this->name;
    $job['scheduled'] =
    $job['last'] = REQUEST_TIME;
    $job['next'] = $job['period'] + REQUEST_TIME;
    drupal_write_record('job_schedule', $job, array('name', 'type', 'id'));
  }

  /**
   * Remove a job from the schedule, replace any existing job.
   *
   * A job is uniquely identified by $job = array(type, id).
   */
  public function remove($job) {
    db_delete('job_schedule')
      ->condition('name', $this->name)
      ->condition('type', $job['type'])
      ->condition('id', isset($job['id']) ? $job['id'] : 0)
      ->execute();
  }

  /**
   * Remove all jobs for a given type.
   */
  public function removeAll($type) {
    db_delete('job_schedule')
      ->condition('name', $this->name)
      ->condition('type', $type)
      ->execute();
  }

  /**
   * Dispatches a job.
   *
   * Executes a worker callback or if schedule declares a queue name, queues a
   * job for execution.
   *
   * @param $job
   *   A $job array as passed into set() or read from job_schedule table.
   *
   * @throws Exception
   *   Exceptions thrown by code called by this method are passed on.
   */
  public function dispatch($job) {
    $info = $this->info();
    if (!$job['periodic']) {
      $this->remove($job);
    }
    if (!empty($info['queue name'])) {
      if (DrupalQueue::get($info['queue name'])->createItem($job)) {
        $this->reserve($job);
      }
    }
    else {
      $this->execute($job);
    }
  }

  /**
   * Executes a job that
   *
   * @param $job
   *   A $job array as passed into set() or read from job_schedule table.
   *
   * @throws Exception
   *   Exceptions thrown by code called by this method are passed on.
   */
  public function execute($job) {
    $info = $this->info();
    // If the job is periodic, re-schedule it before calling the worker, just in case
    if ($job['periodic']) {
      $job['last'] = REQUEST_TIME;
      $this->reschedule($job);
    }
    if (function_exists($info['worker callback'])) {
      call_user_func($info['worker callback'], $job);
    }
    else {
      // @todo If worker doesn't exist anymore we should do something about it, remove and throw exception?
      $this->remove($job);
      throw new JobSchedulerException('Could not find worker callback function: ' . $info['worker callback']);
    }
  }

  /**
   * Re-schedule a job if intended to run again
   *
   * (If cannot determine the next time, drop the job)
   */
  public function reschedule($job) {
    $job['scheduled'] = 0;
    if (!empty($job['crontab'])) {
      $crontab = new JobSchedulerCronTab($job['crontab']);
      $job['next'] = $crontab->nextTime($job['last']);
    }
    else {
      $job['next'] = $job['last'] + $job['period'];
    }

    if ($job['next']) {
      drupal_write_record('job_schedule', $job, array('item_id'));
    }
    else {
      // If no next time, it may mean it wont run again the next year (crontab)
      $this->remove($job);
    }
  }

  /**
   * Check whether a job exists in the queue and update its parameters if so
   */
  public function check($job) {
    $job += array('id' => 0, 'period' => 0, 'crontab' => '');
    $existing = db_select('job_schedule')
      ->fields('job_schedule')
      ->condition('name', $this->name)
      ->condition('type', $job['type'])
      ->condition('id', $job['id'])
      ->execute()
      ->fetchAssoc();
    // If existing, and changed period or crontab, we need to reschedule the job
    if ($existing) {
      if ($job['period'] != $existing['period'] || $job['crontab'] != $existing['crontab']) {
        $existing['period'] = $job['period'];
        $existing['crontab'] = $job['crontab'];
        $this->reschedule($existing);
      }
      return $existing;
    }
  }
}
