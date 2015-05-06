<?php
/**
 * @file
 * A virtual Mandrill API implementation for use in testing.
 */

require 'mandrill_messages_test.inc';
require 'mandrill_senders_test.inc';
require 'mandrill_subaccounts_test.inc';
require 'mandrill_tags_test.inc';
require 'mandrill_templates_test.inc';
require 'mandrill_urls_test.inc';
require 'mandrill_users_test.inc';

class DrupalMandrillTest extends DrupalMandrill {
  /**
   * @see DrupalMandrill::__construct()
   */
  public function __construct($apikey = NULL, $timeout = 60) {
    parent::__construct($apikey, $timeout);

    // Set up test classes.
    $this->messages = new Mandrill_MessagesTest($this);
    $this->senders = new Mandrill_SendersTest($this);
    $this->subaccounts = new Mandrill_SubaccountsTest($this);
    $this->tags = new Mandrill_TagsTest($this);
    $this->templates = new Mandrill_TemplatesTest($this);
    $this->urls = new Mandrill_UrlsTest($this);
    $this->users = new Mandrill_UsersTest($this);
  }

  public function getErrorResponse($code, $name, $message) {
    $response = array(
      'status' => 'error',
      'code' => $code,
      'name' => $name,
      'message' => $message,
    );

    return $response;
  }
}
