<?php

/**
 * @file
 * Make URLs absolute.
 *
 * @todo Use DOM rather than strtr().
 */

$plugin = array(
  'form' => 'feeds_tamper_absolute_url_form',
  'callback' => 'feeds_tamper_absolute_url_callback',
  'name' => 'Make URLs absolute',
  'category' => 'HTML',
);

function feeds_tamper_absolute_url_form($importer, $element_key, $settings) {
  $form = array();
  $links = htmlentities('<a href="/stuff/things"> to <a href="http://example.com/stuff/things">');
  $form['info'] = array(
    '#markup' => t('Make URLs in markup absolute. (i.e. !links).', array('!links' => $links)),
  );
  return $form;
}

function feeds_tamper_absolute_url_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  static $dom;

  $b = parse_url($result->link);
  if (!isset($dom)) {
    $dom = new DOMDocument();
  }

  $field = (string) $field;

  if (!$field) {
    return;
  }

  // Supress warnings for invalid HTML.
  $errors = libxml_use_internal_errors(TRUE);
  $entity_loader = libxml_disable_entity_loader(TRUE);

  $dom->loadHTML($field);

  libxml_clear_errors();
  libxml_use_internal_errors($errors);
  libxml_disable_entity_loader($entity_loader);

  $urls = array();
  $tags = array('a' => 'href', 'img' => 'src', 'iframe' => 'src',
                'script' => 'src', 'object' => 'codebase', 'link' => 'href',
                'applet' => 'code', 'base' => 'href');
  foreach ($tags as $tag => $attr) {
    foreach ($dom->getElementsByTagName($tag) as $t) {
      $value = trim($t->getAttribute($attr));
      _feeds_tamper_absolute_url($value, $urls, $b);
    }
  }
  $field = strtr($field, $urls);
}

function _feeds_tamper_absolute_url($r_url, &$urls, $b) {
  if (!strlen($r_url)) {
    return;
  }

  $r = parse_url($r_url);

  if ($r === FALSE) {
    return;
  }
  if (!empty($r['scheme']) || !empty($r['host'])) {
    return;
  }

  $r['scheme'] = $b['scheme'];

  unset($r['port']);
  unset($r['user']);
  unset($r['pass']);

  // Copy base authority.
  $r['host'] = $b['host'];
  if (isset($b['port'])) {
    $r['port'] = $b['port'];
  }
  if (isset($b['user'])) {
    $r['user'] = $b['user'];
  }
  if (isset($b['pass'])) {
    $r['pass'] = $b['pass'];
  }

  // If relative URL has no path, use base path.
  if (empty($r['path'])) {
    if (!empty($b['path'])) {
      $r['path'] = $b['path'];
    }
    if (!isset($r['query']) && isset($b['query'])) {
      $r['query'] = $b['query'];
    }
    $urls[$r_url] = _feeds_tamper_join_url($r);
    return;
  }

  // If relative URL path doesn't start with /, merge with base path.
  if (strpos($r['path'], '/') !== 0) {
    if (empty($b['path'])) {
      $b['path'] = '';
    }
    $r['path'] = $b['path'] . '/' . $r['path'];
  }
  $urls[$r_url] = _feeds_tamper_join_url($r);
}

function _feeds_tamper_join_url($parts) {
  $url = '';
  if (!empty($parts['scheme'])) {
    $url .= $parts['scheme'] . ':';
  }
  if (isset($parts['host'])) {
    $url .= '//';
    if (isset($parts['user'])) {
      $url .= $parts['user'];
      if (isset($parts['pass'])) {
        $url .= ':' . $parts['pass'];
      }
      $url .= '@';
    }
    // IPv6.
    if (preg_match('/^[\da-f]*:[\da-f.:]+$/ui', $parts['host'])) {
      $url .= '[' . $parts['host'] . ']';
    }
    else {
      // IPv4 or name.
      $url .= $parts['host'];
    }
    if (isset($parts['port'])) {
      $url .= ':' . $parts['port'];
    }
    if (!empty($parts['path']) && $parts['path'][0] != '/') {
      $url .= '/';
    }
  }
  if (!empty($parts['path'])) {
    $url .= $parts['path'];
  }
  if (isset($parts['query'])) {
    $url .= '?' . $parts['query'];
  }
  if (isset($parts['fragment'])) {
    $url .= '#' . $parts['fragment'];
  }
  return $url;
}
