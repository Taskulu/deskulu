<?php

/**
 * @file
 * Rules actions for sending Mime-encoded emails.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implements hook_rules_action_info().
 */
function mimemail_rules_action_info() {
  return array(
    'mimemail' => array(
      'label' => t('Send HTML e-mail'),
      'group' => t('System'),
      'parameter' => array(
        'key' => array(
          'type' => 'text',
          'label' => t('Key'),
          'description' => t('A key to identify the e-mail sent.'),
        ),
        'to' => array(
          'type' => 'text',
          'label' => t('To'),
          'description' => t("The mail's recipient address. The formatting of this string must comply with RFC 2822."),
        ),
        'cc' => array(
          'type' => 'text',
          'label' => t('CC Recipient'),
          'description' => t("The mail's carbon copy address. You may separate multiple addresses with comma."),
          'optional' => TRUE,
        ),
        'bcc' => array(
          'type' => 'text',
          'label' => t('BCC Recipient'),
          'description' => t("The mail's blind carbon copy address. You may separate multiple addresses with comma."),
          'optional' => TRUE,
        ),
        'from_name' => array(
          'type' => 'text',
          'label' => t('Sender name'),
          'description' => t("The sender's name. Leave it empty to use the site-wide configured name."),
          'optional' => TRUE,
        ),
        'from_mail' => array(
          'type' => 'text',
          'label' => t('Sender e-mail address'),
          'description' => t("The sender's address. Leave it empty to use the site-wide configured address."),
          'optional' => TRUE,
        ),
        'reply_to' => array(
          'type' => 'text',
          'label' => t('Reply e-mail address'),
          'description' => t("The address to reply to. Leave it empty to use the sender's address."),
          'optional' => TRUE,
        ),
        'list_unsubscribe' => array(
          'type' => 'text',
          'label' => t('Unsubscription e-mail and/or URL'),
          'description' => t("An e-mail address and/or a URL which can be used for unsubscription. Values must be enclosed by angel brackets and separated by a comma."),
          'optional' => TRUE,
        ),
        'subject' => array(
          'type' => 'text',
          'label' => t('Subject'),
          'description' => t("The mail's subject."),
          'translatable' => TRUE,
        ),
        'body' => array(
          'type' => 'text',
          'label' => t('Body'),
          'description' => t('The mail\'s HTML body. Will be formatted using the text format selected on the <a href="@url">settings</a> page.', array('@url' => url('admin/config/system/mimemail'))),
          'sanitize' => TRUE,
          'optional' => TRUE,
          'translatable' => TRUE,
        ),
        'plaintext' => array(
          'type' => 'text',
          'label' => t('Plain text body'),
          'description' => t("The mail's plaintext body."),
          'optional' => TRUE,
          'translatable' => TRUE,
        ),
        'attachments' => array(
          'type' => 'text',
          'label' => t('Attachments'),
          'description' => t("The mail's attachments, one file per line e.g. \"files/images/mypic.png\" without quotes."),
          'optional' => TRUE,
        ),
        'language' => array(
          'type' => 'token',
          'label' => t('Language'),
          'description' => t('If specified, the language used for getting the mail message and subject.'),
          'options list' => 'entity_metadata_language_list',
          'optional' => TRUE,
          'default value' => LANGUAGE_NONE,
          'default mode' => 'selector',
        ),
      ),
      'base' => 'rules_action_mimemail',
      'access callback' => 'rules_system_integration_access',
    ),
    'mimemail_to_users_of_role' => array(
      'label' => t('Send HTML mail to all users of a role'),
      'group' => t('System'),
      'parameter' => array(
        'key' => array(
          'type' => 'text',
          'label' => t('Key'),
          'description' => t('A key to identify the e-mail sent.'),
        ),
        'roles' => array(
          'type' => 'list<integer>',
          'label' => t('Roles'),
          'options list' => 'entity_metadata_user_roles',
          'description' => t('Select the roles whose users should receive the mail.'),
        ),
        'active' => array(
          'type' => 'boolean',
          'label' =>('Send to active users'),
          'description' => t('Send mail only to active users.'),
        ),
        'from_name' => array(
          'type' => 'text',
          'label' => t('Sender name'),
          'description' => t("The sender's name. Leave it empty to use the site-wide configured name."),
          'optional' => TRUE,
        ),
        'from_mail' => array(
          'type' => 'text',
          'label' => t('Sender e-mail address'),
          'description' => t("The sender's address. Leave it empty to use the site-wide configured address."),
          'optional' => TRUE,
        ),
        'subject' => array(
          'type' => 'text',
          'label' => t('Subject'),
          'description' => t("The mail's subject."),
          'translatable' => TRUE,
        ),
        'body' => array(
          'type' => 'text',
          'label' => t('Body'),
          'description' => t("The mail's message HTML body."),
          'optional' => TRUE,
          'translatable' => TRUE,
        ),
        'plaintext' => array(
          'type' => 'text',
          'label' => t('Plaintext body'),
          'description' => t("The mail's message plaintext body."),
          'optional' => TRUE,
          'translatable' => TRUE,
        ),
        'attachments' => array(
          'type' => 'text',
          'label' => t('Attachments'),
          'description' => t("The mail's attachments, one file per line e.g. \"files/images/mypic.png\" without quotes."),
          'optional' => TRUE,
        ),
        'language_user' => array(
          'type' => 'boolean',
          'label' => t("Send mail in each recipient's language"),
          'description' => t("If checked, the mail message and subject will be sent in each user's preferred language. <strong>You can safely leave the language selector below empty if this option is selected.</strong>"),
        ),
        'language' => array(
          'type' => 'token',
          'label' => t('Fixed language'),
          'description' => t('If specified, the fixed language used for getting the mail message and subject.'),
          'options list' => 'entity_metadata_language_list',
          'optional' => TRUE,
          'default value' => LANGUAGE_NONE,
          'default mode' => 'selector',
        ),
      ),
      'base' => 'rules_action_mimemail_to_users_of_role',
      'access callback' => 'rules_system_integration_access',
    ),
  );
}

