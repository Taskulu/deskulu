<?php

/**
 * @files
 * Contains FeedsXPathParserXML.
 */

/**
 * XPath parsing for XML.
 */
class FeedsXPathParserXML extends FeedsXPathParserBase {

  /**
   * {@inheritdoc}
   */
  protected function setup($source_config, FeedsFetcherResult $fetcher_result) {
    if (!empty($source_config['exp']['tidy']) && extension_loaded('tidy')) {
      $config = array(
        'input-xml' => TRUE,
        'wrap'      => 0,
        'tidy-mark' => FALSE,
      );
      // Default tidy encoding is UTF8.
      $encoding = $source_config['exp']['tidy_encoding'];
      $raw = tidy_repair_string($fetcher_result->getRaw(), $config, $encoding);
    }
    else {
      $raw = $fetcher_result->getRaw();
    }

    $options = LIBXML_NONET;
    $options |= defined('LIBXML_COMPACT') ? LIBXML_COMPACT : 0;
    $options |= defined('LIBXML_PARSEHUGE') ? LIBXML_PARSEHUGE : 0;

    $document = new DOMDocument();
    $document->strictErrorChecking = FALSE;
    $document->recover = TRUE;

    $use = $this->errorStart();

    $success = $document->loadXML($raw, $options);

    $this->errorStop($use, $source_config['exp']['errors']);

    if (!$success) {
      throw new Exception(t('There was an error parsing the XML document.'));
    }

    return $document;
  }

  /**
   * {@inheritdoc}
   */
  protected function getRaw(DOMNode $node) {
    return $this->doc->saveXML($node);
  }

}
