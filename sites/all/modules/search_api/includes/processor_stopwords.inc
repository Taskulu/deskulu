<?php

/**
 * @file
 * Contains SearchApiStopWords.
 */

/**
 * Processor for removing stopwords from index and search terms.
 */
class SearchApiStopWords extends SearchApiAbstractProcessor {

  /**
   * Holds all words ignored for the last query.
   *
   * @var array
   */
  protected $ignored = array();

  public function configurationForm() {
    $form = parent::configurationForm();

    $form += array(
      'help' => array(
        '#markup' => '<p>' . t('Provide a stopwords file or enter the words in this form. If you do both, both will be used. Read about !stopwords.', array('!stopwords' => l(t('stop words'), "http://en.wikipedia.org/wiki/Stop_words"))) . '</p>',
      ),
      'file' => array(
        '#type' => 'textfield',
        '#title' => t('Stopwords file'),
        '#description' => t('This must be a stream-type description like <code>public://stopwords/stopwords.txt</code> or <code>http://example.com/stopwords.txt</code> or <code>private://stopwords.txt</code>.'),
      ),
      'stopwords' => array(
        '#type' => 'textarea',
        '#title' => t('Stopwords'),
        '#description' => t('Enter a space and/or linebreak separated list of stopwords that will be removed from content before it is indexed and from search terms before searching.'),
        '#default_value' => t("but\ndid\nthe this that those\netc"),
      ),
    );

    if (!empty($this->options)) {
      $form['file']['#default_value'] = $this->options['file'];
      $form['stopwords']['#default_value'] = $this->options['stopwords'];
    }
    return $form;
  }

  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    parent::configurationFormValidate($form, $values, $form_state);

    $uri = $values['file'];
    if (!empty($uri) && !@file_get_contents($uri)) {
      $el = $form['file'];
      form_error($el, t('Stopwords file') . ': ' . t('The file %uri is not readable or does not exist.', array('%uri' => $uri)));
    }
  }

  public function process(&$value) {
    $stopwords = $this->getStopWords();
    if (empty($stopwords) || !is_string($value)) {
      return;
    }
    $words = preg_split('/\s+/', $value);
    foreach ($words as $sub_key => $sub_value) {
      if (isset($stopwords[$sub_value])) {
        unset($words[$sub_key]);
        $this->ignored[] = $sub_value;
      }
    }
    $value = implode(' ', $words);
  }

  public function preprocessSearchQuery(SearchApiQuery $query) {
    $this->ignored = array();
    parent::preprocessSearchQuery($query);
  }

  public function postprocessSearchResults(array &$response, SearchApiQuery $query) {
    if ($this->ignored) {
      if (isset($response['ignored'])) {
        $response['ignored'] = array_merge($response['ignored'], $this->ignored);
      }
      else {
        $response['ignored'] = $this->ignored;
      }
    }
  }

  /**
   * @return
   *   An array whose keys are the stopwords set in either the file or the text
   *   field.
   */
  protected function getStopWords() {
    if (isset($this->stopwords)) {
      return $this->stopwords;
    }
    $file_words = $form_words = array();
    if (!empty($this->options['file']) && $stopwords_file = file_get_contents($this->options['file'])) {
      $file_words = preg_split('/\s+/', $stopwords_file);
    }
    if (!empty($this->options['stopwords'])) {
      $form_words = preg_split('/\s+/', $this->options['stopwords']);
    }
    $this->stopwords = array_flip(array_merge($file_words, $form_words));
    return $this->stopwords;
  }
}
