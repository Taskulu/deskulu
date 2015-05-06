<?php
/**
 * @file
 * Variable API module. Definition for some xample variables
 */

/**
 * Implements hook_variable_info().
 */
function variable_example_variable_info($options) {
  // Simple text
  $variables['variable_example_text'] = array(
    'type' => 'text',
    'title' => t('Simple text', array(), $options),
    'default' => 'Example text.',
    'description' => t('Example of text variable.', array(), $options),
    'required' => TRUE,
    'group' => 'variable_example',
  );
  // Simple number, demonstrates validate callback.
  $variables['variable_example_number'] = array(
    'type' => 'number',
    'title' => t('Number', array(), $options),
    'default' => 0,
    'description' => t('Example of numeric variable.', array(), $options),
    'required' => TRUE,
    'group' => 'variable_example',
  );
  // Text with format
  $variables['variable_example_text_format'] = array(
    'type' => 'text_format',
    'title' => t('Text format', array(), $options),
    // The default value may be a string (default format will be added) or
    // an array with 'format' (format name) and 'value' (string) elements
    'default' => 'Example text with default format',
    'description' => t('Example of text variable with text format.', array(), $options),
    'required' => TRUE,
    'group' => 'variable_example',
  );
  // Text with format
  $variables['variable_example_mail_[mail_part]'] = array(
    'type' => 'mail_text',
    'title' => t('Example mail', array(), $options),
    'default' => array(
      'subject' => t('Example mail subject', array(), $options),
      'body' => t('Example mail body.', array(), $options),
    ),
    'description' => t('Example mail variable with subject and body.', array(), $options),
    'required' => TRUE,
    'group' => 'variable_example',
  );
  return $variables;
}

/**
 * Implements hook_variable_group_info().
 */
function variable_example_variable_group_info() {
  $groups['variable_example'] = array(
    'title' => t('Examples'),
    'description' => t('Variable examples of different types.'),
    'access' => 'administer site configuration',
    'path' => array('admin/config/system/variable/example'),
  );
  return $groups;
}

