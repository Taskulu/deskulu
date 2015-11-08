<?php
/**
 * @file
 * Code to add a date repeat selection form to a date field and create
 * an iCal RRULE from the chosen selections.
 *
 * Moved to a separate file since it is not used on most pages
 * so the code is not parsed unless needed.
 *
 * Currently implemented:
 * INTERVAL, UNTIL, EXDATE, RDATE, BYDAY, BYMONTHDAY, BYMONTH,
 * YEARLY, MONTHLY, WEEKLY, DAILY
 *
 * Currently not implemented:
 *
 * BYYEARDAY, MINUTELY, HOURLY, SECONDLY, BYMINUTE, BYHOUR, BYSECOND
 *   These could be implemented in the future.
 *
 * COUNT
 *   The goal of this module is to create a way we can parse an iCal
 *   RRULE and pull out just dates for a specified date range, for
 *   instance with a date that repeats daily for several years, we might
 *   want to only be able to pull out the dates for the current year.
 *
 *   Adding COUNT to the rules we create makes it impossible to do that
 *   without parsing and computing the whole range of dates that the rule
 *   will create. COUNT is left off of the user form completely for this
 *   reason.
 *
 * BYSETPOS
 *   Seldom used anywhere, so no reason to complicated the code.
 */

/**
 * Generate the repeat setting form.
 */
