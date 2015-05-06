<?php

/**
 * @file
 * Functions that handle inbound messages to mimemail.
 */

/**
 * Receive messages POSTed from an external source.
 *
 * This function enables messages to be sent via POST or some other RFC822
 * source input (e.g. directly from a mail server).
 *
 * @return
 *   The POSTed message.
 */
function mimemail_post() {
  if (!isset($_POST['token']) || empty($_POST['token'])) {
    return drupal_access_denied();
  }

  if (isset($_POST['message']) && !empty($_POST['message'])) {
    $key = variable_get('mimemail_key', drupal_random_key());
    $hash = hash_hmac('sha1', $_POST['message'], $key);
    if ($hash != $_POST['token']) {
      watchdog('access denied', 'Authentication error for POST e-mail', WATCHDOG_WARNING);
      return drupal_access_denied();
    }
    else {
      return mimemail_incoming($_POST['message']);
    }
  }

  return drupal_access_denied();
}

/**
 * Parses an externally received message.
 *
 * @param $message
 *   The message to parse.
 */
function mimemail_incoming($message) {
  $mail = mimemail_parse($message);

  foreach (module_implements('mimemail_incoming_alter') as $module) {
    call_user_func_array($module . '_mimemail_incoming_alter', $mail);
  }

  module_invoke_all('mimemail_incoming', $mail);
}

/**
 * Parses a message into its parts.
 *
 * @param string $message
 *   The message to parse.
 *
 * @return array
 *   The parts of the message.
 */
function mimemail_parse($message) {
  // Provides a "headers", "content-type" and "body" element.
  $mail = mimemail_parse_headers($message);

  // Get an address-only version of "From" (useful for user_load() and such).
  $mail['from'] = preg_replace('/.*\b([a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,4})\b.*/i', '\1', drupal_strtolower($mail['headers']['From']));

  // Get a subject line, which may be cleaned up/modified later.
  $mail['subject'] = $mail['headers']['Subject'];

  // Make an array to hold any non-content attachments.
  $mail['attachments'] = array();

  // We're dealing with a multi-part message.
  $mail['parts'] = mimemail_parse_boundary($mail);

  foreach ($mail['parts'] as $i => $part_body) {
    $part = mimemail_parse_headers($part_body);
    $sub_parts = mimemail_parse_boundary($part);

    // Content is encoded in a multipart/alternative section.
    if (count($sub_parts) > 1) {
      foreach ($sub_parts as $j => $sub_part_body) {
        $sub_part = mimemail_parse_headers($sub_part_body);
        if ($sub_part['content-type'] == 'text/plain') {
          $mail['text'] = mimemail_parse_content($sub_part);
        }
        if ($sub_part['content-type'] == 'text/html') {
          $mail['html'] = mimemail_parse_content($sub_part);
        }
        else {
          $mail['attachments'][] = mimemail_parse_attachment($sub_part);
        }
      }
    }

    if (($part['content-type'] == 'text/plain') && !isset($mail['text'])) {
      $mail['text'] = mimemail_parse_content($part);
    }
    elseif (($part['content-type'] == 'text/html') && !isset($mail['html'])) {
      $mail['html'] = mimemail_parse_content($part);
    }
    else {
      $mail['attachments'][] = mimemail_parse_attachment($part);
    }
  }

  // Make sure our text and html parts are accounted for.
  if (isset($mail['html']) && !isset($mail['text'])) {
    $mail['text'] = preg_replace('|<style.*</style>|mis', '', $mail['html']);
    $mail['text'] = drupal_html_to_text($mail['text']);
  }
  elseif (isset($mail['text']) && !isset($mail['html'])) {
    $mail['html'] = check_markup($mail['text'], variable_get('mimemail_format', filter_fallback_format()));
  }

  // Last ditch attempt - use the body as-is.
  if (!isset($mail['text'])) {
    $mail['text'] = mimemail_parse_content($mail);
    $mail['html'] = check_markup($mail['text'], variable_get('mimemail_format', filter_fallback_format()));
  }

  return $mail;
}

/**
 * Split a multi-part message using MIME boundaries.
 */
function mimemail_parse_boundary($part) {
  $m = array();
  if (preg_match('/.*boundary="?([^";]+)"?.*/', $part['headers']['Content-Type'], $m)) {
    $boundary = "\n--" . $m[1];
    $body     = str_replace("$boundary--", '', $part['body']);
    return array_slice(explode($boundary, $body), 1);
  }
  return array($part['body']);
}

/**
 * Split a message (or message part) into its headers and body section.
 */
function mimemail_parse_headers($message) {
  // Split out body and headers.
  if (preg_match("/^(.*?)\r?\n\r?\n(.*)/s", $message, $match)) {
    list($hdr, $body) = array($match[1], $match[2]);
  }

  // Un-fold the headers.
  $hdr = preg_replace(array("/\r/", "/\n(\t| )+/"), array('', ' '), $hdr);

  $headers = array();
  foreach (explode("\n", trim($hdr)) as $row) {
    $split = strpos($row, ':');
    $name = trim(drupal_substr($row, 0, $split));
    $val = trim(drupal_substr($row, $split+1));
    $headers[$name] = $val;
  }

  $type = (preg_replace('/\s*([^;]+).*/', '\1', $headers['Content-Type']));

  return array('headers' => $headers, 'body' => $body, 'content-type' => $type);
}

/**
 * Return a decoded MIME part in UTF-8.
 */
function mimemail_parse_content($part) {
  $content = $part['body'];

  // Decode this part.
  if ($encoding = drupal_strtolower($part['headers']['Content-Transfer-Encoding'])) {
    switch ($encoding) {
      case 'base64':
        $content = base64_decode($content);
        break;
      case 'quoted-printable':
        $content = quoted_printable_decode($content);
        break;
      // 7bit is the RFC default.
      case '7bit':
        break;
    }
  }

  // Try to convert character set to UTF-8.
  if (preg_match('/.*charset="?([^";]+)"?.*/', $part['headers']['Content-Type'], $m)) {
    $content = drupal_convert_to_utf8($content, $m[1]);
  }

  return $content;
}

/**
 * Convert a MIME part into a file array.
 */
function mimemail_parse_attachment($part) {
  $m = array();
  if (preg_match('/.*filename="?([^";])"?.*/', $part['headers']['Content-Disposition'], $m)) {
    $name = $m[1];
  }
  elseif (preg_match('/.*name="?([^";])"?.*/', $part['headers']['Content-Type'], $m)) {
    $name = $m[1];
  }

  return array(
    'filename' => $name,
    'filemime' => $part['content-type'],
    'content'  => mimemail_parse_content($part),
  );
}
