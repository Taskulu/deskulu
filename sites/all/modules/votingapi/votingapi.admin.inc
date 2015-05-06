<?php

/**
 * @file
 * Configuration forms and helper functions for VotingAPI module.
 */

/**
 * Administrative settings for VotingAPI.
 */
function votingapi_settings_form($form_state) {
  $period = array(0 => t('Immediately')) + drupal_map_assoc(array(300, 900, 1800, 3600, 10800, 21600, 32400, 43200, 86400, 172800, 345600, 604800), 'format_interval') + array(-1 => t('Never'));

  $form['votingapi_anonymous_window'] = array(
    '#type' => 'select',
    '#title' => t('Anonymous vote rollover'),
    '#description' => t('The amount of time that must pass before two anonymous votes from the same computer are considered unique. Setting this to \'never\' will eliminate most double-voting, but will make it impossible for multiple anonymous on the same computer (like internet cafe customers) from casting votes.'),
    '#default_value' => variable_get('votingapi_anonymous_window', 86400),
    '#options' => $period
  );

  $form['votingapi_user_window'] = array(
    '#type' => 'select',
    '#title' => t('Registered user vote rollover'),
    '#description' => t('The amount of time that must pass before two registered user votes from the same user ID are considered unique. Setting this to \'never\' will eliminate most double-voting for registered users.'),
    '#default_value' => variable_get('votingapi_user_window', -1),
    '#options' => $period
  );

  $form['votingapi_calculation_schedule'] = array(
    '#type' => 'radios',
    '#title' => t('Vote tallying'),
    '#description' => t('On high-traffic sites, administrators can use this setting to postpone the calculation of vote results.'),
    '#default_value' => variable_get('votingapi_calculation_schedule', 'immediate'),
    '#options' => array(
      'immediate' => t('Tally results whenever a vote is cast'),
      'cron' => t('Tally results at cron-time'),
      'manual' => t('Do not tally results automatically: I am using a module that manages its own vote results.')
    ),
  );

  return system_settings_form($form);
}

/**
 * Developer tool to generate dummy votes.
 */
function votingapi_generate_votes_form() {
  $form['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Which node types should receive votes?'),
    '#options' => node_type_get_names(),
    '#default_value' => array_keys(node_type_get_names()),
  );

  $form['vote_type'] = array(
    '#type' => 'select',
    '#title' => t('What type of votes should be generated?'),
    '#options' => array(
      'percent' => t('Percentage (Fivestar style)'),
      'points' => t('Point-based (Digg style)'),
    ),
    '#default_value' => 'percent',
  );

  $form['kill_votes'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete existing votes before generating new ones.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Do it!'),
  );
  return $form;
}

/**
 * Submit handler for votingapi_generate_votes_form.
 */
function votingapi_generate_votes_form_submit($form, &$form_state) {
  $options = array(
    'types' => $form_state['values']['node_types'],
    'kill' => $form_state['values']['kill_votes'],
  );
  require_once(drupal_get_path('module', 'votingapi') . '/votingapi.devel.inc');
  votingapi_generate_votes('node', $form_state['values']['vote_type'], $options);
}
