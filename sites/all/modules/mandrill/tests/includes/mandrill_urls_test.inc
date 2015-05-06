<?php
/**
 * @file
 * A virtual Mandrill URLs API implementation for use in testing.
 */

class Mandrill_UrlsTest extends Mandrill_Urls {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Urls::getList()
   */
  public function getList() {
    $urls = $this->getUrlsTestData();
    return $urls;
  }

  /**
   * @see Mandrill_Urls::timeSeries()
   */
  public function timeSeries($url) {
    $time_series = array();

    $urls = $this->getUrlsTestData();

    foreach ($urls as $url) {
      $time_series_data = array(
        'sent' => $url['sent'],
        'clicks' => $url['clicks'],
        'unique_clicks' => $url['unique_clicks'],
      );

      $time_series[] = $time_series_data;
    }

    return $time_series;
  }

  /**
   * Gets an array of URLs data used in tests.
   */
  protected function getUrlsTestData() {
    $urls = array();

    // URL One
    $url = array(
      'url' => 'http://example.com/example-page-one',
      'sent' => 42,
      'clicks' => 42,
      'unique_clicks' => 42,
    );

    $urls[] = $url;

    // URL Two
    $url = array(
      'url' => 'http://example.com/example-page-two',
      'sent' => 42,
      'clicks' => 42,
      'unique_clicks' => 42,
    );

    $urls[] = $url;

    return $urls;
  }
}
