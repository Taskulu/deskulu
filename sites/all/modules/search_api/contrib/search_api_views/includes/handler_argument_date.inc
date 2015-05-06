<?php

/**
 * @file
 * Contains the SearchApiViewsHandlerArgumentDate class.
 */

/**
 * Defines a contextual filter searching for a date or date range.
 */
class SearchApiViewsHandlerArgumentDate extends SearchApiViewsHandlerArgument {

  /**
   * {@inheritdoc}
   */
  public function query($group_by = FALSE) {
    if (empty($this->value)) {
      $this->fillValue();
      if ($this->value === FALSE) {
        $this->abort();
        return;
      }
    }

    $outer_conjunction = strtoupper($this->operator);

    if (empty($this->options['not'])) {
      $operator = '=';
      $inner_conjunction = 'OR';
    }
    else {
      $operator = '<>';
      $inner_conjunction = 'AND';
    }

    if (!empty($this->value)) {
      if (!empty($this->value)) {
        $outer_filter = $this->query->createFilter($outer_conjunction);
        foreach ($this->value as $value) {
          $value_filter = $this->query->createFilter($inner_conjunction);
          $values = explode(';', $value);
          $values = array_map(array($this, 'getTimestamp'), $values);
          if (in_array(FALSE, $values, TRUE)) {
            $this->abort();
            return;
          }
          $is_range = (count($values) > 1);

          $inner_filter = ($is_range ? $this->query->createFilter('AND') : $value_filter);
          $range_op = (empty($this->options['not']) ? '>=' : '<');
          $inner_filter->condition($this->real_field, $values[0], $is_range ? $range_op : $operator);
          if ($is_range) {
            $range_op = (empty($this->options['not']) ? '<=' : '>');
            $inner_filter->condition($this->real_field, $values[1], $range_op);
            $value_filter->filter($inner_filter);
          }
          $outer_filter->filter($value_filter);
        }

        $this->query->filter($outer_filter);
      }
    }
  }

  /**
   * Converts a value to a timestamp, if it isn't one already.
   *
   * @param string|int $value
   *   The value to convert. Either a timestamp, or a date/time string as
   *   recognized by strtotime().
   *
   * @return int|false
   *   The parsed timestamp, or FALSE if an illegal string was passed.
   */
  public function getTimestamp($value) {
    if (is_numeric($value)) {
      return $value;
    }

    return strtotime($value);
  }

  /**
   * Fills $this->value with data from the argument.
   */
  protected function fillValue() {
    if (!empty($this->options['break_phrase'])) {
      // Set up defaults:
      if (!isset($this->value)) {
        $this->value = array();
      }

      if (!isset($this->operator)) {
        $this->operator = 'OR';
      }

      if (empty($this->argument)) {
        return;
      }

      if (preg_match('/^([-\d;:\s]+\+)*[-\d;:\s]+$/', $this->argument)) {
        // The '+' character in a query string may be parsed as ' '.
        $this->value = explode('+', $this->argument);
      }
      elseif (preg_match('/^([-\d;:\s]+,)*[-\d;:\s]+$/', $this->argument)) {
        $this->operator = 'AND';
        $this->value = explode(',', $this->argument);
      }

      // Keep an 'error' value if invalid strings were given.
      if (!empty($this->argument) && (empty($this->value) || !is_array($this->value))) {
        $this->value = FALSE;
      }
    }
    else {
      $this->value = array($this->argument);
    }
  }

  /**
   * Aborts the associated query due to an illegal argument.
   */
  protected function abort() {
    $variables['!field'] = $this->definition['group'] . ': ' . $this->definition['title'];
    $this->query->abort(t('Illegal argument passed to !field contextual filter.', $variables));
  }

  /**
   * Computes the title this argument will assign the view, given the argument.
   *
   * @return string
   *   A title fitting for the passed argument.
   */
  public function title() {
    if (!empty($this->argument)) {
      if (empty($this->value)) {
        $this->fillValue();
      }
      $dates = array();
      foreach ($this->value as $date) {
        $date_parts = explode(';', $date);

        $ts = $this->getTimestamp($date_parts[0]);
        $datestr = format_date($ts, 'short');
        if (count($date_parts) > 1) {
          $ts = $this->getTimestamp($date_parts[1]);
          $datestr .= ' - ' . format_date($ts, 'short');
        }

        if ($datestr) {
          $dates[] = $datestr;
        }
      }

      return $dates ? implode(', ', $dates) : check_plain($this->argument);
    }

    return check_plain($this->argument);
  }

}
