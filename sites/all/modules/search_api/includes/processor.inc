<?php

/**
 * @file
 * Contains SearchApiProcessorInterface and SearchApiAbstractProcessor.
 */

/**
 * Interface representing a Search API pre- and/or post-processor.
 *
 * While processors are enabled or disabled for both pre- and postprocessing at
 * once, many processors will only need to run in one of those two phases. Then,
 * the other method(s) should simply be left blank. A processor should make it
 * clear in its description or documentation when it will run and what effect it
 * will have.
 * Usually, processors preprocessing indexed items will likewise preprocess
 * search queries, so these two methods should mostly be implemented either both
 * or neither.
 */
interface SearchApiProcessorInterface {

  /**
   * Construct a processor.
   *
   * @param SearchApiIndex $index
   *   The index for which processing is done.
   * @param array $options
   *   The processor options set for this index.
   */
  public function __construct(SearchApiIndex $index, array $options = array());

  /**
   * Check whether this processor is applicable for a certain index.
   *
   * This can be used for hiding the processor on the index's "Filters" tab. To
   * avoid confusion, you should only use criteria that are immutable, such as
   * the index's item type. Also, since this is only used for UI purposes, you
   * should not completely rely on this to ensure certain index configurations
   * and at least throw an exception with a descriptive error message if this is
   * violated on runtime.
   *
   * @param SearchApiIndex $index
   *   The index to check for.
   *
   * @return boolean
   *   TRUE if the processor can run on the given index; FALSE otherwise.
   */
  public function supportsIndex(SearchApiIndex $index);

  /**
   * Display a form for configuring this processor.
   * Since forcing users to specify options for disabled processors makes no
   * sense, none of the form elements should have the '#required' attribute set.
   *
   * @return array
   *   A form array for configuring this processor, or FALSE if no configuration
   *   is possible.
   */
  public function configurationForm();

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state);

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * This method should both return the new options and set them internally.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   *
   * @return array
   *   The new options array for this callback.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state);

  /**
   * Preprocess data items for indexing.
   *
   * Typically, a preprocessor will execute its preprocessing (e.g. stemming,
   * n-grams, word splitting, stripping stop words, etc.) only on the items'
   * search_api_fulltext fields, if set. Other fields should usually be left
   * untouched.
   *
   * @param array $items
   *   An array of items to be preprocessed for indexing, formatted as specified
   *   by SearchApiServiceInterface::indexItems().
   */
  public function preprocessIndexItems(array &$items);

  /**
   * Preprocess a search query.
   *
   * The same applies as when preprocessing indexed items: typically, only the
   * fulltext search keys should be processed, queries on specific fields should
   * usually not be altered.
   *
   * @param SearchApiQuery $query
   *   The object representing the query to be executed.
   */
  public function preprocessSearchQuery(SearchApiQuery $query);

  /**
   * Postprocess search results before display.
   *
   * If a class is used for both pre- and post-processing a search query, the
   * same object will be used for both calls (so preserving some data or state
   * locally is possible).
   *
   * @param array $response
   *   An array containing the search results. See the return value of
   *   SearchApiQueryInterface->execute() for the detailed format.
   * @param SearchApiQuery $query
   *   The object representing the executed query.
   */
  public function postprocessSearchResults(array &$response, SearchApiQuery $query);

}

/**
 * Abstract processor implementation that provides an easy framework for only
 * processing specific fields.
 *
 * Simple processors can just override process(), while others might want to
 * override the other process*() methods, and test*() (for restricting
 * processing to something other than all fulltext data).
 */
abstract class SearchApiAbstractProcessor implements SearchApiProcessorInterface {

  /**
   * @var SearchApiIndex
   */
  protected $index;

  /**
   * @var array
   */
  protected $options;

  /**
   * Constructor, saving its arguments into properties.
   */
  public function __construct(SearchApiIndex $index, array $options = array()) {
    $this->index   = $index;
    $this->options = $options;
  }

  public function supportsIndex(SearchApiIndex $index) {
    return TRUE;
  }

