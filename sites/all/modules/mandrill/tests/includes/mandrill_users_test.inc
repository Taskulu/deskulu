<?php
/**
 * @file
 * A virtual Mandrill Users API implementation for use in testing.
 */

class Mandrill_UsersTest extends Mandrill_Users {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Users::info()
   */
  public function info() {
    $info = $this->getUserTestData();
    return $info;
  }

  /**
   * Gets user data used in tests.
   */
  protected function getUserTestData() {
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
      'all_time' => $stats_data,
    );

    $user = array(
      'username' => 'testuser',
      'created_at' => '2013-01-01 15:30:27',
      'public_id' => 'aaabbbccc112233',
      'reputation' => 42,
      'hourly_quota' => 42,
      'backlog' => 42,
      'stats' => $stats,
    );

    return $user;
  }
}
