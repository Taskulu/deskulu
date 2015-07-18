<?php

/**
 * @file
 * Pubsubhubbub subscriber library.
 *
 * Readme
 * http://github.com/lxbarth/PuSHSubscriber
 *
 * License
 * http://github.com/lxbarth/PuSHSubscriber/blob/master/LICENSE.txt
 */

/**
 * PubSubHubbub subscriber.
 */
class PuSHSubscriber {
  protected $domain;
  protected $subscriber_id;
  protected $subscription_class;
  protected $env;

  /**
   * Singleton.
   *
   * PuSHSubscriber identifies a unique subscription by a domain and a numeric
   * id. The numeric id is assumed to e unique in its domain.
   *
   * @param $domain
   *   A string that identifies the domain in which $subscriber_id is unique.
   * @param $subscriber_id
   *   A numeric subscriber id.
   * @param $subscription_class
   *   The class to use for handling subscriptions. Class MUST implement
   *   PuSHSubscriberSubscriptionInterface
   * @param PuSHSubscriberEnvironmentInterface $env
   *   Environmental object for messaging and logging.
   */
  public static function instance($domain, $subscriber_id, $subscription_class, PuSHSubscriberEnvironmentInterface $env) {
    static $subscribers;
    if (!isset($subscriber[$domain][$subscriber_id])) {
      $subscriber = new PuSHSubscriber($domain, $subscriber_id, $subscription_class, $env);
    }
    return $subscriber;
  }

  /**
   * Protect constructor.
   */
  protected function __construct($domain, $subscriber_id, $subscription_class, PuSHSubscriberEnvironmentInterface $env) {
    $this->domain = $domain;
    $this->subscriber_id = $subscriber_id;
    $this->subscription_class = $subscription_class;
    $this->env = $env;
  }

  /**
   * Subscribe to a given URL. Attempt to retrieve 'hub' and 'self' links from
   * document at $url and issue a subscription request to the hub.
   *
   * @param $url
   *   The URL of the feed to subscribe to.
   * @param $callback_url
   *   The full URL that hub should invoke for subscription verification or for
   *   notifications.
   * @param $hub
   *   The URL of a hub. If given overrides the hub URL found in the document
   *   at $url.
   */
  public function subscribe($url, $callback_url, $hub = '') {
    // Fetch document, find rel=hub and rel=self.
    // If present, issue subscription request.
    $request = curl_init($url);
    curl_setopt($request, CURLOPT_FOLLOWLOCATION, TRUE);
    curl_setopt($request, CURLOPT_RETURNTRANSFER, TRUE);
    $data = curl_exec($request);
    if (curl_getinfo($request, CURLINFO_HTTP_CODE) == 200) {
      try {
        $xml = @ new SimpleXMLElement($data);
        $xml->registerXPathNamespace('atom', 'http://www.w3.org/2005/Atom');
        if (empty($hub) && $hub = @current($xml->xpath("//atom:link[attribute::rel='hub']"))) {
          $hub = (string) $hub->attributes()->href;
        }
        if ($self = @current($xml->xpath("//atom:link[attribute::rel='self']"))) {
          $self = (string) $self->attributes()->href;
        }
      }
      catch (Exception $e) {
        // Do nothing.
      }
    }
    curl_close($request);
    // Fall back to $url if $self is not given.
    if (!$self) {
      $self = $url;
    }
    if (!empty($hub) && !empty($self)) {
      $this->request($hub, $self, 'subscribe', $callback_url);
    }
  }

  /**
   * @todo Unsubscribe from a hub.
   * @todo Make sure we unsubscribe with the correct topic URL as it can differ
   * from the initial subscription URL.
   *
   * @param $topic_url
   *   The URL of the topic to unsubscribe from.
   * @param $callback_url
   *   The callback to unsubscribe.
   */
  public function unsubscribe($topic_url, $callback_url) {
    if ($sub = $this->subscription()) {
      $this->request($sub->hub, $sub->topic, 'unsubscribe', $callback_url);
      $sub->delete();
    }
  }

