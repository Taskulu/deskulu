<?php

/**
 * @file
 * Contains the FeedsFetcher and related classes.
 */

/**
 * Base class for all fetcher results.
 */
class FeedsFetcherResult extends FeedsResult {
  protected $raw;
  protected $file_path;

  /**
   * Constructor.
   */
  public function __construct($raw) {
    $this->raw = $raw;
  }

  /**
   * @return
   *   The raw content from the source as a string.
   *
   * @throws Exception
   *   Extending classes MAY throw an exception if a problem occurred.
   */
  public function getRaw() {
    return $this->sanitizeRaw($this->raw);
  }

  /**
   * Get a path to a temporary file containing the resource provided by the
   * fetcher.
   *
   * File will be deleted after DRUPAL_MAXIMUM_TEMP_FILE_AGE.
   *
   * @return
   *   A path to a file containing the raw content as a source.
   *
   * @throws Exception
   *   If an unexpected problem occurred.
   */
  public function getFilePath() {
    if (!isset($this->file_path)) {
      $destination = 'public://feeds';
      if (!file_prepare_directory($destination, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
        throw new Exception(t('Feeds directory either cannot be created or is not writable.'));
      }
      $this->file_path = FALSE;
      if ($file = file_save_data($this->getRaw(), $destination . '/' . get_class($this) . REQUEST_TIME)) {
        $file->status = 0;
        file_save($file);
        $this->file_path = $file->uri;
      }
      else {
        throw new Exception(t('Cannot write content to %dest', array('%dest' => $destination)));
      }
    }
    return $this->sanitizeFile($this->file_path);
  }

  /**
   * Sanitize the raw content string. Currently supported sanitizations:
   *
   * - Remove BOM header from UTF-8 files.
   *
   * @param string $raw
   *   The raw content string to be sanitized.
   * @return
   *   The sanitized content as a string.
   */
  public function sanitizeRaw($raw) {
    if (substr($raw, 0, 3) == pack('CCC', 0xef, 0xbb, 0xbf)) {
      $raw = substr($raw, 3);
    }
    return $raw;
  }

  /**
   * Sanitize the file in place. Currently supported sanitizations:
   *
   * - Remove BOM header from UTF-8 files.
   *
   * @param string $filepath
   *   The file path of the file to be sanitized.
   * @return
   *   The file path of the sanitized file.
   */
  public function sanitizeFile($filepath) {
    $handle = fopen($filepath, 'r');
    $line = fgets($handle);
    fclose($handle);
    // If BOM header is present, read entire contents of file and overwrite
    // the file with corrected contents.
    if (substr($line, 0, 3) == pack('CCC', 0xef, 0xbb, 0xbf)) {
      $contents = file_get_contents($filepath);
      $contents = substr($contents, 3);
      $status = file_put_contents($filepath, $contents);
      if ($status === FALSE) {
        throw new Exception(t('File @filepath is not writeable.', array('@filepath' => $filepath)));
      }
    }
    return $filepath;
  }
}

/**
 * Abstract class, defines shared functionality between fetchers.
 *
 * Implements FeedsSourceInfoInterface to expose source forms to Feeds.
 */
abstract class FeedsFetcher extends FeedsPlugin {

  /**
   * Implements FeedsPlugin::pluginType().
   */
  public function pluginType() {
    return 'fetcher';
  }

  /**
   * Fetch content from a source and return it.
   *
   * Every class that extends FeedsFetcher must implement this method.
   *
   * @param $source
   *   Source value as entered by user through sourceForm().
   *
   * @return
   *   A FeedsFetcherResult object.
   */
  public abstract function fetch(FeedsSource $source);

  /**
   * Clear all caches for results for given source.
   *
   * @param FeedsSource $source
   *   Source information for this expiry. Implementers can choose to only clear
   *   caches pertaining to this source.
   */
  public function clear(FeedsSource $source) {}

  /**
   * Request handler invoked if callback URL is requested. Locked down by
   * default. For a example usage see FeedsHTTPFetcher.
   *
   * Note: this method may exit the script.
   *
   * @return
   *   A string to be returned to the client.
   */
  public function request($feed_nid = 0) {
    drupal_access_denied();
  }

  /**
   * Construct a path for a concrete fetcher/source combination. The result of
   * this method matches up with the general path definition in
   * FeedsFetcher::menuItem(). For example usage look at FeedsHTTPFetcher.
   *
   * @return
   *   Path for this fetcher/source combination.
   */
  public function path($feed_nid = 0) {
    $id = urlencode($this->id);
    if ($feed_nid && is_numeric($feed_nid)) {
      return "feeds/importer/$id/$feed_nid";
    }
    return "feeds/importer/$id";
  }

  /**
   * Menu item definition for fetchers of this class. Note how the path
   * component in the item definition matches the return value of
   * FeedsFetcher::path();
   *
   * Requests to this menu item will be routed to FeedsFetcher::request().
   *
   * @return
   *   An array where the key is the Drupal menu item path and the value is
   *   a valid Drupal menu item definition.
   */
  public function menuItem() {
    return array(
      'feeds/importer/%feeds_importer' => array(
        'page callback' => 'feeds_fetcher_callback',
        'page arguments' => array(2, 3),
        'access callback' => TRUE,
        'file' => 'feeds.pages.inc',
        'type' => MENU_CALLBACK,
        ),
      );
  }

  /**
   * Subscribe to a source. Only implement if fetcher requires subscription.
   *
   * @param FeedsSource $source
   *   Source information for this subscription.
   */
  public function subscribe(FeedsSource $source) {}

  /**
   * Unsubscribe from a source. Only implement if fetcher requires subscription.
   *
   * @param FeedsSource $source
   *   Source information for unsubscribing.
   */
  public function unsubscribe(FeedsSource $source) {}

  /**
   * Override import period settings. This can be used to force a certain import
   * interval.
   *
   * @param $source
   *   A FeedsSource object.
   *
   * @return
   *   A time span in seconds if periodic import should be overridden for given
   *   $source, NULL otherwise.
   */
  public function importPeriod(FeedsSource $source) {}
}
