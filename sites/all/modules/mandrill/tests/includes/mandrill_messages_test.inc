<?php
/**
 * @file
 * A virtual Mandrill Messages API implementation for use in testing.
 */

class Mandrill_MessagesTest extends Mandrill_Messages {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Messages::send()
   */
  public function send($message, $async = FALSE, $ip_pool = NULL, $send_at = NULL) {
    if (!isset($message['to']) || empty($message['to'])) {
      return $this->master->getErrorResponse(12, 'ValidationError', 'No recipients defined.');
    }

    $response = array();

    foreach ($message['to'] as $recipient) {
      $recipient_response = array(
        'email' => $recipient['email'],
        'status' => '',
        'reject_reason' => '',
        '_id' => uniqid(),
      );

      if (valid_email_address($recipient['email'])) {
        $recipient_response['status'] = 'sent';
      }
      else {
        $recipient_response['status'] = 'invalid';
      }

      $response[] = $recipient_response;
    }

    return $response;
  }

  /**
   * @see Mandrill_Messages::sendTemplate()
   */
  public function sendTemplate($template_name, $template_content, $message, $async = FALSE, $ip_pool = NULL, $send_at = NULL) {
    if (!isset($message['to']) || empty($message['to'])) {
      return $this->master->getErrorResponse(12, 'ValidationError', 'No recipients defined.');
    }

    $response = array();

    $templates = mandrill_get_templates();

    $matched_template = NULL;
    foreach ($templates as $template) {
      if ($template['name'] == $template_name) {
        $matched_template = $template;
        continue;
      }
    }

    if ($matched_template == NULL) {
      return $this->master->getErrorResponse(12, 'Unknown_Template', 'No template with name: ' . $template_name);
    }

    $recipients = mandrill_get_to($message['to']);

    foreach ($recipients as $recipient) {
      $recipient_response = array(
        'email' => $recipient['email'],
        'status' => 'sent',
        'reject_reason' => '',
        '_id' => uniqid(),
      );

      $response[] = $recipient_response;
    }

    return $response;
  }

  /**
   * @see Mandrill_Messages::search()
   */
  public function search($query = '*', $date_from = NULL, $date_to = NULL, $tags = NULL, $senders = NULL, $api_keys = NULL, $limit = 100) {
    $matched_messages = array();

    $query_parts = explode(':', $query);
    $query_key = $query_parts[0];
    $query_value = $query_parts[1];

    $messages = $this->getTestMessagesData();

    foreach ($messages as $message) {
      if (isset($message[$query_key]) && ($message[$query_key] == $query_value)) {
        $matched_messages[] = $message;
      }
    }

    return $matched_messages;
  }

  /**
   * Gets an array of messages used in tests.
   */
  protected function getTestMessagesData() {
    $messages = array();

    $message = array(
      'ts' => 1365190000,
      '_id' => 'abc123abc123abc123abc123',
      'sender' => 'sender@example.com',
      'template' => 'test-template',
      'subject' => 'Test Subject',
      'email' => 'recipient@example.com',
      'tags' => array(
        'test-tag'
      ),
      'opens' => 42,
      'opens_detail' => array(
        'ts' => 1365190001,
        'ip' => '55.55.55.55',
        'location' => 'Georgia, US',
        'ua' => 'Linux/Ubuntu/Chrome/Chrome 28.0.1500.53',
      ),
      'clicks' => 42,
      'clicks_detail' => array(
        'ts' => 1365190001,
        'url' => 'http://www.example.com',
        'ip' => '55.55.55.55',
        'location' => 'Georgia, US',
        'ua' => 'Linux/Ubuntu/Chrome/Chrome 28.0.1500.53',
      ),
      'state' => 'sent',
      'metadata' => array(
        'user_id' => 123,
        'website' => 'www.example.com',
      ),
    );

    $messages[] = $message;

    return $messages;
  }
}
