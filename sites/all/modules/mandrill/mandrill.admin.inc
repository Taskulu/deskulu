<?php
/**
 * @file
 * Administrative forms for Mandrill module.
 */


/**
 * Administrative settings.
 *
 * @param array $form
 *   Form render array.
 * @param array $form_state
 *   Array containing form state values.
 *
 * @return array
 *   An array containing form items to place on the module settings page.
 */
function mandrill_admin_settings($form, &$form_state) {
  $key = variable_get('mandrill_api_key');
  $form['mandrill_api_key'] = array(
    '#title' => t('Mandrill API Key'),
    '#type' => 'textfield',
    '#description' => t('Create or grab your API key from the !link.',
      array('!link' => l(t('Mandrill settings'), 'https://mandrillapp.com/settings/index'))),
    '#default_value' => $key,
  );

  $library = libraries_detect('mandrill');

  if (!$library['installed']) {
    drupal_set_message(t('The Mandrill PHP library is not installed. Please see installation directions in README.txt'),
      'warning');
  }

  if ($key && $library['installed']) {
    $mailsystem_config_keys = mailsystem_get();
    $in_use = FALSE;
    $usage_rows = array();
    foreach ($mailsystem_config_keys as $key => $sys) {
      if ($sys === 'MandrillMailSystem' && $key != 'mandrill_test') {
        $in_use = TRUE;
        $usage_rows[] = array(
          $key,
          $sys,
        );
      }
    }
    if ($in_use) {
      $usage_array = array(
        '#theme' => 'table',
        '#header' => array(
          t('Module Key'),
          t('Mail System'),
        ),
        '#rows' => $usage_rows,
      );
      $form['mandrill_status'] = array(
        '#type' => 'markup',
        '#markup' => t('Mandrill is currently configured to be used by the following Module Keys. To change these settings or configure additional systems to use Mandrill, use !link.<br /><br />!table',
          array(
            '!link' => l(t('Mail System'), 'admin/config/system/mailsystem'),
            '!table' => drupal_render($usage_array),
          )),
      );
    }
    elseif (!$form_state['rebuild']) {
      drupal_set_message(t('PLEASE NOTE: Mandrill is not currently configured for use by Drupal. In order to route your email through Mandrill, you must configure at least one MailSystemInterface (other than mandrill) to use "MandrillMailSystem" in !link, or you will only be able to send Test Emails through Mandrill.',
        array('!link' => l(t('Mail System'), 'admin/config/system/mailsystem'))), 'warning');
    }

    $form['email_options'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#title' => t('Email options'),
    );

    $from = mandrill_from();
    $form['email_options']['mandrill_from'] = array(
      '#title' => t('From address'),
      '#type' => 'textfield',
      '#description' => t('The sender email address. If this address has not been verified, messages will be queued and not sent until it is. This address will appear in the "from" field, and any emails sent through Mandrill with a "from" address will have that address moved to the Reply-To field.'),
      '#default_value' => $from['email'],
    );
    $form['email_options']['mandrill_from_name'] = array(
      '#type' => 'textfield',
      '#title' => t('From name'),
      '#default_value' => $from['name'],
      '#description' => t('Optionally enter a from name to be used.'),
    );
    $subaccounts = mandrill_get_subaccounts();
    $sub_acct_options = array('' => '-- Select --');
    if (count($subaccounts)) {
      foreach ($subaccounts as $acct) {
        if ($acct['status'] == 'active') {
          $sub_acct_options[$acct['id']] = $acct['name'] . ' (' . $acct['reputation'] . ')';
        }
      }
    }
    elseif (variable_get('mandrill_subaccount', FALSE)) {
      variable_set('mandrill_subaccount', FALSE);
    }
    if (count($sub_acct_options) > 1) {
      $form['email_options']['mandrill_subaccount'] = array(
        '#type' => 'select',
        '#title' => t('Subaccount'),
        '#options' => $sub_acct_options,
        '#default_value' => variable_get('mandrill_subaccount', ''),
        '#description' => t('Choose a subaccount to send through.'),
      );
    }
    $formats = filter_formats();
    $options = array('' => t('-- Select --'));
    foreach ($formats as $v => $format) {
      $options[$v] = $format->name;
    }
    $form['email_options']['mandrill_filter_format'] = array(
      '#type' => 'select',
      '#title' => t('Input format'),
      '#description' => t('If selected, the input format to apply to the message body before sending to the Mandrill API.'),
      '#options' => $options,
      '#default_value' => array(variable_get('mandrill_filter_format', 'full_html')),
    );
    $form['send_options'] = array(
      '#title' => t('Send options'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
    );
    $form['send_options']['mandrill_track_opens'] = array(
      '#title' => t('Track opens'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to turn on open tracking for messages.'),
      '#default_value' => variable_get('mandrill_track_opens', TRUE),
    );
    $form['send_options']['mandrill_track_clicks'] = array(
      '#title' => t('Track clicks'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to turn on click tracking for messages.'),
      '#default_value' => variable_get('mandrill_track_clicks', TRUE),
    );
    $form['send_options']['mandrill_url_strip_qs'] = array(
      '#title' => t('Strip query string'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to strip the query string from URLs when aggregating tracked URL data.'),
      '#default_value' => variable_get('mandrill_url_strip_qs', FALSE),
    );
    $form['send_options']['mandrill_mail_key_blacklist'] = array(
      '#title' => t('Content logging blacklist'),
      '#type' => 'textarea',
      '#description' => t('Comma delimited list of Drupal mail keys to exclude content logging for. CAUTION: Removing the default password reset key may expose a security risk.'),
      '#default_value' => mandrill_mail_key_blacklist(),
    );
    $form['send_options']['mandrill_log_defaulted_sends'] = array(
      '#title' => t('Log sends from module/key pairs that are not registered independently in mailsystem.'),
      '#type' => 'checkbox',
      '#description' => t('If you select Mandrill as the site-wide default email sender in !mailsystem and check this box, any messages that are sent through Mandrill using module/key pairs that are not specifically registered in mailsystem will cause a message to be written to the !systemlog (type: Mandrill, severity: info). Enable this to identify keys and modules for automated emails for which you would like to have more granular control. It is not recommended to leave this box checked for extended periods, as it slows Mandrill and can clog your logs.',
        array(
          '!mailsystem' => l(t('Mail System'), 'admin/config/system/mailsystem'),
          '!systemlog' => l(t('system log'), 'admin/reports/dblog'),
        )),
      '#default_value' => variable_get('mandrill_log_defaulted_sends', FALSE),
    );

    $form['analytics'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#title' => t('Google analytics'),
    );
    $form['analytics']['mandrill_analytics_domains'] = array(
      '#title' => t('Google analytics domains'),
      '#type' => 'textfield',
      '#description' => t('One or more domains for which any matching URLs will automatically have Google Analytics parameters appended to their query string. Separate each domain with a comma.'),
      '#default_value' => variable_get('mandrill_analytics_domains', ''),
    );
    $form['analytics']['mandrill_analytics_campaign'] = array(
      '#title' => t('Google analytics campaign'),
      '#type' => 'textfield',
      '#description' => t("The value to set for the utm_campaign tracking parameter. If this isn't provided the messages from address will be used instead."),
      '#default_value' => variable_get('mandrill_analytics_campaign', ''),
    );
    $form['asynchronous_options'] = array(
      '#title' => t('Asynchronous options'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#attributes' => array(
        'id' => array('mandrill-async-options'),
      ),
    );
    $form['asynchronous_options']['mandrill_process_async'] = array(
      '#title' => t('Queue outgoing messages'),
      '#type' => 'checkbox',
      '#description' => t('When set, emails will not be immediately sent. Instead, they will be placed in a queue and sent when cron is triggered.'),
      '#default_value' => mandrill_process_async(),
    );
    $form['asynchronous_options']['mandrill_batch_log_queued'] = array(
      '#title' => t('Log queued emails in watchdog'),
      '#type' => 'checkbox',
      '#description' => t('Do you want to create a watchdog entry when an email is queued to be sent?'),
      '#default_value' => variable_get('mandrill_batch_log_queued', TRUE),
      '#states' => array(
        'invisible' => array(
          ':input[name="mandrill_process_async"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['asynchronous_options']['mandrill_queue_worker_timeout'] = array(
      '#title' => t('Queue worker timeout'),
      '#type' => 'textfield',
      '#size' => '12',
      '#description' => t('Number of seconds to spend processing messages during cron. Zero or negative values are not allowed.'),
      '#required' => TRUE,
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => variable_get('mandrill_queue_worker_timeout', 15),
      '#states' => array(
        'invisible' => array(
          ':input[name="mandrill_process_async"]' => array('checked' => FALSE),
        ),
      ),
    );
  }
  return system_settings_form($form);
}

/**
 * Javascript callback for the mandrill_admin_settings form.
 *
 * @param array $form
 *   a drupal form
 * @param array $form_state
 *   drupal form_state
 * 
 * @return array
 *   a form section
 */
function mandrill_admin_settings_form_callback($form, &$form_state) {
  return $form['asynchronous_options'];
}

/**
 * Return a form for sending a test email.
 */
function mandrill_test_form($form, &$form_state) {
  drupal_set_title(t('Send test email'));

  $form['mandrill_test_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Email address to send a test email to'),
    '#default_value' => variable_get('site_mail', ''),
    '#description' => t('Type in an address to have a test email sent there.'),
    '#required' => TRUE,
  );
  $form['mandrill_test_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Test body contents'),
    '#default_value' => t('If you receive this message it means your site is capable of using Mandrill to send email. This url is here to test click tracking: !link',
      array('!link' => l(t('link'), 'http://www.drupal.org/project/mandrill'))),
  );
  $form['include_attachment'] = array(
    '#title' => t('Include attachment'),
    '#type' => 'checkbox',
    '#description' => t('If checked, the Drupal icon will be included as an attachment with the test email.'),
    '#default_value' => TRUE,
  );

  $form['test_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send test email'),
  );
  $form['test_cancel'] = array(
    '#type' => 'link',
    '#href' => 'admin/config/services/mandrill',
    '#title' => t('Cancel'),
  );

  return $form;
}

/**
 * Submit handler for mandrill_test_form(), sends the test email.
 */
function mandrill_test_form_submit($form, &$form_state) {
  // If an address was given, send a test email message.
  $test_address = $form_state['values']['mandrill_test_address'];
  global $language;
  $params['subject'] = t('Drupal Mandrill test email');
  $params['body'] = $form_state['values']['mandrill_test_body'];
  $params['include_attachment'] = $form_state['values']['include_attachment'];
  $mailsystem = mailsystem_get();
  // Check for empty mailsystem config for Mandrill:
  $primary_config = empty($mailsystem['mandrill_test']) ? 'mandrill' : 'mandrill_test';
  if (empty($mailsystem[$primary_config])) {
    drupal_set_message(t('Automatically setting Mandrill tests to go through Mandrill API: MandrillMailSystem was not previously configured in Mail System.'));
    mailsystem_set(array('mandrill_test' => 'MandrillMailSystem'));
  }
  // Check for wrong mailsystem config for Mandrill, if not empty, and issue a
  // warning:
  elseif ($mailsystem[$primary_config] != 'MandrillMailSystem') {
    drupal_set_message(
      t('Mail System is configured to send Mandrill Test messages through %system, not Mandrill. To send tests through Mandrill, go to !link and change the setting.',
        array(
          '%system' => $mailsystem['mandrill_test'],
          '!link' => l(t('Mandrill'), 'https://mandrillapp.com/templates'))),
      'warning');
    // Hack because we are apparently formatting the body differently than
    // default drupal messages.
    $params['body'] = array('0' => $params['body']);
  }
  $result = drupal_mail('mandrill', 'test', $test_address, $language, $params);
  if (isset($result['result']) && $result['result'] == 'true') {
    drupal_set_message(t('Mandrill test email sent from %from to %to.', array('%from' => $result['from'], '%to' => $result['to'])), 'status');
  }
}