  public function configurationForm() {
    $form['#attached']['css'][] = drupal_get_path('module', 'search_api') . '/search_api.admin.css';

    $fields = $this->index->getFields();
    $field_options = array();
    $default_fields = array();
    if (isset($this->options['fields'])) {
      $default_fields = drupal_map_assoc(array_keys($this->options['fields']));
    }
    foreach ($fields as $name => $field) {
      $field_options[$name] = check_plain($field['name']);
      if (!empty($default_fields[$name]) || (!isset($this->options['fields']) && $this->testField($name, $field))) {
        $default_fields[$name] = $name;
      }
    }

    $form['fields'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Fields to run on'),
      '#options' => $field_options,
      '#default_value' => $default_fields,
      '#attributes' => array('class' => array('search-api-checkboxes-list')),
    );

    return $form;
  }

  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    $fields = array_filter($values['fields']);
    if ($fields) {
      $fields = array_fill_keys($fields, TRUE);
    }
    $values['fields'] = $fields;
  }

  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $this->options = $values;
    return $values;
  }

  /**
   * Calls processField() for all appropriate fields.
   */
  public function preprocessIndexItems(array &$items) {
    foreach ($items as &$item) {
      foreach ($item as $name => &$field) {
        if ($this->testField($name, $field)) {
          $this->processField($field['value'], $field['type']);
        }
      }
    }
  }

  /**
   * Calls processKeys() for the keys and processFilters() for the filters.
   */
  public function preprocessSearchQuery(SearchApiQuery $query) {
    $keys = &$query->getKeys();
    $this->processKeys($keys);
    $filter = $query->getFilter();
    $filters = &$filter->getFilters();
    $this->processFilters($filters);
  }

  /**
   * Does nothing.
   */
  public function postprocessSearchResults(array &$response, SearchApiQuery $query) {
    return;
  }

  /**
   * Method for preprocessing field data.
   *
   * Calls process() either for the whole text, or each token, depending on the
   * type. Also takes care of extracting list values and of fusing returned
   * tokens back into a one-dimensional array.
   */
  protected function processField(&$value, &$type) {
    if (!isset($value) || $value === '') {
      return;
    }
    if (substr($type, 0, 5) == 'list<') {
      $inner_type = $t = $t1 = substr($type, 5, -1);
      foreach ($value as &$v) {
        $t1 = $inner_type;
        $this->processField($v, $t1);
        // If one value got tokenized, all others have to follow.
        if ($t1 != $inner_type) {
          $t = $t1;
        }
      }
      if ($t == 'tokens') {
        foreach ($value as $i => &$v) {
          if (!$v) {
            unset($value[$i]);
            continue;
          }
          if (!is_array($v)) {
            $v = array(array('value' => $v, 'score' => 1));
          }
        }
      }
      $type = "list<$t>";
      return;
    }
    if ($type == 'tokens') {
      foreach ($value as &$token) {
        $this->processFieldValue($token['value']);
      }
    }
    else {
      $this->processFieldValue($value);
    }
    if (is_array($value)) {
      // Don't tokenize non-fulltext content!
      if (in_array($type, array('text', 'tokens'))) {
        $type = 'tokens';
        $value = $this->normalizeTokens($value);
      }
      else {
        $value = $this->implodeTokens($value);
      }
    }
  }

  /**
   * Internal helper function for normalizing tokens.
   */
  protected function normalizeTokens($tokens, $score = 1) {
    $ret = array();
    foreach ($tokens as $token) {
      if (empty($token['value']) && !is_numeric($token['value'])) {
        // Filter out empty tokens.
        continue;
      }
      if (!isset($token['score'])) {
        $token['score'] = $score;
      }
      else {
        $token['score'] *= $score;
      }
      if (is_array($token['value'])) {
        foreach ($this->normalizeTokens($token['value'], $token['score']) as $t) {
          $ret[] = $t;
        }
      }
      else {
        $ret[] = $token;
      }
    }
    return $ret;
  }

  /**
   * Internal helper function for imploding tokens into a single string.
   *
   * @param array $tokens
   *   The tokens array to implode.
   *
   * @return string
   *   The text data from the tokens concatenated into a single string.
   */
  protected function implodeTokens(array $tokens) {
    $ret = array();
    foreach ($tokens as $token) {
      if (empty($token['value']) && !is_numeric($token['value'])) {
        // Filter out empty tokens.
        continue;
      }
      if (is_array($token['value'])) {
        $ret[] = $this->implodeTokens($token['value']);
      }
      else {
        $ret[] = $token['value'];
      }
    }
    return implode(' ', $ret);
  }

  /**
   * Method for preprocessing search keys.
   */
  protected function processKeys(&$keys) {
    if (is_array($keys)) {
      foreach ($keys as $key => &$v) {
        if (element_child($key)) {
          $this->processKeys($v);
          if (!$v && !is_numeric($v)) {
            unset($keys[$key]);
          }
        }
      }
    }
    else {
      $this->processKey($keys);
    }
  }

  /**
   * Method for preprocessing query filters.
   */
  protected function processFilters(array &$filters) {
    $fields = $this->index->options['fields'];
    foreach ($filters as $key => &$f) {
      if (is_array($f)) {
        if (isset($fields[$f[0]]) && $this->testField($f[0], $fields[$f[0]])) {
          // We want to allow processors also to easily remove complete filters.
          // However, we can't use empty() or the like, as that would sort out
          // filters for 0 or NULL. So we specifically check only for the empty
          // string, and we also make sure the filter value was actually changed
          // by storing whether it was empty before.
          $empty_string = $f[1] === '';
          $this->processFilterValue($f[1]);

          if ($f[1] === '' && !$empty_string) {
            unset($filters[$key]);
          }
        }
      }
      else {
        $child_filters = &$f->getFilters();
        $this->processFilters($child_filters);
      }
    }
  }

  /**
   * Determines whether to process data from the given field.
   *
   * @param $name
   *   The field's machine name.
   * @param array $field
   *   The field's information.
   *
   * @return bool
   *   TRUE, if the field should be processed, FALSE otherwise.
   */
  protected function testField($name, array $field) {
    if (empty($this->options['fields'])) {
      return $this->testType($field['type']);
    }
    return !empty($this->options['fields'][$name]);
  }

  /**
   * Determines whether fields of the given type should normally be processed.
   *
   * Defaults to processing text types, but can easily be overridden by
   * subclasses.
   *
   * @return bool
   *   TRUE, if the type should be processed, FALSE otherwise.
   */
  protected function testType($type) {
    return search_api_is_text_type($type, array('text', 'tokens'));
  }

  /**
   * Called for processing a single text element in a field. The default
   * implementation just calls process().
   *
   * $value can either be left a string, or changed into an array of tokens. A
   * token is an associative array containing:
   * - value: Either the text inside the token, or a nested array of tokens. The
   *   score of nested tokens will be multiplied by their parent's score.
   * - score: The relative importance of the token, as a float, with 1 being
   *   the default.
   */
  protected function processFieldValue(&$value) {
    $this->process($value);
  }

  /**
   * Called for processing a single search keyword. The default implementation
   * just calls process().
   *
   * $value can either be left a string, or be changed into a nested keys array,
   * as defined by SearchApiQueryInterface.
   */
  protected function processKey(&$value) {
    $this->process($value);
  }

  /**
   * Called for processing a single filter value. The default implementation
   * just calls process().
   *
   * $value has to remain a string.
   */
  protected function processFilterValue(&$value) {
    $this->process($value);
  }

  /**
   * Function that is ultimately called for all text by the standard
   * implementation, and does nothing by default.
   *
   * @param $value
   *   The value to preprocess as a string. Can be manipulated directly, nothing
   *   has to be returned. Since this can be called for all value types, $value
   *   has to remain a string.
   */
  protected function process(&$value) {

  }

}
