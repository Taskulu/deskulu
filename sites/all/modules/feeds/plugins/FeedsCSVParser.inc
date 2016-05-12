<?php

/**
 * @file
 * Contains the FeedsCSVParser class.
 */

/**
 * Parses a given file as a CSV file.
 */
class FeedsCSVParser extends FeedsParser {

  /**
   * Implements FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    $source_config = $source->getConfigFor($this);
    $state = $source->state(FEEDS_PARSE);

    // Load and configure parser.
    feeds_include_library('ParserCSV.inc', 'ParserCSV');
    $parser = new ParserCSV();
    $delimiter = $this->getDelimiterChar($source_config);
    $parser->setDelimiter($delimiter);
    if (isset($source_config['encoding'])) {
      // Encoding can only be set when the mbstring extension is loaded.
      $parser->setEncoding($source_config['encoding']);
    }

    $iterator = new ParserCSVIterator($fetcher_result->getFilePath());
    if (empty($source_config['no_headers'])) {
      // Get first line and use it for column names, convert them to lower case.
      $header = $this->parseHeader($parser, $iterator);
      if (!$header) {
        drupal_set_message(t('The CSV file is empty.'), 'warning', FALSE);
        return new FeedsParserResult();
      }
      $parser->setColumnNames($header);
    }

    // Determine section to parse, parse.
    $start = $state->pointer ? $state->pointer : $parser->lastLinePos();
    $limit = $source->importer->getLimit();
    $rows = $this->parseItems($parser, $iterator, $start, $limit);

    // Report progress.
    $state->total = filesize($fetcher_result->getFilePath());
    $state->pointer = $parser->lastLinePos();
    $progress = $parser->lastLinePos() ? $parser->lastLinePos() : $state->total;
    $state->progress($state->total, $progress);

    // Create a result object and return it.
    return new FeedsParserResult($rows, $source->feed_nid);
  }

  /**
   * Get first line and use it for column names, convert them to lower case.
   * Be aware that the $parser and iterator objects can be modified in this
   * function since they are passed in by reference
   *
   * @param ParserCSV $parser
   * @param ParserCSVIterator $iterator
   * @return
   *   An array of lower-cased column names to use as keys for the parsed items.
   */
  protected function parseHeader(ParserCSV $parser, ParserCSVIterator $iterator) {
    $parser->setLineLimit(1);
    $rows = $parser->parse($iterator);
    if (!count($rows)) {
      return FALSE;
    }
    $header = array_shift($rows);
    foreach ($header as $i => $title) {
      $header[$i] = trim(drupal_strtolower($title));
    }
    return $header;
  }

  /**
   * Parse all of the items from the CSV.
   *
   * @param ParserCSV $parser
   * @param ParserCSVIterator $iterator
   * @return
   *   An array of rows of the CSV keyed by the column names previously set
   */
  protected function parseItems(ParserCSV $parser, ParserCSVIterator $iterator, $start = 0, $limit = 0) {
    $parser->setLineLimit($limit);
    $parser->setStartByte($start);
    $rows = $parser->parse($iterator);
    return $rows;
  }

  /**
   * Override parent::getMappingSources().
   */
  public function getMappingSources() {
    return FALSE;
  }

  /**
   * Override parent::getSourceElement() to use only lower keys.
   */
  public function getSourceElement(FeedsSource $source, FeedsParserResult $result, $element_key) {
    return parent::getSourceElement($source, $result, drupal_strtolower($element_key));
  }

  /**
   * Override parent::getMappingSourceList() to use only lower keys.
   */
  public function getMappingSourceList() {
    return array_map('drupal_strtolower', parent::getMappingSourceList());
  }

  /**
   * Define defaults.
   */
  public function sourceDefaults() {
    return array(
      'delimiter' => $this->config['delimiter'],
      'encoding' => $this->config['encoding'],
      'no_headers' => $this->config['no_headers'],
    );
  }

  /**
   * Source form.
   *
   * Show mapping configuration as a guidance for import form users.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['#weight'] = -10;

    $mappings = feeds_importer($this->id)->processor->config['mappings'];
    $sources = $uniques = array();
    foreach ($mappings as $mapping) {
      if (strpos($mapping['source'], ',') !== FALSE) {
        $sources[] = '"' . $mapping['source'] . '"';
      }
      else {
        $sources[] = $mapping['source'];
      }
      if (!empty($mapping['unique'])) {
        $uniques[] = $mapping['source'];
      }
    }
    $sources = array_unique($sources);

    $output = t('Import !csv_files with one or more of these columns: @columns.', array('!csv_files' => l(t('CSV files'), 'http://en.wikipedia.org/wiki/Comma-separated_values'), '@columns' => implode(', ', $sources)));
    $items = array();
    $items[] = format_plural(count($uniques), 'Column <strong>@columns</strong> is mandatory and considered unique: only one item per @columns value will be created.', 'Columns <strong>@columns</strong> are mandatory and values in these columns are considered unique: only one entry per value in one of these column will be created.', array('@columns' => implode(', ', $uniques)));
    $items[] = l(t('Download a template'), 'import/' . $this->id . '/template');
    $form['help'] = array(
      '#prefix' => '<div class="help">',
      '#suffix' => '</div>',
      'description' => array(
        '#prefix' => '<p>',
        '#markup' => $output,
        '#suffix' => '</p>',
      ),
      'list' => array(
        '#theme' => 'item_list',
        '#items' => $items,
      ),
    );
    $form['delimiter'] = array(
      '#type' => 'select',
      '#title' => t('Delimiter'),
      '#description' => t('The character that delimits fields in the CSV file.'),
      '#options' => $this->getAllDelimiterTypes(),
      '#default_value' => isset($source_config['delimiter']) ? $source_config['delimiter'] : ',',
    );
    $form['no_headers'] = array(
      '#type' => 'checkbox',
      '#title' => t('No Headers'),
      '#description' => t('Check if the imported CSV file does not start with a header row. If checked, mapping sources must be named \'0\', \'1\', \'2\' etc.'),
      '#default_value' => isset($source_config['no_headers']) ? $source_config['no_headers'] : 0,
    );
    $form['encoding'] = $this->configEncodingForm();
    if (isset($source_config['encoding'])) {
      $form['encoding']['#default_value'] = $source_config['encoding'];
    }
    return $form;
  }

  /**
   * Define default configuration.
   */
  public function configDefaults() {
    return array(
      'delimiter' => ',',
      'encoding' => 'UTF-8',
      'no_headers' => 0,
    );
  }

