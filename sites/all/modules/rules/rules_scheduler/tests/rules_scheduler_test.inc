<?php

/**
 * @file
 * Include file for Rules Scheduler tests.
 */

/**
 * Test task handler class.
 */
class RulesTestTaskHandler extends RulesSchedulerDefaultTaskHandler {

  /**
   * Overrides RulesSchedulerDefaultTaskHandler::runTask().
   */
  public function runTask() {
    $task = $this->getTask();
    $data = unserialize($task['data']);

    // Set the variable defined in the test to TRUE.
    variable_set($data['variable'], TRUE);
  }

}
