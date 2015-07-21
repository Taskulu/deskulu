<?php

/**
 * Implements hook_rules_data_info().
 */
function incoming_mail_rules_data_info() {
  $data_info['field_item_file'] = array(
    'label' => t('Field File'),
    'wrap' => TRUE,
    'property info' => entity_property_field_item_file_info(),
  );
  $data_info['field_item_image'] = array(
    'label' => t('Field Image'),
    'wrap' => TRUE,
    'property info' => entity_property_field_item_image_info(),
  );

  return $data_info;
}

/**
 * Implements hook_rules_event_info().
 */
function incoming_mail_rules_event_info() {
  $events = array();

  $events['incoming_mail_ticket_new'] = array(
    'label' => t('A new ticket is received through email'),
    'group' => t('Incoming Email Handler'),
    'variables' => array(
      'subject' => array(
        'type' => 'text',
        'label' => t('Subject'),
      ),
      'body' => array(
        'type' => 'text',
        'label' => t('Body'),
      ),
      'files' => array(
        'type' => 'list<field_item_file>',
        'label' => t('Attachments'),
      ),
      'name' => array(
        'type' => 'text',
        'label' => t('Sender name'),
      ),
      'email' => array(
        'type' => 'text',
        'label' => t('Sender email'),
      ),
    ),
  );

  $events['incoming_mail_ticket_reply'] = array(
    'label' => t('New reply for ticket is received through email'),
    'group' => t('Incoming Email Handler'),
    'variables' => array(
      'ticket' => array(
        'type' => 'node',
        'label' => t('Ticket'),
      ),
      'body' => array(
        'type' => 'text',
        'label' => t('Body'),
      ),
      'files' => array(
        'type' => 'list<field_item_file>',
        'label' => t('Attachments'),
      ),
      'name' => array(
        'type' => 'text',
        'label' => t('Sender name'),
      ),
      'email' => array(
        'type' => 'text',
        'label' => t('Sender email'),
      ),
    ),
  );

  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function incoming_mail_rules_action_info() {
  $actions = array(
    'incoming_mail_get_user_by_email' => array(
      'label' => t('Get or create user by email'),
      'group' => t('User'),
      'parameter' => array(
        'email' => array(
          'type' => 'text',
          'label' => t('Email'),
        ),
        'name' => array(
          'type' => 'text',
          'label' => t('Name'),
          'optional' => TRUE,
        ),
      ),
      'provides' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
        ),
      ),
    ),
    'incoming_mail_generate_random_string' => array(
      'label' => t('Generate random string'),
      'group' => t('Strings'),
      'provides' => array(
        'random_string' => array(
          'type' => 'text',
          'label' => t('Random string')
        ),
      ),
    ),
  );

  return $actions;
}

function incoming_mail_get_user_by_email($email, $name = '') {
  $account = user_load_by_mail($email);
  if ($account) {
    if ($name && $account->field_full_name[LANGUAGE_NONE][0]['value'] != $name) {
      $account->field_full_name[LANGUAGE_NONE][0]['value'] = $name;
      user_save($account);
    }
    return array('user' => $account);
  }
  else {
    $fields = array(
      'name' => substr($email, 0, strpos($email, '@')),
      'mail' => $email,
      'pass' => user_password(8),
      'status' => 1,
      'init' => 1,
      'roles' => array(
        DRUPAL_AUTHENTICATED_RID => 'authenticated user',
      ),
    );
    if (!empty($name)) {
      $fields += array(
        'field_full_name' => array(
          LANGUAGE_NONE => array(
            0 => array(
              'value' => $name,
            ),
          ),
        )
      );
    }
    $account = user_save('', $fields);
    watchdog('incoming_mail', 'user: <pre>@u</pre>', ['@u' => var_export($account, 1)]);
    drupal_mail('user', 'register_no_approval_required', $email, NULL, array('account' => $account), variable_get('site_mail', 'support@taskulu.com'));
    return array('user' => $account);
  }
}

function incoming_mail_generate_random_string() {
  return array('random_string' => str_random());
}
