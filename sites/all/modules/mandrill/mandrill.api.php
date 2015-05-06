<?php

/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Allows other modules to alter the Mandrill message and sender arguments.
 *
 * @array $mandrill_params
 *   The mandril message array
 * @see MandrillMailSystem::mail()
 *
 * @array $message
 *   The drupal_mail message array.
 * @see drupal_mail()
 */
function hook_mandrill_mail_alter(&$mandrill_params, $message) {
  // No example.
}

/**
 * Allows other modules to alter the allowed attachment file types.
 *
 * @array $types
 *   An array of file types indexed numerically.
 */
function hook_mandrill_valid_attachment_types_alter(&$types) {
  // Example, allow word docs:
  $types[] = 'application/msword';
  // Allow openoffice docs:
  $types[] = 'application/vnd.openxmlformats-officedocument.wordprocessingml.document';
}

/**
 * Allow other modules to respond to the result of sending an email.
 *
 * @param array $result
 *   Associative array containing the send result, including the status.
 */
function hook_mandrill_mailsend_result($result) {
  if ($result['status'] == 'rejected') {
    // Delete user.
    $user = user_load_by_mail($result['email']);
    user_delete($user->uid);
  }
}
