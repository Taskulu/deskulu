<?php

/**
 * @file
 * Contains SearchApiHtmlFilter.
 */

/**
 * Processor for stripping HTML from indexed fulltext data. Supports assigning
 * custom boosts for any HTML element.
 */
// @todo Process query?
class SearchApiHtmlFilter extends SearchApiAbstractProcessor {

  /**
   * @var array
   */
  protected $tags;

  public function __construct(SearchApiIndex $index, array $options = array()) {
    parent::__construct($index, $options);
    $this->options += array(
      'title' => FALSE,
      'alt'   => TRUE,
      'tags'  => "h1 = 5\n" .
          "h2 = 3\n" .
          "h3 = 2\n" .
          "strong = 2\n" .
          "b = 2\n" .
          "em = 1.5\n" .
          'u = 1.5',
    );
    $this->tags = drupal_parse_info_format($this->options['tags']);
    // Specifying empty tags doesn't make sense.
    unset($this->tags['br'], $this->tags['hr']);
  }

  public function configurationForm() {
    $form = parent::configurationForm();
    $form += array(
      'title' => array(
        '#type' => 'checkbox',
        '#title' => t('Index title attribute'),
        '#description' => t('If set, the contents of title attributes will be indexed.'),
        '#default_value' => $this->options['title'],
      ),
      'alt' => array(
        '#type' => 'checkbox',
        '#title' => t('Index alt attribute'),
        '#description' => t('If set, the alternative text of images will be indexed.'),
        '#default_value' => $this->options['alt'],
      ),
      'tags' => array(
        '#type' => 'textarea',
        '#title' => t('Tag boosts'),
        '#description' => t('Specify special boost values for certain HTML elements, in <a href="@link">INI file format</a>. ' .
            'The boost values of nested elements are multiplied, elements not mentioned will have the default boost value of 1. ' .
            'Assign a boost of 0 to ignore the text content of that HTML element.',
            array('@link' => url('http://api.drupal.org/api/function/drupal_parse_info_format/7'))),
        '#default_value' => $this->options['tags'],
      ),
    );

    return $form;
  }

  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    parent::configurationFormValidate($form, $values, $form_state);

    if (empty($values['tags'])) {
      return;
    }
    $tags = drupal_parse_info_format($values['tags']);
    $errors = array();
    foreach ($tags as $key => $value) {
      if (is_array($value)) {
        $errors[] = t("Boost value for tag &lt;@tag&gt; can't be an array.", array('@tag' => $key));
      }
      elseif (!is_numeric($value)) {
        $errors[] = t("Boost value for tag &lt;@tag&gt; must be numeric.", array('@tag' => $key));
      }
      elseif ($value < 0) {
        $errors[] = t('Boost value for tag &lt;@tag&gt; must be non-negative.', array('@tag' => $key));
      }
    }
    if ($errors) {
      form_error($form['tags'], implode("<br />\n", $errors));
    }
  }

  protected function processFieldValue(&$value) {
    $text = str_replace(array('<', '>'), array(' <', '> '), $value); // Let removed tags still delimit words.
    if ($this->options['title']) {
      $text = preg_replace('/(<[-a-z_]+[^>]+)\btitle\s*=\s*("([^"]+)"|\'([^\']+)\')([^>]*>)/i', '$1 $5 $3$4 ', $text);
    }
    if ($this->options['alt']) {
      $text = preg_replace('/<img\b[^>]+\balt\s*=\s*("([^"]+)"|\'([^\']+)\')[^>]*>/i', ' <img>$2$3</img> ', $text);
    }
    if ($this->tags) {
      $text = strip_tags($text, '<' . implode('><', array_keys($this->tags)) . '>');
      $value = $this->parseText($text);
    }
    else {
      $value = strip_tags($text);
      // Remove any multiple or leading/trailing spaces we might have introduced.
      $value = preg_replace('/\s\s+/', ' ', trim($value));
    }
  }

  protected function parseText(&$text, $active_tag = NULL, $boost = 1) {
    $ret = array();
    while (($pos = strpos($text, '<')) !== FALSE) {
      if ($boost && $pos > 0) {
        $token = html_entity_decode(substr($text, 0, $pos), ENT_QUOTES, 'UTF-8');
        // Remove any multiple or leading/trailing spaces we might have introduced.
        $token = preg_replace('/\s\s+/', ' ', trim($token));
        $ret[] = array(
          'value' => $token,
          'score' => $boost,
        );
      }
      $text = substr($text, $pos + 1);
      if (!preg_match('#^(/?)([-:_a-zA-Z]+)#', $text, $m)) {
        continue;
      }
      $text = substr($text, strpos($text, '>') + 1);
      if ($m[1]) {
        // Closing tag.
        if ($active_tag && $m[2] == $active_tag) {
          return $ret;
        }
      }
      else {
        // Opening tag => recursive call.
        $inner_boost = $boost * (isset($this->tags[$m[2]]) ? $this->tags[$m[2]] : 1);
        $ret = array_merge($ret, $this->parseText($text, $m[2], $inner_boost));
      }
    }
    if ($text) {
      $token = html_entity_decode($text, ENT_QUOTES, 'UTF-8');
      // Remove any multiple or leading/trailing spaces we might have introduced.
      $token = preg_replace('/\s\s+/', ' ', trim($token));
      $ret[] = array(
        'value' => $token,
        'score' => $boost,
      );
      $text = '';
    }
    return $ret;
  }

}
