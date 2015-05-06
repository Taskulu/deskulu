<?php

/**
 * Default scheduled task handler.
 */
class RulesSchedulerDefaultTaskHandler implements RulesSchedulerTaskHandlerInterface {

  /**
   * The task array.
   *
   * @var array
   */
  protected $task;

  /**
   * Constructs a repetitive task handler object.
   */
  public function __construct(array $task) {
    $this->task = $task;
  }

  /**
   * Implements RulesSchedulerTaskHandlerInterface::runTask().
   */
  public function runTask() {
    if ($component = rules_get_cache('comp_' . $this->task['config'])) {
      $replacements = array('%label' => $component->label(), '%plugin' => $component->plugin());
      $replacements['%identifier'] = $this->task['identifier'] ? $this->task['identifier'] : t('without identifier');
      rules_log('Scheduled evaluation of %plugin %label, task %identifier.', $replacements, RulesLog::INFO, $component, TRUE);
      $state = unserialize($this->task['data']);
      $state->restoreBlocks();
      // Block the config to prevent any future recursion.
      $state->block($component);
      // Finally evaluate the component with the given state.
      $component->evaluate($state);
      $state->unblock($component);
      rules_log('Finished evaluation of %plugin %label, task %identifier.', $replacements, RulesLog::INFO, $component, FALSE);
      $state->cleanUp();
    }
  }

  /**
   * Implements RulesSchedulerTaskHandlerInterface::afterTaskQueued().
   */
  public function afterTaskQueued() {
    // Delete the task from the task list.
    db_delete('rules_scheduler')
      ->condition('tid', $this->task['tid'])
      ->execute();
  }

  /**
   * Implements RulesSchedulerTaskHandlerInterface::getTask().
   */
  public function getTask() {
    return $this->task;
  }

}

/**
 * Interface for scheduled task handlers.
 *
 * Task handlers control the behavior of a task when it's queued or executed.
 * Unless specified otherwise, the RulesSchedulerDefaultTaskHandler task handler
 * is used.
 *
 * @see rules_scheduler_run_task()
 * @see rules_scheduler_cron()
 * @see RulesSchedulerDefaultTaskHandler
 */
interface RulesSchedulerTaskHandlerInterface {

  /**
   * Processes a queue item.
   *
   * @throws RulesEvaluationException
   *   If there are any problems executing the task.
   *
   * @see rules_scheduler_run_task()
   */
  public function runTask();

  /**
   * Processes a task after it has been queued.
   *
   * @see rules_scheduler_cron()
   */
  public function afterTaskQueued();

  /**
   * Returns the task associated with the task handler.
   *
   * @return array
   *   The task (queue item) array.
   */
  public function getTask();

}
