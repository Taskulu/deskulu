<?php
/**
 * @file
 * Variable module hook implementations
 */

/**
 * Implements hook_variable_group_info().
 */
function variable_variable_group_info() {
  // Group for variable that have no group
  $groups['default'] = array(
    'title' => t('Other'),
    'description' => t("Variables that don't belong to any other group."),
  );
  $groups['debug'] = array(
    'title' => t('Debug'),
    'description' => t('Debug and development options.'),
  );
  $groups['variable'] = array(
    'title' => t('Variable'),
    'description' => t('Variables that contain metadata about the variable system.'),
  );
  return $groups;
}

/**
 * Implements hook_variable_type_info().
 */
function variable_variable_type_info() {
  // Array of values
  $type['array'] = array(
    'title' => t('Array'),
    'element' => array('#type' => 'fieldset', '#tree' => TRUE),
    // Properties for each array item
    'repeat' => array(
      'element' => array('#type' => 'textfield'),
    ),
    'format callback' => 'variable_format_array',
    'element callback' => 'variable_form_element_array',
    'default' => array(),
  );
  // Array whose keys are named properties.
  $type['properties'] = array(
    'title' => t('Properties'),
    'format callback' => 'variable_format_properties',
    'type' => 'array',
  );
  // TRUE / FALSE value, checkbox
  $type['boolean'] = array(
    'title' => t('Boolean'),
    'element' => array('#type' => 'checkbox'),
    'format callback' => 'variable_format_boolean',
  );
  // Default type for variables with no other type
  $type['default'] = array(
    'title' => t('Default'),
    'element' => array('#type' => 'textfield'),
    'access' => 'administer site configuration',
  );
  // Enable/Disable
  $type['enable'] = array(
    'title' => t('Enable'),
    'options' => array(t('Disabled'), t('Enabled')),
    'default' => 0,
    'element' => array('#type' => 'radios'),
    'format callback' => 'variable_format_selection',
  );
  // Multiple variable that will spawn into multiple elements
  $type['multiple'] = array(
    'title' => t('Multiple'),
    'element' => array('#type' => 'fieldset'),
    'build callback' => 'variable_build_multiple',
    'format callback' => 'variable_format_multiple',
    'element callback' => 'variable_form_element_multiple',
    'value callback' => 'variable_multiple_get_value',
    'default callback' => 'variable_multiple_get_default',
  );
  $type['mail_address'] = array(
    'title' => t('E-mail address'),
    'element' => array('#type' => 'textfield'),
    'token' => TRUE,
  );
  $type['mail_text'] = array(
    'title' => t('Mail text'),
    'multiple' => array('subject' => t('Subject'), 'body' => t('Body')),
    'build callback' => 'variable_build_mail_text',
    'localize' => TRUE,
    'type' => 'multiple',
  );
  $type['number'] = array(
    'title' => t('Number'),
    'element' => array('#type' => 'textfield', '#size' => 15, '#maxlength' => 10),
    'token' => TRUE,
    'validate callback' => 'variable_validate_number',
    'format callback' => 'variable_format_number',
  );
  // Select multiple options from multiple choices
  $type['options'] = array(
    'title' => t('Options'),
    'options' => TRUE,
    'element' => array('#type' => 'checkboxes'),
    'element callback' => 'variable_form_element_options',
    'format callback' => 'variable_format_options',
  );
  // Select single option from multiple choices
  $type['select'] = array(
    'title' => t('Select'),
    'options' => TRUE,
    // This will become radios or drop-down depending on the number of options
    'element callback' => 'variable_form_element_options',
    'format callback' => 'variable_format_selection',
  );
  // Select number from array of values. Options array that can be list of numbers will be converted to a value => value
  $type['select_number'] = array(
    'title' => t('Select'),
    'options' => TRUE,
    'element callback' => 'variable_form_element_options',
    'options callback' => 'variable_options_select_number',
  );
  $type['string'] = array(
    'title' => t('String'),
    'element' => array('#type' => 'textfield'),
    'localize' => TRUE,
    'format callback' => 'variable_format_string',
    'token' => TRUE,
    // This type may have an 'allowed tags' attribute.
    // If empty it will be formatted as plain text
    'allowed tags' => array(),
  );
  $type['text'] = array(
    'title' => t('Text'),
    'element' => array('#type' => 'textarea'),
    'localize' => TRUE,
    'format callback' => 'variable_format_text',
    'token' => TRUE,
    // This type may have an 'allowed tags' attribute.
    // If empty it will be formatted with filter_xss_admin.
    'allowed tags' => array(),
  );
  // Default type for variables with no other type
  $type['unknown'] = array(
    'title' => t('Unknown'),
    'access' => 'administer site configuration',
    'format' => 'variable_format_unknown',
    'element callback' => 'variable_form_element_unknown',
    'element' => array('#type' => 'item'),
  );
  $type['url'] = array(
    'title' => t('URL'),
    'element' => array('#type' => 'textfield', '#size' => 80, '#maxlength' => 255),
    'token' => TRUE,
  );
  $type['mail_part'] = array(
    'title' => t('Mail parts'),
    'options' => array('subject' => t('Subject'), 'body' => t('Body')),
  );
  $type['text_format'] = array(
    'title' => t('Formatted text'),
    'element' => array('#type' => 'text_format'),
    'element callback' => 'variable_form_element_text_format',
    'format callback' => 'variable_format_text_format',
    'default callback' => 'variable_text_format_default',
    'localize' => TRUE,
  );
  return $type;
}

