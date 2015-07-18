<?php

/**
 * @file
 * Job Scheduler Trigger admin pages
 */
function job_scheduler_trigger_admin_overview() {
  $base_path = 'admin/config/system/job_scheduler';
  $destination = drupal_get_destination();
  $header = array(
    'title' => t('Title'),
    'crontab' => t('Cron tab'),
    'status' => t('Status'),
    'last' => t('Last'),
    'next' => t('Next'),
    'operations' => t('Operations'),
  );

  $options = array();
  foreach (job_scheduler_trigger_list() as $trigger) {
    $next_time = job_scheduler_trigger_next_time($trigger->crontab);
    $options[$trigger->trid] = array(
      'title' => check_plain($trigger->title),
      'crontab' => check_plain($trigger->crontab),
      'status' => $trigger->status ? t('Enabled') : t('Disabled'),
      'last' => $trigger->last ? format_date($trigger->last) : t('Never'),
      'next' => $next_time ? format_date($next_time) : t('Never'),
    );
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => $base_path . '/' . $trigger->trid,
      'query' => $destination,
    );
    $options[$trigger->trid]['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );
  }

  $form['triggers'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $options,
    '#empty' => t('No triggers created yet.'),
  );

  $form['add']['#markup'] = l(t('Add a new trigger'), $base_path . '/add', array('query' => $destination));

  return $form;
}


/**
 * Edit form for a trigger
 */
function job_scheduler_trigger_edit_form($form, &$form_state, $trigger) {
  if (!$trigger) {
    $trigger = (object)array('trid' => NULL, 'title' => t('New trigger'), 'crontab' => '* * * * *', 'status' => 0, 'last' => 0);
  }
  $form['trigger'] = array('#tree' => TRUE);
  $form['trigger']['trid'] = array('#type' => 'value', '#value' => $trigger->trid);
  $form['trigger']['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $trigger->title,
    '#required' => TRUE,
    '#description' => t('This is the name of the trigger that will be created when enabled.'),
  );
  $form['trigger']['crontab'] = array(
    '#type' => 'textfield',
    '#title' => t('Cron tab'),
    '#default_value' => $trigger->crontab,
    '#required' => TRUE,
    '#description' => t('Unix-style crontab line with: <i>minutes</i> <i>hours</i> <i>day-of-month</i> <i>month</i> <i>day-of-week</i>. Read <a href=@man-crontab>more about crontab</a>.', array('@man-crontab' => url('http://linux.die.net/man/5/crontab'))),
  );
  $form['trigger']['status'] = array(
    '#title' => t('Status'),
    '#type' => 'radios',
    '#options' => array(t('disabled'), t('enabled')),
    '#default_value' => $trigger->status,
    '#description' => t('When enabled, a trigger will be created with this crontab and jobs will be scheduled for it. Check everything is OK before enabling the trigger.'),
  );
  $form['trigger']['last'] = array(
     '#type' => 'item',
     '#title' => t('Last triggered'),
     '#markup' => format_date($trigger->last),
  );
  if ($trigger->trid && $trigger->crontab) {
    $next = job_scheduler_trigger_next_time($trigger->crontab);
    $form['trigger']['next'] = array(
       '#type' => 'item',
       '#title' => t('Next trigger'),
       '#markup' => $next ? format_date($next) : t('Never in the next year.'),
    );
  }
  $form['buttons']['update'] = array('#type' => 'submit', '#value' => t('Save'));

  if ($trigger->trid) {
    $form['buttons']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
  }

  return $form;
}

/**
 * Validate trigger
 */
function job_scheduler_trigger_edit_form_validate($form, &$form_state) {
  $trigger = $form_state['values']['trigger'];
  if ($trigger['crontab']) {
    if (!JobSchedulerCronTab::parse($trigger['crontab'])) {
      form_set_error('trigger][crontab', t('Invalid cron parameters.'));
    }
  }
}

/**
 * Submit trigger
 */
function job_scheduler_trigger_edit_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $trigger = $form_state['values']['trigger'];
  if ($op == t('Save')) {
    if (empty($trigger['trid'])) {
      // Create and then assign trigger name and create trigger
      drupal_write_record('job_scheduler_trigger', $trigger);
      $trigger['hook'] = 'job_scheduler_' . $trigger['trid'];
      drupal_write_record('job_scheduler_trigger', $trigger, 'trid');
      drupal_set_message(t('A new trigger has been created.'));
    }
    else {
      drupal_write_record('job_scheduler_trigger', $trigger, 'trid');
      drupal_set_message(t('The trigger has been updated.'));
    }
  }
  elseif ($op == t('Delete')) {
    db_delete('job_scheduler_trigger')
      ->condition('trid', $trigger['trid'])
      ->execute();
    drupal_set_message(t('The trigger has been deleted.'));
    // @todo Delete scheduled jobs
  }
  // Synchronize triggers, actions, scheduled jobs, etc..
  actions_synchronize();
  job_scheduler_rebuild_all();
}

/**
 * Quick calculate next time cron
 */
function job_scheduler_trigger_next_time($crontab) {
  $crontab = new JobSchedulerCronTab($crontab);
  return $crontab->cron ? $crontab->nextTime(REQUEST_TIME) : 0;
}
