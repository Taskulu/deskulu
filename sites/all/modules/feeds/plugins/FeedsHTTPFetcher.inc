<?php

/**
 * @file
 * Home of the FeedsHTTPFetcher and related classes.
 */

feeds_include_library('PuSHSubscriber.inc', 'PuSHSubscriber');

/**
 * Result of FeedsHTTPFetcher::fetch().
 */
class FeedsHTTPFetcherResult extends FeedsFetcherResult {

  /**
   * The URL of the feed being fetched.
   *
   * @var string
   */
  protected $url;

  /**
   * The timeout in seconds to wait for a download.
   *
   * @var int
   */
  protected $timeout;

  /**
   *
   * Whether to ignore SSL validation errors.
   *
   * @var bool
   */
  protected $acceptInvalidCert;

  /**
   * Constructor.
   */
  public function __construct($url = NULL) {
    $this->url = $url;
  }

  /**
   * Overrides FeedsFetcherResult::getRaw();
   */
  public function getRaw() {
    if (!isset($this->raw)) {
      feeds_include_library('http_request.inc', 'http_request');
      $result = http_request_get($this->url, NULL, NULL, $this->acceptInvalidCert, $this->timeout);
      if (!in_array($result->code, array(200, 201, 202, 203, 204, 205, 206))) {
        throw new Exception(t('Download of @url failed with code !code.', array('@url' => $this->url, '!code' => $result->code)));
      }
      $this->raw = $result->data;
    }

    return $this->sanitizeRaw($this->raw);
  }

  public function getTimeout() {
    return $this->timeout;
  }

  public function setTimeout($timeout) {
    $this->timeout = $timeout;
  }

  /**
   * Sets the accept invalid certificates option.
   *
   * @param bool $accept_invalid_cert
   *   Whether to accept invalid certificates.
   */
  public function setAcceptInvalidCert($accept_invalid_cert) {
    $this->acceptInvalidCert = (bool) $accept_invalid_cert;
  }

}

/**
 * Fetches data via HTTP.
 */
class FeedsHTTPFetcher extends FeedsFetcher {

  /**
   * Implements FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    if ($this->config['use_pubsubhubbub'] && ($raw = $this->subscriber($source->feed_nid)->receive())) {
      return new FeedsFetcherResult($raw);
    }
    $fetcher_result = new FeedsHTTPFetcherResult($source_config['source']);
    // When request_timeout is empty, the global value is used.
    $fetcher_result->setTimeout($this->config['request_timeout']);
    $fetcher_result->setAcceptInvalidCert($this->config['accept_invalid_cert']);
    return $fetcher_result;
  }

  /**
   * Clear caches.
   */
  public function clear(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $url = $source_config['source'];
    feeds_include_library('http_request.inc', 'http_request');
    http_request_clear_cache($url);
  }

