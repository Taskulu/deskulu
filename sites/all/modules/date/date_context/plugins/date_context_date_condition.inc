<?php

/**
 * @file
 * Context date condition plugin.
 */

/**
 * Expose term views/term forms by vocabulary as a context condition.
 */
// @codingStandardsIgnoreStart
class date_context_date_condition extends context_condition_node {

  /**
   * {@inheritdoc}
   */
  public function condition_values() {
    $values = array();
    $fields = field_info_fields();
    foreach ($fields as $field_name => $field) {
      if (in_array($field['type'], array('date', 'datetime', 'datestamp'))) {
        $values[$field_name] = $field_name;
      }
    }
    return $values;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form($context) {
    $defaults = $this->fetch_from_context($context, 'options');
    $options = array(
      '<' => t('Is less than'),
      '<=' => t('Is less than or equal to'),
      '>=' => t('Is greater than or equal to'),
      '>' => t('Is greater than'),
      '=' => t('Is equal to'),
      '!=' => t('Is not equal to'),
      'empty' => t('Is empty'),
      'not empty' => t('Is not Empty'),
    );
    $dependency_options = array('<', '<=', '>', '>=', '=', '!=');

    $form['operation'] = array(
      '#title' => t('Operation'),
      '#type' => 'select',
      '#options' => $options,
      '#description' => t('The comparison to perform to determine if the date field meets the condition. For multiple value date fields, all values will be checked to see if any meet the condition.'),
      '#default_value' => isset($defaults['operation']) ? $defaults['operation'] : '',
      '#required' => TRUE,
    );
    $form['value'] = array(
      '#title' => t('Value'),
      '#type' => 'textfield',
      '#description' => t("The value the field should contain to meet the condition. This can either be an absolute date in ISO format (YYYY-MM-DDTHH:MM:SS) or a relative string like '12AM today'. Examples: 2011-12-31T00:00:00, now, now +1 day, 12AM today, Monday next week. <a href=\"@relative_format\">More examples of relative date formats in the PHP documentation</a>.", array('@relative_format' => 'http://www.php.net/manual/en/datetime.formats.relative.php')),
      '#default_value' => isset($defaults['value']) ? $defaults['value'] : '',
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array('edit-conditions-plugins-date-context-date-condition-options-operation' => $dependency_options),
    );
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function execute($entity, $op) {
    if (in_array($op, array('view', 'form'))) {
      foreach ($this->get_contexts() as $context) {
        $options = $this->fetch_from_context($context, 'options');
        $fields = $this->fetch_from_context($context, 'values');

        foreach ($fields as $field_name => $label) {

          // If this field does not exist on this entity, just move along.
          if (empty($entity->{$field_name})) {
            continue;
          }

          $items = field_get_items('node', $entity, $field_name);

          // If there are no values, nothing to do unless we were looking for 'empty' or '!='.
          if (empty($items)) {
            if ($options['operation'] == '!=' || $options['operation'] == 'empty') {
              $this->condition_met($context, $field_name);
            }
          }

          // We don't have to construct the date values if we're just testing for 'not empty'.
          elseif ($options['operation'] == 'not empty') {
            $this->condition_met($context, $field_name);
          }

          // All other operations need to retrieve the date values for comparision.
          else {
            $field = field_info_field($field_name);
            $timezone_db = date_get_timezone_db($field['settings']['tz_handling']);
            foreach ($items as $delta => $item) {
              $timezone = date_get_timezone($field['settings']['tz_handling'], $item['timezone']);
              $date = new DateObject($item['value'], $timezone_db);
              date_timezone_set($date, timezone_open($timezone));
              $date1 = $date->format(DATE_FORMAT_DATETIME);
              if (empty($item['value2'])) {
                $item['value2'] = $item['value'];
              }
              $date = new DateObject($item['value2'], $timezone_db);
              date_timezone_set($date, timezone_open($timezone));
              $date2 = $date->format(DATE_FORMAT_DATETIME);
              str_replace('now', 'today', $options['value']);
              $date = date_create($options['value'], date_default_timezone_object());
              $compdate = $date->format(DATE_FORMAT_DATETIME);
              switch ($options['operation']) {
                case '=':
                  if ($date2 >= $compdate && $date1 <= $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;

                case '>':
                  if ($date1 > $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;

                case '>=':
                  if ($date1 >= $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;

                case '<':
                  if ($date2 < $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;

                case '<=':
                  if ($date2 <= $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;

                case '!=':
                  if ($date1 < $compdate || $date2 > $compdate) {
                    $this->condition_met($context, $field_name);
                  }
                  break;
              }
            }
          }
        }
      }
    }
  }
}
// @codingStandardsIgnoreEnd