  /**
   * Build configuration form.
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['delimiter'] = array(
      '#type' => 'select',
      '#title' => t('Default delimiter'),
      '#description' => t('Default field delimiter.'),
      '#options' => $this->getAllDelimiterTypes(),
      '#default_value' => $this->config['delimiter'],
    );
    $form['no_headers'] = array(
      '#type' => 'checkbox',
      '#title' => t('No headers'),
      '#description' => t('Check if the imported CSV file does not start with a header row. If checked, mapping sources must be named \'0\', \'1\', \'2\' etc.'),
      '#default_value' => $this->config['no_headers'],
    );
    $form['encoding'] = $this->configEncodingForm();
    return $form;
  }

  /**
   * Builds configuration field for setting file encoding.
   *
   * If the mbstring extension is not available a markup render array
   * will be returned instead.
   *
   * @return array
   *   A renderable array.
   */
  public function configEncodingForm() {
    if (extension_loaded('mbstring') && variable_get('feeds_use_mbstring', TRUE)) {
      // Get the system's list of available encodings.
      $options = mb_list_encodings();
      // Make the key/values the same in the array.
      $options = array_combine($options, $options);
      // Sort alphabetically not-case sensitive.
      natcasesort($options);
      return array(
        '#type' => 'select',
        '#title' => t('File encoding'),
        '#description' => t('Performs character encoding conversion from selected option to UTF-8.'),
        '#options' => $options,
        '#default_value' => $this->config['encoding'],
      );
    }
    else {
      return array(
        '#markup' => '<em>' . t('PHP mbstring extension must be available for character encoding conversion.') . '</em>',
      );
    }
  }

  public function getTemplate() {
    $mappings = feeds_importer($this->id)->processor->config['mappings'];
    $sources = $uniques = array();

    foreach ($mappings as $mapping) {
      if (in_array($mapping['source'], $uniques) || in_array($mapping['source'], $sources)) {
        // Skip columns we've already seen.
        continue;
      }

      if (!empty($mapping['unique'])) {
        $uniques[] = $mapping['source'];
      }
      else {
        $sources[] = $mapping['source'];
      }
    }

    $sep = $this->getDelimiterChar($this->config);
    $columns = array();

    foreach (array_merge($uniques, $sources) as $col) {
      if (strpos($col, $sep) !== FALSE) {
        $col = '"' . str_replace('"', '""', $col) . '"';
      }

      $columns[] = $col;
    }

    $template_file_details = $this->getTemplateFileDetails($this->config);

    $filename = "{$this->id}_template.{$template_file_details['extension']}";
    $cache_control = 'max-age=60, must-revalidate';
    $content_disposition = 'attachment; filename="' . $filename . '"';
    $content_type = "{$template_file_details['mime_type']}; charset=utf-8";

    drupal_add_http_header('Cache-Control', $cache_control);
    drupal_add_http_header('Content-Disposition', $content_disposition);
    drupal_add_http_header('Content-type', $content_type);

    print implode($sep, $columns);
  }

  /**
   * Gets an associative array of the delimiters supported by this parser.
   *
   * The keys represent the value that is persisted into the database, and the
   * value represents the text that is shown in the admins UI.
   *
   * @return array
   *   The associative array of delimiter types to display name.
   */
  protected function getAllDelimiterTypes() {
    $delimiters = array(
      ',',
      ';',
      'TAB',
      '|',
      '+',
    );

    return array_combine($delimiters, $delimiters);
  }

  /**
   * Gets the appropriate delimiter character for the delimiter in the config.
   *
   * @param array $config
   *   The configuration for the parser.
   *
   * @return string
   *   The delimiter character.
   */
  protected function getDelimiterChar(array $config) {
    $config_delimiter = $config['delimiter'];

    switch ($config_delimiter) {
      case 'TAB':
        $delimiter = "\t";
        break;

      default:
        $delimiter = $config_delimiter;
        break;
    }

    return $delimiter;
  }

  /**
   * Gets details about the template file, for the delimiter in the config.
   *
   * The resulting details indicate the file extension and mime type for the
   * delimiter type.
   *
   * @param array $config
   *   The configuration for the parser.
   *
   * @return array
   *   An array with the following information:
   *     - 'extension': The file extension for the template ('tsv', 'csv', etc).
   *     - 'mime-type': The mime type for the template
   *       ('text/tab-separated-values', 'text/csv', etc).
   */
  protected function getTemplateFileDetails(array $config) {
    switch ($config['delimiter']) {
      case 'TAB':
        $extension = 'tsv';
        $mime_type = 'text/tab-separated-values';
        break;

      default:
        $extension = 'csv';
        $mime_type = 'text/csv';
        break;
    }

    return array(
      'extension' => $extension,
      'mime_type' => $mime_type,
    );
  }
}
