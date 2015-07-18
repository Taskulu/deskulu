<?php

/**
 * @files
 * Contains FeedsXPathParserHTML.
 */

/**
 * XPath parsing for HTML.
 */
class FeedsXPathParserHTML extends FeedsXPathParserBase {

  /**
   * Whether this version of PHP has a useable saveHTML() method.
   *
   * @var bool
   */
  protected $hasSaveHTML = FALSE;

  /**
   * {@inheritdoc}
   */
  public function __construct($id) {
    parent::__construct($id);

    // DOMDocument::saveHTML() cannot take $node as an argument prior to 5.3.6.
    if (version_compare(phpversion(), '5.3.6', '>=')) {
      $this->hasSaveHTML = TRUE;
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function setup($source_config, FeedsFetcherResult $fetcher_result) {
    if (!empty($source_config['exp']['tidy']) && extension_loaded('tidy')) {
      $config = array(
        'merge-divs'       => FALSE,
        'merge-spans'      => FALSE,
        'join-styles'      => FALSE,
        'drop-empty-paras' => FALSE,
        'wrap'             => 0,
        'tidy-mark'        => FALSE,
        'escape-cdata'     => TRUE,
        'word-2000'        => TRUE,
      );
      // Default tidy encoding is UTF8.
      $encoding = $source_config['exp']['tidy_encoding'];
      $raw = tidy_repair_string($fetcher_result->getRaw(), $config, $encoding);
    }
    else {
      $raw = $fetcher_result->getRaw();
    }

    $document = new DOMDocument();
    $document->strictErrorChecking = FALSE;
    $document->recover = TRUE;

    // Use our own error handling.
    $use = $this->errorStart();

    if (version_compare(PHP_VERSION, '5.4.0', '>=')) {
      $options = LIBXML_NONET;
      $options |= defined('LIBXML_COMPACT') ? LIBXML_COMPACT : 0;
      $options |= defined('LIBXML_PARSEHUGE') ? LIBXML_PARSEHUGE : 0;

      $success = $document->loadHTML($raw, $options);
    }
    else {
      $success = $document->loadHTML($raw);
    }

    $this->errorStop($use, $source_config['exp']['errors']);

    if (!$success) {
      throw new Exception(t('There was an error parsing the HTML document.'));
    }

    return $document;
  }

  /**
   * {@inheritdoc}
   */
  protected function getRaw(DOMNode $node) {
    if ($this->hasSaveHTML) {
      return $this->doc->saveHTML($node);
    }

    return $this->doc->saveXML($node, LIBXML_NOEMPTYTAG);
  }

}
