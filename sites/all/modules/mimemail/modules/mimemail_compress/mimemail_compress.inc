<?php

/**
 * @file
 * Converts CSS styles into inline style attributes.
 *
 * Code based on Emogrifier by Pelago Design (http://www.pelagodesign.com).
 */

/**
 * Separate CSS from HTML for processing
 */
function mimemail_compress_clean_message($message) {
  $parts = array();
  preg_match('|(<style[^>]+)>(.*)</style>|mis', $message, $matches);
  if (isset($matches[0]) && isset($matches[2])) {
    $css = str_replace('<!--', '', $matches[2]);
    $css = str_replace('-->', '', $css);
    $css = preg_replace('|\{|', "\n{\n", $css);
    $css = preg_replace('|\}|', "\n}\n", $css);
    $html = str_replace($matches[0], '', $message);
    $parts = array('html' => $html, 'css' => $css);
  }
  return $parts;
}

/**
 * Compress HTML and CSS into combined message
 */
class mimemail_compress {
  private $html = '';
  private $css = '';
  private $unprocessable_tags = array('wbr');

  public function mimemail_compress($html = '', $css = '') {
    $this->html = $html;
    $this->css  = $css;
  }

  // There are some HTML tags that DOMDocument cannot process,
  // and will throw an error if it encounters them.
  // These functions allow you to add/remove them if necessary.
  // It only strips them from the code (does not remove actual nodes).
  public function add_unprocessable_tag($tag) {
    $this->unprocessable_tags[] = $tag;
  }

  public function remove_unprocessable_tag($tag) {
    if (($key = array_search($tag, $this->unprocessable_tags)) !== FALSE) {
      unset($this->unprocessableHTMLTags[$key]);
    }
  }

  public function compress() {
    if (!class_exists('DOMDocument', FALSE)) {
      return $this->html;
    }

    $body = $this->html;
    // Process the CSS here, turning the CSS style blocks into inline CSS.
    if (count($this->unprocessable_tags)) {
      $unprocessable_tags = implode('|', $this->unprocessable_tags);
      $body = preg_replace("/<($unprocessable_tags)[^>]*>/i", '', $body);
    }

    $err = error_reporting(0);
    $doc = new DOMDocument();

    // Try to set character encoding.
    if (function_exists('mb_convert_encoding')) {
      $body = mb_convert_encoding($body, 'HTML-ENTITIES', "UTF-8");
      $doc->encoding= "UTF-8";
    }

    $doc->strictErrorChecking = FALSE;
    $doc->formatOutput = TRUE;
    $doc->loadHTML($body);
    $doc->normalizeDocument();

    $xpath = new DOMXPath($doc);

    // Get rid of comments.
    $css = preg_replace('/\/\*.*\*\//sU', '', $this->css);

    // Process the CSS file for selectors and definitions.
    preg_match_all('/^\s*([^{]+){([^}]+)}/mis', $css, $matches);

    $all_selectors = array();
    foreach ($matches[1] as $key => $selector_string) {
      // If there is a blank definition, skip.
      if (!strlen(trim($matches[2][$key]))) continue;
      // Else split by commas and duplicate attributes so we can sort by selector precedence.
      $selectors = explode(',', $selector_string);
      foreach ($selectors as $selector) {
        // Don't process pseudo-classes.
        if (strpos($selector, ':') !== FALSE) continue;
        $all_selectors[] = array(
          'selector' => $selector,
          'attributes' => $matches[2][$key],
          'index' => $key, // Keep track of where it appears in the file, since order is important.
        );
      }
    }

    // Now sort the selectors by precedence.
    usort($all_selectors, array('self', 'sort_selector_precedence'));

    // Before we begin processing the CSS file, parse the document for inline
    // styles and append the normalized properties (i.e., 'display: none'
    // instead of 'DISPLAY: none') as selectors with full paths (highest
    // precedence), so they override any file-based selectors.
    $nodes = @$xpath->query('//*[@style]');
    if ($nodes->length > 0) {
      foreach ($nodes as $node) {
        $style = preg_replace_callback('/[A-z\-]+(?=\:)/S', create_function('$matches', 'return strtolower($matches[0]);'), $node->getAttribute('style'));
        $all_selectors[] = array(
          'selector' => $this->calculateXPath($node),
          'attributes' => $style,
        );
      }
    }

    foreach ($all_selectors as $value) {
      // Query the body for the xpath selector.
      $nodes = $xpath->query($this->css_to_xpath(trim($value['selector'])));

      foreach ($nodes as $node) {
        // If it has a style attribute, get it, process it, and append (overwrite) new stuff.
        if ($node->hasAttribute('style')) {
          // Break it up into an associative array.
          $old_style = $this->css_style_to_array($node->getAttribute('style'));
          $new_style = $this->css_style_to_array($value['attributes']);
          // New styles overwrite the old styles (not technically accurate, but close enough).
          $compressed = array_merge($old_style, $new_style);
          $style = '';
          foreach ($compressed as $k => $v) {
            $style .= (drupal_strtolower($k) . ':' . $v . ';');
          }
        }
        else {
          // Otherwise create a new style.
          $style = trim($value['attributes']);
        }
        $node->setAttribute('style', $style);

        // Convert float to align for images.
        $float = preg_match ('/float:(left|right)/', $style, $matches);
        if ($float) {
          $node->setAttribute('align', $matches[1]);
          $node->setAttribute('vspace', 5);
          $node->setAttribute('hspace', 5);
        }
      }
    }

    // This removes styles from your email that contain display:none. You could comment these out if you want.
    $nodes = $xpath->query('//*[contains(translate(@style," ",""), "display:none")]');
    foreach ($nodes as $node) {
      $node->parentNode->removeChild($node);
    }

    if (variable_get('mimemail_preserve_class', 0) == FALSE) {
      $nodes = $xpath->query('//*[@class]');
      foreach ($nodes as $node) {
        $node->removeAttribute('class');
      }
    }

    error_reporting($err);

    return $doc->saveHTML();
  }