/**
 * Implements hook_rules_action_base_upgrade_map_name().
 */
function mimemail_rules_action_mail_upgrade_map_name($element) {
  return 'mimemail';
}

/**
 * Implements hook_rules_action_base_upgrade_map_name().
 */
function mimemail_rules_action_mail_to_user_upgrade_map_name($element) {
  return 'mimemail';
}

/**
 * Implements hook_rules_action_base_upgrade_map_name().
 */
function mimemail_rules_action_mail_to_users_of_role_upgrade_map_name($element) {
  return 'mimemail_to_users_of_role';
}

/**
 * Implements hook_rules_action_base_upgrade().
 */
function mimemail_rules_action_mail_upgrade($element, RulesPlugin $target) {
  $target->settings['key'] = $element['#settings']['key'];
  $target->settings['from_name'] = $element['#settings']['sender'];
  $target->settings['from_mail'] = $element['#settings']['from'];
  $target->settings['body'] = $element['#settings']['message_html'];
  $target->settings['plaintext'] = $element['#settings']['message_plaintext'];
}

/**
 * Implements hook_rules_action_base_upgrade().
 */
function mimemail_rules_action_mail_to_user_upgrade($element, RulesPlugin $target) {
  switch ($element['#settings']['#argument map']['user']) {
    case 'author':
      $token = 'node:author';
      break;
    case 'author_unchanged':
      $token = 'node-unchanged:author';
      break;
    case 'user':
      $token = 'site:current-user';
      break;
  }
  $target->settings['to:select'] = $token . ':mail';
  mimemail_rules_action_mail_upgrade($element, $target);
}

/**
 * Implements hook_rules_action_base_upgrade().
 */
function mimemail_rules_action_mail_to_users_of_role_upgrade($element, RulesPlugin $target) {
  $target->settings['roles'] = $element['#settings']['recipients'];
  mimemail_rules_action_mail_upgrade($element, $target);
}

/**
 * Action Implementation: Send HTML mail.
 */
function rules_action_mimemail($key, $to, $cc = NULL, $bcc = NULL, $from_name = NULL, $from_mail = NULL, $reply_to = NULL, $list_unsubscribe = NULL, $subject, $body, $plaintext = NULL, $attachments = array(), $langcode, $settings, RulesState $state, RulesPlugin $element) {
  module_load_include('inc', 'mimemail');

  // Set the sender name and from address.
  if (empty($from_mail)) {
    $from = NULL;
  }
  else {
    $from = array(
      'name' => $from_name,
      'mail' => $from_mail,
    );
    // Create an address string.
    $from = mimemail_address($from);
  }

  // Figure out the language to use - fallback is the system default.
  $languages = language_list();
  $language = isset($languages[$langcode]) ? $languages[$langcode] : language_default();

  $params = array(
    'context' => array(
      'subject' => $subject,
      'body' => $body,
      'action' => $element,
      'state' => $state,
    ),
    'cc' => $cc,
    'bcc' => $bcc,
    'reply-to' => $reply_to,
    'list-unsubscribe' => $list_unsubscribe,
    'plaintext' => $plaintext,
    'attachments' => $attachments,
  );

  drupal_mail('mimemail', $key, $to, $language, $params, $from);
}

/**
 * Action: Send HTML mail to all users of a specific role group(s).
 */
function rules_action_mimemail_to_users_of_role($key, $roles, $active, $from_name = NULL, $from_mail = NULL, $subject, $body, $plaintext = NULL, $attachments = array(), $use_userlang = FALSE, $langcode= NULL, $settings, RulesState $state, RulesPlugin $element) {
  module_load_include('inc', 'mimemail');

  // Set the sender name and from address.
  if (empty($from_mail)) {
    $from = NULL;
  }
  else {
    $from = array(
      'name' => $from_name,
      'mail' => $from_mail,
    );
    // Create an address string.
    $from = mimemail_address($from);
  }

  $query = db_select('users', 'u');
  $query->fields('u', array('mail', 'language'));

  if ($active) {
    $query->condition('u.status', 1, '=');
  }

  if (in_array(DRUPAL_AUTHENTICATED_RID, $roles)) {
    $query->condition('u.uid', 0, '>');
  }
  else {
    $query->join('users_roles', 'r', 'u.uid = r.uid');
    $query->condition('r.rid', $roles, 'IN');
    $query->distinct();
  }

  $result = $query->execute();

  $params = array(
    'context' => array(
      'subject' => $subject,
      'body' => $body,
      'action' => $element,
      'state' => $state,
    ),
    'plaintext' => $plaintext,
    'attachments' => $attachments,
  );

  // Create language list before initializing foreach.
  $languages = language_list();

  $message = array('result' => TRUE);
  foreach ($result as $row) {
    // Decide which language to use.
    if (!$use_userlang || empty($row->language) || !isset($languages[$row->language])) {
      $language = isset($languages[$langcode]) ? $languages[$langcode] : language_default();
    }
    else {
      $language = $languages[$row->language];
    }

    $message = drupal_mail('mimemail', $key, $row->mail, $language, $params, $from);
    if (!$message['result']) {
      break;
    }
  }
  if ($message['result']) {
    $role_names = array_intersect_key(user_roles(TRUE), array_flip($roles));
    watchdog('rules', 'Successfully sent HTML email to the role(s) %roles.', array('%roles' => implode(', ', $role_names)));
  }
}

/**
 * @}
 */