/**
 * Build multiple mail variable
 */
function variable_build_mail_text($variable, $options = array()) {
  $name = str_replace('[mail_part]', '', $variable['name']);
  // For mail text, children have different types
  $variable['children'][$name . 'subject']['type'] = 'string';
  $variable['children'][$name . 'body']['type'] = 'text';
  $variable = variable_build_multiple($variable, $options);
  return $variable;
}

/**
 * Format select variable
 */
function variable_format_selection($variable, $options = array()) {
  $variable = variable_build_options($variable, $options);
  if (isset($variable['value'])) {
    return isset($variable['options'][$variable['value']]) ? $variable['options'][$variable['value']] : '<' . t('Invalid option') . '>';
  }
  else {
    return variable_format_empty($variable);
  }
}

/**
 * Format options variable. Value is an array of options.
 */
function variable_format_options($variable, $options = array()) {
  $variable = variable_build_options($variable, $options);
  $names = array();
  if (isset($variable['value']) && $variable['value']) {
    if (is_array($variable['value'])) {
      foreach ($variable['value'] as $index => $value) {
         $names[$index] = isset($variable['options'][$value]) ? $variable['options'][$value] : '<' . t('Invalid option') . '>';
      }
      return implode(', ', $names);
    }
    else {
      return '<' . t('Invalid value') . '>';
    }
  }
  else {
    return variable_format_empty($variable);
  }
}

/**
 * Format array variable, handling nested arrays
 */
function variable_format_array($variable = NULL, $options = array()) {
  if (empty($variable['value'])) {
    return variable_format_empty($variable);
  }
  else {
    $list = array();
    foreach ($variable['value'] as $index => $item) {
      if (is_array($item) || is_object($item)) {
        $list[$index] = variable_format_array(array('value' => (array)$item), $options);
      }
      else {
        $list[$index] = check_plain((string)$item);
      }
    }
    return theme('item_list', array('items' => $list));
  }
}
/**
 * Format array variable with known keys, handling nested arrays
 */
function variable_format_properties($variable = NULL, $options = array()) {
  if (empty($variable['value'])) {
    return variable_format_empty($variable);
  }
  else {
    $rows = array();
    foreach ($variable['value'] as $name => $item) {
      $title = check_plain((string)$name);
      if (is_array($item) || is_object($item)) {
        $value = variable_format_array(array('value' => (array)$item), $options);
      }
      else {
        $value = check_plain((string)$item);
      }
      $rows[] = array('<em>' . $title . '</em>', $value);
    }
    return theme('table', array('rows' => $rows));
  }
}

/**
 * Format boolean variable
 */
function variable_format_boolean($variable, $options = array()) {
  if (isset($variable['value'])) {
    return $variable['value'] ? t('True') : t('False');
  }
  else {
    return t('Undefined');
  }
}

/**
 * Format variable empty value
 */
function variable_format_empty($variable) {
  return isset($variable['empty']) ? $variable['empty'] : t('Empty');
}

/**
 * Format variable as number.
 */
function variable_format_number($variable, $options = array()) {
  if (is_numeric($variable['value'])) {
    return (string)$variable['value'];
  }
  elseif (empty($variable['value'])) {
    return '';
  }
  else {
    return check_plain($variable['value']);
  }
}

/**
 * Format variable as string. Either check plain for filter_xss.
 */
function variable_format_string($variable, $options = array()) {
  if (empty($variable['value'])) {
    return '';
  }
  elseif (!empty($variable['allowed tags'])) {
    return filter_xss($variable['value'], $variable['allowed tags']);
  }
  else {
    return check_plain($variable['value']);
  }
}

/**
 * Format text variable
 */
function variable_format_text($variable, $options = array()) {
  if (empty($variable['value'])) {
    return '';
  }
  elseif (!empty($variable['allowed tags'])) {
    return filter_xss($variable['value'], $variable['allowed tags']);
  }
  else {
    return filter_xss_admin($variable['value']);
  }
}

/**
 * Options callback for numeric select
 */
function variable_options_select_number($variable, $options = array()) {
  return drupal_map_assoc($variable['options']);
}

/**
 * Default callback for text_format.
 */
function variable_text_format_default($variable, $options = array()) {
  $out = array(
    'value' => '',
    'format' => filter_default_format(),
  );
  if (!empty($variable['default'])) {
    if (is_string($variable['default'])) {
      $out['value'] = $variable['default'];
    }
    elseif (is_array($variable['default'])) {
      if (isset($variable['default']['value'])) {
        $out['value'] = $variable['default']['value'];
      }
      if (isset($variable['default']['format'])) {
        $out['format'] = $variable['default']['format'];
      }
    }
  }
  return $out;
}

/**
 * Format text_format.
 */
function variable_format_text_format($variable, $options = array()) {
  return check_markup($variable['value']['value'], $variable['value']['format']);
}

/**
 * Format multiple variable.
 */
function variable_format_multiple($variable, $options = array()) {
  $rows = array();
  foreach ($variable['children'] as $name => $child) {
    $rows[$name] = array(
      array('data' => check_plain($child['title']), 'header' => TRUE),
      variable_format_value($child)
    );
  }
  return theme('table', array('rows' => $rows));
}

/**
 * Validate numeric variable.
 */
function variable_validate_number($variable) {
  if (empty($variable['required']) && empty($variable['value'])) {
    return;
  }
  elseif (!is_numeric($variable['value'])) {
    return t('The value is not a number.');
  }
}
