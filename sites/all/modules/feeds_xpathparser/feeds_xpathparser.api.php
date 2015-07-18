<?php

/**
 * @file
 * Documentation of Feeds XPath Parser hooks.
 */

/**
 * Implements hook_feeds_xpathparser_filter_domnode().
 *
 * Allows arbitrary manipulation of the feed item being processed while it is
 * still a DOMNode.
 *
 * This hook can also be used to skip parsing of a specific feed item by
 * returning TRUE.
 *
 * @param DOMNode $node
 *   The feed item being parsed, as a dom node.
 * @param DOMDocument $document
 *   The entire XML/HTML document being parsed.
 * @param FeedsSource $source
 *   The feed source being imported.
 *
 * @return bool
 *   Returns TRUE if the dom node should be skipped.
 */
function hook_feeds_xpathparser_filter_domnode(DOMNode $node, DOMDocument $document, FeedsSource $source) {

  if (my_module_node_is_bad($node)) {
    return TRUE;
  }

  // To print out the raw XML.
  $debug = $document->saveXML($node);

  // For HTML.
  if (version_compare(phpversion(), '5.3.6', '>=')) {
    $debug = $document->saveHTML($node);
  }
  else {
    $debug = $document->saveXML($node, LIBXML_NOEMPTYTAG);
  }

  drupal_set_message($debug);
}
