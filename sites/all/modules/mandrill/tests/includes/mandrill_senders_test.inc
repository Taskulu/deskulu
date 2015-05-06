<?php
/**
 * @file
 * A virtual Mandrill Senders API implementation for use in testing.
 */

class Mandrill_SendersTest extends Mandrill_Senders {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Senders::getList()
   */
  public function getList() {
    $senders = $this->getSendersTestData();

    foreach ($senders as $sender) {
      unset($sender['stats']);
    }

    return $senders;
  }

  /**
   * @see Mandrill_Senders::info()
   */
  public function info($address) {
    $senders = $this->getSendersTestData();

    $matched_sender = NULL;

    foreach ($senders as $sender) {
      if ($sender['address'] == $address) {
        $matched_sender = $sender;
        unset($matched_sender['unique_opens']);
        unset($matched_sender['unique_clicks']);
      }
    }

    return $matched_sender;
  }

  /**
   * @see Mandrill_Senders::timeSeries()
   */
  public function timeSeries($address) {
    $time_series = array();

    $senders = $this->getSendersTestData();

    foreach ($senders as $sender) {
      if ($sender['address'] == $address) {
        $time_series[] = $sender['stats']['last_30_days'];
      }
    }

    return $time_series;
  }

  /**
   * Gets an array of sender data used in tests.
   */
  protected function getSendersTestData() {
    $senders = array();

    $stats_data = array(
      'sent' => 42,
      'hard_bounces' => 42,
      'soft_bounces' => 42,
      'rejects' => 42,
      'complaints' => 42,
      'unsubs' => 42,
      'opens' => 42,
      'unique_opens' => 42,
      'clicks' => 42,
      'unique_clicks' => 42,
    );

    $stats = array(
      'today' => $stats_data,
      'last_7_days' => $stats_data,
      'last_30_days' => $stats_data,
      'last_60_days' => $stats_data,
      'last_90_days' => $stats_data,
    );

    // Sender One
    $sender = array(
      'address' => 'sender.one@mandrillapp.com',
      'created_at' => '2013-01-01 15:30:27',
      'sent' => 42,
      'hard_bounces' => 42,
      'soft_bounces' => 42,
      'rejects' => 42,
      'complaints' => 42,
      'unsubs' => 42,
      'opens' => 42,
      'clicks' => 42,
      'unique_opens' => 42,
      'unique_clicks' => 42,
      'stats' => $stats,
    );

    $senders[] = $sender;

    // Sender Two
    $sender = array(
      'address' => 'sender.two@mandrillapp.com',
      'created_at' => '2013-01-01 15:30:27',
      'sent' => 42,
      'hard_bounces' => 42,
      'soft_bounces' => 42,
      'rejects' => 42,
      'complaints' => 42,
      'unsubs' => 42,
      'opens' => 42,
      'clicks' => 42,
      'unique_opens' => 42,
      'unique_clicks' => 42,
      'stats' => $stats,
    );

    $senders[] = $sender;

    return $senders;
  }
}