function _date_repeat_rrule_process($element, &$form_state, $form) {

  // If the RRULE field is not visible to the user,
  // needs no processing or validation.
  // The Date field module is not adding this element to forms
  // if the field is hidden,
  // this test is just in case some other module attempts to do so.

  if (date_hidden_element($element)) {
    return $element;
  }

  module_load_include('inc', 'date_api', 'date_api_ical');
  if (empty($element['#date_repeat_widget'])) {
    $element['#date_repeat_widget'] = module_exists('date_popup') ? 'date_popup' : 'date_select';
  }
  if (is_array($element['#default_value'])) {
    $element['#value'] = date_repeat_merge($element['#value'], $element);
    $rrule = date_api_ical_build_rrule($element['#value']);
  }
  else {
    $rrule = $element['#default_value'];
  }

  // Empty the original string value of the RRULE so we can create
  // an array of values for the form from the RRULE's contents.
  $element['#value'] = '';

  $parts = date_repeat_split_rrule($rrule);
  $rrule = $parts[0];

  $exceptions = $parts[1];
  $additions = $parts[2];
  $timezone = !empty($element['#date_timezone']) ? $element['#date_timezone'] : date_default_timezone();
  $merged_values = date_repeat_merge($rrule, $element);

  $until = '';
  if (!empty($merged_values['UNTIL']['datetime'])) {
    $until_date = new DateObject($merged_values['UNTIL']['datetime'], $merged_values['UNTIL']['tz']);
    date_timezone_set($until_date, timezone_open($timezone));
    $until = date_format($until_date, DATE_FORMAT_DATETIME);
  }

  $count = '';
  if (!empty($merged_values['COUNT'])) {
    $count = $merged_values['COUNT'];
  }

  $element['FREQ'] = array(
    '#type' => 'select',
    '#title' => t('Repeats', array(), array('context' => 'Date repeat')),
    '#default_value' => !empty($rrule['FREQ']) ? $rrule['FREQ'] : 'WEEKLY',
    '#options' => date_repeat_freq_options(),
    '#prefix' => '<div class="date-repeat-input">',
    '#suffix' => '</div>',
  );

  $element['daily'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear daily">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'DAILY'),
      ),
    ),
  );

  $element['weekly'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear weekly">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'WEEKLY'),
      ),
    ),
  );

  $element['monthly'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear monthly">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'MONTHLY'),
      ),
    ),
  );

  $element['yearly'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear yearly">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'YEARLY'),
      ),
    ),
  );

  list($prefix, $suffix) = explode('@interval', t('Every @interval days', array(), array('context' => 'Date repeat')));
  $daily_interval = array(
    '#type' => 'textfield',
    '#title' => t('Repeats', array(), array('context' => 'Date repeat')),
    '#title_display' => 'invisible',
    '#default_value' => (!empty($rrule['INTERVAL']) ? $rrule['INTERVAL'] : 1),
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array('placeholder' => array('#')),
    '#size' => 3,
    '#maxlength' => 3,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => t('days') . '</div>',
    '#field_prefix' => $prefix,
    '#field_suffix' => $suffix,
  );

  list($prefix, $suffix) = explode('@interval', t('Every @interval weeks', array(), array('context' => 'Date repeat')));
  $element['weekly']['INTERVAL'] = array(
    '#type' => 'textfield',
    '#title' => t('Repeats', array(), array('context' => 'Date repeat')),
    '#default_value' => (!empty($rrule['INTERVAL']) ? $rrule['INTERVAL'] : 1),
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array('placeholder' => array('#')),
    '#size' => 3,
    '#maxlength' => 3,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#field_prefix' => $prefix,
    '#field_suffix' => $suffix,
  );

  list($prefix, $suffix) = explode('@interval', t('Every @interval months', array(), array('context' => 'Date repeat')));
  $element['monthly']['INTERVAL'] = array(
    '#access' => FALSE,
    '#type' => 'textfield',
    '#title' => t('Repeats', array(), array('context' => 'Date repeat')),
    '#default_value' => (!empty($rrule['INTERVAL']) ? $rrule['INTERVAL'] : 1),
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array('placeholder' => array('#')),
    '#size' => 3,
    '#maxlength' => 3,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#field_prefix' => $prefix,
    '#field_suffix' => $suffix,
  );

  list($prefix, $suffix) = explode('@interval', t('Every @interval years', array(), array('context' => 'Date repeat')));
  $element['yearly']['INTERVAL'] = array(
    '#type' => 'textfield',
    '#title' => t('Repeats', array(), array('context' => 'Date repeat')),
    '#default_value' => (!empty($rrule['INTERVAL']) ? $rrule['INTERVAL'] : 1),
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array('placeholder' => array('#')),
    '#size' => 3,
    '#maxlength' => 3,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#field_prefix' => $prefix,
    '#field_suffix' => $suffix,
  );
  $options = date_repeat_dow_day_options_abbr(TRUE);
  $options = date_repeat_dow_day_options_ordered($options);
  $element['weekly']['BYDAY'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Repeat on', array(), array('context' => 'Date repeat')),
    '#default_value' => !empty($rrule['BYDAY']) && $rrule['FREQ'] === 'WEEKLY' ? $rrule['BYDAY'] : array(),
    '#options' => $options,
    '#attributes' => array('class' => array('container-inline byday')),
    '#multiple' => TRUE,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
  );

  $daily_radios_default = 'INTERVAL';
  if (isset($rrule['FREQ']) && $rrule['FREQ'] === 'DAILY' && !empty($rrule['BYDAY'])) {
    switch (count($rrule['BYDAY'])) {
      case 2:
        $daily_radios_default = 'every_tu_th';
        break;

      case 3:
        $daily_radios_default = 'every_mo_we_fr';
        break;

      case 5:
        $daily_radios_default = 'every_weekday';
        break;
    }
  }

  $daily_every_weekday = array(
    '#type' => 'item',
    '#markup' => '<div>' . t('Every weekday', array(), array('context' => 'Date repeat')) . '</div>',
  );

  $daily_mo_we_fr = array(
    '#type' => 'item',
    '#markup' => '<div>' . t('Every Mon, Wed, Fri', array(), array('context' => 'Date repeat')) . '</div>',
  );

  $daily_tu_th = array(
    '#type' => 'item',
    '#markup' => '<div>' . t('Every Tue, Thu', array(), array('context' => 'Date repeat')) . '</div>',
  );

  $element['daily']['byday_radios'] = array(
    '#type' => 'date_repeat_form_element_radios',
    '#tree' => TRUE,
    '#title' => t('Repeats every', array(), array('context' => 'Date repeat')),
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'DAILY'),
      ),
    ),
    '#default_value' => $daily_radios_default,
    '#options' => array(
      'INTERVAL' => t('interval'),
      'every_weekday' => t('every weekday'),
      'every_mo_we_fr' => t('monday wednesday friday'),
      'every_tu_th' => t('tuesday thursday'),
    ),
    'INTERVAL_child' => $daily_interval,
    'every_weekday_child' => $daily_every_weekday,
    'mo_we_fr_child' => $daily_mo_we_fr,
    'tu_th_child' => $daily_tu_th,
    '#div_classes' => array(
      'container-inline interval',
      'container-inline weekday',
      'container-inline mo-we-fr',
      'container-inline tu-th',
    ),
  );

  $monthly_day_month_default = 'BYMONTHDAY_BYMONTH';
  if (isset($rrule['FREQ']) && $rrule['FREQ'] === 'MONTHLY' && !empty($rrule['BYDAY'])) {
    $monthly_day_month_default = 'BYDAY_BYMONTH';
  }

  $monthly_on_day_bymonthday_of_bymonth = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  list($bymonthday_title, $bymonthday_suffix) = explode('@bymonthday', t('On day @bymonthday of', array(), array('context' => 'Date repeat')));
  $monthly_on_day_bymonthday_of_bymonth['BYMONTHDAY'] = array(
    '#type' => 'select',
    '#title' => $bymonthday_title,
    '#default_value' => !empty($rrule['BYMONTHDAY']) && $rrule['FREQ'] === 'MONTHLY' ? $rrule['BYMONTHDAY'] : '',
    '#options' => drupal_map_assoc(range(1, 31)) + drupal_map_assoc(range(-1, -31)),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-clear bymonthday">',
    '#suffix' => '</div>',
    '#field_suffix' => $bymonthday_suffix,
  );

  $monthly_on_day_bymonthday_of_bymonth['BYMONTH'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Bymonth', array(), array('context' => 'Date repeat')),
    '#title_display' => 'invisible',
    '#default_value' => !empty($rrule['BYMONTH']) && $rrule['FREQ'] === 'MONTHLY' && $monthly_day_month_default === 'BYMONTHDAY_BYMONTH' ? $rrule['BYMONTH'] : array(),
    '#options' => date_month_names_abbr(TRUE),
    '#attributes' => array('class' => array('container-inline')),
    '#multiple' => TRUE,
    '#prefix' => '<div class="date-clear bymonth">',
    '#suffix' => '</div>',
  );

  $monthly_on_the_byday_of_bymonth = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $monthly_byday_count = '';
  $monthly_byday_day = '';
  if (isset($rrule['BYDAY']) && !empty($rrule['BYDAY']) && $rrule['FREQ'] === 'MONTHLY') {
    $monthly_byday_count = substr($rrule['BYDAY'][0], 0, -2);
    $monthly_byday_day = substr($rrule['BYDAY'][0], -2);;
  }

  list($byday_count_title, $byday_day_title) = explode('@byday', t('On the @byday of', array(), array('context' => 'Date repeat')));
  $monthly_on_the_byday_of_bymonth['BYDAY_COUNT'] = array(
    '#type' => 'select',
    '#title' => $byday_count_title,
    '#default_value' => !empty($monthly_byday_count) ? $monthly_byday_count : '',
    '#options' => date_order_translated(),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-repeat-input byday-count">',
    '#suffix' => '</div>',
  );

  $monthly_on_the_byday_of_bymonth['BYDAY_DAY'] = array(
    '#type' => 'select',
    '#title' => $byday_day_title,
    '#title_display' => 'after',
    '#default_value' => !empty($monthly_byday_day) ? $monthly_byday_day : '',
    '#options' => date_repeat_dow_day_options(TRUE),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-repeat-input byday-day">',
    '#suffix' => '</div>',
  );

  $monthly_on_the_byday_of_bymonth['BYMONTH'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Bymonth', array(), array('context' => 'Date repeat')),
    '#title_display' => 'invisible',
    '#default_value' => !empty($rrule['BYMONTH']) && $rrule['FREQ'] === 'MONTHLY' && $monthly_day_month_default === 'BYDAY_BYMONTH' ? $rrule['BYMONTH'] : array(),
    '#options' => date_month_names_abbr(TRUE),
    '#attributes' => array('class' => array('container-inline')),
    '#multiple' => TRUE,
    '#prefix' => '<div class="date-clear bymonth">',
    '#suffix' => '</div>',
  );

  $element['monthly']['day_month'] = array(
    '#type' => 'date_repeat_form_element_radios',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'MONTHLY'),
      ),
    ),
    '#attributes' => array('class' => array('date-repeat-radios clearfix')),
    '#default_value' => $monthly_day_month_default,
    '#options' => array(
      'BYMONTHDAY_BYMONTH' => t('On day ... of ...'),
      'BYDAY_BYMONTH' => t('On the ... of ...'),
    ),
    'BYMONTHDAY_BYMONTH_child' => $monthly_on_day_bymonthday_of_bymonth,
    'BYDAY_BYMONTH_child' => $monthly_on_the_byday_of_bymonth,
    '#div_classes' => array(
      'date-repeat-radios-item date-clear clearfix bymonthday-bymonth',
      'date-repeat-radios-item date-clear clearfix byday-bymonth',
    ),
  );

  $yearly_day_month_default = 'BYMONTHDAY_BYMONTH';
  if (isset($rrule['FREQ']) && $rrule['FREQ'] === 'YEARLY' && !empty($rrule['BYDAY'])) {
    $yearly_day_month_default = 'BYDAY_BYMONTH';
  }

  $yearly_on_day_bymonthday_of_bymonth = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  list($bymonthday_title, $bymonthday_suffix) = explode('@bymonthday', t('On day @bymonthday of', array(), array('context' => 'Date repeat')));
  $yearly_on_day_bymonthday_of_bymonth['BYMONTHDAY'] = array(
    '#type' => 'select',
    '#title' => $bymonthday_title,
    '#default_value' => !empty($rrule['BYMONTHDAY']) && $rrule['FREQ'] === 'YEARLY' ? $rrule['BYMONTHDAY'] : '',
    '#options' => drupal_map_assoc(range(1, 31)) + drupal_map_assoc(range(-1, -31)),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-clear bymonthday">',
    '#suffix' => '</div>',
    '#field_suffix' => $bymonthday_suffix,
  );

  $yearly_on_day_bymonthday_of_bymonth['BYMONTH'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Bymonth', array(), array('context' => 'Date repeat')),
    '#title_display' => 'invisible',
    '#default_value' => !empty($rrule['BYMONTH']) && $rrule['FREQ'] === 'YEARLY' && $yearly_day_month_default === 'BYMONTHDAY_BYMONTH' ? $rrule['BYMONTH'] : array(),
    '#options' => date_month_names_abbr(TRUE),
    '#attributes' => array('class' => array('container-inline')),
    '#multiple' => TRUE,
    '#prefix' => '<div class="date-clear bymonth">',
    '#suffix' => '</div>',
  );

  $yearly_on_the_byday_of_bymonth = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $yearly_byday_count = '';
  $yearly_byday_day = '';
  if (isset($rrule['BYDAY']) && !empty($rrule['BYDAY']) && $rrule['FREQ'] === 'YEARLY') {
    $yearly_byday_count = substr($rrule['BYDAY'][0], 0, -2);
    $yearly_byday_day = substr($rrule['BYDAY'][0], -2);;
  }

  list($byday_count_title, $byday_day_title) = explode('@byday', t('On the @byday of', array(), array('context' => 'Date repeat')));
  $yearly_on_the_byday_of_bymonth['BYDAY_COUNT'] = array(
    '#type' => 'select',
    '#title' => $byday_count_title,
    '#default_value' => !empty($yearly_byday_count) ? $yearly_byday_count : '',
    '#options' => date_order_translated(),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-repeat-input byday-count">',
    '#suffix' => '</div>',
  );

  $yearly_on_the_byday_of_bymonth['BYDAY_DAY'] = array(
    '#type' => 'select',
    '#title' => $byday_day_title,
    '#title_display' => 'after',
    '#default_value' => !empty($yearly_byday_day) ? $yearly_byday_day : '',
    '#options' => date_repeat_dow_day_options(TRUE),
    '#multiple' => FALSE,
    '#prefix' => '<div class="date-repeat-input byday-day">',
    '#suffix' => '</div>',
  );

  $yearly_on_the_byday_of_bymonth['BYMONTH'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Bymonth', array(), array('context' => 'Date repeat')),
    '#title_display' => 'invisible',
    '#default_value' => !empty($rrule['BYMONTH']) && $rrule['FREQ'] === 'YEARLY' && $yearly_day_month_default === 'BYDAY_BYMONTH' ? $rrule['BYMONTH'] : array(),
    '#options' => date_month_names_abbr(TRUE),
    '#attributes' => array('class' => array('container-inline')),
    '#multiple' => TRUE,
    '#prefix' => '<div class="date-clear bymonth">',
    '#suffix' => '</div>',
  );

  $element['yearly']['day_month'] = array(
    '#type' => 'date_repeat_form_element_radios',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-clear">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'YEARLY'),
      ),
    ),
    '#attributes' => array('class' => array('date-repeat-radios clearfix')),
    '#default_value' => $yearly_day_month_default,
    '#options' => array(
      'BYMONTHDAY_BYMONTH' => t('On day ... of ...'),
      'BYDAY_BYMONTH' => t('On the ... of ...'),
    ),
    'BYMONTHDAY_BYMONTH_child' => $yearly_on_day_bymonthday_of_bymonth,
    'BYDAY_BYMONTH_child' => $yearly_on_the_byday_of_bymonth,
    '#div_classes' => array(
      'date-repeat-radios-item date-clear clearfix bymonthday-bymonth',
      'date-repeat-radios-item date-clear clearfix byday-bymonth',
    ),
  );

  list($prefix, $suffix) = explode('@count', t('After @count occurrences', array(), array('context' => 'Date repeat')));
  $count_form_element = array(
    '#type' => 'textfield',
    '#title' => t('Count', array(), array('context' => 'Date repeat')),
    '#default_value' => $count,
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array('placeholder' => array('#')),
    '#prefix' => $prefix,
    '#suffix' => $suffix,
    '#size' => 10,
    '#maxlength' => 10,
  );

  $until_form_element = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#prefix' => '<div class="date-prefix-inline">' . t('On', array(), array('context' => 'Date repeat')) . '</div>',
    'datetime' => array(
      '#type' => $element['#date_repeat_widget'],
      '#title' => t('Until', array(), array('context' => 'Date repeat')),
      '#title_display' => 'invisible',
      '#default_value' => $until,
      '#date_format' => !empty($element['#date_format']) ?
      date_limit_format($element['#date_format'], array('year', 'month', 'day')) : 'Y-m-d',
      '#date_timezone' => $timezone,
      '#date_text_parts'  => !empty($element['#date_text_parts']) ? $element['#date_text_parts'] : array(),
      '#date_year_range'  => !empty($element['#date_year_range']) ? $element['#date_year_range'] : '-3:+3',
      '#date_label_position' => !empty($element['#date_label_position']) ?
      $element['#date_label_position'] : 'within',
      '#date_flexible' => 0,
    ),
    'tz' => array('#type' => 'hidden', '#value' => $element['#date_timezone']),
    'all_day' => array('#type' => 'hidden', '#value' => 1),
    'granularity' => array(
      '#type' => 'hidden',
      '#value' => serialize(array('year', 'month', 'day')),
    ),
  );

  $range_of_repeat_default = 'COUNT';
  if (!empty($until)) {
    $range_of_repeat_default = 'UNTIL';
  }
  $element['range_of_repeat'] = array(
    '#type' => 'date_repeat_form_element_radios',
    '#tree' => TRUE,
    '#title' => t('Stop repeating', array(), array('context' => 'Date repeat')),
    '#title_display' => 'before',
    '#prefix' => '<div class="date-clear range-of-repeat">',
    '#suffix' => '</div>',
    '#states' => array(
      'invisible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'NONE'),
      ),
    ),
    '#default_value' => $range_of_repeat_default,
    '#options' => array(
      'COUNT' => t('Count'),
      'UNTIL' => t('Until'),
    ),
    'count_child' => $count_form_element,
    'until_child' => $until_form_element,
    '#div_classes' => array(
      'container-inline count',
      "until widget-{$element['#date_repeat_widget']} label-{$element['#date_label_position']}",
    ),
  );

  $parents = $element['#array_parents'];
  $instance = implode('-', $parents);

  // Make sure this will work right either in the normal
  // form or in an ajax callback from the 'Add more' button.
  if (empty($form_state['num_exceptions'][$instance])) {
    $form_state['num_exceptions'][$instance] = count($exceptions);
  }
  if ($form_state['num_exceptions'][$instance] == 0) {
    $collapsed = TRUE;
  }
  else {
    $collapsed = FALSE;
  }

  $element['show_exceptions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Exclude dates', array(), array('context' => 'Date repeat')),
    '#states' => array(
      'invisible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'NONE'),
      ),
    ),
    '#default_value' => empty($form_state['num_exceptions'][$instance]) ? 0 : 1,
  );

  $element['exceptions'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="date-repeat-exceptions-' . $instance . '" class="date-repeat">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[show_exceptions]\"]" => array('checked' => TRUE),
      ),
    ),
  );
  for ($i = 0; $i < max($form_state['num_exceptions'][$instance], 1); $i++) {
    $except = '';
    if (!empty($exceptions[$i]['datetime'])) {
      $ex_date = new DateObject($exceptions[$i]['datetime'], $exceptions[$i]['tz']);
      date_timezone_set($ex_date, timezone_open($timezone));
      $except = date_format($ex_date, DATE_FORMAT_DATETIME);
    }
    $date_format = 'Y-m-d';
    if (!empty($element['#date_format'])) {
      $grans = array('year', 'month', 'day');
      $date_format = date_limit_format($element['#date_format'], $grans);
    }
    $element['exceptions']['EXDATE'][$i] = array(
      '#tree' => TRUE,
      'datetime' => array(
        '#name' => 'exceptions|' . $instance,
        '#type' => $element['#date_repeat_widget'],
        '#default_value' => $except,
        '#date_timezone' => !empty($element['#date_timezone']) ?
        $element['#date_timezone'] : date_default_timezone(),
        '#date_format' => $date_format,
        '#date_text_parts'  => !empty($element['#date_text_parts']) ? $element['#date_text_parts'] : array(),
        '#date_year_range'  => !empty($element['#date_year_range']) ? $element['#date_year_range'] : '-3:+3',
        '#date_label_position' => !empty($element['#date_label_position']) ? $element['#date_label_position'] : 'within',
        '#date_flexible' => 0,
      ),
      'tz' => array(
        '#type' => 'hidden',
        '#value' => $element['#date_timezone'],
      ),
      'all_day' => array(
        '#type' => 'hidden',
        '#value' => 1,
      ),
      'granularity' => array(
        '#type' => 'hidden',
        '#value' => serialize(array('year', 'month', 'day')),
      ),
    );
  }

  // Collect additions in the same way as exceptions - implements RDATE.
  if (empty($form_state['num_additions'][$instance])) {
    $form_state['num_additions'][$instance] = count($additions);
  }
  if ($form_state['num_additions'][$instance] == 0) {
    $collapsed = TRUE;
  }
  else {
    $collapsed = FALSE;
  }

  $element['show_additions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include dates', array(), array('context' => 'Date repeat')),
    '#states' => array(
      'invisible' => array(
        ":input[name=\"{$element['#name']}[FREQ]\"]" => array('value' => 'NONE'),
      ),
    ),
    '#default_value' => empty($form_state['num_additions'][$instance]) ? 0 : 1,
  );

  $element['additions'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="date-repeat-additions-' . $instance . '" class="date-repeat">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ":input[name=\"{$element['#name']}[show_additions]\"]" => array('checked' => TRUE),
      ),
    ),
  );
  for ($i = 0; $i < max($form_state['num_additions'][$instance], 1); $i++) {
    $r_date = '';
    if (!empty($additions[$i]['datetime'])) {
      $rdate = new DateObject($additions[$i]['datetime'], $additions[$i]['tz']);
      date_timezone_set($rdate, timezone_open($timezone));
      $r_date = date_format($rdate, DATE_FORMAT_DATETIME);
    }
    $date_format = 'Y-m-d';
    if (!empty($element['#date_format'])) {
      $grans = array('year', 'month', 'day');
      $date_format = date_limit_format($element['#date_format'], $grans);
    }
    $element['additions']['RDATE'][$i] = array(
      '#tree' => TRUE,
      'datetime' => array(
        '#type' => $element['#date_repeat_widget'],
        '#name' => 'additions|' . $instance,
        '#default_value' => $r_date,
        '#date_timezone' => !empty($element['#date_timezone']) ?
        $element['#date_timezone'] : date_default_timezone(),
        '#date_format' => $date_format,
        '#date_text_parts'  => !empty($element['#date_text_parts']) ? $element['#date_text_parts'] : array(),
        '#date_year_range'  => !empty($element['#date_year_range']) ? $element['#date_year_range'] : '-3:+3',
        '#date_label_position' => !empty($element['#date_label_position']) ? $element['#date_label_position'] : 'within',
        '#date_flexible' => 0,
      ),
      'tz' => array(
        '#type' => 'hidden',
        '#value' => $element['#date_timezone'],
      ),
      'all_day' => array(
        '#type' => 'hidden',
        '#value' => 1,
      ),
      'granularity' => array(
        '#type' => 'hidden',
        '#value' => serialize(array('year', 'month', 'day')),
      ),
    );
  }

  $element['exceptions']['exceptions_add'] = array(
    '#type' => 'submit',
    '#name' => 'exceptions_add|' . $instance,
    '#value' => t('Add exception'),
    '#submit' => array('date_repeat_add_exception'),
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'date_repeat_add_exception_callback',
      'wrapper' => 'date-repeat-exceptions-' . $instance,
    ),
  );
  $element['additions']['additions_add'] = array(
    '#type' => 'submit',
    '#name' => 'additions_add|' . $instance,
    '#value' => t('Add addition'),
    '#submit' => array('date_repeat_add_addition'),
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'date_repeat_add_addition_callback',
      'wrapper' => 'date-repeat-additions-' . $instance,
    ),
  );

  $element['#date_repeat_collapsed'] = !empty($rrule['INTERVAL']) || !empty($rrule['FREQ']) ? 0 : (!empty($element['#date_repeat_collapsed']) ? $element['#date_repeat_collapsed'] : 0);
  return $element;
}

