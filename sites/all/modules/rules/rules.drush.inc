<?php

/**
 * @file
 * Rules module drush integration.
 */

/**
 * Implements hook_drush_command().
 *
 * @return
 *   An associative array describing your command(s).
 *
 * @see drush_parse_command()
 */
function rules_drush_command() {
  $items = array();

  $items['rules-list'] = array(
    'description' => "List all the active and inactive rules for your site.",
    'drupal dependencies' => array('rules'),
    'aliases' => array('rules'),
  );
  $items['rules-enable'] = array(
    'description' => "Enable a rule on your site.",
    'arguments' => array(
      'rule' => 'Rule name to enable.',
    ),
    'drupal dependencies' => array('rules'),
    'aliases' => array('re'),
  );
  $items['rules-disable'] = array(
    'description' => "Disable a rule on your site.",
    'arguments' => array(
      'rule' => 'Rule name to export.',
    ),
    'drupal dependencies' => array('rules'),
    'aliases' => array('rd'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function rules_drush_help($section) {
  switch ($section) {
    case 'drush:rules':
      return dt("List all the rules on your site.");
    case 'drush:rules-enable':
      return dt("Enable/activate a rule on your site.");
    case 'drush:rules-disable':
      return dt("Disable/deactivate a rule on your site.");
  }
}

/**
 * Get a list of all rules.
 */
function drush_rules_list() {
  $rules = rules_config_load_multiple(FALSE);
  $rows = array(array(dt('Rule'), dt('Label'), dt('Event'), dt('Active'), dt('Status')));
  foreach ($rules as $rule) {
    if (!empty($rule->name) && !empty($rule->label)) {
      $events = array();
      $event_info = rules_fetch_data('event_info');
      if ($rule instanceof RulesTriggerableInterface) {
        foreach ($rule->events() as $event_name) {
          $event_info += array($event_name => array('label' => dt('Unknown event "!event_name"', array('!event_name' => $event_name))));
          $events[] = check_plain($event_info[$event_name]['label']);
        }
      }
      $rows[] = array(
        $rule->name,
        $rule->label,
        implode(', ', $events),
        $rule->active ? dt('Enabled') : dt('Disabled'),
        $rule->status ? theme('entity_status', array('status' => $rule->status, 'html' => FALSE)) : '',
      );
    }
  }
  drush_print_table($rows, TRUE);
}

/**
 * Enable a rule on the site.
 */
function drush_rules_enable() {
  $args = func_get_args();
  $rule_name = (!empty($args) && is_array($args)) ? array_shift($args) : '';
  if (empty($rule_name)) {
    return drush_set_error('', 'No rule name given.');
  }

  $rule = rules_config_load($rule_name);
  if (empty($rule)) {
    return drush_set_error('', dt('Could not load rule named "!rule-name".', array('!rule-name' => $rule_name)));
  }

  if (empty($rule->active)) {
    $rule->active = TRUE;
    $rule->save();
    drush_log(dt('The rule "!name" has been enabled.', array('!name' => $rule_name)), 'success');
  }
  else {
    drush_log(dt('The rule "!name" is already enabled.', array('!name' => $rule_name)), 'warning');
  }
}

/**
 * Disable a rule on the site.
 */
function drush_rules_disable() {
  $args = func_get_args();
  $rule_name = (!empty($args) && is_array($args)) ? array_shift($args) : '';
  if (empty($rule_name)) {
    return drush_set_error('', 'No rule name given.');
  }

  $rule = rules_config_load($rule_name);
  if (empty($rule)) {
    return drush_set_error('', dt('Could not load rule named "!rule-name".', array('!rule-name' => $rule_name)));
  }

  if (!empty($rule->active)) {
    $rule->active = FALSE;
    $rule->save();
    drush_log(dt('The rule "!name" has been disabled.', array('!name' => $rule_name)), 'success');
  }
  else {
    drush_log(dt('The rule "!name" is already disabled.', array('!name' => $rule_name)), 'warning');
  }
}
