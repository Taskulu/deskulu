<?php

/**
 * @file
 * Provides a custom version of DOMXPath for use with feeds_xpathparser.
 */

/**
 * Wraps DOMXPath providing enhanced debugging and special namespace handling.
 */
class FeedsXPathParserDOMXPath extends DOMXPath {

  /**
   * The DOMDocument to parse.
   *
   * @var DOMDocument
   */
  protected $doc;

  /**
   * Configuration array.
   *
   * @var array
   */
  protected $config = array();

  /**
   * Modified query cache.
   *
   * @var arrray
   */
  protected $modifiedQueries = array();

  /**
   * The namespaces in the document.
   *
   * @var arrray
   */
  protected $namespaces = array();

  /**
   * The most recent error from parsing.
   *
   * @var stdClass
   */
  protected $error;

  /**
   * Constructs a FeedsXPathParserDOMXPath object.
   *
   * @param DOMDocument $doc
   *   The DOMDocument that we're operating on.
   */
  public function __construct(DOMDocument $doc) {

    $simple = simplexml_import_dom($doc);

    // An empty DOMDocument will make $simple NULL.
    if ($simple !== NULL) {
      $this->namespaces = $simple->getNamespaces(TRUE);
    }
    $this->doc = $doc;

    parent::__construct($doc);
  }

  /**
   * Sets the extended configuration.
   *
   * @param array $config
   *   The config array.
   */
  public function setConfig(array $config) {
    $this->config = $config;
  }

  /**
   * Renders our debug messages into a list.
   *
   * @param mixed $data
   *   The result of an XPath query. Either a scalar or a DOMNodeList.
   * @param string $source
   *   The source key that produced this query.
   *
   * @todo Use theme_item_list().
   */
  protected function debug($data, $source) {
    $output = "$source : <ul>";

    if ($data instanceof DOMNodeList) {
      foreach ($data as $node) {
        $output .= '<li>' . check_plain($this->doc->saveXML($node)) . '</li>';
      }
    }
    else {
      $output .= '<li>' . check_plain($data) . '</li>';
    }
    $output .= '</ul>';

    drupal_set_message($output);
  }

  /**
   * Executes an XPath query with namespace support.
   *
   * @param string $query
   *   An XPath query.
   * @param DOMNode $context
   *   The current context of the XPath query.
   * @param string $source
   *   The source key for this query.
   *
   * @return array
   *   An array containing the results of the query.
   */
  public function namespacedQuery($query, $context, $source) {
    $this->addDefaultNamespace($query);

    $results = $this->executeQuery($query, $context);

    if (in_array($source, $this->config['debug'])) {
      $this->debug($results, $source);
    }

    if (is_object($this->error) && $this->config['errors']) {

      if ($this->error->level == LIBXML_ERR_ERROR) {
        drupal_set_message(
          t('There was an error during the XPath query: %query.<br />Libxml returned the message: %message, with the error code: %code.', array(
            '%query' => $query,
            '%message' => trim($this->error->message),
            '%code' => $this->error->code,
          )),
          'error',
          FALSE);
      }
      elseif ($this->error->level == LIBXML_ERR_WARNING) {
        drupal_set_message(
          t('There was an error during the XPath query: %query.<br />Libxml returned the message: %message, with the error code: %code.', array(
            '%query' => $query,
            '%message' => trim($this->error->message),
            '%code' => $this->error->code,
          )),
          'warning',
          FALSE);
      }
    }

    // DOMXPath::evaluate() and DOMXPath::query() will return FALSE on error or
    // if the value is FALSE. We check error result and return NULL in case
    // of error.
    if (is_object($this->error) && $this->error->level == LIBXML_ERR_ERROR) {
      return NULL;
    }

    return $results;
  }

  /**
   * Normalizes XPath queries, adding the default namespace.
   *
   * @param string $query
   *   An XPath query string
   */
  protected function addDefaultNamespace(&$query) {
    foreach ($this->namespaces as $prefix => $namespace) {
      if ($prefix === '') {
        $this->registerNamespace('__default__', $namespace);

        // Replace all the elements without prefix by the default prefix.
        if (!isset($this->modifiedQueries[$query])) {
          $parser = new FeedsXPathParserQueryParser($query);
          $mod_query = $parser->getQuery();
          $this->modifiedQueries[$query] = $mod_query;
          $query = $mod_query;
        }
        else {
          $query = $this->modifiedQueries[$query];
        }
      }
      else {
        $this->registerNamespace($prefix, $namespace);
      }
    }
  }

  /**
   * Performs a XPath query.
   *
   * Here we set libxml_use_internal_errors() to TRUE because depending on the
   * libxml version, $xml->xpath() might return FALSE or an empty array() when
   * a query doesn't match.
   *
   * @param string $query
   *   The XPath query string.
   * @param DOMNode $context
   *   (optional) A context object. Defaults to NULL.
   *
   * @return mixed
   *   The result of the XPath query.
   */
  protected function executeQuery($query, DOMNode $context = NULL) {
    $use_errors = libxml_use_internal_errors(TRUE);

    // Perfom XPath query.
    // So, grrr. FALSE is returned when there is an error. However, FALSE is
    // also a valid return value from DOMXPath::evaluate(). Ex: '1 = 2'
    if ($context) {
      $results = $this->evaluate($query, $context);
    }
    else {
      $results = $this->query($query);
    }

    $this->error = libxml_get_last_error();
    libxml_clear_errors();
    libxml_use_internal_errors($use_errors);
    return $results;
  }

}