  /**
   * Request handler for subscription callbacks.
   */
  public function handleRequest($callback) {
    if (isset($_GET['hub_challenge'])) {
      $this->verifyRequest();
    }
    // No subscription notification has ben sent, we are being notified.
    else {
      if ($raw = $this->receive()) {
        $callback($raw, $this->domain, $this->subscriber_id);
      }
    }
  }

  /**
   * Receive a notification.
   *
   * @param $ignore_signature
   *   If FALSE, only accept payload if there is a signature present and the
   *   signature matches the payload. Warning: setting to TRUE results in
   *   unsafe behavior.
   *
   * @return
   *   An XML string that is the payload of the notification if valid, FALSE
   *   otherwise.
   */
  public function receive($ignore_signature = FALSE) {
    /**
     * Verification steps:
     *
     * 1) Verify that this is indeed a POST reuest.
     * 2) Verify that posted string is XML.
     * 3) Per default verify sender of message by checking the message's
     *    signature against the shared secret.
     */
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
      $raw = file_get_contents('php://input');
      if (@simplexml_load_string($raw)) {
        if ($ignore_signature) {
          return $raw;
        }
        if (isset($_SERVER['HTTP_X_HUB_SIGNATURE']) && ($sub = $this->subscription())) {
          $result = array();
          parse_str($_SERVER['HTTP_X_HUB_SIGNATURE'], $result);
          if (isset($result['sha1']) && $result['sha1'] === hash_hmac('sha1', $raw, $sub->secret)) {
            return $raw;
          }
          else {
            $this->log('Could not verify signature.', 'error');
          }
        }
        else {
          $this->log('No signature present.', 'error');
        }
      }
    }
    return FALSE;
  }

  /**
   * Verify a request. After a hub has received a subscribe or unsubscribe
   * request (see PuSHSubscriber::request()) it sends back a challenge verifying
   * that an action indeed was requested ($_GET['hub_challenge']). This
   * method handles the challenge.
   */
  public function verifyRequest() {
    if (!isset($_GET['hub_challenge'])) {
      return $this->rejectRequest();
    }

    // Don't accept modes of 'subscribed' or 'unsubscribed'.
    if ($_GET['hub_mode'] !== 'subscribe' && $_GET['hub_mode'] !== 'unsubscribe') {
      return $this->rejectRequest();
    }

    // No available subscription.
    if (!$sub = $this->subscription()) {
      return $this->rejectRequest();
    }

    // Not what we asked for.
    if ($_GET['hub_mode'] !== $sub->status) {
      return $this->rejectRequest();
    }

    // Wrong URL.
    if ($_GET['hub_topic'] !== $sub->topic) {
      return $this->rejectRequest();
    }

    if ($sub->status === 'subscribe') {
      $sub->status = 'subscribed';
      $this->log('Verified "subscribe" request.');
    }
    else {
      $sub->status = 'unsubscribed';
      $this->log('Verified "unsubscribe" request.');
    }

    $sub->post_fields = array();
    $sub->save();

    header('HTTP/1.1 200 "Found"', NULL, 200);
    print check_plain($_GET['hub_challenge']);
    drupal_exit();
  }

  /**
   * Issue a subscribe or unsubcribe request to a PubsubHubbub hub.
   *
   * @param $hub
   *   The URL of the hub's subscription endpoint.
   * @param $topic
   *   The topic URL of the feed to subscribe to.
   * @param $mode
   *   'subscribe' or 'unsubscribe'.
   * @param $callback_url
   *   The subscriber's notifications callback URL.
   *
   * Compare to http://pubsubhubbub.googlecode.com/svn/trunk/pubsubhubbub-core-0.2.html#anchor5
   *
   * @todo Make concurrency safe.
   */
  protected function request($hub, $topic, $mode, $callback_url) {
    $secret = drupal_random_key(40);
    $post_fields = array(
      'hub.callback' => $callback_url,
      'hub.mode' => $mode,
      'hub.topic' => $topic,
      'hub.verify' => 'sync',
      'hub.lease_seconds' => '', // Permanent subscription.
      'hub.secret' => $secret,
    );
    $sub = new $this->subscription_class($this->domain, $this->subscriber_id, $hub, $topic, $secret, $mode, $post_fields);
    $sub->save();
    // Issue subscription request.
    $request = curl_init($hub);
    curl_setopt($request, CURLOPT_POST, TRUE);
    curl_setopt($request, CURLOPT_POSTFIELDS, $post_fields);
    curl_setopt($request, CURLOPT_RETURNTRANSFER, TRUE);
    curl_exec($request);
    $code = curl_getinfo($request, CURLINFO_HTTP_CODE);
    if (in_array($code, array(202, 204))) {
      $this->log("Positive response to \"$mode\" request ($code).");
    }
    else {
      $sub->status = $mode . ' failed';
      $sub->save();
      $this->log("Error issuing \"$mode\" request to $hub ($code).", 'error');
    }
    curl_close($request);
  }

  /**
   * Get the subscription associated with this subscriber.
   *
   * @return
   *   A PuSHSubscriptionInterface object if a subscription exist, NULL
   *   otherwise.
   */
  public function subscription() {
    return call_user_func(array($this->subscription_class, 'load'), $this->domain, $this->subscriber_id);
  }

  /**
   * Determine whether this subscriber is successfully subscribed or not.
   */
  public function subscribed() {
    if ($sub = $this->subscription()) {
      if ($sub->status == 'subscribed') {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Helper for messaging.
   */
  protected function msg($msg, $level = 'status') {
    $this->env->msg($msg, $level);
  }

  /**
   * Helper for logging.
   */
  protected function log($msg, $level = 'status') {
    $this->env->log("{$this->domain}:{$this->subscriber_id}\t$msg", $level);
  }

  /**
   * Rejects a request subscription request.
   */
  protected function rejectRequest() {
    header('HTTP/1.1 404 "Not Found"', NULL, 404);
    $this->log('Could not verify subscription.', 'error');
    drupal_exit();
  }

}

/**
 * Implement to provide a storage backend for subscriptions.
 *
 * Variables passed in to the constructor must be accessible as public class
 * variables.
 */
interface PuSHSubscriptionInterface {
  /**
   * @param $domain
   *   A string that defines the domain in which the subscriber_id is unique.
   * @param $subscriber_id
   *   A unique numeric subscriber id.
   * @param $hub
   *   The URL of the hub endpoint.
   * @param $topic
   *   The topic to subscribe to.
   * @param $secret
   *   A secret key used for message authentication.
   * @param $status
   *   The status of the subscription.
   *   'subscribe' - subscribing to a feed.
   *   'unsubscribe' - unsubscribing from a feed.
   *   'subscribed' - subscribed.
   *   'unsubscribed' - unsubscribed.
   *   'subscribe failed' - subscribe request failed.
   *   'unsubscribe failed' - unsubscribe request failed.
   * @param $post_fields
   *   An array of the fields posted to the hub.
   */
  public function __construct($domain, $subscriber_id, $hub, $topic, $secret, $status = '', $post_fields = '');

  /**
   * Save a subscription.
   */
  public function save();

  /**
   * Load a subscription.
   *
   * @return
   *   A PuSHSubscriptionInterface object if a subscription exist, NULL
   *   otherwise.
   */
  public static function load($domain, $subscriber_id);

  /**
   * Delete a subscription.
   */
  public function delete();
}

/**
 * Implement to provide environmental functionality like user messages and
 * logging.
 */
interface PuSHSubscriberEnvironmentInterface {
  /**
   * A message to be displayed to the user on the current page load.
   *
   * @param $msg
   *   A string that is the message to be displayed.
   * @param $level
   *   A string that is either 'status', 'warning' or 'error'.
   */
  public function msg($msg, $level = 'status');

  /**
   * A log message to be logged to the database or the file system.
   *
   * @param $msg
   *   A string that is the message to be displayed.
   * @param $level
   *   A string that is either 'status', 'warning' or 'error'.
   */
  public function log($msg, $level = 'status');
}
