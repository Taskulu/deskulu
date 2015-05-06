<?php

/**
 * @file
 * Mime Mail implementations of MailSystemInterface.
 */

/**
 * Modify the Drupal mail system to send HTML emails.
 */
class MimeMailSystem implements MailSystemInterface {
  /**
   * Concatenate and wrap the e-mail body for HTML mails.
   *
   * @param array $message
   *   A message array, as described in hook_mail_alter() with optional
   *   parameters described in mimemail_prepare_message().
   *
   * @return array
   *   The formatted $message.
   */
  public function format(array $message) {
    if (is_array($message['body'])) {
      $message['body'] = implode("\n\n", $message['body']);
    }

    if (preg_match('/plain/', $message['headers']['Content-Type'])) {
      $message['body'] = check_markup($message['body'], variable_get('mimemail_format', filter_fallback_format()));
    }

    $engine = variable_get('mimemail_engine', 'mimemail');
    $mailengine = $engine . '_mailengine';
    $engine_prepare_message = $engine . '_prepare_message';

    if (function_exists($engine_prepare_message)) {
      $message = $engine_prepare_message($message);
    }
    else {
      $message = mimemail_prepare_message($message);
    }

    return $message;
  }
  /**
   * Send an HTML e-mail message, using Drupal variables and default settings.
   *
   * @param array $message
   *   A message array, as described in hook_mail_alter() with optional
   *   parameters described in mimemail_prepare_message().
   *
   * @return boolean
   *   TRUE if the mail was successfully accepted, otherwise FALSE.
   */
  public function mail(array $message) {
    $engine = variable_get('mimemail_engine', 'mimemail');
    $mailengine = $engine . '_mailengine';

    if (!$engine || !function_exists($mailengine)) {
      return FALSE;
    }

    return $mailengine('send', $message);
  }
}
