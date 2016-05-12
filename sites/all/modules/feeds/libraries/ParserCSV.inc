<?php

/**
 * @file
 * Contains CSV Parser.
 *
 * Functions in this file are independent of the Feeds specific implementation.
 * Thanks to jpetso http://drupal.org/user/56020 for most of the code in this
 * file.
 */

/**
 * Text lines from file iterator.
 */
class ParserCSVIterator implements Iterator {
  private $handle;
  private $currentLine;
  private $currentPos;

  public function __construct($filepath) {
    $this->handle = fopen($filepath, 'r');
    $this->currentLine = NULL;
    $this->currentPos = NULL;
  }

  function __destruct() {
    if ($this->handle) {
      fclose($this->handle);
    }
  }

  public function rewind($pos = 0) {
    if ($this->handle) {
      fseek($this->handle, $pos);
      $this->next();
    }
  }

  public function next() {
    if ($this->handle) {
      $this->currentLine = feof($this->handle) ? NULL : fgets($this->handle);
      $this->currentPos = ftell($this->handle);
      return $this->currentLine;
    }
  }

  public function valid() {
    return isset($this->currentLine);
  }

  public function current() {
    return $this->currentLine;
  }

  public function currentPos() {
    return $this->currentPos;
  }

  public function key() {
    return 'line';
  }
}

/**
 * Functionality to parse CSV files into a two dimensional array.
 */
class ParserCSV {
  private $delimiter;
  private $fromEncoding;
  private $toEncoding;
  private $skipFirstLine;
  private $columnNames;
  private $timeout;
  private $timeoutReached;
  private $startByte;
  private $lineLimit;
  private $lastLinePos;
  private $useMbString;

  public function __construct() {
    $this->delimiter = ',';
    $this->fromEncoding = 'UTF-8';
    $this->toEncoding = 'UTF-8';
    $this->skipFirstLine = FALSE;
    $this->columnNames = FALSE;
    $this->timeout = FALSE;
    $this->timeoutReached = FALSE;
    $this->startByte = 0;
    $this->lineLimit = 0;
    $this->lastLinePos = 0;
    ini_set('auto_detect_line_endings', TRUE);
    if (extension_loaded('mbstring') && variable_get('feeds_use_mbstring', TRUE)) {
      $this->useMbString = TRUE;
    }
  }

  /**
   * Set the column delimiter string.
   * By default, the comma (',') is used as delimiter.
   */
  public function setDelimiter($delimiter) {
    $this->delimiter = $delimiter;
  }

  /**
   * Sets the source file encoding.
   *
   * By default, the encoding is UTF-8.
   *
   * @param string $encoding
   *   The encoding to set.
   */
  public function setEncoding($encoding) {
    $this->fromEncoding = $encoding;
  }

  /**
   * Set this to TRUE if the parser should skip the first line of the CSV text,
   * which might be desired if the first line contains the column names.
   * By default, this is set to FALSE and the first line is not skipped.
   */
  public function setSkipFirstLine($skipFirstLine) {
    $this->skipFirstLine = $skipFirstLine;
  }

  /**
   * Specify an array of column names if you know them in advance, or FALSE
   * (which is the default) to unset any prior column names. If no column names
   * are set, the parser will put each row into a simple numerically indexed
   * array. If column names are given, the parser will create arrays with
   * these column names as array keys instead.
   */
  public function setColumnNames($columnNames) {
    $this->columnNames = $columnNames;
  }

  /**
   * Define the time (in milliseconds) after which the parser stops parsing,
   * even if it has not yet finished processing the CSV data. If the timeout
   * has been reached before parsing is done, the parse() method will return
   * an incomplete list of rows - a single row will never be cut off in the
   * middle, though. By default, no timeout (@p $timeout == FALSE) is defined.
   *
   * You can check if the timeout has been reached by calling the
   * timeoutReached() method after parse() has been called.
   */
  public function setTimeout($timeout) {
    $this->timeout = $timeout;
  }

  /**
   * After calling the parse() method, determine if the timeout (set by the
   * setTimeout() method) has been reached.
   *
   * @deprecated Use lastLinePos() instead to determine whether a file has
   *   finished parsing.
   */
  public function timeoutReached() {
    return $this->timeoutReached;
  }

  /**
   * Define the number of lines to parse in one parsing operation.
   *
   * By default, all lines of a file are being parsed.
   */
  public function setLineLimit($lines) {
    $this->lineLimit = $lines;
  }

  /**
   * Get the byte number where the parser left off after last parse() call.
   *
   * @return
   *  0 if all lines or no line has been parsed, the byte position of where a
   *  timeout or the line limit has been reached otherwise. This position can be
   *  used to set the start byte for the next iteration after parse() has
   *  reached the timeout set with setTimeout() or the line limit set with
   *  setLineLimit().
   *
   * @see ParserCSV::setStartByte()
   */
  public function lastLinePos() {
    return $this->lastLinePos;
  }

  /**
   * Set the byte where file should be started to read.
   *
   * Useful when parsing a file in batches.
   */
  public function setStartByte($start) {
    return $this->startByte = $start;
  }

