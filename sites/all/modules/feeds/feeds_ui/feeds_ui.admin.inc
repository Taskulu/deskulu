<?php

/**
 * @file
 * Contains all page callbacks, forms and theming functions for Feeds
 * administrative pages.
 */

/**
 * Introductory help for admin/structure/feeds/%feeds_importer page
 */
function feeds_ui_edit_help() {
  return t('
    <p>
    You can create as many Feeds importer configurations as you would like to. Each can have a distinct purpose like letting your users aggregate RSS feeds or importing a CSV file for content migration. Here are a couple of things that are important to understand in order to get started with Feeds:
    </p>
    <ul>
    <li>
    Every importer configuration consists of basic settings, a fetcher, a parser and a processor and their settings.
    </li>
    <li>
    The <strong>basic settings</strong> define the general behavior of the importer. <strong>Fetchers</strong> are responsible for loading data, <strong>parsers</strong> for organizing it and <strong>processors</strong> for "doing stuff" with it, usually storing it.
    </li>
    <li>
    In Basic settings, you can <strong>attach an importer configuration to a content type</strong>. This is useful when many imports of a kind should be created, for example in an RSS aggregation scenario. If you don\'t attach a configuration to a content type, you can use it on the !import page.
    </li>
    <li>
    Imports can be <strong>scheduled periodically</strong> - see the periodic import select box in the Basic settings.
    </li>
    <li>
    Processors can have <strong>mappings</strong> in addition to settings. Mappings allow you to define what elements of a data feed should be mapped to what content fields on a granular level. For instance, you can specify that a feed item\'s author should be mapped to a node\'s body.
    </li>
    </ul>
    ', array('!import' => l(t('Import'), 'import')));
}

/**
 * Help text for mapping.
 */
function feeds_ui_mapping_help() {
  return t('
  <p>
  Define which elements of a single item of a feed (= <em>Sources</em>) map to which content pieces in Drupal (= <em>Targets</em>). Make sure that at least one definition has a <em>Unique target</em>. A unique target means that a value for a target can only occur once. E. g. only one item with the URL <em>http://example.com/content/1</em> can exist.
  </p>
  ');
}

/**
 * Build overview of available configurations.
 */
function feeds_ui_overview_form($form, &$form_status) {
  $form = $form['enabled'] = $form['disabled'] = array();

  $form['#header'] = array(
    t('Name'),
    t('Description'),
    t('Attached to'),
    t('Status'),
    t('Operations'),
    t('Enabled'),
  );
  foreach (feeds_importer_load_all(TRUE) as $importer) {
    $importer_form = array();
    $importer_form['name']['#markup'] = check_plain($importer->config['name']);
    $importer_form['description']['#markup'] = check_plain($importer->config['description']);
    if (empty($importer->config['content_type'])) {
      $importer_form['attached']['#markup'] = '[none]';
    }
    else {
      if (!$importer->disabled) {
        $importer_form['attached']['#markup'] = l(node_type_get_name($importer->config['content_type']), 'node/add/' . str_replace('_', '-', $importer->config['content_type']));
      }
      else {
        $importer_form['attached']['#markup'] = check_plain(node_type_get_name($importer->config['content_type']));
      }
    }

    if ($importer->export_type == EXPORT_IN_CODE) {
      $status = t('Default');
      $edit = t('Override');
      $delete = '';
    }
    elseif ($importer->export_type == EXPORT_IN_DATABASE) {
      $status = t('Normal');
      $edit = t('Edit');
      $delete = t('Delete');
    }
    elseif ($importer->export_type == (EXPORT_IN_CODE | EXPORT_IN_DATABASE)) {
      $status = t('Overridden');
      $edit = t('Edit');
      $delete = t('Revert');
    }
    $importer_form['status'] = array(
      '#markup' => $status,
    );
    $importer_form['operations'] = array(
      '#markup' =>
        l($edit, 'admin/structure/feeds/' . $importer->id) . ' | ' .
        l(t('Export'), 'admin/structure/feeds/' . $importer->id . '/export') . ' | ' .
        l(t('Clone'), 'admin/structure/feeds/' . $importer->id . '/clone') .
        (empty($delete) ? '' :  ' | ' . l($delete, 'admin/structure/feeds/' . $importer->id . '/delete')),
    );

    $importer_form[$importer->id] = array(
      '#type' => 'checkbox',
      '#default_value' => !$importer->disabled,
    );

    if ($importer->disabled) {
      $form['disabled'][$importer->id] = $importer_form;
    }
    else {
      $form['enabled'][$importer->id] = $importer_form;
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for feeds_ui_overview_form().
 */
function feeds_ui_overview_form_submit($form, &$form_state) {

  $disabled = array();
  foreach (feeds_importer_load_all(TRUE) as $importer) {
    $disabled[$importer->id] = !$form_state['values'][$importer->id];
  }
  variable_set('default_feeds_importer', $disabled);
  feeds_cache_clear();
}

/**
 * Create a new configuration.
 *
 * @param $form_state
 *  Form API form state array.
 * @param $from_importer
 *   FeedsImporter object. If given, form will create a new importer as a copy
 *   of $from_importer.
 */
function feeds_ui_create_form($form, &$form_state, $from_importer = NULL) {
  $form['#from_importer'] = $from_importer;
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A natural name for this configuration. Example: RSS Feed. You can always change this name later.'),
    '#required' => TRUE,
    '#maxlength' => 128,
  );
  $form['id'] = array(
    '#type' => 'machine_name',
    '#required' => TRUE,
    '#maxlength' => 128,
    '#machine_name' => array(
      'exists' => 'feeds_ui_importer_machine_name_exists',
    ),
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('A description of this configuration.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  return $form;
}

/**
 * Validation callback for the importer machine name field.
 */
function feeds_ui_importer_machine_name_exists($id) {
  if ($id == 'create') {
    // Create is a reserved path for the add importer form.
    return TRUE;
  }
  ctools_include('export');
  if (ctools_export_load_object('feeds_importer', 'conditions', array('id' => $id))) {
    return TRUE;
  }
}

/**
 * Validation handler for feeds_build_create_form().
 */
function feeds_ui_create_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['id'])) {
    $importer = feeds_importer($form_state['values']['id']);
    $importer->configFormValidate($form_state['values']);
  }
}

/**
 * Submit handler for feeds_build_create_form().
 */
function feeds_ui_create_form_submit($form, &$form_state) {
  // Create feed.
  $importer = feeds_importer($form_state['values']['id']);
  // If from_importer is given, copy its configuration.
  if (!empty($form['#from_importer'])) {
    $importer->copy($form['#from_importer']);
  }
  // In any case, we want to set this configuration's title and description.
  $importer->addConfig($form_state['values']);
  $importer->save();

  // Set a message and redirect to settings form.
  if (empty($form['#from_importer'])) {
    drupal_set_message(t('Your configuration has been created with default settings. If they do not fit your use case you can adjust them here.'));
  }
  else {
    drupal_set_message(t('A clone of the @name configuration has been created.', array('@name' => $form['#from_importer']->config['name'])));
  }
  $form_state['redirect'] = 'admin/structure/feeds/' . $importer->id;
  feeds_cache_clear();
}

/**
 * Delete configuration form.
 */
function feeds_ui_delete_form($form, &$form_state, $importer) {
  $form['#importer'] = $importer->id;
  if ($importer->export_type & EXPORT_IN_CODE) {
    $title = t('Would you really like to revert the importer @importer?', array('@importer' => $importer->config['name']));
    $button_label = t('Revert');
  }
  else {
    $title = t('Would you really like to delete the importer @importer?', array('@importer' => $importer->config['name']));
    $button_label = t('Delete');
  }
  return confirm_form(
    $form,
    $title,
    'admin/structure/feeds',
    t('This action cannot be undone.'),
    $button_label
  );
}

/**
 * Submit handler for feeds_ui_delete_form().
 */
function feeds_ui_delete_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/feeds';

  // Remove importer.
  feeds_importer($form['#importer'])->delete();

  // Clear cache, deleting a configuration may have an affect on menu tree.
  feeds_cache_clear();
}

/**
 * Export a feed configuration.
 */
function feeds_ui_export_form($form, &$form_state, $importer) {
  $code = feeds_export($importer->id);

  $form['export'] = array(
    '#title' => t('Export feed configuration'),
    '#type' => 'textarea',
    '#value' => $code,
    '#rows' => substr_count($code, "\n"),
  );
  return $form;
}

/**
 * Edit feed configuration.
 */
function feeds_ui_edit_page(FeedsImporter $importer, $active = 'help', $plugin_key = '') {
  // Get plugins and configuration.
  $plugins = FeedsPlugin::all();

  $config = $importer->config;
  // Base path for changing the active container.
  $path = 'admin/structure/feeds/' . $importer->id;

  $active_container = array(
    'class' => array('active-container'),
    'actions' => array(l(t('Help'), $path)),
  );
  switch ($active) {
    case 'help':
      $active_container['title'] = t('Getting started');
      $active_container['body'] = '<div class="help feeds-admin-ui">' . feeds_ui_edit_help() . '</div>';
      unset($active_container['actions']);
      break;

    case 'fetcher':
    case 'parser':
    case 'processor':
      $active_container['title'] = t('Select a !plugin_type', array('!plugin_type' => $active));
      $active_container['body'] = drupal_get_form('feeds_ui_plugin_form', $importer, $active);
      break;

    case 'settings':
      drupal_add_js(drupal_get_path('module', 'ctools') . '/js/dependent.js');
      ctools_include('dependent');
      if (empty($plugin_key)) {
        $active_container['title'] = t('Basic settings');
        $active_container['body'] = feeds_get_form($importer, 'configForm');
      }
      // feeds_plugin() returns a correct result because feed has been
      // instantiated previously.
      elseif (in_array($plugin_key, array_keys($plugins)) && $plugin = feeds_plugin($plugin_key, $importer->id)) {
        $active_container['title'] = t('Settings for !plugin', array('!plugin' => $plugins[$plugin_key]['name']));
        $active_container['body'] = feeds_get_form($plugin, 'configForm');
      }
      break;

    case 'mapping':
      $processor_name = isset($plugins[$config['processor']['plugin_key']]['name']) ? $plugins[$config['processor']['plugin_key']]['name'] : $plugins['FeedsMissingPlugin']['name'];
      $active_container['title'] = t('Mapping for @processor', array('@processor' => $processor_name));
      $active_container['body'] = drupal_get_form('feeds_ui_mapping_form', $importer);
      break;
  }

  // Build config info.
  $config_info = $info = array();
  $info['class'] = array('config-set');

  // Basic information.
  $items = array();
  $items[] = t('Attached to: @type', array('@type' => $importer->config['content_type'] ? node_type_get_name($importer->config['content_type']) : t('[none]')));
  if ($importer->config['import_period'] == FEEDS_SCHEDULE_NEVER) {
    $import_period = t('off');
  }
  elseif ($importer->config['import_period'] == 0) {
    $import_period = t('as often as possible');
  }
  else {
    $import_period = t('every !interval', array('!interval' => format_interval($importer->config['import_period'])));
  }
  $items[] = t('Periodic import: !import_period', array('!import_period' => $import_period));
  $items[] = $importer->config['import_on_create'] ? t('Import on submission') : t('Do not import on submission');

  $info['title'] = t('Basic settings');
  $info['body'] = array(
    array(
      'body' => theme('item_list', array('items' => $items)),
      'actions' => array(l(t('Settings'), $path . '/settings')),
    ),
  );
  $config_info[] = $info;

  // Fetcher.
  $fetcher = isset($plugins[$config['fetcher']['plugin_key']]) ? $plugins[$config['fetcher']['plugin_key']] : $plugins['FeedsMissingPlugin'];
  $actions = array();
  if ($importer->fetcher->hasConfigForm()) {
    $actions = array(l(t('Settings'), $path . '/settings/' . $config['fetcher']['plugin_key']));
  }
  $info['title'] = t('Fetcher');
  $info['body'] = array(
    array(
      'title' => $fetcher['name'],
      'body' => $fetcher['description'],
      'actions' => $actions,
    ),
  );
  $info['actions'] = array(l(t('Change'), $path . '/fetcher'));
  $config_info[] = $info;

  // Parser.
  $parser = isset($plugins[$config['parser']['plugin_key']]) ? $plugins[$config['parser']['plugin_key']] : $plugins['FeedsMissingPlugin'];
  $actions = array();
  if ($importer->parser->hasConfigForm()) {
    $actions = array(l(t('Settings'), $path . '/settings/' . $config['parser']['plugin_key']));
  }
  $info['title'] = t('Parser');
  $info['body'] = array(
    array(
      'title' => $parser['name'],
      'body' => $parser['description'],
      'actions' => $actions,
    )
  );
  $info['actions'] = array(l(t('Change'), $path . '/parser'));
  $config_info[] = $info;

  // Processor.
  $processor = isset($plugins[$config['processor']['plugin_key']]) ? $plugins[$config['processor']['plugin_key']] : $plugins['FeedsMissingPlugin'];
  $actions = array();
  if ($importer->processor->hasConfigForm()) {
    $actions[] = l(t('Settings'), $path . '/settings/' . $config['processor']['plugin_key']);
  }
  $actions[] = l(t('Mapping'), $path . '/mapping');
  $info['title'] = t('Processor');
  $info['body'] = array(
    array(
      'title' => $processor['name'],
      'body' => $processor['description'],
      'actions' => $actions,
    )
  );
  $info['actions'] = array(l(t('Change'), $path . '/processor'));
  $config_info[] = $info;

  return theme('feeds_ui_edit_page', array(
    'info' => $config_info,
    'active' => $active_container,
  ));
}

/**
 * Build a form of plugins to pick from.
 *
 * @param $form_state
 *   Form API form state array.
 * @param $importer
 *   FeedsImporter object.
 * @param $type
 *   Plugin type. One of 'fetcher', 'parser', 'processor'.
 *
 * @return
 *   A Form API form definition.
 */
function feeds_ui_plugin_form($form, &$form_state, $importer, $type) {
  $plugins = FeedsPlugin::byType($type);

  $form['#importer'] = $importer->id;
  $form['#plugin_type'] = $type;

  $importer_key = $importer->config[$type]['plugin_key'];

  foreach ($plugins as $key => $plugin) {

    $form['plugin_key'][$key] = array(
      '#type' => 'radio',
      '#parents' => array('plugin_key'),
      '#title' => check_plain($plugin['name']),
      '#description' => filter_xss(isset($plugin['help']) ? $plugin['help'] : $plugin['description']),
      '#return_value' => $key,
      '#default_value' => ($key == $importer_key) ? $key : '',
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for feeds_ui_plugin_form().
 */
function feeds_ui_plugin_form_submit($form, &$form_state) {
  // Set the plugin and save feed.
  $importer = feeds_importer($form['#importer']);
  $importer->setPlugin($form_state['values']['plugin_key']);
  $importer->save();
  drupal_set_message(t('Changed @type plugin.', array('@type' => $form['#plugin_type'])));
}

/**
 * Theme feeds_ui_plugin_form().
 */
function theme_feeds_ui_plugin_form($variables) {
  $form = $variables['form'];
  $output = '';

  foreach (element_children($form['plugin_key']) as $key) {

    // Assemble container, render form elements.
    $container = array(
      'title' => $form['plugin_key'][$key]['#title'],
      'body' => isset($form['plugin_key'][$key]['#description']) ? $form['plugin_key'][$key]['#description'] : '',
    );
    $form['plugin_key'][$key]['#title'] = t('Select');
    $form['plugin_key'][$key]['#attributes']['class'] = array('feeds-ui-radio-link');
    unset($form['plugin_key'][$key]['#description']);
    $container['actions'] = array(drupal_render($form['plugin_key'][$key]));

    $output .= theme('feeds_ui_container', array('container' => $container));
  }

  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Edit mapping.
 *
 * @todo Completely merge this into config form handling. This is just a
 *   shared form of configuration, most of the common functionality can live in
 *   FeedsProcessor, a flag can tell whether mapping is supported or not.
 */
function feeds_ui_mapping_form($form, &$form_state, $importer) {
  $form['#importer'] = $importer->id;
  $form['#mappings'] = $mappings = $importer->processor->getMappings();
  $form['help']['#markup'] = feeds_ui_mapping_help();
  $form['#prefix'] = '<div id="feeds-ui-mapping-form-wrapper">';
  $form['#suffix'] = '</div>';

  // Show message when target configuration gets changed.
  if (!empty($form_state['mapping_settings'])) {
    $form['#mapping_settings'] = $form_state['mapping_settings'];
    $form['changed'] = array(
      '#theme_wrappers' => array('container'),
      '#attributes' => array('class' => array('messages', 'warning')),
      '#markup' => t('* Changes made to target configuration are stored temporarily. Click Save to make your changes permanent.'),
    );
  }

  // Get mapping sources from parsers and targets from processor, format them
  // for output.
  // Some parsers do not define mapping sources but let them define on the fly.
  if ($sources = $importer->parser->getMappingSources()) {
    $source_options = _feeds_ui_format_options($sources);
    foreach ($sources as $k => $source) {
      $legend['sources'][$k]['name']['#markup'] = empty($source['name']) ? $k : $source['name'];
      $legend['sources'][$k]['description']['#markup'] = empty($source['description']) ? '' : $source['description'];
    }
  }
  else {
    $legend['sources']['#markup'] = t('This parser supports free source definitions. Enter the name of the source field in lower case into the Source text field above.');
  }
  $targets = $importer->processor->getMappingTargets();
  $target_options = _feeds_ui_format_options($targets);
  $legend['targets'] = array();
  foreach ($targets as $k => $target) {
    $legend['targets'][$k]['name']['#markup'] = empty($target['name']) ? $k : $target['name'];
    $legend['targets'][$k]['description']['#markup'] = empty($target['description']) ? '' : $target['description'];
  }

  // Legend explaining source and target elements.
  $form['legendset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Legend'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['legendset']['legend'] = $legend;

  // Add config forms and remove flags to mappings.
  $form['config'] = $form['remove_flags'] = $form['mapping_weight'] = array(
    '#tree' => TRUE,
  );
  if (is_array($mappings)) {

    $delta = count($mappings) + 2;

    foreach ($mappings as $i => $mapping) {
      if (isset($targets[$mapping['target']])) {
        $form['config'][$i] = feeds_ui_mapping_settings_form($form, $form_state, $i, $mapping, $targets[$mapping['target']]);
      }

      $form['remove_flags'][$i] = array(
        '#type' => 'checkbox',
        '#title' => t('Remove'),
        '#prefix' => '<div class="feeds-ui-checkbox-link">',
        '#suffix' => '</div>',
      );

      $form['mapping_weight'][$i] = array(
        '#type' => 'weight',
        '#title' => '',
        '#default_value' => $i,
        '#delta' => $delta,
        '#attributes' => array(
          'class' => array(
            'feeds-ui-mapping-weight'
          ),
        ),
      );
    }
  }

  if (isset($source_options)) {
    $form['source'] = array(
      '#type' => 'select',
      '#title' => t('Source'),
      '#title_display' => 'invisible',
      '#options' => $source_options,
      '#empty_option' => t('- Select a source -'),
      '#description' => t('An element from the feed.'),
    );
  }
  else {
    $form['source'] = array(
      '#type' => 'textfield',
      '#title' => t('Source'),
      '#title_display' => 'invisible',
      '#size' => 20,
      '#default_value' => '',
      '#description' => t('The name of source field.'),
    );
  }
  $form['target'] = array(
    '#type' => 'select',
    '#title' => t('Target'),
    '#title_display' => 'invisible',
    '#options' => $target_options,
    '#empty_option' => t('- Select a target -'),
    '#description' => t('The field that stores the data.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Per mapper configuration form that is a part of feeds_ui_mapping_form().
 */
function feeds_ui_mapping_settings_form($form, $form_state, $i, $mapping, $target) {
  $form_state += array(
    'mapping_settings_edit' => NULL,
    'mapping_settings' => array(),
  );

  $base_button = array(
    '#submit' => array('feeds_ui_mapping_form_multistep_submit'),
    '#ajax' => array(
      'callback' => 'feeds_ui_mapping_settings_form_callback',
      'wrapper' => 'feeds-ui-mapping-form-wrapper',
      'effect' => 'fade',
      'progress' => 'none',
    ),
    '#i' => $i,
  );

  if (isset($form_state['mapping_settings'][$i])) {
    $mapping = $form_state['mapping_settings'][$i] + $mapping;
  }

  if ($form_state['mapping_settings_edit'] === $i) {
    $settings_form = array();

    foreach ($target['form_callbacks'] as $callback) {
      $settings_form += call_user_func($callback, $mapping, $target, $form, $form_state);
    }

    // Merge in the optional unique form.
    $settings_form += feeds_ui_mapping_settings_optional_unique_form($mapping, $target, $form, $form_state);

    return array(
      '#type' => 'container',
      'settings' => $settings_form,
      'save_settings' => $base_button + array(
        '#type' => 'submit',
        '#name' => 'mapping_settings_update_' . $i,
        '#value' => t('Update'),
        '#op' => 'update',
      ),
      'cancel_settings' => $base_button + array(
        '#type' => 'submit',
        '#name' => 'mapping_settings_cancel_' . $i,
        '#value' => t('Cancel'),
        '#op' => 'cancel',
      ),
    );
  }
  else {
    // Build the summary.
    $summary = array();

    foreach ($target['summary_callbacks'] as $callback) {
      $summary[] = call_user_func($callback, $mapping, $target, $form, $form_state);
    }

    // Filter out empty summary values.
    $summary = implode('<br />', array_filter($summary));

    // Append the optional unique summary.
    if ($optional_unique_summary = feeds_ui_mapping_settings_optional_unique_summary($mapping, $target, $form, $form_state)) {
      $summary .= ' ' . $optional_unique_summary;
    }

    if ($summary) {
      return array(
        'summary' => array(
          '#prefix' => '<div>',
          '#markup' => $summary,
          '#suffix' => '</div>',
        ),
       'edit_settings' => $base_button + array(
          '#type' => 'image_button',
          '#name' => 'mapping_settings_edit_' . $i,
          '#src' => 'misc/configure.png',
          '#attributes' => array('alt' => t('Edit')),
          '#op' => 'edit',
        ),
      );
    }
  }
  return array();
}

/**
 * Submit callback for a per mapper configuration form. Switches between edit
 * and summary mode.
 */
function feeds_ui_mapping_form_multistep_submit($form, &$form_state) {
  $trigger = $form_state['triggering_element'];

  switch ($trigger['#op']) {
    case 'edit':
      $form_state['mapping_settings_edit'] = $trigger['#i'];
      break;

    case 'update':
      $values = $form_state['values']['config'][$trigger['#i']]['settings'];
      $form_state['mapping_settings'][$trigger['#i']] = $values;
      unset($form_state['mapping_settings_edit']);
      break;

    case 'cancel':
      unset($form_state['mapping_settings_edit']);
      break;
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX callback that returns the whole feeds_ui_mapping_form().
 */
function feeds_ui_mapping_settings_form_callback($form, $form_state) {
  return $form;
}

/**
 * Validation handler for feeds_ui_mapping_form().
 */
function feeds_ui_mapping_form_validate($form, &$form_state) {
  if (!strlen($form_state['values']['source']) xor !strlen($form_state['values']['target'])) {

    // Check triggering_element here so we can react differently for ajax
    // submissions.
    switch ($form_state['triggering_element']['#name']) {

      // Regular form submission.
      case 'op':
        if (!strlen($form_state['values']['source'])) {
          form_error($form['source'], t('You must select a mapping source.'));
        }
        else {
          form_error($form['target'], t('You must select a mapping target.'));
        }
        break;

      // Be more relaxed on ajax submission.
      default:
        form_set_value($form['source'], '', $form_state);
        form_set_value($form['target'], '', $form_state);
        break;
    }
  }
}

/**
 * Submission handler for feeds_ui_mapping_form().
 */
function feeds_ui_mapping_form_submit($form, &$form_state) {
  $importer = feeds_importer($form['#importer']);
  $processor = $importer->processor;

  $form_state += array(
    'mapping_settings' => array(),
    'mapping_settings_edit' => NULL,
  );

  // If an item is in edit mode, prepare it for saving.
  if ($form_state['mapping_settings_edit'] !== NULL) {
    $values = $form_state['values']['config'][$form_state['mapping_settings_edit']]['settings'];
    $form_state['mapping_settings'][$form_state['mapping_settings_edit']] = $values;
  }

  // We may set some settings to mappings that we remove in the subsequent step,
  // that's fine.
  $mappings = $form['#mappings'];
  foreach ($form_state['mapping_settings'] as $k => $v) {
    $mappings[$k] = array(
      'source' => $mappings[$k]['source'],
      'target' => $mappings[$k]['target'],
    ) + $v;
  }

  if (!empty($form_state['values']['remove_flags'])) {
    $remove_flags = array_keys(array_filter($form_state['values']['remove_flags']));

    foreach ($remove_flags as $k) {
      unset($mappings[$k]);
      unset($form_state['values']['mapping_weight'][$k]);
      drupal_set_message(t('Mapping has been removed.'), 'status', FALSE);
    }
  }

  // Keep our keys clean.
  $mappings = array_values($mappings);

  if (!empty($mappings)) {
    array_multisort($form_state['values']['mapping_weight'], $mappings);
  }

  $processor->addConfig(array('mappings' => $mappings));

  if (strlen($form_state['values']['source']) && strlen($form_state['values']['target'])) {
    try {
      $mappings = $processor->getMappings();
      $mappings[] = array(
        'source' => $form_state['values']['source'],
        'target' => $form_state['values']['target'],
        'unique' => FALSE,
      );
      $processor->addConfig(array('mappings' => $mappings));
      drupal_set_message(t('Mapping has been added.'));
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
  }

  $importer->save();
  drupal_set_message(t('Your changes have been saved.'));
}

/**
 * Walk the result of FeedsParser::getMappingSources() or
 * FeedsProcessor::getMappingTargets() and format them into
 * a Form API options array.
 */
function _feeds_ui_format_options($options) {
  $result = array();
  foreach ($options as $k => $v) {
    if (is_array($v) && !empty($v['name'])) {
      $result[$k] = $v['name'] . ' (' . $k . ')';
    }
    elseif (is_array($v)) {
      $result[$k] = $k;
    }
    else {
      $result[$k] = $v;
    }
  }
  asort($result);
  return $result;
}

/**
 * Per mapping settings summary callback. Shows whether a mapping is used as
 * unique or not.
 */
function feeds_ui_mapping_settings_optional_unique_summary($mapping, $target, $form, $form_state) {
  if (!empty($target['optional_unique'])) {
    if ($mapping['unique']) {
      return t('Used as <strong>unique</strong>.');
    }
    else {
      return t('Not used as unique.');
    }
  }
}

/**
 * Per mapping settings form callback. Lets the user choose if a target is as
 * unique or not.
 */
function feeds_ui_mapping_settings_optional_unique_form($mapping, $target, $form, $form_state) {
  $settings_form = array();

  if (!empty($target['optional_unique'])) {
    $settings_form['unique'] = array(
      '#type' => 'checkbox',
      '#title' => t('Unique'),
      '#default_value' => !empty($mapping['unique']),
    );
  }

  return $settings_form;
}

/**
 * Theme feeds_ui_overview_form().
 */
function theme_feeds_ui_overview_form($variables) {
  $form = $variables['form'];
  drupal_add_css(drupal_get_path('module', 'feeds_ui') . '/feeds_ui.css');

  // Iterate through all importers and build a table.
  $rows = array();
  foreach (array('enabled', 'disabled') as $type) {
    if (isset($form[$type])) {
      foreach (element_children($form[$type]) as $id) {
        $row = array();
        foreach (element_children($form[$type][$id]) as $col) {
          $row[$col] = array(
            'data' => drupal_render($form[$type][$id][$col]),
            'class' => array($type),
          );
        }
        $rows[] = array(
          'data' => $row,
          'class' => array($type),
        );
      }
    }
  }

  $output = theme('table', array(
    'header' => $form['#header'],
    'rows' => $rows,
    'attributes' => array('class' => array('feeds-admin-importers')),
    'empty' => t('No importers available.'),
  ));

  if (!empty($rows)) {
    $output .= drupal_render_children($form);
  }

  return $output;
}

/**
 * Theme feeds_ui_edit_page().
 */
function theme_feeds_ui_edit_page($variables) {
  $config_info = $variables['info'];
  $active_container = $variables['active'];
  drupal_add_css(drupal_get_path('module', 'feeds_ui') . '/feeds_ui.css');

  // Outer wrapper.
  $output = '<div class="feeds-settings clear-block">';

  // Build left bar.
  $output .= '<div class="left-bar">';
  foreach ($config_info as $info) {
    $output .= theme('feeds_ui_container', array('container' => $info));
  }
  $output .= '</div>';

  // Build configuration space.
  $output .= '<div class="configuration">';
  $output .= '<div class="configuration-squeeze">';
  $output .= theme('feeds_ui_container', array('container' => $active_container));
  $output .= '</div>';
  $output .= '</div>';

  $output .= '</div>'; // ''<div class="feeds-settings">';

  return $output;
}

/**
 * Render a simple container. A container can have a title, a description and
 * one or more actions. Recursive.
 *
 * @todo Replace with theme_fieldset or a wrapper to theme_fieldset?
 *
 * @param $variables
 *   An array containing an array at 'container'.
 *   A 'container' array may contain one or more of the following keys:
 *   array(
 *     'title' => 'the title',
 *     'body' => 'the body of the container, may also be an array of more
 *                containers or a renderable array.',
 *     'class' => array('the class of the container.'),
 *     'id' => 'the id of the container',
 *   );
 */
function theme_feeds_ui_container($variables) {
  $container = $variables['container'];

  $class = array_merge(array('feeds-container'), empty($container['class']) ? array('plain') : $container['class']);
  $id = empty($container['id']) ? '': ' id="' . $container['id'] . '"';
  $output = '<div class="' . implode(' ', $class) . '"' . $id . '>';

  if (isset($container['actions']) && count($container['actions'])) {
    $output .= '<ul class="container-actions">';
    foreach ($container['actions'] as $action) {
      $output .= '<li>' . $action . '</li>';
    }
    $output .= '</ul>';
  }

  if (!empty($container['title'])) {
    $output .= '<h4 class="feeds-container-title">';
    $output .= $container['title'];
    $output .= '</h4>';
  }

  if (!empty($container['body'])) {
    $output .= '<div class="feeds-container-body">';
    if (is_array($container['body'])) {
      if (isset($container['body']['#type'])) {
        $output .= drupal_render($container['body']);
      }
      else {
        foreach ($container['body'] as $c) {
          $output .= theme('feeds_ui_container', array('container' => $c));
        }
      }
    }
    else {
      $output .= $container['body'];
    }
    $output .= '</div>';
  }

  $output .= '</div>';
  return $output;
}

/**
 * Theme function for feeds_ui_mapping_form().
 */
function theme_feeds_ui_mapping_form($variables) {
  $form = $variables['form'];

  // Build the actual mapping table.
  $header = array(
    t('Source'),
    t('Target'),
    t('Target configuration'),
    '&nbsp;',
    t('Weight'),
  );
  $rows = array();
  if (is_array($form['#mappings'])) {
    foreach ($form['#mappings'] as $i => $mapping) {
      // Some parsers do not define source options.
      $source = isset($form['source']['#options'][$mapping['source']]) ? $form['source']['#options'][$mapping['source']] : $mapping['source'];
      $target = isset($form['target']['#options'][$mapping['target']]) ? check_plain($form['target']['#options'][$mapping['target']]) : '<em>' . t('Missing') . '</em>';
      // Add indicator to target if target configuration changed.
      if (isset($form['#mapping_settings'][$i])) {
        $target .= '<span class="warning">*</span>';
      }
      $rows[] = array(
        'data' => array(
          check_plain($source),
          $target,
          drupal_render($form['config'][$i]),
          drupal_render($form['remove_flags'][$i]),
          drupal_render($form['mapping_weight'][$i]),
        ),
        'class' => array('draggable', 'tabledrag-leaf'),
      );
    }
  }
  if (!count($rows)) {
    $rows[] = array(
      array(
        'colspan' => 5,
        'data' => t('No mappings defined.'),
      ),
    );
  }
  $rows[] = array(
    drupal_render($form['source']),
    drupal_render($form['target']),
    '',
    drupal_render($form['add']),
    '',
  );
  $output = '';
  if (!empty($form['changed'])) {
    // This form element is only available if target configuration changed.
    $output .= drupal_render($form['changed']);
  }
  $output .= '<div class="help feeds-admin-ui">' . drupal_render($form['help']) . '</div>';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'feeds-ui-mapping-overview')));

  // Build the help table that explains available sources.
  $legend = '';
  $rows = array();
  foreach (element_children($form['legendset']['legend']['sources']) as $k) {
    $rows[] = array(
      check_plain(drupal_render($form['legendset']['legend']['sources'][$k]['name'])),
      check_plain(drupal_render($form['legendset']['legend']['sources'][$k]['description'])),
    );
  }
  if (count($rows)) {
    $legend .= '<h4>' . t('Sources') . '</h4>';
    $legend .= theme('table', array('header' => array(t('Name'), t('Description')), 'rows' => $rows));
  }

  // Build the help table that explains available targets.
  $rows = array();
  foreach (element_children($form['legendset']['legend']['targets']) as $k) {
    $rows[] = array(
      check_plain(drupal_render($form['legendset']['legend']['targets'][$k]['name']) . ' (' . $k . ')'),
      check_plain(drupal_render($form['legendset']['legend']['targets'][$k]['description'])),
    );
  }
  $legend .= '<h4>' . t('Targets') . '</h4>';
  $legend .= theme('table', array('header' => array(t('Name'), t('Description')), 'rows' => $rows));

  // Stick tables into collapsible fieldset.
  $form['legendset']['legend'] = array(
    '#markup' => '<div>' . $legend . '</div>',
  );

  $output .= drupal_render($form['legendset']);
  $output .= drupal_render_children($form);

  drupal_add_tabledrag('feeds-ui-mapping-overview', 'order', 'sibling', 'feeds-ui-mapping-weight');
  return $output;
}

/**
 * Page callback to import a Feeds importer.
 */
function feeds_ui_importer_import($form, &$form_state) {
  $form['id'] = array(
    '#type' => 'textfield',
    '#title' => t('Importer id'),
    '#description' => t('Enter the id to use for this importer if it is different from the source importer. Leave blank to use the id of the importer.'),
  );
  $form['id_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('Replace an existing importer if one exists with the same id.'),
  );
  $form['bypass_validation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Bypass importer validation'),
    '#description' => t('Bypass the validation of plugins when importing.'),
  );
  $form['importer'] = array(
    '#type' => 'textarea',
    '#rows' => 10,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );
  return $form;
}

/**
 * Form validation handler for feeds_ui_importer_import().
 *
 * @see feeds_ui_importer_import_submit()
 */
function feeds_ui_importer_import_validate($form, &$form_state) {
  $form_state['values']['importer'] = trim($form_state['values']['importer']);
  $form_state['values']['id'] = trim($form_state['values']['id']);

  if (!empty($form_state['values']['id']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['id'])) {
    form_error($form['id'], t('Feeds importer id must be alphanumeric with underscores only.'));
  }

  if (substr($form_state['values']['importer'], 0, 5) == '<?php') {
    $form_state['values']['importer'] = substr($form_state['values']['importer'], 5);
  }

  $feeds_importer = NULL;
  ob_start();
  eval($form_state['values']['importer']);
  ob_end_clean();

  if (!is_object($feeds_importer)) {
    return form_error($form['importer'], t('Unable to interpret Feeds importer code.'));
  }

  if (empty($feeds_importer->api_version) || $feeds_importer->api_version < 1) {
    form_error($form['importer'], t('The importer is not compatible with this version of Feeds.'));
  }
  elseif (version_compare($feeds_importer->api_version, feeds_api_version(), '>')) {
    form_error($form['importer'], t('That importer is created for the version %import_version of Feeds, but you only have version %api_version.', array(
      '%import_version' => $feeds_importer->api_version,
      '%api_version' => feeds_api_version())));
  }

  // Change to user-supplied id.
  if ($form_state['values']['id']) {
    $feeds_importer->id = $form_state['values']['id'];
  }

  $exists = feeds_ui_importer_machine_name_exists($feeds_importer->id);

  if ($exists && !$form_state['values']['id_override']) {
    if (feeds_importer($feeds_importer->id)->export_type != EXPORT_IN_CODE) {
      return form_error($form['id'], t('Feeds importer already exists with that id.'));
    }
  }

  if (!$form_state['values']['bypass_validation']) {
    foreach (array('fetcher', 'parser', 'processor') as $type) {
      $plugin = feeds_plugin($feeds_importer->config[$type]['plugin_key'], $feeds_importer->id);
      if (get_class($plugin) == 'FeedsMissingPlugin') {
        form_error($form['importer'], t('The plugin %plugin is unavailable.', array('%plugin' => $feeds_importer->config[$type]['plugin_key'])));
      }
    }
  }

  $form_state['importer'] = $feeds_importer;
}

/**
 * Form submission handler for feeds_ui_importer_import().
 *
 * @see feeds_ui_importer_import_validate()
 */
function feeds_ui_importer_import_submit($form, &$form_state) {
  $importer = $form_state['importer'];

  // Create a copy of the importer to preserve config.
  $save = feeds_importer($importer->id);
  $save->setConfig($importer->config);
  foreach (array('fetcher', 'parser', 'processor') as $type) {
    $save->setPlugin($importer->config[$type]['plugin_key']);
    $save->$type->setConfig($importer->config[$type]['config']);
  }
  $save->save();

  drupal_set_message(t('Successfully imported the %id feeds importer.', array('%id' => $importer->id)));
  $form_state['redirect'] = 'admin/structure/feeds';
}
