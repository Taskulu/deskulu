<?php
/**
  * @file
  * VBO action to cancel user accounts.
  */

function views_bulk_operations_user_cancel_action_info() {
  return array('views_bulk_operations_user_cancel_action' => array(
    'type' => 'user',
    'label' => t('Cancel user account'),
    'configurable' => TRUE,
    'behavior' => array('deletes_property'),
    'triggers' => array('any'),
  ));
}

function views_bulk_operations_user_cancel_action_form($context) {
  module_load_include('inc', 'user', 'user.pages');
  $form['user_cancel_method'] = array(
    '#type' => 'item',
    '#title' => t('When cancelling these accounts'),
  );
  $form['user_cancel_method'] += user_cancel_methods();
  // Remove method descriptions.
  foreach (element_children($form['user_cancel_method']) as $element) {
    unset($form['user_cancel_method'][$element]['#description']);
  }
  $admin_access = user_access('administer users');
  $default_notify = variable_get('user_mail_status_canceled_notify', FALSE);
  $form['user_cancel_notify'] = array(
    '#type' => 'checkbox',
    '#title' => t('Notify user when account is canceled.'),
    '#default_value' => ($admin_access ? FALSE : $default_notify),
    '#access' => $admin_access && $default_notify,
    '#description' => t('When enabled, the user will receive an e-mail notification after the account has been cancelled.'),
  );

  return $form;
}

function views_bulk_operations_user_cancel_action_submit($form, $form_state) {
  return array(
    'user_cancel_method' => $form_state['values']['user_cancel_method'],
    'user_cancel_notify' => $form_state['values']['user_cancel_notify'],
  );
}

function views_bulk_operations_user_cancel_action($account, $context) {
  global $user;
  // Prevent the user from cancelling itself.
  if ($account->uid == $user->uid) {
    return;
  }

  // Allow other modules to react on the cancellation.
  if ($context['user_cancel_method'] != 'user_cancel_delete') {
    module_invoke_all('user_cancel', array(), $account, $context['user_cancel_method']);
  }

  switch ($context['user_cancel_method']) {
    case 'user_cancel_block':
    case 'user_cancel_block_unpublish':
    default:
      // Send account blocked notification if option was checked.
      if (!empty($context['user_cancel_notify'])) {
        _user_mail_notify('status_blocked', $account);
      }
      user_save($account, array('status' => 0));
      watchdog('user', 'Blocked user: %name %email.', array('%name' => $account->name, '%email' => '<' . $account->mail . '>'), WATCHDOG_NOTICE);
      break;

    case 'user_cancel_reassign':
    case 'user_cancel_delete':
      // Send account canceled notification if option was checked.
      if (!empty($context['user_cancel_notify'])) {
        _user_mail_notify('status_canceled', $account);
      }
      user_delete($account->uid);
      watchdog('user', 'Deleted user: %name %email.', array('%name' => $account->name, '%email' => '<' . $account->mail . '>'), WATCHDOG_NOTICE);
      break;
  }
}
