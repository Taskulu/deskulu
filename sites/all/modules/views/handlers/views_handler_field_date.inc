<?php

/**
 * @file
 * Definition of views_handler_field_date.
 */

/**
 * A handler to provide proper displays for dates.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_date extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();

    $options['date_format'] = array('default' => 'small');
    $options['custom_date_format'] = array('default' => '');
    $options['second_date_format_custom'] = array('default' => '');
    $options['second_date_format'] = array('default' => 'small');
    $options['timezone'] = array('default' => '');

    return $options;
  }

  function options_form(&$form, &$form_state) {

    $date_formats = array();
    $date_types = system_get_date_types();
    foreach ($date_types as $key => $value) {
      $date_formats[$value['type']] = t('@date_format format', array('@date_format' => $value['title'])) . ': ' . format_date(REQUEST_TIME, $value['type']);
    }

    $form['date_format'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => $date_formats + array(
        'custom' => t('Custom'),
        'raw time ago' => t('Time ago'),
        'time ago' => t('Time ago (with "ago" appended)'),
        'today time ago' => t('Time ago (with "ago" appended) for today\'s date, but not for other dates'),
        'raw time hence' => t('Time hence'),
        'time hence' => t('Time hence (with "hence" appended)'),
        'raw time span' => t('Time span (future dates have "-" prepended)'),
        'inverse time span' => t('Time span (past dates have "-" prepended)'),
        'time span' => t('Time span (with "ago/hence" appended)'),
      ),
      '#default_value' => isset($this->options['date_format']) ? $this->options['date_format'] : 'small',
    );
    $form['custom_date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom date format'),
      '#description' => t('If "Custom", see the <a href="@url" target="_blank">PHP manual</a> for date formats. Otherwise, enter the number of different time units to display, which defaults to 2.', array('@url' => 'http://php.net/manual/function.date.php')),
      '#default_value' => isset($this->options['custom_date_format']) ? $this->options['custom_date_format'] : '',
      '#dependency' => array('edit-options-date-format' => array('custom', 'raw time ago', 'time ago', 'today time ago', 'raw time hence', 'time hence', 'raw time span', 'time span', 'raw time span', 'inverse time span', 'time span')),
    );
    $form['second_date_format'] = array(
     '#type' => 'select',
     '#title' => t('Second date format'),
     '#options' => $date_formats + array(
       'custom' => t('Custom'),
     ),
     '#description' => t('The date format which will be used for rendering dates other than today.'),
     '#default_value' => isset($this->options['second_date_format']) ? $this->options['second_date_format'] : 'small',
     '#dependency' => array('edit-options-date-format' => array('today time ago')),
   );
   $form['second_date_format_custom'] = array(
     '#type' => 'textfield',
     '#title' => t('Custom date format of second date'),
     '#description' => t('If "Custom" is selected in "Second date format", see the <a href="@url" target="_blank">PHP manual</a> for date formats. Otherwise, enter the number of different time units to display, which defaults to 2.', array('@url' => 'http://php.net/manual/function.date.php')),
     '#default_value' => isset($this->options['second_date_format_custom']) ? $this->options['second_date_format_custom'] : '',
     // We have to use states instead of ctools dependency because dependency
     // doesn't handle multiple conditions.
     '#states' => array(
       'visible' => array(
         '#edit-options-date-format' => array('value' => 'today time ago'),
         '#edit-options-second-date-format' => array('value' => 'custom'),
       ),
     ),
     // We have to use ctools dependency too because states doesn't add the
     // correct left margin to the element's wrapper.
     '#dependency' => array(
       // This condition is handled by form API's states.
//        'edit-options-date-format' => array('today time ago'),
       'edit-options-second-date-format' => array('custom'),
     ),
   );
    $form['timezone'] = array(
      '#type' => 'select',
      '#title' => t('Timezone'),
      '#description' => t('Timezone to be used for date output.'),
      '#options' => array('' => t('- Default site/user timezone -')) + system_time_zones(FALSE),
      '#default_value' => $this->options['timezone'],
      '#dependency' => array('edit-options-date-format' => array_merge(array('custom'), array_keys($date_formats))),
    );

    parent::options_form($form, $form_state);
  }

  function render($values) {
    $value = $this->get_value($values);
    $format = $this->options['date_format'];
    if (in_array($format, array('custom', 'raw time ago', 'time ago', 'today time ago', 'raw time hence', 'time hence', 'raw time span', 'time span', 'raw time span', 'inverse time span', 'time span'))) {
      $custom_format = $this->options['custom_date_format'];
    }

    if ($value) {
      $timezone = !empty($this->options['timezone']) ? $this->options['timezone'] : NULL;
      $time_diff = REQUEST_TIME - $value; // will be positive for a datetime in the past (ago), and negative for a datetime in the future (hence)
      switch ($format) {
        case 'raw time ago':
          return format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2);
        case 'time ago':
          return t('%time ago', array('%time' => format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2)));
        case 'today time ago':
          $second_format = $this->options['second_date_format'];
          $second_custom_format = $this->options['second_date_format_custom'];
          if (format_date(REQUEST_TIME, 'custom', 'Y-m-d', $timezone) == format_date($value, 'custom', 'Y-m-d', $timezone)) {
            return t('%time ago', array('%time' => format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2)));
          }
          elseif ($second_format == 'custom') {
            if ($second_custom_format == 'r') {
              return format_date($value, $second_format, $second_custom_format, $timezone, 'en');
            }
            return format_date($value, $second_format, $second_custom_format, $timezone);
          }
          else {
            return format_date($value, $this->options['second_date_format'], '', $timezone);
          }
        case 'raw time hence':
          return format_interval(-$time_diff, is_numeric($custom_format) ? $custom_format : 2);
        case 'time hence':
          return t('%time hence', array('%time' => format_interval(-$time_diff, is_numeric($custom_format) ? $custom_format : 2)));
        case 'raw time span':
          return ($time_diff < 0 ? '-' : '') . format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2);
        case 'inverse time span':
          return ($time_diff > 0 ? '-' : '') . format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2);
        case 'time span':
          return t(($time_diff < 0 ? '%time hence' : '%time ago'), array('%time' => format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2)));
        case 'custom':
          if ($custom_format == 'r') {
            return format_date($value, $format, $custom_format, $timezone, 'en');
          }
          return format_date($value, $format, $custom_format, $timezone);
        default:
          return format_date($value, $format, '', $timezone);
      }
    }
  }
}
