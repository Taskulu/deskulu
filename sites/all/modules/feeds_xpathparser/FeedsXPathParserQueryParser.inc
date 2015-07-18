<?php

/**
 * @file
 * Pseudo-parser of XPath queries.  When an XML document has a default
 * namespace this gets called so that adding the __default__ namepace where
 * appropriate. Aren't we nice?
 *
 * @todo
 *   Cleanup.
 * @param $query
 *   An XPath query string.
 * @return string
 *   An XPath query string with the __default__ namespace added.
 */
class FeedsXPathParserQueryParser {
  function __construct($query) {
    $this->query = preg_replace('/\s+\(\s*/', '(', $query);

    $this->word_boundaries = array(
      '[', ']', '=', '(', ')', '.', '<', '>', '*', '!', '|', '/', ',', ' ', ':',
    );
    $this->in_quotes = FALSE;
    $this->quote_char = '';
    $this->word = '';
    $this->output = '';
    $this->prev_boundary = '';
    $this->axis = '';
    $this->skip_next_word = FALSE;
    $this->start();
  }

  function start() {
    for ($i=0; $i < drupal_strlen($this->query); $i++) {
      $this->i = $i;
      $c = drupal_substr($this->query, $i, 1);

      if ($c == '"' || $c == "'") {
        $this->handle_quote($c);
        continue;
      }
      if ($this->in_quotes) {
        $this->word .= $c;
        continue;
      }

      if (in_array($c, $this->word_boundaries)) {
        $this->handle_word_boundary($c);
      }
      else {
        $this->word .= $c;
      }
    }
    $this->handle_word();
  }

  function handle_quote($c) {
    if ($this->in_quotes && $c == $this->quote_char) {
      $this->in_quotes = FALSE;
      $this->word .= $c;
      $this->output .= $this->word;
      $this->word = '';
    }
    elseif (!$this->in_quotes) {
      $this->in_quotes = TRUE;
      $this->handle_word();
      $this->word = $c;
      $this->quote_char = $c;
    }
    else {
      $this->word .= $c;
    }
  }

  function handle_word_boundary($c) {
    if (in_array($this->word, array('div', 'or', 'and', 'mod')) &&
        $this->prev_boundary == ' ' && $c == ' ') {
      $this->output .= $this->word;
    }
    else {
      $this->handle_word($c);
    }
    $this->output .= $c;
    $this->word = '';
    $this->prev_boundary = $c;
  }

  function handle_word($c='') {
    if ($this->word == '') {
      return;
    }

    if ($c == ':' && drupal_substr($this->query, $this->i + 1, 1) == ':') {
      $this->axis = $this->word;
    }

    if ($c == ':' && drupal_substr($this->query, $this->i - 1, 1) != ':'  &&
        drupal_substr($this->query, $this->i + 1, 1) != ':') {
      $this->output .= $this->word;
      $this->skip_next_word = TRUE;
      return;
    }
    if ($this->skip_next_word) {
      $this->skip_next_word = FALSE;
      $this->output .= $this->word;
      return;
    }
    if (is_numeric($this->word) ||
        $this->axis == 'attribute' ||
        strpos($this->word, '@') === 0 ||
        $c == '(' ||
        $c == ':') {
      $this->output .= $this->word;
      return;
    }
    $this->output .= '__default__:' . $this->word;
  }

  function getQuery() {
    return $this->output;
  }
}