/**
 * Add callback to date repeat.
 */
function date_repeat_add_exception_callback($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $button_key = array_pop($parents);
  $element = drupal_array_get_nested_value($form, $parents);
  return $element;
}

/**
 * Add addition callback to date repeat.
 */
function date_repeat_add_addition_callback($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $button_key = array_pop($parents);
  $element = drupal_array_get_nested_value($form, $parents);
  return $element;
}

/**
 * Add exception to date repeat.
 */
function date_repeat_add_exception($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $instance = implode('-', array_slice($parents, 0, count($parents) - 2));
  $form_state['num_exceptions'][$instance]++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Add addition to date repeat.
 */
function date_repeat_add_addition($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $instance = implode('-', array_slice($parents, 0, count($parents) - 2));
  $form_state['num_additions'][$instance]++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Regroup values back into a consistant array, no matter what state it is in.
 */
function date_repeat_merge($form_values, $element) {
  if (empty($form_values) || !is_array($form_values)) {
    return $form_values;
  }
  if (array_key_exists('exceptions', $form_values) || array_key_exists('additions', $form_values)) {
    if (!array_key_exists('exceptions', $form_values)) {
      $form_values['exceptions'] = array();
    }

    if (!array_key_exists('additions', $form_values)) {
      $form_values['additions'] = array();
    }

    $form_values = array_merge($form_values, (array) $form_values['exceptions'], (array) $form_values['additions']);
    unset($form_values['exceptions']);
    unset($form_values['additions']);
  }

  if (array_key_exists('FREQ', $form_values)) {
    switch ($form_values['FREQ']) {
      case 'DAILY':
        if (array_key_exists('daily', $form_values)) {
          switch ($form_values['daily']['byday_radios']) {
            case 'INTERVAL':
              $form_values['INTERVAL'] = $form_values['daily']['INTERVAL_child'];
              break;

            case 'every_weekday':
              $form_values['BYDAY'] = array('MO', 'TU', 'WE', 'TH', 'FR');
              break;

            case 'every_mo_we_fr':
              $form_values['BYDAY'] = array('MO', 'WE', 'FR');
              break;

            case 'every_tu_th':
              $form_values['BYDAY'] = array('TU', 'TH');
              break;
          }
        }
        break;

      case 'WEEKLY':
        if (array_key_exists('weekly', $form_values)) {
          $form_values = array_merge($form_values, (array) $form_values['weekly']);
          if (array_key_exists('BYDAY', $form_values)) {
            $form_values['BYDAY'] = date_repeat_transform_checkbox_values_to_select_values($form_values['BYDAY']);
          }
        }
        break;

      case 'MONTHLY':
        if (array_key_exists('monthly', $form_values)) {
          switch ($form_values['monthly']['day_month']) {
            case 'BYMONTHDAY_BYMONTH':
              $form_values['monthly'] = array_merge($form_values['monthly'], (array) $form_values['monthly']['BYMONTHDAY_BYMONTH_child']);
              break;

            case 'BYDAY_BYMONTH':
              $form_values['monthly']['BYDAY_BYMONTH_child']['BYDAY'] = $form_values['monthly']['BYDAY_BYMONTH_child']['BYDAY_COUNT'] . $form_values['monthly']['BYDAY_BYMONTH_child']['BYDAY_DAY'];
              $form_values['monthly'] = array_merge($form_values['monthly'], (array) $form_values['monthly']['BYDAY_BYMONTH_child']);
              break;
          }
          unset($form_values['monthly']['BYDAY_BYMONTH_child']);
          unset($form_values['monthly']['BYMONTHDAY_BYMONTH_child']);
          $form_values = array_merge($form_values, (array) $form_values['monthly']);
          if (array_key_exists('BYMONTH', $form_values)) {
            $form_values['BYMONTH'] = date_repeat_transform_checkbox_values_to_select_values($form_values['BYMONTH']);
          }
          if (array_key_exists('BYMONTHDAY', $form_values) && !is_array($form_values['BYMONTHDAY'])) {
            $form_values['BYMONTHDAY'] = (array) $form_values['BYMONTHDAY'];
          }
          if (array_key_exists('BYDAY', $form_values) && !is_array($form_values['BYDAY'])) {
            $form_values['BYDAY'] = (array) $form_values['BYDAY'];
          }
        }
        break;

      case 'YEARLY':
        if (array_key_exists('yearly', $form_values)) {
          switch ($form_values['yearly']['day_month']) {
            case 'BYMONTHDAY_BYMONTH':
              $form_values['yearly'] = array_merge($form_values['yearly'], (array) $form_values['yearly']['BYMONTHDAY_BYMONTH_child']);
              break;

            case 'BYDAY_BYMONTH':
              $form_values['yearly']['BYDAY_BYMONTH_child']['BYDAY'] = $form_values['yearly']['BYDAY_BYMONTH_child']['BYDAY_COUNT'] . $form_values['yearly']['BYDAY_BYMONTH_child']['BYDAY_DAY'];
              $form_values['yearly'] = array_merge($form_values['yearly'], (array) $form_values['yearly']['BYDAY_BYMONTH_child']);
              break;
          }
          unset($form_values['yearly']['BYDAY_BYMONTH_child']);
          unset($form_values['yearly']['BYMONTHDAY_BYMONTH_child']);
          $form_values = array_merge($form_values, (array) $form_values['yearly']);
          if (array_key_exists('BYMONTH', $form_values)) {
            $form_values['BYMONTH'] = date_repeat_transform_checkbox_values_to_select_values($form_values['BYMONTH']);
          }
          if (array_key_exists('BYMONTHDAY', $form_values) && !is_array($form_values['BYMONTHDAY'])) {
            $form_values['BYMONTHDAY'] = (array) $form_values['BYMONTHDAY'];
          }
          if (array_key_exists('BYDAY', $form_values) && !is_array($form_values['BYDAY'])) {
            $form_values['BYDAY'] = (array) $form_values['BYDAY'];
          }
        }
        break;

      default:
        break;
    }
  }

  unset($form_values['daily']);
  unset($form_values['weekly']);
  unset($form_values['monthly']);
  unset($form_values['yearly']);

  if (array_key_exists('range_of_repeat', $form_values)) {
    switch ($form_values['range_of_repeat']) {
      case 'COUNT':
        $form_values['COUNT'] = $form_values['count_child'];
        break;

      case 'UNTIL':
        $form_values['UNTIL'] = $form_values['until_child'];
        break;
    }
  }

  unset($form_values['count_child']);
  unset($form_values['until_child']);

  if (array_key_exists('BYDAY', $form_values) && is_array($form_values['BYDAY'])) {
    unset($form_values['BYDAY']['']);
  }

  if (array_key_exists('BYMONTH', $form_values) && is_array($form_values['BYMONTH'])) {
    unset($form_values['BYMONTH']['']);
  }

  if (array_key_exists('BYMONTHDAY', $form_values) && is_array($form_values['BYMONTHDAY'])) {
    unset($form_values['BYMONTHDAY']['']);
  }

  if (array_key_exists('UNTIL', $form_values) && is_array($form_values['UNTIL']['datetime'])) {
    $function = $element['#date_repeat_widget'] . '_input_date';
    $until_element = $element;
    $until_element['#date_format'] = !empty($element['#date_format']) ?
    date_limit_format($element['#date_format'], array('year', 'month', 'day')) : 'Y-m-d';
    $date = $function($until_element, $form_values['UNTIL']['datetime']);
    $form_values['UNTIL']['datetime'] = is_object($date) ? $date->format(DATE_FORMAT_DATETIME) : '';
  }
  if (array_key_exists('show_exceptions', $form_values) && $form_values['show_exceptions'] === 0) {
    unset($form_values['EXDATE']);
  }
  if (array_key_exists('EXDATE', $form_values) && is_array($form_values['EXDATE'])) {
    $function = $element['#date_repeat_widget'] . '_input_date';
    $exdate_element = $element;
    $date_format = 'Y-m-d';
    if (!empty($element['#date_format'])) {
      $grans = array('year', 'month', 'day');
      $date_format = date_limit_format($element['#date_format'], $grans);
    }
    foreach ($form_values['EXDATE'] as $delta => $value) {
      if (is_array($value['datetime'])) {
        $exdate_element['#date_format'] = $date_format;
        $date = $function($exdate_element, $form_values['EXDATE'][$delta]['datetime']);
        $form_values['EXDATE'][$delta]['datetime'] = is_object($date) ? $date->format(DATE_FORMAT_DATETIME) : '';
      }
    }
  }

  if (array_key_exists('show_additions', $form_values) && $form_values['show_additions'] === 0) {
    unset($form_values['RDATE']);
  }
  if (array_key_exists('RDATE', $form_values) && is_array($form_values['RDATE'])) {
    $function = $element['#date_repeat_widget'] . '_input_date';
    $rdate_element = $element;
    $date_format = 'Y-m-d';
    if (!empty($element['#date_format'])) {
      $grans = array('year', 'month', 'day');
      $date_format = date_limit_format($element['#date_format'], $grans);
    }
    foreach ($form_values['RDATE'] as $delta => $value) {
      if (is_array($value['datetime'])) {
        $rdate_element['#date_format'] = $date_format;
        $date = $function($rdate_element, $form_values['RDATE'][$delta]['datetime']);
        $form_values['RDATE'][$delta]['datetime'] = is_object($date) ? $date->format(DATE_FORMAT_DATETIME) : '';
      }
    }
  }
  return $form_values;
}

/**
 * Build a RRULE out of the form values.
 */
function date_repeat_rrule_validate($element, &$form_state) {

  if (date_hidden_element($element)) {
    return;
  }

  $parents = $element['#parents'];
  array_pop($parents);
  $field_values = drupal_array_get_nested_value($form_state['values'], $parents);
  if ($field_values['show_repeat_settings'] === 0 || $field_values['rrule']['FREQ'] === 'NONE') {
    form_set_value($element, NULL, $form_state);
    return;
  }

  // Clean the buttons off of the form. Needed to avoid errors when
  // the date is used on a user object, which then passes the form
  // through form_state_values_clean().
  foreach ($form_state['buttons'] as $delta => $item) {
    if (!empty($item['#ajax']['callback']) && in_array($item['#ajax']['callback'], array('date_repeat_add_exception_callback', 'date_repeat_add_addition_callback'))) {
      unset($form_state['buttons'][$delta]);
    }
  }

  module_load_include('inc', 'date_api', 'date_api_ical');

  $item = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  $item = date_repeat_merge($item, $element);
  $rrule = date_api_ical_build_rrule($item);
  form_set_value($element, $rrule, $form_state);
}

/**
 * Theme the exception list as a table so the buttons line up.
 */
function theme_date_repeat_current_exceptions($vars) {
  $rows = $vars['rows'];
  $rows_info = array();
  foreach ($rows as $key => $value) {
    if (substr($key, 0, 1) != '#') {
      $rows_info[] = array(drupal_render($value['action']), drupal_render($value['display']));
    }
  }
  return theme('table', array(
    'header' => array(t('Delete'), t('Current exceptions')),
    'rows' => $rows_info)
  );
}

/**
 * Theme the exception list as a table so the buttons line up.
 */
function theme_date_repeat_current_additions($rows = array()) {
  $rows_info = array();
  foreach ($rows as $key => $value) {
    if (substr($key, 0, 1) != '#') {
      $rows_info[] = array(drupal_render($value['action']), drupal_render($value['display']));
    }
  }
  return theme('table', array(
    'header' => array(t('Delete'), t('Current additions')),
    'rows' => $rows_info)
  );
}

/**
 * Wrapper fieldset for repeat rule.
 */
function theme_date_repeat_rrule($vars) {
  $element = $vars['element'];
  $id = drupal_html_id('repeat-settings-fieldset');
  $parents = $element['#parents'];

  $selector = $parents[0];
  for ($i = 1; $i < count($parents) - 1; $i++) {
    $selector .= '[' . $parents[$i] . ']';
  }
  $selector .= '[show_repeat_settings]';

  $fieldset = array(
    '#type' => 'item',
    '#title' => t('Repeat settings'),
    '#title_display' => 'invisible',
    '#markup' => $element['#children'],
    '#states' => array(
      'invisible' => array(
        ":input[name=\"{$selector}\"]" => array('checked' => FALSE),
      ),
    ),
    '#id' => $id,
  );

  return drupal_render($fieldset);
}

/**
 * Filter non zero values.
 */
function date_repeat_filter_non_zero_value($value) {
  return $value !== 0;
}

/**
 * Helper function for transforming the return value of checkbox(es) element.
 *
 * Can be used for transforming the returned value of checkbox(es) element
 * to the format of returned value of multiple select element.
 */
function date_repeat_transform_checkbox_values_to_select_values($values) {
  return array_filter($values, 'date_repeat_filter_non_zero_value');
}
