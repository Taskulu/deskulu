<?php

/**
 * @file
 * OPML Parser.
 */

/**
 * Parse OPML file.
 *
 * @param $raw
 *   File contents.
 * @return
 *   An array of the parsed OPML file.
 */
function opml_parser_parse($raw) {
  $feeds = $items = array();
  $xml = @ new SimpleXMLElement($raw);
  $feeds['title'] = (string)current($xml->xpath('//head/title'));

  // @todo Make xpath case insensitive.
  $outlines = $xml->xpath('//outline[@xmlUrl]');
  foreach ($outlines as $outline) {
    $item = array();
    foreach ($outline->attributes() as $k => $v) {
      if (in_array(strtolower($k), array('title', 'text', 'xmlurl'))) {
        $item[strtolower($k)] = (string) $v;
      }
    }

    // If no title, forge it from text.
    if (!isset($item['title']) && isset($item['text'])) {
      if (strlen($item['text']) < 40) {
        $item['title'] = $item['text'];
      }
      else {
        $item['title'] = trim(substr($item['text'], 0, 30)) . ' ...';
      }
    }
    if (isset($item['title']) && isset($item['xmlurl'])) {
      $items[] = $item;
    }
  }
  $feeds['items'] = $items;
  return $feeds;
}
