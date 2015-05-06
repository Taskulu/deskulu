<?php

/**
 * @file
 * Contains rules integration for the user module needed during evaluation.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Condition user: condition to check whether user has particular roles
 */
function rules_condition_user_has_role($account, $roles, $operation = 'AND') {
  switch ($operation) {
    case 'OR':
      foreach ($roles as $rid) {
        if (isset($account->roles[$rid])) {
          return TRUE;
        }
      }
      return FALSE;

    case 'AND':
      foreach ($roles as $rid) {
        if (!isset($account->roles[$rid])) {
          return FALSE;
        }
      }
      return TRUE;
  }
}

/**
 * Condition: User is blocked.
 */
function rules_condition_user_is_blocked($account) {
  return $account->status == 0;
}

/**
 * Action: Adds roles to a particular user.
 */
function rules_action_user_add_role($account, $roles) {
  if ($account->uid || !empty($account->is_new)) {
    // Get role list (minus the anonymous)
    $role_list = user_roles(TRUE);

    foreach ($roles as $rid) {
      $account->roles[$rid] = $role_list[$rid];
    }
    if (!empty($account->is_new) && $account->uid) {
      // user_save() inserts roles after invoking hook_user_insert() anyway, so
      // we skip saving to avoid errors due saving them twice.
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

/**
 * Action: Remove roles from a given user.
 */
function rules_action_user_remove_role($account, $roles) {
  if ($account->uid || !empty($account->is_new)) {
    foreach ($roles as $rid) {
      // If the user has this role, remove it.
      if (isset($account->roles[$rid])) {
        unset($account->roles[$rid]);
      }
    }
    if (!empty($account->is_new) && $account->uid) {
      // user_save() inserts roles after invoking hook_user_insert() anyway, so
      // we skip saving to avoid errors due saving them twice.
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

/**
 * Action: Block a user.
 */
function rules_action_user_block($account) {
  $account->status = 0;
  drupal_session_destroy_uid($account->uid);
}

/**
 * Action: Unblock a user.
 */
function rules_action_user_unblock($account) {
  $account->status = 1;
}

/**
 * Action: Send a user account e-mail.
 */
function rules_action_user_send_account_email($account, $email_type) {
  // If we received an authenticated user account...
  if (!empty($account->uid)) {
    module_load_include('inc', 'rules', 'modules/user.rules');
    $types = rules_user_account_email_options_list();

    // Attempt to send the account e-mail.
    // This code is adapted from _user_mail_notify().
    $params = array('account' => $account);
    $language = user_preferred_language($account);
    $mail = drupal_mail('user', $email_type, $account->mail, $language, $params);
    if ($email_type == 'register_pending_approval') {
      // If a user registered requiring admin approval, notify the admin, too.
      // We use the site default language for this.
      drupal_mail('user', 'register_pending_approval_admin', variable_get('site_mail', ini_get('sendmail_from')), language_default(), $params);
    }

    $result = empty($mail) ? NULL : $mail['result'];

    // Log the success or failure.
    if ($result) {
      watchdog('rules', '%type e-mail sent to %recipient.', array('%type' => $types[$email_type], '%recipient' => $account->mail));
    }
    else {
      watchdog('rules', 'Failed to send %type e-mail to %recipient.', array('%type' => $types[$email_type], '%recipient' => $account->mail));
    }
  }
}

/**
 * @}
 */
