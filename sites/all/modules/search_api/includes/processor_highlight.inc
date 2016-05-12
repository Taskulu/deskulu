<?php

/**
 * @file
 * Contains the SearchApiHighlight class.
 */

/**
 * Processor for highlighting search results.
 */
class SearchApiHighlight extends SearchApiAbstractProcessor {

  /**
   * PREG regular expression for a word boundary.
   *
   * We highlight around non-indexable or CJK characters.
   *
   * @var string
   */
  protected static $boundary;

  /**
   * PREG regular expression for splitting words.
   *
   * @var string
   */
  protected static $split;

  /**
   * {@inheritdoc}
   */
  public function __construct(SearchApiIndex $index, array $options = array()) {
    parent::__construct($index, $options);

    $cjk = '\x{1100}-\x{11FF}\x{3040}-\x{309F}\x{30A1}-\x{318E}' .
        '\x{31A0}-\x{31B7}\x{31F0}-\x{31FF}\x{3400}-\x{4DBF}\x{4E00}-\x{9FCF}' .
        '\x{A000}-\x{A48F}\x{A4D0}-\x{A4FD}\x{A960}-\x{A97F}\x{AC00}-\x{D7FF}' .
        '\x{F900}-\x{FAFF}\x{FF21}-\x{FF3A}\x{FF41}-\x{FF5A}\x{FF66}-\x{FFDC}' .
        '\x{20000}-\x{2FFFD}\x{30000}-\x{3FFFD}';
    self::$boundary = '(?:(?<=[' . PREG_CLASS_UNICODE_WORD_BOUNDARY . $cjk . '])|(?=[' . PREG_CLASS_UNICODE_WORD_BOUNDARY . $cjk . ']))';
    self::$split = '/[' . PREG_CLASS_UNICODE_WORD_BOUNDARY . ']+/iu';
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm() {
    $this->options += array(
      'prefix' => '<strong>',
      'suffix' => '</strong>',
      'excerpt' => TRUE,
      'excerpt_length' => 256,
      'highlight' => 'always',
      'exclude_fields' => array(),
    );

    $form['prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Highlighting prefix'),
      '#description' => t('Text/HTML that will be prepended to all occurrences of search keywords in highlighted text.'),
      '#default_value' => $this->options['prefix'],
    );
    $form['suffix'] = array(
      '#type' => 'textfield',
      '#title' => t('Highlighting suffix'),
      '#description' => t('Text/HTML that will be appended to all occurrences of search keywords in highlighted text.'),
      '#default_value' => $this->options['suffix'],
    );
    $form['excerpt'] = array(
      '#type' => 'checkbox',
      '#title' => t('Create excerpt'),
      '#description' => t('When enabled, an excerpt will be created for searches with keywords, containing all occurrences of keywords in a fulltext field.'),
      '#default_value' => $this->options['excerpt'],
    );
    $form['excerpt_length'] = array(
      '#type' => 'textfield',
      '#title' => t('Excerpt length'),
      '#description' => t('The requested length of the excerpt, in characters.'),
      '#default_value' => $this->options['excerpt_length'],
      '#element_validate' => array('element_validate_integer_positive'),
      '#states' => array(
        'visible' => array(
          '#edit-processors-search-api-highlighting-settings-excerpt' => array(
            'checked' => TRUE,
          ),
        ),
      ),
    );
    // Exclude certain fulltext fields.
    $fields = $this->index->getFields();
    $fulltext_fields = array();
    foreach ($this->index->getFulltextFields() as $field) {
      if (isset($fields[$field])) {
        $fulltext_fields[$field] = check_plain($fields[$field]['name'] . ' (' . $field . ')');
      }
    }
    $form['exclude_fields'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Exclude fields from excerpt'),
      '#description' => t('Exclude certain fulltext fields from being displayed in the excerpt.'),
      '#options' => $fulltext_fields,
      '#default_value' => $this->options['exclude_fields'],
      '#attributes' => array('class' => array('search-api-checkboxes-list')),
    );
    $form['highlight'] = array(
      '#type' => 'select',
      '#title' => t('Highlight returned field data'),
      '#description' => t('Select whether returned fields should be highlighted.'),
      '#options' => array(
        'always' => t('Always'),
        'server' => t('If the server returns fields'),
        'never' => t('Never'),
      ),
      '#default_value' => $this->options['highlight'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    $values['exclude_fields'] = array_filter($values['exclude_fields']);
  }

  /**
   * {@inheritdoc}
   */
  public function postprocessSearchResults(array &$response, SearchApiQuery $query) {
    if (empty($response['results']) || !($keys = $this->getKeywords($query))) {
      return;
    }

    $fulltext_fields = $this->index->getFulltextFields();
    if (!empty($this->options['exclude_fields'])) {
      $fulltext_fields = drupal_map_assoc($fulltext_fields);
      foreach ($this->options['exclude_fields'] as $field) {
        unset($fulltext_fields[$field]);
      }
    }

    foreach ($response['results'] as $id => &$result) {
      if ($this->options['excerpt']) {
        $text = array();
        $fields = $this->getFulltextFields($response['results'], $id, $fulltext_fields);
        foreach ($fields as $data) {
          if (is_array($data)) {
            $text = array_merge($text, $data);
          }
          else {
            $text[] = $data;
          }
        }

        $result['excerpt'] = $this->createExcerpt($this->flattenArrayValues($text), $keys);
      }
      if ($this->options['highlight'] != 'never') {
        $fields = $this->getFulltextFields($response['results'], $id, $fulltext_fields, $this->options['highlight'] == 'always');
        foreach ($fields as $field => $data) {
          $result['fields'][$field] = array('#sanitize_callback' => FALSE);
          if (is_array($data)) {
            foreach ($data as $i => $text) {
              $result['fields'][$field]['#value'][$i] = $this->highlightField($text, $keys);
            }
          }
          else {
            $result['fields'][$field]['#value'] = $this->highlightField($data, $keys);
          }
        }
      }
    }
  }

  /**
   * Retrieves the fulltext data of a result.
   *
   * @param array $results
   *   All results returned in the search, by reference.
   * @param int|string $i
   *   The index in the results array of the result whose data should be
   *   returned.
   * @param array $fulltext_fields
   *   The fulltext fields from which the excerpt should be created.
   * @param bool $load
   *   TRUE if the item should be loaded if necessary, FALSE if only fields
   *   already returned in the results should be used.
   *
   * @return array
   *   An array containing fulltext field names mapped to the text data
   *   contained in them for the given result.
   */
  protected function getFulltextFields(array &$results, $i, array $fulltext_fields, $load = TRUE) {
    global $language;
    $data = array();

    $result = &$results[$i];
    // Act as if $load is TRUE if we have a loaded item.
    $load |= !empty($result['entity']);
    $result += array('fields' => array());
    // We only need detailed fields data if $load is TRUE.
    $fields = $load ? $this->index->getFields() : array();
    $needs_extraction = array();
    $returned_fields = search_api_get_sanitized_field_values(array_intersect_key($result['fields'], array_flip($fulltext_fields)));
    foreach ($fulltext_fields as $field) {
      if (array_key_exists($field, $returned_fields)) {
        $data[$field] = $returned_fields[$field];
      }
      elseif ($load) {
        $needs_extraction[$field] = $fields[$field];
      }
    }

    if (!$needs_extraction) {
      return $data;
    }

    if (empty($result['entity'])) {
      $items = $this->index->loadItems(array_keys($results));
      foreach ($items as $id => $item) {
        $results[$id]['entity'] = $item;
      }
    }
    // If we still don't have a loaded item, we should stop trying.
    if (empty($result['entity'])) {
      return $data;
    }
    $wrapper = $this->index->entityWrapper($result['entity'], FALSE);
    $wrapper->language($language->language);
    $extracted = search_api_extract_fields($wrapper, $needs_extraction, array('sanitize' => TRUE));

    foreach ($extracted as $field => $info) {
      if (isset($info['value'])) {
        $data[$field] = $info['value'];
      }
    }

    return $data;
  }

  /**
   * Extracts the positive keywords used in a search query.
   *
   * @param SearchApiQuery $query
   *   The query from which to extract the keywords.
   *
   * @return array
   *   An array of all unique positive keywords used in the query.
   */
  protected function getKeywords(SearchApiQuery $query) {
    $keys = $query->getKeys();
    if (!$keys) {
      return array();
    }
    if (is_array($keys)) {
      return $this->flattenKeysArray($keys);
    }

    $keywords = preg_split(self::$split, $keys);
    // Assure there are no duplicates. (This is actually faster than
    // array_unique() by a factor of 3 to 4.)
    $keywords = drupal_map_assoc(array_filter($keywords));
    // Remove quotes from keywords.
    foreach ($keywords as $key) {
      $keywords[$key] = trim($key, "'\"");
    }
    return drupal_map_assoc(array_filter($keywords));
  }

  /**
   * Extracts the positive keywords from a keys array.
   *
   * @param array $keys
   *   A search keys array, as specified by SearchApiQueryInterface::getKeys().
   *
   * @return array
   *   An array of all unique positive keywords contained in the keys.
   */
  protected function flattenKeysArray(array $keys) {
    if (!empty($keys['#negation'])) {
      return array();
    }

    $keywords = array();
    foreach ($keys as $i => $key) {
      if (!element_child($i)) {
        continue;
      }
      if (is_array($key)) {
        $keywords += $this->flattenKeysArray($key);
      }
      else {
        $keywords[$key] = $key;
      }
    }

    return $keywords;
  }

  /**
   * Returns snippets from a piece of text, with certain keywords highlighted.
   *
   * Largely copied from search_excerpt().
   *
   * @param string $text
   *   The text to extract fragments from.
   * @param array $keys
   *   Search keywords entered by the user.
   *
   * @return string
   *   A string containing HTML for the excerpt.
   */
  protected function createExcerpt($text, array $keys) {
    // Prepare text by stripping HTML tags and decoding HTML entities.
    $text = strip_tags(str_replace(array('<', '>'), array(' <', '> '), $text));
    $text = ' ' . decode_entities($text);

    // Extract fragments around keywords.
    // First we collect ranges of text around each keyword, starting/ending
    // at spaces, trying to get to the requested length.
    // If the sum of all fragments is too short, we look for second occurrences.
    $ranges = array();
    $included = array();
    $length = 0;
    $workkeys = $keys;
    while ($length < $this->options['excerpt_length'] && count($workkeys)) {
      foreach ($workkeys as $k => $key) {
        if ($length >= $this->options['excerpt_length']) {
          break;
        }
        // Remember occurrence of key so we can skip over it if more occurrences
        // are desired.
        if (!isset($included[$key])) {
          $included[$key] = 0;
        }
        // Locate a keyword (position $p, always >0 because $text starts with a
        // space).
        $p = 0;
        if (preg_match('/' . self::$boundary . preg_quote($key, '/') . self::$boundary . '/iu', $text, $match, PREG_OFFSET_CAPTURE, $included[$key])) {
          $p = $match[0][1];
        }
        // Now locate a space in front (position $q) and behind it (position $s),
        // leaving about 60 characters extra before and after for context.
        // Note that a space was added to the front and end of $text above.
        if ($p) {
          if (($q = strpos(' ' . $text, ' ', max(0, $p - 61))) !== FALSE) {
            $end = substr($text . ' ', $p, 80);
            if (($s = strrpos($end, ' ')) !== FALSE) {
              // Account for the added spaces.
              $q = max($q - 1, 0);
              $s = min($s, strlen($end) - 1);
              $ranges[$q] = $p + $s;
              $length += $p + $s - $q;
              $included[$key] = $p + 1;
            }
            else {
              unset($workkeys[$k]);
            }
          }
          else {
            unset($workkeys[$k]);
          }
        }
        else {
          unset($workkeys[$k]);
        }
      }
    }

    if (count($ranges) == 0) {
      // We didn't find any keyword matches, so just return NULL.
      return NULL;
    }

    // Sort the text ranges by starting position.
    ksort($ranges);

    // Now we collapse overlapping text ranges into one. The sorting makes it O(n).
    $newranges = array();
    foreach ($ranges as $from2 => $to2) {
      if (!isset($from1)) {
        $from1 = $from2;
        $to1 = $to2;
        continue;
      }
      if ($from2 <= $to1) {
        $to1 = max($to1, $to2);
      }
      else {
        $newranges[$from1] = $to1;
        $from1 = $from2;
        $to1 = $to2;
      }
    }
    $newranges[$from1] = $to1;

    // Fetch text
    $out = array();
    foreach ($newranges as $from => $to) {
      $out[] = substr($text, $from, $to - $from);
    }

    // Let translators have the ... separator text as one chunk.
    $dots = explode('!excerpt', t('... !excerpt ... !excerpt ...'));

    $text = (isset($newranges[0]) ? '' : $dots[0]) . implode($dots[1], $out) . $dots[2];
    $text = check_plain($text);

    // Since we stripped the tags at the beginning, highlighting doesn't need to
    // handle HTML anymore.
    return $this->highlightField($text, $keys, FALSE);
  }

  /**
   * Marks occurrences of the search keywords in a text field.
   *
   * @param string $text
   *   The text of the field.
   * @param array $keys
   *   Search keywords entered by the user.
   * @param bool $html
   *   Whether the text can contain HTML tags or not. In the former case, text
   *   inside tags (i.e., tag names and attributes) won't be highlighted.
   *
   * @return string
   *   The field's text with all occurrences of search keywords highlighted.
   */
  protected function highlightField($text, array $keys, $html = TRUE) {
    if (is_array($text)) {
      $text = $this->flattenArrayValues($text);
    }

    if ($html) {
      $texts = preg_split('#((?:</?[[:alpha:]](?:[^>"\']*|"[^"]*"|\'[^\']\')*>)+)#i', $text, -1, PREG_SPLIT_DELIM_CAPTURE);
      for ($i = 0; $i < count($texts); $i += 2) {
        $texts[$i] = $this->highlightField($texts[$i], $keys, FALSE);
      }
      return implode('', $texts);
    }
    $replace = $this->options['prefix'] . '\0' . $this->options['suffix'];

    $keys = implode('|', array_map('preg_quote', $keys, array_fill(0, count($keys), '/')));
    $text = preg_replace('/' . self::$boundary . '(' . $keys . ')' . self::$boundary . '/iu', $replace, ' ' . $text . ' ');
    return substr($text, 1, -1);
  }

  /**
   * Flattens a (possibly multidimensional) array into a string.
   *
   * @param array $array
   *   The array to flatten.
   * @param string $glue
   *   The separator to insert between individual array items.
   *
   * @return string
   *   The glued string.
   */
  protected function flattenArrayValues(array $array, $glue = "\n\n") {
    $ret = array();
    foreach ($array as $item) {
      if (is_array($item)) {
        $ret[] = $this->flattenArrayValues($item, $glue);
      }
      else {
        $ret[] = $item;
      }
    }

    return implode($glue, $ret);
  }

}