  /**
   * Implements FeedsFetcher::request().
   */
  public function request($feed_nid = 0) {
    feeds_dbg($_GET);
    @feeds_dbg(file_get_contents('php://input'));
    // A subscription verification has been sent, verify.
    if (isset($_GET['hub_challenge'])) {
      $this->subscriber($feed_nid)->verifyRequest();
    }
    // No subscription notification has ben sent, we are being notified.
    else {
      try {
        feeds_source($this->id, $feed_nid)->existing()->import();
      }
      catch (Exception $e) {
        // In case of an error, respond with a 503 Service (temporary) unavailable.
        header('HTTP/1.1 503 "Not Found"', NULL, 503);
        drupal_exit();
      }
    }
    // Will generate the default 200 response.
    header('HTTP/1.1 200 "OK"', NULL, 200);
    drupal_exit();
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'auto_detect_feeds' => FALSE,
      'use_pubsubhubbub' => FALSE,
      'designated_hub' => '',
      'request_timeout' => NULL,
      'auto_scheme' => 'http',
      'accept_invalid_cert' => FALSE,
    );
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['auto_detect_feeds'] = array(
      '#type' => 'checkbox',
      '#title' => t('Auto detect feeds'),
      '#description' => t('If the supplied URL does not point to a feed but an HTML document, attempt to extract a feed URL from the document.'),
      '#default_value' => $this->config['auto_detect_feeds'],
    );
    $form['use_pubsubhubbub'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use PubSubHubbub'),
      '#description' => t('Attempt to use a <a href="http://en.wikipedia.org/wiki/PubSubHubbub">PubSubHubbub</a> subscription if available.'),
      '#default_value' => $this->config['use_pubsubhubbub'],
    );
    $form['advanced'] = array(
      '#title' => t('Advanced settings'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['advanced']['auto_scheme'] = array(
      '#type' => 'textfield',
      '#title' => t('Automatically add scheme'),
      '#description' => t('If the supplied URL does not contain the scheme, use this one automatically. Keep empty to force the user to input the scheme.'),
      '#default_value' => $this->config['auto_scheme'],
    );
    $form['advanced']['designated_hub'] = array(
      '#type' => 'textfield',
      '#title' => t('Designated hub'),
      '#description' => t('Enter the URL of a designated PubSubHubbub hub (e. g. superfeedr.com). If given, this hub will be used instead of the hub specified in the actual feed.'),
      '#default_value' => $this->config['designated_hub'],
      '#states' => array(
        'visible' => array(':input[name="use_pubsubhubbub"]' => array('checked' => TRUE)),
      ),
    );
    // Per importer override of global http request timeout setting.
    $form['advanced']['request_timeout'] = array(
      '#type' => 'textfield',
      '#title' => t('Request timeout'),
      '#description' => t('Timeout in seconds to wait for an HTTP get request to finish.</br>' .
                         '<b>Note:</b> this setting will override the global setting.</br>' .
                         'When left empty, the global value is used.'),
      '#default_value' => $this->config['request_timeout'],
      '#element_validate' => array('element_validate_integer_positive'),
      '#maxlength' => 3,
      '#size'=> 30,
    );
    $form['advanced']['accept_invalid_cert'] = array(
      '#type' => 'checkbox',
      '#title' => t('Accept invalid SSL certificates'),
      '#description' => t('<strong>IMPORTANT:</strong> This setting will force cURL to completely ignore all SSL errors. This is a <strong>major security risk</strong> and should only be used during development.'),
      '#default_value' => $this->config['accept_invalid_cert'],
    );

    return $form;
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['source'] = array(
      '#type' => 'textfield',
      '#title' => t('URL'),
      '#description' => t('Enter a feed URL.'),
      '#default_value' => isset($source_config['source']) ? $source_config['source'] : '',
      '#maxlength' => NULL,
      '#required' => TRUE,
    );
    return $form;
  }

  /**
   * Override parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
    $values['source'] = trim($values['source']);

    // Keep a copy for error messages.
    $original_url = $values['source'];

    $parts = parse_url($values['source']);
    if (empty($parts['scheme']) && $this->config['auto_scheme']) {
      $values['source'] = $this->config['auto_scheme'] . '://' . $values['source'];
    }

    if (!feeds_valid_url($values['source'], TRUE)) {
      $form_key = 'feeds][' . get_class($this) . '][source';
      form_set_error($form_key, t('The URL %source is invalid.', array('%source' => $original_url)));
    }
    elseif ($this->config['auto_detect_feeds']) {
      feeds_include_library('http_request.inc', 'http_request');
      $url = http_request_get_common_syndication($values['source'], array(
        'accept_invalid_cert' => $this->config['accept_invalid_cert'],
      ));
      if ($url) {
        $values['source'] = $url;
      }
    }
  }

  /**
   * Override sourceSave() - subscribe to hub.
   */
  public function sourceSave(FeedsSource $source) {
    if ($this->config['use_pubsubhubbub']) {
      // If this is a feeds node we want to delay the subscription to
      // feeds_exit() to avoid transaction race conditions.
      if ($source->feed_nid) {
        $job = array('fetcher' => $this, 'source' => $source);
        feeds_set_subscription_job($job);
      }
      else {
        $this->subscribe($source);
      }
    }
  }

  /**
   * Override sourceDelete() - unsubscribe from hub.
   */
  public function sourceDelete(FeedsSource $source) {
    if ($this->config['use_pubsubhubbub']) {
      // If we're in a feed node, queue the unsubscribe,
      // else process immediately.
      if ($source->feed_nid) {
        $job = array(
          'type' => $source->id,
          'id' => $source->feed_nid,
          'period' => 0,
          'periodic' => FALSE,
        );
        JobScheduler::get('feeds_push_unsubscribe')->set($job);
      }
      else {
        $this->unsubscribe($source);
      }
    }
  }

  /**
   * Implement FeedsFetcher::subscribe() - subscribe to hub.
   */
  public function subscribe(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $this->subscriber($source->feed_nid)->subscribe($source_config['source'], url($this->path($source->feed_nid), array('absolute' => TRUE)), valid_url($this->config['designated_hub']) ? $this->config['designated_hub'] : '');
  }

  /**
   * Implement FeedsFetcher::unsubscribe() - unsubscribe from hub.
   */
  public function unsubscribe(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $this->subscriber($source->feed_nid)->unsubscribe($source_config['source'], url($this->path($source->feed_nid), array('absolute' => TRUE)));
  }

  /**
   * Implement FeedsFetcher::importPeriod().
   */
  public function importPeriod(FeedsSource $source) {
    if ($this->subscriber($source->feed_nid)->subscribed()) {
      return 259200; // Delay for three days if there is a successful subscription.
    }
  }

  /**
   * Convenience method for instantiating a subscriber object.
   */
  protected function subscriber($subscriber_id) {
    return PushSubscriber::instance($this->id, $subscriber_id, 'PuSHSubscription', PuSHEnvironment::instance());
  }
}

/**
 * Implement a PuSHSubscriptionInterface.
 */
class PuSHSubscription implements PuSHSubscriptionInterface {
  public $domain;
  public $subscriber_id;
  public $hub;
  public $topic;
  public $status;
  public $secret;
  public $post_fields;
  public $timestamp;