  /**
   * Parse CSV files into a two dimensional array.
   *
   * @param Iterator $lineIterator
   *   An Iterator object that yields line strings, e.g. ParserCSVIterator.
   * @param $start
   *   The byte number from where to start parsing the file.
   * @param $lines
   *   The number of lines to parse, 0 for all lines.
   * @return
   *   Two dimensional array that contains the data in the CSV file.
   */
  public function parse(Iterator $lineIterator) {
    $skipLine = $this->skipFirstLine;
    $rows = array();

    $this->timeoutReached = FALSE;
    $this->lastLinePos = 0;
    $maxTime = empty($this->timeout) ? FALSE : (microtime() + $this->timeout);
    $linesParsed = 0;

    for ($lineIterator->rewind($this->startByte); $lineIterator->valid(); $lineIterator->next()) {

      // Make really sure we've got lines without trailing newlines.
      $line = trim($this->fixEncoding($lineIterator->current()), "\r\n");

      // Skip empty lines.
      if (empty($line)) {
        continue;
      }
      // If the first line contains column names, skip it.
      if ($skipLine) {
        $skipLine = FALSE;
        continue;
      }

      // The actual parser. explode() is unfortunately not suitable because the
      // delimiter might be located inside a quoted field, and that would break
      // the field and/or require additional effort to re-join the fields.
      $quoted = FALSE;
      $currentIndex = 0;
      $currentField = '';
      $fields = array();

      // We must use strlen() as we're parsing byte by byte using strpos(), so
      // drupal_strlen() will not work properly.
      while ($currentIndex <= strlen($line)) {
        if ($quoted) {
          $nextQuoteIndex = strpos($line, '"', $currentIndex);

          if ($nextQuoteIndex === FALSE) {
            // There's a line break before the quote is closed, so fetch the
            // next line and start from there.
            $currentField .= substr($line, $currentIndex);
            $lineIterator->next();

            if (!$lineIterator->valid()) {
              // Whoa, an unclosed quote! Well whatever, let's just ignore
              // that shortcoming and record it nevertheless.
              $fields[] = $currentField;
              break;
            }
            // Ok, so, on with fetching the next line, as mentioned above.
            $currentField .= "\n";
            $line = trim($this->fixEncoding($lineIterator->current()), "\r\n");
            $currentIndex = 0;
            continue;
          }

          // There's actually another quote in this line...
          // find out whether it's escaped or not.
          $currentField .= substr($line, $currentIndex, $nextQuoteIndex - $currentIndex);

          if (isset($line[$nextQuoteIndex + 1]) && $line[$nextQuoteIndex + 1] === '"') {
            // Escaped quote, add a single one to the field and proceed quoted.
            $currentField .= '"';
            $currentIndex = $nextQuoteIndex + 2;
          }
          else {
            // End of the quoted section, close the quote and let the
            // $quoted == FALSE block finalize the field.
            $quoted = FALSE;
            $currentIndex = $nextQuoteIndex + 1;
          }
        }
        else { // $quoted == FALSE
          // First, let's find out where the next character of interest is.
          $nextQuoteIndex = strpos($line, '"', $currentIndex);
          $nextDelimiterIndex = strpos($line, $this->delimiter, $currentIndex);

          if ($nextQuoteIndex === FALSE) {
            $nextIndex = $nextDelimiterIndex;
          }
          elseif ($nextDelimiterIndex === FALSE) {
            $nextIndex = $nextQuoteIndex;
          }
          else {
            $nextIndex = min($nextQuoteIndex, $nextDelimiterIndex);
          }

          if ($nextIndex === FALSE) {
            // This line is done, add the rest of it as last field.
            $currentField .= substr($line, $currentIndex);
            $fields[] = $currentField;
            break;
          }
          elseif ($line[$nextIndex] === $this->delimiter[0]) {
            $length = ($nextIndex + strlen($this->delimiter) - 1) - $currentIndex;
            $currentField .= substr($line, $currentIndex, $length);
            $fields[] = $currentField;
            $currentField = '';
            $currentIndex += $length + 1;
            // Continue with the next field.
          }
          else { // $line[$nextIndex] == '"'
            $quoted = TRUE;
            $currentField .= substr($line, $currentIndex, $nextIndex - $currentIndex);
            $currentIndex = $nextIndex + 1;
            // Continue this field in the $quoted == TRUE block.
          }
        }
      }
      // End of CSV parser. We've now got all the fields of the line as strings
      // in the $fields array.

      if (empty($this->columnNames)) {
        $row = $fields;
      }
      else {
        $row = array();
        foreach ($this->columnNames as $columnName) {
          $field = array_shift($fields);
          $row[$columnName] = isset($field) ? $field : '';
        }
      }
      $rows[] = $row;

      // Quit parsing if timeout has been reached or requested lines have been
      // reached.
      if (!empty($maxTime) && microtime() > $maxTime) {
        $this->timeoutReached = TRUE;
        $this->lastLinePos = $lineIterator->currentPos();
        break;
      }
      $linesParsed++;
      if ($this->lineLimit && $linesParsed >= $this->lineLimit) {
        $this->lastLinePos = $lineIterator->currentPos();
        break;
      }
    }
    return $rows;
  }

  /**
   * Converts encoding of input data.
   *
   * @param string $data
   *   A chunk of data.
   *
   * @return string
   *   The encoded data.
   *
   * @throws ParserCSVEncodingException
   *   Thrown when a given encoding does not match.
   */
  public function fixEncoding($data) {
    if ($this->useMbString) {
      if (mb_check_encoding($data, $this->fromEncoding)) {
        if ($this->toEncoding != $this->fromEncoding) {
          // Convert encoding. The conversion is to UTF-8 by default to prevent
          // SQL errors.
          $data = mb_convert_encoding($data, $this->toEncoding, $this->fromEncoding);
        }
      }
      else {
        throw new ParserCSVEncodingException(t('Source file is not in %encoding encoding.', array('%encoding' => $this->fromEncoding)));
      }
    }

    return $data;
  }
}

/**
 * Exception thrown when an encoding error occurs during parsing.
 */
class ParserCSVEncodingException extends Exception {}