  private static function sort_selector_precedence($a, $b) {
    $precedenceA = self::get_selector_precedence($a['selector']);
    $precedenceB = self::get_selector_precedence($b['selector']);

    // We want these sorted ascendingly so selectors with lesser precedence get processed first and selectors with greater precedence get sorted last.
    return ($precedenceA == $precedenceB) ? ($a['index'] < $b['index'] ? -1 : 1) : ($precedenceA < $precedenceB ? -1 : 1);
  }

  private static function get_selector_precedence($selector) {
    $precedence = 0;
    $value = 100;
    // Ids: worth 100, classes: worth 10, elements: worth 1.
    $search = array('\#', '\.', '');

    foreach ($search as $s) {
      if (trim($selector == '')) break;
      $num = 0;
      $selector = preg_replace('/' . $s . '\w+/', '', $selector, -1, $num);
      $precedence += ($value * $num);
      $value /= 10;
    }

    return $precedence;
  }

  /**
   * Right now we only support CSS 1 selectors, but include CSS2/3 selectors are fully possible.
   *
   * @see http://plasmasturm.org/log/444
   */
  private function css_to_xpath($selector) {
    if (drupal_substr($selector, 0, 1) == '/') {
      // Already an XPath expression.
      return $selector;
    }
    // Returns an Xpath selector.
    $search = array(
      '/\s+>\s+/', // Matches any F element that is a child of an element E.
      '/(\w+)\s+\+\s+(\w+)/', // Matches any F element that is a child of an element E.
      '/\s+/', // Matches any F element that is a descendant of an E element.
      '/(\w)\[(\w+)\]/', // Matches element with attribute.
      '/(\w)\[(\w+)\=[\'"]?(\w+)[\'"]?\]/', // Matches element with EXACT attribute.
      '/(\w+)?\#([\w\-]+)/e', // Matches id attributes.
      '/(\w+|\*)?((\.[\w\-]+)+)/e', // Matches class attributes.
    );
    $replace = array(
      '/',
      '\\1/following-sibling::*[1]/self::\\2',
      '//',
      '\\1[@\\2]',
      '\\1[@\\2="\\3"]',
      "(strlen('\\1') ? '\\1' : '*').'[@id=\"\\2\"]'",
      "(strlen('\\1') ? '\\1' : '*').'[contains(concat(\" \",normalize-space(@class),\" \"),concat(\" \",\"'.implode('\",\" \"))][contains(concat(\" \",normalize-space(@class),\" \"),concat(\" \",\"',explode('.',substr('\\2',1))).'\",\" \"))]'",
    );
    return '//' . preg_replace($search, $replace, trim($selector));
  }

  private function css_style_to_array($style) {
    $definitions = explode(';', $style);
    $css_styles = array();
    foreach ($definitions as $def) {
      if (empty($def) || strpos($def, ':') === FALSE) continue;
      list($key, $value) = explode(':', $def, 2);
      if (empty($key) || empty($value)) continue;
      $css_styles[trim($key)] = trim($value);
    }
    return $css_styles;
  }

  /**
   * Get the full path to a DOM node.
   *
   * @param DOMNode $node
   *   The node to analyze.
   *
   * @return string
   *   The full xpath to a DOM node.
   *
   * @see http://stackoverflow.com/questions/2643533/php-getting-xpath-of-a-domnode
   */
  function calculateXPath(DOMNode $node) {
    $xpath = '';
    $q = new DOMXPath($node->ownerDocument);

    do {
        $position = 1 + $q->query('preceding-sibling::*[name()="' . $node->nodeName . '"]', $node)->length;
        $xpath = '/' . $node->nodeName . '[' . $position . ']' . $xpath;
        $node = $node->parentNode;
    }
    while (!$node instanceof DOMDocument);

    return $xpath;
  }
}
