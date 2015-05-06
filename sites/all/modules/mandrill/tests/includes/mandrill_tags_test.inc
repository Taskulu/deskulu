<?php
/**
 * @file
 * A virtual Mandrill Tags API implementation for use in testing.
 */

class Mandrill_TagsTest extends Mandrill_Tags {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Tags::getList()
   */
  public function getList() {
    $tags = $this->getTagsTestData();

    foreach ($tags as $tag) {
      unset($tag['stats']);
    }

    return $tags;
  }

  /**
   * @see Mandrill_Tags::info()
   */
  public function info($tag) {
    $tags = $this->getTagsTestData();

    foreach ($tags as $tag) {
      unset($tag['reputation']);
      unset($tag['unique_opens']);
      unset($tag['unique_clicks']);
    }

    return $tags;
  }

  /**
   * @see Mandrill_Tags::timeSeries()
   */
  public function timeSeries($tag) {
    $time_series = array();

    $tags = $this->getTagsTestData();

    foreach ($tags as $tag) {
      if ($tag['tag'] == $tag) {
        $time_series_data = $tag['stats']['last_30_days'];
        $time_series_data['time'] = date('Y-m-d H:i:s', time());

        $time_series[] = $time_series_data;
      }
    }

    return $time_series;
  }

  /**
   * @see Mandrill_Tags::allTimeSeries()
   */
  public function allTimeSeries() {
    $time_series = array();

    $tags = $this->getTagsTestData();

    foreach ($tags as $tag) {
      $stats = $tag['stats']['last_30_days'];

      if (!isset($time_series_data)) {
        $time_series_data = $stats;
        $time_series_data['time'] = date('Y-m-d H:i:s', time());
      }
      else {
        $time_series_data['sent'] += $stats['sent'];
        $time_series_data['hard_bounces'] += $stats['hard_bounces'];
        $time_series_data['soft_bounces'] += $stats['soft_bounces'];
        $time_series_data['rejects'] += $stats['rejects'];
        $time_series_data['complaints'] += $stats['complaints'];
        $time_series_data['unsubs'] += $stats['unsubs'];
        $time_series_data['opens'] += $stats['opens'];
        $time_series_data['unique_opens'] += $stats['unique_opens'];
        $time_series_data['clicks'] += $stats['clicks'];
        $time_series_data['unique_clicks'] += $stats['unique_clicks'];
      }

      $time_series[] = $time_series_data;
    }

    return $time_series;
  }

  /**
   * Gets an array of tags used in tests.
   */
  protected function getTagsTestData() {
    $tags = array();

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

    // Test Tag One
    $tag = array(
      'tag' => 'test-tag-one',
      'reputation' => 42,
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

    $tags[] = $tag;

    // Test Tag Two
    $tag = array(
      'tag' => 'test-tag-two',
      'reputation' => 42,
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

    $tags[] = $tag;

    return $tags;
  }
}
