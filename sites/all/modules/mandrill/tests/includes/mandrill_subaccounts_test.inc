<?php
/**
 * @file
 * A virtual Mandrill Subaccounts API implementation for use in testing.
 */

class Mandrill_SubaccountsTest extends Mandrill_Subaccounts {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Subaccounts::getList()
   */
  public function getList($q = NULL) {
    $subaccounts = array();

    // Test Customer One
    $subaccount = array(
      'id' => 'test-customer-1',
      'name' => 'Test Customer One',
      'custom_quota' => 42,
      'status' => 'active',
      'reputation' => 42,
      'created_at' => '2013-01-01 15:30:27',
      'first_sent_at' => '2013-01-01 15:30:29',
      'sent_weekly' => 42,
      'sent_monthly' => 42,
      'sent_total' => 42,
    );

    $subaccounts[] = $subaccount;

    // Test Customer Two
    $subaccount = array(
      'id' => 'test-customer-2',
      'name' => 'Test Customer Two',
      'custom_quota' => 42,
      'status' => 'active',
      'reputation' => 42,
      'created_at' => '2013-01-01 15:30:27',
      'first_sent_at' => '2013-01-01 15:30:29',
      'sent_weekly' => 42,
      'sent_monthly' => 42,
      'sent_total' => 42,
    );

    $subaccounts[] = $subaccount;

    return $subaccounts;
  }
}
