<?php

/**
 * @file
 * Helper class for dispatching calls when different classes are responsible
 * for formatting and actually sending a message.
 */


/**
 * Implements a MailSystemInterface which simply forwards calls to another
 * MailSystem class configured previously.
 */
class MailsystemDelegateMailSystem implements MailSystemInterface {

  /**
   * Implements MailSystemInterface::format().
   *
   * Retrieve the configured class responsible for formatting a message of a
   * given module and mail-key. Forwards the formatting to that class.
   */
  public function format(array $message) {
    $module = $message['module'];
    $key = $message['key'];
    $mailsystem = _mailsystem_delegate_get_mailsystem($module, $key, 'format');
    return $mailsystem->format($message);
  }


  /**
   * Implements MailSystemInterface::mail().
   *
   * Retrieve the configured class responsible for delivering a message of a
   * given module and mail-key. Forwards the call to that class.
   */
  public function mail(array $message) {
    $module = $message['module'];
    $key = $message['key'];
    $mailsystem = _mailsystem_delegate_get_mailsystem($module, $key, 'mail');
    return $mailsystem->mail($message);
  }
}
