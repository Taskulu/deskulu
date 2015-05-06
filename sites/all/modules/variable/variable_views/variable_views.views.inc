<?php
/**
 * @file
 * Views hooks for variables.
 */
/**
 * Implements hook_views_data().
 */
function variable_views_views_data() {
  // Basic table information.

  // Define the base group of this table.
  $data['variable']['table']['group']  = t('Variable');

  // Advertise this table as a possible base table.
  $data['variable']['table']['base'] = array(
    'field' => 'name',
    'title' => t('Variable'),
    'help' => t('Variables from the system variable table.'),
  );
  // Variable name
  $data['variable']['name'] = array(
    'title' => t('Name'),
    'help' => t('The low level name of the variable.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // Varaible value
  $data['variable']['value'] = array(
    'title' => t('Value'),
    'help' => t('The value of the variable.'),
    'field' => array(
      'handler' => 'views_handler_field_variable_value',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // Variable title
  $data['variable']['title'] = array(
    'group' => t('Variable'),
    'real field' => 'name',
    'field' => array(
      'title' => t('Title'),
      'help' => t('Human readable variable name.'),
      'handler' => 'views_handler_field_variable_title',
    ),
  );
  return $data;
}
