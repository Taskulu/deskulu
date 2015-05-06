<?php
/**
 * @file
 * bef_test_content.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function bef_test_content_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'bef_test_view';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'BEF Test View';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'BEF Test View';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'better_exposed_filters';
  $handler->display->display_options['exposed_form']['options']['bef'] = array(
    'general' => array(
      'allow_secondary' => 0,
      'secondary_label' => 'Advanced options',
    ),
    'sort' => array(
      'bef_format' => 'bef_toggle_links',
      'advanced' => array(
        'collapsible' => 0,
        'collapsible_label' => 'Sort options',
        'combine' => 1,
        'combine_rewrite' => '',
        'reset' => 0,
        'reset_label' => '',
        'is_secondary' => 0,
      ),
    ),
    'pager' => array(
      'bef_format' => 'bef_links',
      'is_secondary' => 0,
    ),
    'field_bef_test_letters_value' => array(
      'bef_format' => 'default',
      'more_options' => array(
        'bef_select_all_none' => FALSE,
        'bef_collapsible' => 0,
        'is_secondary' => 0,
        'any_label' => 'Any Letter',
        'bef_filter_description' => '',
        'tokens' => array(
          'available' => array(
            0 => 'global_types',
          ),
        ),
        'rewrite' => array(
          'filter_rewrite_values' => '',
        ),
      ),
    ),
    'field_bef_test_numbers_value' => array(
      'bef_format' => 'default',
      'more_options' => array(
        'bef_select_all_none' => FALSE,
        'bef_collapsible' => 0,
        'is_secondary' => 0,
        'any_label' => 'Any Number',
        'bef_filter_description' => '',
        'tokens' => array(
          'available' => array(
            0 => 'global_types',
          ),
        ),
        'rewrite' => array(
          'filter_rewrite_values' => '',
        ),
      ),
    ),
    'type' => array(
      'bef_format' => 'default',
      'more_options' => array(
        'bef_select_all_none' => FALSE,
        'bef_collapsible' => 0,
        'is_secondary' => 0,
        'any_label' => 'Any Type',
        'bef_filter_description' => '',
        'tokens' => array(
          'available' => array(
            0 => 'global_types',
          ),
        ),
        'rewrite' => array(
          'filter_rewrite_values' => '',
        ),
      ),
    ),
    'field_bef_test_location_tid' => array(
      'bef_format' => 'default',
      'more_options' => array(
        'bef_select_all_none' => FALSE,
        'bef_select_all_none_nested' => FALSE,
        'bef_collapsible' => 0,
        'is_secondary' => 0,
        'any_label' => 'Anywhere',
        'bef_filter_description' => '',
        'tokens' => array(
          'available' => array(
            0 => 'global_types',
          ),
        ),
        'rewrite' => array(
          'filter_rewrite_values' => '',
        ),
      ),
    ),
  );
  $handler->display->display_options['exposed_form']['options']['input_required'] = 0;
  $handler->display->display_options['exposed_form']['options']['text_input_required_format'] = 'filtered_html';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['pager']['options']['expose']['items_per_page'] = TRUE;
  $handler->display->display_options['pager']['options']['expose']['items_per_page_label'] = 'Yo! Show me';
  $handler->display->display_options['pager']['options']['expose']['items_per_page_options_all'] = TRUE;
  $handler->display->display_options['pager']['options']['expose']['items_per_page_options_all_label'] = 'All dem things!';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
    'field_letters' => 'field_letters',
    'field_numbers' => 'field_numbers',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'title' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_letters' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_numbers' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Post date */
  $handler->display->display_options['fields']['created']['id'] = 'created';
  $handler->display->display_options['fields']['created']['table'] = 'node';
  $handler->display->display_options['fields']['created']['field'] = 'created';
  $handler->display->display_options['fields']['created']['date_format'] = 'short';
  /* Field: Content: Integer */
  $handler->display->display_options['fields']['field_bef_test_integer']['id'] = 'field_bef_test_integer';
  $handler->display->display_options['fields']['field_bef_test_integer']['table'] = 'field_data_field_bef_test_integer';
  $handler->display->display_options['fields']['field_bef_test_integer']['field'] = 'field_bef_test_integer';
  $handler->display->display_options['fields']['field_bef_test_integer']['settings'] = array(
    'thousand_separator' => ' ',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Letters */
  $handler->display->display_options['fields']['field_bef_test_letters']['id'] = 'field_bef_test_letters';
  $handler->display->display_options['fields']['field_bef_test_letters']['table'] = 'field_data_field_bef_test_letters';
  $handler->display->display_options['fields']['field_bef_test_letters']['field'] = 'field_bef_test_letters';
  $handler->display->display_options['fields']['field_bef_test_letters']['delta_offset'] = '0';
  /* Field: Content: Numbers */
  $handler->display->display_options['fields']['field_bef_test_numbers']['id'] = 'field_bef_test_numbers';
  $handler->display->display_options['fields']['field_bef_test_numbers']['table'] = 'field_data_field_bef_test_numbers';
  $handler->display->display_options['fields']['field_bef_test_numbers']['field'] = 'field_bef_test_numbers';
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  $handler->display->display_options['sorts']['created']['exposed'] = TRUE;
  $handler->display->display_options['sorts']['created']['expose']['label'] = 'Post date';
  /* Filter criterion: Content: Letters (field_bef_test_letters) */
  $handler->display->display_options['filters']['field_bef_test_letters_value']['id'] = 'field_bef_test_letters_value';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['table'] = 'field_data_field_bef_test_letters';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['field'] = 'field_bef_test_letters_value';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['exposed'] = TRUE;
  $handler->display->display_options['filters']['field_bef_test_letters_value']['expose']['operator_id'] = 'field_bef_test_letters_value_op';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['expose']['label'] = 'Letters';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['expose']['operator'] = 'field_bef_test_letters_value_op';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['expose']['identifier'] = 'field_bef_test_letters_value';
  $handler->display->display_options['filters']['field_bef_test_letters_value']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Content: Numbers (field_bef_test_numbers) */
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['id'] = 'field_bef_test_numbers_value';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['table'] = 'field_data_field_bef_test_numbers';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['field'] = 'field_bef_test_numbers_value';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['exposed'] = TRUE;
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['expose']['operator_id'] = 'field_bef_test_numbers_value_op';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['expose']['label'] = 'Numbers';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['expose']['operator'] = 'field_bef_test_numbers_value_op';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['expose']['identifier'] = 'field_bef_test_numbers_value';
  $handler->display->display_options['filters']['field_bef_test_numbers_value']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['exposed'] = TRUE;
  $handler->display->display_options['filters']['type']['expose']['operator_id'] = 'type_op';
  $handler->display->display_options['filters']['type']['expose']['label'] = 'Type';
  $handler->display->display_options['filters']['type']['expose']['operator'] = 'type_op';
  $handler->display->display_options['filters']['type']['expose']['identifier'] = 'type';
  $handler->display->display_options['filters']['type']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Content: Location (field_bef_test_location) */
  $handler->display->display_options['filters']['field_bef_test_location_tid']['id'] = 'field_bef_test_location_tid';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['table'] = 'field_data_field_bef_test_location';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['field'] = 'field_bef_test_location_tid';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['exposed'] = TRUE;
  $handler->display->display_options['filters']['field_bef_test_location_tid']['expose']['operator_id'] = 'field_bef_test_location_tid_op';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['expose']['label'] = 'Location';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['expose']['operator'] = 'field_bef_test_location_tid_op';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['expose']['identifier'] = 'field_bef_test_location_tid';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['field_bef_test_location_tid']['type'] = 'select';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['vocabulary'] = 'location';
  $handler->display->display_options['filters']['field_bef_test_location_tid']['hierarchy'] = 1;

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'test';

  /* Display: Content pane */
  $handler = $view->new_display('panel_pane', 'Content pane', 'panel_pane_1');
  $handler->display->display_options['exposed_block'] = TRUE;
  $export['bef_test_view'] = $view;

  return $export;
}
