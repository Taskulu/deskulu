<?php

/**
 * @file
 * Adds default views.
 */

/**
 * Implements hook_views_default_views().
 */
function google_fonts_api_views_default_views() {
  /*
   * View 'google_fonts_api_browse'
   */
  $view = new view;
  $view->name = 'google_fonts_api_browse';
  $view->description = 'Browsing interface for fonts.';
  $view->tag = '@font-your-face';
  $view->base_table = 'fontyourface_font';
  $view->human_name = 'Google Fonts API Browse';
  $view->core = 0;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Defaults */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->display->display_options['title'] = 'Browse fonts';
  $handler->display->display_options['css_class'] = 'view-fontyourface-browse';
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'administer @font-your-face';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['distinct'] = TRUE;
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['reset_button'] = TRUE;
  $handler->display->display_options['exposed_form']['options']['autosubmit'] = 1;
  $handler->display->display_options['exposed_form']['options']['autosubmit_hide'] = 0;
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '50';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['expose']['items_per_page_options_all'] = 0;
  $handler->display->display_options['style_plugin'] = 'list';
  $handler->display->display_options['style_options']['row_class'] = 'font';
  $handler->display->display_options['style_options']['class'] = 'fonts-listing';
  $handler->display->display_options['style_options']['wrapper_class'] = '';
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Header: Global: Text area */
  $handler->display->display_options['header']['area']['id'] = 'area';
  $handler->display->display_options['header']['area']['table'] = 'views';
  $handler->display->display_options['header']['area']['field'] = 'area';
  $handler->display->display_options['header']['area']['empty'] = TRUE;
  $handler->display->display_options['header']['area']['content'] = '0 fonts currently enabled.';
  $handler->display->display_options['header']['area']['format'] = 'filtered_html';
  $handler->display->display_options['header']['area']['tokenize'] = 0;
  /* Relationship: Font: Font Tag */
  $handler->display->display_options['relationships']['tid']['id'] = 'tid';
  $handler->display->display_options['relationships']['tid']['table'] = 'fontyourface_tag_font';
  $handler->display->display_options['relationships']['tid']['field'] = 'tid';
  $handler->display->display_options['relationships']['tid']['required'] = 0;
  /* Field: Font: Name */
  $handler->display->display_options['fields']['name_1']['id'] = 'name_1';
  $handler->display->display_options['fields']['name_1']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['name_1']['field'] = 'name';
  $handler->display->display_options['fields']['name_1']['label'] = '';
  $handler->display->display_options['fields']['name_1']['exclude'] = TRUE;
  $handler->display->display_options['fields']['name_1']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['name_1']['alter']['text'] = '<span>Edit</span>';
  $handler->display->display_options['fields']['name_1']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name_1']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name_1']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name_1']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name_1']['element_type'] = '0';
  $handler->display->display_options['fields']['name_1']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['name_1']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['name_1']['element_default_classes'] = 0;
  $handler->display->display_options['fields']['name_1']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name_1']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name_1']['hide_alter_empty'] = 0;
  $handler->display->display_options['fields']['name_1']['link_to_font'] = 'edit';
  $handler->display->display_options['fields']['name_1']['display_as'] = '0';
  /* Field: Font: Font ID */
  $handler->display->display_options['fields']['fid']['id'] = 'fid';
  $handler->display->display_options['fields']['fid']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['fid']['field'] = 'fid';
  $handler->display->display_options['fields']['fid']['label'] = '';
  $handler->display->display_options['fields']['fid']['exclude'] = TRUE;
  $handler->display->display_options['fields']['fid']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['external'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['fid']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['fid']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['fid']['alter']['html'] = 0;
  $handler->display->display_options['fields']['fid']['element_type'] = '0';
  $handler->display->display_options['fields']['fid']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['fid']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['fid']['element_default_classes'] = 0;
  $handler->display->display_options['fields']['fid']['hide_empty'] = 0;
  $handler->display->display_options['fields']['fid']['empty_zero'] = 0;
  $handler->display->display_options['fields']['fid']['hide_alter_empty'] = 0;
  $handler->display->display_options['fields']['fid']['separator'] = '';
  $handler->display->display_options['fields']['fid']['format_plural'] = 0;
  /* Field: Font: Enabled Yes/No */
  $handler->display->display_options['fields']['enabled_yes_no']['id'] = 'enabled_yes_no';
  $handler->display->display_options['fields']['enabled_yes_no']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['enabled_yes_no']['field'] = 'enabled_yes_no';
  $handler->display->display_options['fields']['enabled_yes_no']['label'] = '';
  $handler->display->display_options['fields']['enabled_yes_no']['exclude'] = TRUE;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['external'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['alter']['html'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['enabled_yes_no']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['enabled_yes_no']['hide_empty'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['empty_zero'] = 0;
  $handler->display->display_options['fields']['enabled_yes_no']['hide_alter_empty'] = 1;
  /* Field: Font: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['label'] = '';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['name']['alter']['text'] = '<div class="fontyourface-header enabled-[enabled_yes_no]"><h4>[name]</h4> [name_1]</div>';
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_type'] = '0';
  $handler->display->display_options['fields']['name']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['name']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['name']['element_default_classes'] = 0;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['hide_alter_empty'] = 0;
  $handler->display->display_options['fields']['name']['link_to_font'] = '0';
  $handler->display->display_options['fields']['name']['display_as'] = '0';
  /* Field: Font: Enable/Disable */
  $handler->display->display_options['fields']['enable_disable_link']['id'] = 'enable_disable_link';
  $handler->display->display_options['fields']['enable_disable_link']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['enable_disable_link']['field'] = 'enable_disable_link';
  $handler->display->display_options['fields']['enable_disable_link']['label'] = '';
  $handler->display->display_options['fields']['enable_disable_link']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['text'] = '<div class="fontyourface-footer enabled-[enabled_yes_no]" data-fid="[fid]">[enable_disable_link]</div>';
  $handler->display->display_options['fields']['enable_disable_link']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['external'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['alter']['html'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['element_type'] = '0';
  $handler->display->display_options['fields']['enable_disable_link']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['enable_disable_link']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['enable_disable_link']['element_default_classes'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['hide_empty'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['empty_zero'] = 0;
  $handler->display->display_options['fields']['enable_disable_link']['hide_alter_empty'] = 0;
  /* Field: Font: Preview */
  $handler->display->display_options['fields']['preview']['id'] = 'preview';
  $handler->display->display_options['fields']['preview']['table'] = 'fontyourface_font';
  $handler->display->display_options['fields']['preview']['field'] = 'preview';
  $handler->display->display_options['fields']['preview']['label'] = '';
  $handler->display->display_options['fields']['preview']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['preview']['alter']['text'] = '<div class="fontyourface-preview enabled-[enabled_yes_no]">[preview]</div>';
  $handler->display->display_options['fields']['preview']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['external'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['preview']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['preview']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['preview']['alter']['html'] = 0;
  $handler->display->display_options['fields']['preview']['element_type'] = '0';
  $handler->display->display_options['fields']['preview']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['preview']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['preview']['element_default_classes'] = 0;
  $handler->display->display_options['fields']['preview']['hide_empty'] = 0;
  $handler->display->display_options['fields']['preview']['empty_zero'] = 0;
  $handler->display->display_options['fields']['preview']['hide_alter_empty'] = 0;
  /* Sort criterion: Font: Name */
  $handler->display->display_options['sorts']['name']['id'] = 'name';
  $handler->display->display_options['sorts']['name']['table'] = 'fontyourface_font';
  $handler->display->display_options['sorts']['name']['field'] = 'name';
  /* Contextual filter: Font: Provider */
  $handler->display->display_options['arguments']['provider']['id'] = 'provider';
  $handler->display->display_options['arguments']['provider']['table'] = 'fontyourface_font';
  $handler->display->display_options['arguments']['provider']['field'] = 'provider';
  $handler->display->display_options['arguments']['provider']['title_enable'] = 1;
  $handler->display->display_options['arguments']['provider']['title'] = 'Browse fonts: %1';
  $handler->display->display_options['arguments']['provider']['breadcrumb_enable'] = 1;
  $handler->display->display_options['arguments']['provider']['breadcrumb'] = '@font-your-face %1';
  $handler->display->display_options['arguments']['provider']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['provider']['default_argument_skip_url'] = 0;
  $handler->display->display_options['arguments']['provider']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['provider']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['provider']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['provider']['glossary'] = 0;
  $handler->display->display_options['arguments']['provider']['limit'] = '0';
  $handler->display->display_options['arguments']['provider']['transform_dash'] = 0;
  $handler->display->display_options['arguments']['provider']['break_phrase'] = 0;
  /* Filter criterion: Font: Name */
  $handler->display->display_options['filters']['name']['id'] = 'name';
  $handler->display->display_options['filters']['name']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['name']['field'] = 'name';
  $handler->display->display_options['filters']['name']['operator'] = 'contains';
  $handler->display->display_options['filters']['name']['group'] = 0;
  $handler->display->display_options['filters']['name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['name']['expose']['operator_id'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['label'] = 'Name';
  $handler->display->display_options['filters']['name']['expose']['operator'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['identifier'] = 'name';
  /* Filter criterion: Tag */
  $handler->display->display_options['filters']['tid_3']['id'] = 'tid_3';
  $handler->display->display_options['filters']['tid_3']['table'] = 'fontyourface_tag';
  $handler->display->display_options['filters']['tid_3']['field'] = 'tid';
  $handler->display->display_options['filters']['tid_3']['relationship'] = 'tid';
  $handler->display->display_options['filters']['tid_3']['ui_name'] = 'Tag';
  $handler->display->display_options['filters']['tid_3']['group'] = 0;
  $handler->display->display_options['filters']['tid_3']['exposed'] = TRUE;
  $handler->display->display_options['filters']['tid_3']['expose']['operator_id'] = 'tid_3_op';
  $handler->display->display_options['filters']['tid_3']['expose']['label'] = 'Tag';
  $handler->display->display_options['filters']['tid_3']['expose']['operator'] = 'tid_3_op';
  $handler->display->display_options['filters']['tid_3']['expose']['identifier'] = 'tag_id';
  $handler->display->display_options['filters']['tid_3']['expose']['reduce'] = 0;
  $handler->display->display_options['filters']['tid_3']['reduce_duplicates'] = 0;
  $handler->display->display_options['filters']['tid_3']['type'] = 'select';
  $handler->display->display_options['filters']['tid_3']['tag_type'] = 'tag';
  /* Filter criterion: Font: Foundry */
  $handler->display->display_options['filters']['foundry']['id'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['foundry']['field'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['group'] = 0;
  $handler->display->display_options['filters']['foundry']['exposed'] = TRUE;
  $handler->display->display_options['filters']['foundry']['expose']['operator_id'] = 'foundry_op';
  $handler->display->display_options['filters']['foundry']['expose']['label'] = 'Foundry';
  $handler->display->display_options['filters']['foundry']['expose']['operator'] = 'foundry_op';
  $handler->display->display_options['filters']['foundry']['expose']['identifier'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['expose']['reduce'] = 0;
  /* Filter criterion: Font: License */
  $handler->display->display_options['filters']['license']['id'] = 'license';
  $handler->display->display_options['filters']['license']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['license']['field'] = 'license';
  $handler->display->display_options['filters']['license']['operator'] = 'contains';
  $handler->display->display_options['filters']['license']['group'] = 0;
  $handler->display->display_options['filters']['license']['exposed'] = TRUE;
  $handler->display->display_options['filters']['license']['expose']['operator_id'] = 'license_op';
  $handler->display->display_options['filters']['license']['expose']['label'] = 'License';
  $handler->display->display_options['filters']['license']['expose']['operator'] = 'license_op';
  $handler->display->display_options['filters']['license']['expose']['identifier'] = 'license';
  $handler->display->display_options['filters']['license']['expose']['required'] = 0;
  $handler->display->display_options['filters']['license']['expose']['multiple'] = FALSE;
  
  /* Display: Google Fonts */
  $handler = $view->new_display('page', 'Google Fonts', 'page_2');
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Font: Name */
  $handler->display->display_options['filters']['name']['id'] = 'name';
  $handler->display->display_options['filters']['name']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['name']['field'] = 'name';
  $handler->display->display_options['filters']['name']['operator'] = 'contains';
  $handler->display->display_options['filters']['name']['group'] = 0;
  $handler->display->display_options['filters']['name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['name']['expose']['operator_id'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['label'] = 'Name';
  $handler->display->display_options['filters']['name']['expose']['operator'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['identifier'] = 'name';
  /* Filter criterion: Tag */
  $handler->display->display_options['filters']['tid_3']['id'] = 'tid_3';
  $handler->display->display_options['filters']['tid_3']['table'] = 'fontyourface_tag';
  $handler->display->display_options['filters']['tid_3']['field'] = 'tid';
  $handler->display->display_options['filters']['tid_3']['relationship'] = 'tid';
  $handler->display->display_options['filters']['tid_3']['ui_name'] = 'Tag';
  $handler->display->display_options['filters']['tid_3']['group'] = 0;
  $handler->display->display_options['filters']['tid_3']['exposed'] = TRUE;
  $handler->display->display_options['filters']['tid_3']['expose']['operator_id'] = 'tid_3_op';
  $handler->display->display_options['filters']['tid_3']['expose']['label'] = 'Tag';
  $handler->display->display_options['filters']['tid_3']['expose']['operator'] = 'tid_3_op';
  $handler->display->display_options['filters']['tid_3']['expose']['identifier'] = 'tag_id';
  $handler->display->display_options['filters']['tid_3']['expose']['reduce'] = 0;
  $handler->display->display_options['filters']['tid_3']['reduce_duplicates'] = 0;
  $handler->display->display_options['filters']['tid_3']['type'] = 'select';
  $handler->display->display_options['filters']['tid_3']['tag_type'] = 'tag';
  /* Filter criterion: Subset */
  $handler->display->display_options['filters']['tid']['id'] = 'tid';
  $handler->display->display_options['filters']['tid']['table'] = 'fontyourface_tag';
  $handler->display->display_options['filters']['tid']['field'] = 'tid';
  $handler->display->display_options['filters']['tid']['relationship'] = 'tid';
  $handler->display->display_options['filters']['tid']['ui_name'] = 'Subset';
  $handler->display->display_options['filters']['tid']['group'] = 0;
  $handler->display->display_options['filters']['tid']['exposed'] = TRUE;
  $handler->display->display_options['filters']['tid']['expose']['operator_id'] = 'tid_op';
  $handler->display->display_options['filters']['tid']['expose']['label'] = 'Subset';
  $handler->display->display_options['filters']['tid']['expose']['operator'] = 'tid_op';
  $handler->display->display_options['filters']['tid']['expose']['identifier'] = 'subset_id';
  $handler->display->display_options['filters']['tid']['expose']['reduce'] = 0;
  $handler->display->display_options['filters']['tid']['reduce_duplicates'] = 0;
  $handler->display->display_options['filters']['tid']['type'] = 'select';
  $handler->display->display_options['filters']['tid']['tag_type'] = 'subset';
  /* Filter criterion: Font: Foundry */
  $handler->display->display_options['filters']['foundry']['id'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['foundry']['field'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['group'] = 0;
  $handler->display->display_options['filters']['foundry']['exposed'] = TRUE;
  $handler->display->display_options['filters']['foundry']['expose']['operator_id'] = 'foundry_op';
  $handler->display->display_options['filters']['foundry']['expose']['label'] = 'Foundry';
  $handler->display->display_options['filters']['foundry']['expose']['operator'] = 'foundry_op';
  $handler->display->display_options['filters']['foundry']['expose']['identifier'] = 'foundry';
  $handler->display->display_options['filters']['foundry']['expose']['reduce'] = 0;
  /* Filter criterion: Font: License */
  $handler->display->display_options['filters']['license']['id'] = 'license';
  $handler->display->display_options['filters']['license']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['license']['field'] = 'license';
  $handler->display->display_options['filters']['license']['operator'] = 'contains';
  $handler->display->display_options['filters']['license']['group'] = 0;
  $handler->display->display_options['filters']['license']['exposed'] = TRUE;
  $handler->display->display_options['filters']['license']['expose']['operator_id'] = 'license_op';
  $handler->display->display_options['filters']['license']['expose']['label'] = 'License';
  $handler->display->display_options['filters']['license']['expose']['operator'] = 'license_op';
  $handler->display->display_options['filters']['license']['expose']['identifier'] = 'license';
  $handler->display->display_options['filters']['license']['expose']['required'] = 0;
  $handler->display->display_options['filters']['license']['expose']['multiple'] = FALSE;
  /* Filter criterion: Font: Provider */
  $handler->display->display_options['filters']['provider']['id'] = 'provider';
  $handler->display->display_options['filters']['provider']['table'] = 'fontyourface_font';
  $handler->display->display_options['filters']['provider']['field'] = 'provider';
  $handler->display->display_options['filters']['provider']['value'] = array(
    'google_fonts_api' => 'google_fonts_api',
  );
  $handler->display->display_options['path'] = 'admin/appearance/fontyourface/browse/google_fonts_api';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Google Fonts';
  $handler->display->display_options['menu']['weight'] = '0';
  $translatables['google_fonts_api_browse'] = array(
    t('Defaults'),
    t('Browse fonts'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('0 fonts currently enabled.'),
    t('Font Tag'),
    t('<span>Edit</span>'),
    t('.'),
    t('<div class="fontyourface-header enabled-[enabled_yes_no]"><h4>[name]</h4> [name_1]</div>'),
    t('<div class="fontyourface-footer enabled-[enabled_yes_no]" data-fid="[fid]">[enable_disable_link]</div>'),
    t('<div class="fontyourface-preview enabled-[enabled_yes_no]">[preview]</div>'),
    t('All'),
    t('Browse fonts: %1'),
    t('@font-your-face %1'),
    t('Name'),
    t('Tag'),
    t('Foundry'),
    t('License'),
    t('Google Fonts'),
    t('Subset'),
  );

  $views[$view->name] = $view;

  return $views;

} // google_fonts_api_views_default_views
