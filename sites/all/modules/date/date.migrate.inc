<?php

/**
 * @file
 * Support for migration into Date fields.
 */

/**
 * Implements hook_migrate_api().
 */
function date_migrate_api() {
  $api = array(
    'api' => 2,
    'field handlers' => array('DateMigrateFieldHandler'),
  );
  return $api;
}

class DateMigrateFieldHandler extends MigrateFieldHandler {

  /**
   * Declares the types of fields used.
   */
  public function __construct() {
    $this->registerTypes(array('date', 'datestamp', 'datetime'));
  }

  /**
   * Arguments for a date field migration.
   *
   * @param string $timezone
   *   Timezone (such as UTC, America/New_York, etc.) to apply.
   * @param string $timezone_db
   *   Timezone_db value for the field.
   * @param string $rrule
   *   Rule string for a repeating date field.
   * @param string $language
   *   Language of the text (defaults to destination language)
   *
   * @return array
   *   An array of the defined variables in this scope.
   */
  public static function arguments($timezone = 'UTC', $timezone_db = 'UTC', $rrule = NULL, $language = NULL) {
    return get_defined_vars();
  }

  /**
   * Converts incoming data into the proper field arrays for Date fields.
   *
   * @param object $entity
   *   The destination entity which will hold the field arrays.
   * @param array $field_info
   *   Metadata for the date field being populated.
   * @param array $instance
   *   Metadata for this instance of the date field being populated.
   * @param array $values
   *   Array of date values to be fielded.
   *
   * @return array|null
   *   An array of date fields.
   */
  public function prepare($entity, array $field_info, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    $language = $this->getFieldLanguage($entity, $field_info, $arguments);

    foreach ($values as $delta => $from) {
      if (!empty($arguments['timezone'])) {
        if (is_array($arguments['timezone'])) {
          $timezone = $arguments['timezone'][$delta];
        }
        else {
          $timezone = $arguments['timezone'];
        }
      }
      else {
        $timezone = 'UTC';
      }

      if (!empty($arguments['rrule'])) {
        if (is_array($arguments['rrule'])) {
          $rrule = $arguments['rrule'][$delta];
        }
        else {
          $rrule = $arguments['rrule'];
        }
      }
      else {
        $rrule = NULL;
      }

      if (!empty($arguments['to'])) {
        if (is_array($arguments['to'])) {
          $to = $arguments['to'][$delta];
        }
        else {
          $to = $arguments['to'];
        }
      }
      else {
        $to = NULL;
      }

      // Legacy support for JSON containing a set of properties - deprecated
      // now that we have subfields.
      if (!empty($from) && $from{0} == '{') {
        $properties = drupal_json_decode($from);
        $from = $properties['from'];
        // Properties passed in with the date override any set via arguments.
        if (!empty($properties['to'])) {
          $to = $properties['to'];
        }
        if (!empty($properties['timezone'])) {
          $timezone = $properties['timezone'];
        }
        if (!empty($properties['rrule'])) {
          $rrule = $properties['rrule'];
        }
      }

      // Missing data? Create an empty value and return;
      // Don't try to turn the empty value into a bogus
      // timestamp for 'now'.
      if (empty($from)) {
        $return[$language][$delta]['value'] = NULL;
        $return[$language][$delta]['timezone'] = NULL;
        if (!empty($field_info['settings']['todate'])) {
          $return[$language][$delta]['value2'] = NULL;
        }
        return $return;
      }

      // If there is no 'to' date, just use the 'from' date.
      if (!empty($field_info['settings']['todate']) && empty($to)) {
        $to = $from;
      }

      // If we have a value, work from a timestamp.
      $from = MigrationBase::timestamp($from);
      if ($to) {
        $to = MigrationBase::timestamp($to);
      }

      // What does the destination field expect?
      switch ($field_info['type']) {
        case 'datestamp':
          // Already done.
          break;

        case 'datetime':
          // YYYY-MM-DD HH:MM:SS.
          $from = format_date($from, 'custom', 'Y-m-d H:i:s', $timezone);
          if ($to) {
            $to = format_date($to, 'custom', 'Y-m-d H:i:s', $timezone);
          }
          break;

        case 'date':
          // ISO date: YYYY-MM-DDTHH:MM:SS.
          $from = format_date($from, 'custom', 'Y-m-d\TH:i:s', $timezone);
          if ($to) {
            $to = format_date($to, 'custom', 'Y-m-d\TH:i:s', $timezone);
          }
          break;

        default:
          break;
      }

      // Handle repeats, coming in as RRULEs. Many field instances may be
      // created.
      if (function_exists('date_repeat_build_dates') && !empty($field_info['settings']['repeat']) && $rrule) {
        include_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'date_api') . '/date_api_ical.inc';
        $item = array(
          'value' => $from,
          'value2' => $to,
          'timezone' => $timezone,
        );
        // Can be de-uglified when http://drupal.org/node/1159404 is committed.
        $return[$language] = date_repeat_build_dates(NULL, date_ical_parse_rrule($field_info, $rrule), $field_info, $item);
      }
      else {
        $return[$language][$delta]['value'] = $from;
        $return[$language][$delta]['timezone'] = $timezone;
        if (!empty($to)) {
          $return[$language][$delta]['value2'] = $to;
        }
      }
    }
    if (!isset($return)) {
      $return = NULL;
    }
    return $return;
  }

  /**
   * {@inheritdoc}
   */
  public function fields($migration = NULL) {
    return array(
      'timezone' => t('Timezone'),
      'rrule' => t('Recurring event rule'),
      'to' => t('End date date'),
    );
  }
}
