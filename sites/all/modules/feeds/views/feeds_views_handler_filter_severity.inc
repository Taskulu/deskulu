<?php

/**
 * @file
 * Filter by severity.
 */

class feeds_views_handler_filter_severity extends views_handler_filter_in_operator {
  function get_value_options() {
    $this->value_title = t('Severity');
    $this->value_options = watchdog_severity_levels();
  }
}