  /**
   * Load a subscription.
   */
  public static function load($domain, $subscriber_id) {
    if ($v = db_query("SELECT * FROM {feeds_push_subscriptions} WHERE domain = :domain AND subscriber_id = :sid", array(':domain' => $domain, ':sid' => $subscriber_id))->fetchAssoc()) {
      $v['post_fields'] = unserialize($v['post_fields']);
      return new PuSHSubscription($v['domain'], $v['subscriber_id'], $v['hub'], $v['topic'], $v['secret'], $v['status'], $v['post_fields'], $v['timestamp']);
    }
  }

  /**
   * Create a subscription.
   */
  public function __construct($domain, $subscriber_id, $hub, $topic, $secret, $status = '', $post_fields = '') {
    $this->domain = $domain;
    $this->subscriber_id = $subscriber_id;
    $this->hub = $hub;
    $this->topic = $topic;
    $this->status = $status;
    $this->secret = $secret;
    $this->post_fields = $post_fields;
  }

  /**
   * Save a subscription.
   */
  public function save() {
    $this->timestamp = time();
    $this->delete($this->domain, $this->subscriber_id);
    drupal_write_record('feeds_push_subscriptions', $this);
  }

  /**
   * Delete a subscription.
   */
  public function delete() {
    db_delete('feeds_push_subscriptions')
      ->condition('domain', $this->domain)
      ->condition('subscriber_id', $this->subscriber_id)
      ->execute();
  }
}

/**
 * Provide environmental functions to the PuSHSubscriber library.
 */
class PuSHEnvironment implements PuSHSubscriberEnvironmentInterface {
  /**
   * Singleton.
   */
  public static function instance() {
    static $env;
    if (empty($env)) {
      $env = new PuSHEnvironment();
    }
    return $env;
  }

  /**
   * Implements PuSHSubscriberEnvironmentInterface::msg().
   */
  public function msg($msg, $level = 'status') {
    drupal_set_message(check_plain($msg), $level);
  }

  /**
   * Implements PuSHSubscriberEnvironmentInterface::log().
   */
  public function log($msg, $level = 'status') {
    switch ($level) {
      case 'error':
        $severity = WATCHDOG_ERROR;
        break;
      case 'warning':
        $severity = WATCHDOG_WARNING;
        break;
      default:
        $severity = WATCHDOG_NOTICE;
        break;
    }
    feeds_dbg($msg);
    watchdog('FeedsHTTPFetcher', $msg, array(), $severity);
  }
}
