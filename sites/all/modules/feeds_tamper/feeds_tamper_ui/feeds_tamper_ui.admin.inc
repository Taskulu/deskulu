<?php

/**
 * @file
 * Forms and their accompanying validation and submit functions for Feeds Tamper
 * UI.
 */

 /**
  * List of plugins for current importer.
  */
function feeds_tamper_ui_list_form($form, &$form_state, $importer) {
  $mappings = $importer->processor->config['mappings'];

  if (!$mappings) {
    $args = array('@url' => url('admin/structure/feeds/' . $importer->id . '/mapping'));
    drupal_set_message(t('There are no <a href="@url">mappings</a> defined for this importer.', $args), 'warning');
    return $form;
  }

  $sources = $importer->parser->getMappingSources();
  $targets = $importer->processor->getMappingTargets();
  $instances = feeds_tamper_load_by_importer($importer->id, TRUE);

  // Help message at the top. I have a seceret, I added the link back to the
  // mappings because it makes my life a lot easier while testing this.
  $args = array('@url' => url('admin/structure/feeds/' . $importer->id . '/mapping'));
  $message = t('Configure plugins to modify Feeds data before it gets saved. Each <a href="@url">mapping</a> can be manipulated individually.', $args);
  $form['help']['#markup'] = '<div class="help"><p>' . $message . '</p></div>';

  // Helpful shortcut.
  $form['mappings'] = array();
  $map = &$form['mappings'];
  $form['#importer'] = $importer;
  $mapped = array();
  foreach ($mappings as $delta => $mapping) {
    $source = $mapping['source'];
    $target = $mapping['target'];

    // We will skip building the table if it has been built for a source, but we
    // need to add the target label to indicate to the user that more than one
    // field will be affected.
    $target_label = !empty($targets[$target]['name']) ? check_plain($targets[$target]['name']) : check_plain($target);
    $prev = $delta;
    if (isset($mapped[$source])) {
      $prev = $mapped[$source];
    }
    $map[$prev]['#title']['targets'][] = $target_label;
    // We don't need to build the table again for this source.
    if (isset($mapped[$source])) {
      continue;
    }
    $mapped[$source] = $delta;

    $source_label = !empty($sources[$source]['name']) ? check_plain($sources[$source]['name']) : check_plain($source);
    $map[$delta]['#title']['source'] = $source_label;

    $map[$delta]['#tree'] = TRUE;
    $map[$delta]['table'] = array();
    $map[$delta]['#add_link'] = l(t('Add plugin'), 'admin/structure/feeds/' . $importer->id . '/tamper/add/' . bin2hex($source));

    // There are no plugins for this source. We've built all we need to.
    if (!isset($instances[$source])) {
      continue;
    }
    foreach ($instances[$source] as $instance) {

      // Plugin instance description.
      $description = !empty($instance->description) ? check_plain($instance->description) : $instance->id;
      $map[$delta]['table'][$instance->id]['#description'] = $description;

      // Weight field.
      $map[$delta]['table'][$instance->id]['weight'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#default_value' => $instance->weight,
        '#attributes' => array('class' => array('weight')),
      );

      // Name of plugin.
      $plugin = feeds_tamper_get_plugin($instance->plugin_id);
      $map[$delta]['table'][$instance->id]['#name'] = $plugin['name'];

      // Calculate where the plugin lives and it's corresponding operations.
      if ($instance->export_type == EXPORT_IN_CODE) {
        $status = t('Default');
        $edit = t('Override');
        $delete = '';
      }
      elseif ($instance->export_type == EXPORT_IN_DATABASE) {
        $status = t('Normal');
        $edit = t('Edit');
        $delete = t('Delete');
      }
      elseif ($instance->export_type == (EXPORT_IN_CODE | EXPORT_IN_DATABASE)) {
        $status = t('Overridden');
        $edit = t('Edit');
        $delete = t('Revert');
      }

      // Status column, Default, Normal, Overridden.
      $map[$delta]['table'][$instance->id]['#status'] = $status;

      // Build Edit | Delete, Edit | Revert, Override links.
      $ops = l(t($edit), 'admin/structure/feeds/' . $importer->id . '/tamper/' . $instance->id . '/edit');

      // Plugins provided in code can't be deleted but they can be reverted.
      if (!empty($delete)) {
        $ops .= ' | ';
        $ops .= l(t($delete), 'admin/structure/feeds/' . $importer->id . '/tamper/' . $instance->id . '/delete');
      }

      $map[$delta]['table'][$instance->id]['#edit'] = $ops;

      // Enable/disable checkbox.
      $map[$delta]['table'][$instance->id]['enabled'] = array(
        '#type' => 'checkbox',
        '#default_value' => !$instance->disabled,
      );
    }
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Validate handler for plugin list.
 */
function feeds_tamper_ui_list_form_validate($form, &$form_state) {
  $mappings = element_children($form['mappings']);
  foreach ($mappings as $i) {
    if (empty($form_state['values'][$i]['table'])) {
      continue;
    }
    foreach ($form_state['values'][$i]['table'] as $id => $value) {
      // I'm anal.
      $value['weight'] = (int) $value['weight'];
      // Someone would have to type this in manually, but that's not too far of
      // a stretch.
      if ($value['weight'] < 0) {
        form_set_error($i . '][table][' . $id . '][weight', t('Weight must be greater than or equal to zero.'));
      }
    }
  }
}

/**
 * Submit handler for plugin list.
 *
 * Sets weight and enabled/disabled status of plugin instances.
 */
function feeds_tamper_ui_list_form_submit($form, &$form_state) {
  $mappings = element_children($form['mappings']);
  foreach ($mappings as $i) {
    if (empty($form_state['values'][$i]['table'])) {
      continue;
    }
    foreach ($form_state['values'][$i]['table'] as $id => $value) {
      $instance = feeds_tamper_load_instance($id);

      $instance->disabled = !(bool) $value['enabled'];
      $instance->weight = $value['weight'];

      feeds_tamper_save_instance($instance);
    }
  }
  drupal_set_message(t('Your changes have been saved.'));
}

/**
 * Theme callback for plugin list tables.
 */
function theme_feeds_tamper_ui_list_form($variables) {
  $form = $variables['form'];
  // Shortcut to save typing and screen space.

  if (empty($form['mappings'])) {
    return;
  }

  $map = &$form['mappings'];
  $mappings = element_children($map);

  $header_normal = array(
    t('Description'),
    t('Weight'),
    t('Plugin'),
    t('Status'),
    t('Operations'),
    t('Enabled'),
  );
  // Remove weight column for empty tables. It looks nicer.
  $header_empty = $header_normal;
  unset($header_empty[1]);

  foreach ($mappings as $key => $i) {
    $table_rows = $disabled_rows = array();
    // We used to use the source name in the table id, but it just needs to be
    // unique, so use the key to avoid problems.
    $table_id = 'feeds-tamper-' . $key . '-table';

    // Plugin instances for a particluar source.
    $instances = element_children($map[$i]['table']);
    if (empty($instances)) {
      $header = $header_empty;
      $help_text = t('No plugins defined.');
      $table_rows = array(
        'data' => array(array('data' => $help_text, 'colspan' => 5)),
      );
    }
    else {
      $header = $header_normal;
      foreach ($instances as $id) {
        $enabled = !empty($map[$i]['table'][$id]['enabled']['#default_value']);
        // Assemble enabled plugin rows.
        if ($enabled) {
          $this_row = array();
          // Add description column.
          $this_row[] = $map[$i]['table'][$id]['#description'];
          // Add weight column, it will be hidden if javascript is enabled.
          $this_row[] = drupal_render($map[$i]['table'][$id]['weight']);
          // The name of the plugin, as defined in a PLUGIN.inc.
          $this_row[] = $map[$i]['table'][$id]['#name'];
          // One of Normal, Default, Overridden.
          $this_row[] = $map[$i]['table'][$id]['#status'];
          // Operations to perform on a plugin instance.
          $this_row[] = $map[$i]['table'][$id]['#edit'];
          // Enabled checkbox.
          $this_row[] = drupal_render($map[$i]['table'][$id]['enabled']);
          // Add draggable class for sortable table.
          $table_rows[] = array('data' => $this_row, 'class' => array('draggable'));
        }
        else {
          // Assemble disabled plugin rows.
          $this_row = array();
          // Add description column.
          $this_row[] = $map[$i]['table'][$id]['#description'];
          // Add weight column, it will be hidden if javascript is enabled.
          $this_row[] = drupal_render($map[$i]['table'][$id]['weight']);
          // The name of the plugin, as defined in a PLUGIN.inc.
          $this_row[] = $map[$i]['table'][$id]['#name'];
          // One of Normal, Default, Overridden.
          $this_row[] = $map[$i]['table'][$id]['#status'];
          // Operations to perform on a plugin instance. It's disabled, so make
          // it just text.
          $this_row[] = strip_tags($map[$i]['table'][$id]['#edit']);
          // Enabled checkbox.
          $this_row[] = drupal_render($map[$i]['table'][$id]['enabled']);
          // Add disabled class to differentiate.
          $disabled_rows[] = array('data' => $this_row, 'class' => array('disabled'));
        }
      }
      // Only add tabledrag if there were plugins for that source.
      drupal_add_tabledrag($table_id, 'order', 'sibling', 'weight');
    }
    // Combine enabled and disabled rows.
    $table_rows = array_merge($table_rows, $disabled_rows);

    // Put action-link class on for fanciness sake.
    $add = '<ul class="feeds-tamper-add action-links"><li>' . $map[$i]['#add_link'] . '</li></ul>';
    // "Add plugin" link is the last row.
    $table_rows[] = array(
      'data' => array(array('data' => $add, 'colspan' => 7)),
      'class' => array('feeds-tamper-add'),
    );
    // Table caption, in the form of source_name -> target_name1, target_name2
    $caption = $map[$i]['#title']['source'] . ' -> ' . implode(', ', $map[$i]['#title']['targets']);
    $table = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $table_rows,
      '#caption' => $caption,
      '#sticky' => FALSE,
    );
    $table['#attributes']['id'] = $table_id;
    $table['#attached']['css'][] = drupal_get_path('module', 'feeds_tamper_ui') . '/feeds_tamper_ui.css';
    $map[$i]['table'] = $table;
    $map[$i]['#prefix'] = '<div class="feeds-tamper-table">';
    $map[$i]['#suffix'] = '</div>';
  }
  return drupal_render_children($form);
}

/**
 * Add plugin form.
 */
function feeds_tamper_ui_add_plugin_form($form, &$form_state, $importer, $source) {
  // Set importer and source for use in validate and submit.
  $form_state['importer'] = $importer;
  $form_state['source'] = $source;

  // Build plugin select list.
  $feeds_tamper_plugins = feeds_tamper_get_plugins();
  $plugins = array();
  foreach ($feeds_tamper_plugins as $plugin_id => $plugin) {
    $plugins[t($plugin['category'])][$plugin_id] = t($plugin['name']);
  }
  ksort($plugins);
  foreach ($plugins as &$p) {
    asort($p);
  }

  $machine_name = key(reset($plugins));
  if (!empty($form_state['values']['plugin_id'])) {
    $machine_name = $form_state['values']['plugin_id'];
  }
  // Add css.
  $form['#attached']['css'][] = drupal_get_path('module', 'feeds_tamper_ui') . '/feeds_tamper_ui.css';

  $plugin = feeds_tamper_get_plugin($machine_name);

  $form['plugin_id'] = array(
    '#title' => t('The plugin to add'),
    '#type' => 'select',
    '#options' => $plugins,
    '#default_value' => '',
    '#tree' => TRUE,
    '#ajax' => array(
      'callback' => 'feeds_tamper_ajax_callback',
      'wrapper' => 'feeds-tamper-plugin',
      'progress' => 'none',
    ),
  );
  $form['update'] = array(
    '#type' => 'submit',
    '#limit_validation_errors' => array(
      array('plugin_id')
    ),
    '#submit' => array('feeds_tamper_ui_add_plugin_form_submit'),
    '#value' => t('Choose'),
    '#attributes' => array('class' => array('no-js')),
  );
  $form['plugin']['#prefix'] = '<div class="clear-fix" id="feeds-tamper-plugin">';
  $form['plugin']['#suffix'] = '</div>';

  $form['plugin']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => $plugin['default description'] ? t($plugin['default description']) : t($plugin['name']),
    '#required' => TRUE,
    '#description' => t('A useful description of what this plugin is doing.'),
  );
  $form['plugin']['id'] = array(
    '#title' => t('Machine name'),
    '#type' => 'machine_name',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'feeds_tamper_machine_name_callback',
      'source' => array('plugin', 'description'),
    ),
    '#default_value' => $machine_name,
    // '#disabled' => FALSE,
    // '#size' => 60,
  );
  $form['plugin']['settings'] = array(
    '#title' => t('Configure @name', array('@name' => $plugin['name'])),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  $form['plugin']['settings'] += $plugin['form']($importer, $source, array(), $form_state);

  $form['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Add plugin form validate handler.
 */
function feeds_tamper_ui_add_plugin_form_validate($form, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Add')) {
    if (feeds_tamper_machine_name_callback($form_state['values']['id'], $form, $form_state)) {
      form_set_error('id', t('The machine-readable name is already in use. It must be unique.'));
      return;
    }
    $plugin_id = $form_state['values']['plugin_id'];
    $plugin = feeds_tamper_get_plugin($plugin_id);
    if ($plugin['validate'] && isset($form_state['values']['settings'])) {
      $plugin['validate']($form_state['values']['settings']);
    }
    $id = $form_state['values']['id'];
    $importer_id = $form_state['importer']->id;
    $source = feeds_tamper_make_machine($form_state['source']);
    $form_state['values']['id'] = "$importer_id-$source-$id";
    return;
  }
  unset($form_state['input']['id']);
  unset($form_state['input']['description']);
  unset($form_state['input']['settings']);
}

/**
 * Add plugin form submit handler.
 */
function feeds_tamper_ui_add_plugin_form_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Add')) {
    $obj = feeds_tamper_new_instance();
    $obj->plugin_id = $form_state['values']['plugin_id'];
    if (isset($form_state['values']['settings'])) {
      $obj->settings = $form_state['values']['settings'];
    }
    $obj->importer = $form_state['importer']->id;

    $obj->source = $form_state['source'];
    $obj->description = $form_state['values']['description'];
    $obj->id = $form_state['values']['id'];

    feeds_tamper_save_instance($obj);
    $form_state['redirect'] = 'admin/structure/feeds/' . $obj->importer . '/tamper';
    $source_name = feeds_tamper_ui_source_name($obj);
    drupal_set_message(t('Plugin %description was successfully added to %source.', array('%description' => $obj->description, '%source' => $source_name)));
    return;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Edit plugin form.
 *
 * @todo Combine add and edit forms?
 */
function feeds_tamper_ui_edit_plugin_form($form, &$form_state, $instance) {
  // Set breadcrumb.
  $importer = feeds_importer($instance->importer);

  $form_state['instance'] = $instance;
  $plugin = feeds_tamper_get_plugin($instance->plugin_id);
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#description' => t('A useful description of what this plugin is doing.'),
    '#default_value' => $instance->description,
  );
  $form['settings'] = array(
    '#title' => t('Configure @plugin', array('@plugin' => $plugin['name'])),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  $form['settings'] += $plugin['form']($importer, $instance->source, $instance->settings, $form_state);

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'feeds_tamper_ui') . '/feeds_tamper_ui.css';

  return $form;
}

/**
 * Edit plugin form validate handler.
 */
function feeds_tamper_ui_edit_plugin_form_validate($form, &$form_state) {
  $plugin_id = $form_state['instance']->plugin_id;
  $plugin = feeds_tamper_get_plugin($plugin_id);
  if ($plugin['validate']) {
    $plugin['validate']($form_state['values']['settings']);
  }
}

/**
 * Edit plugin form submit handler.
 */
function feeds_tamper_ui_edit_plugin_form_submit($form, &$form_state) {
  $instance = $form_state['instance'];
  if (isset($form_state['values']['settings'])) {
    $instance->settings = $form_state['values']['settings'];
  }
  $instance->description = $form_state['values']['description'];

  feeds_tamper_save_instance($instance);
  drupal_set_message(t('The plugin %plugin has been updated.', array('%plugin' => $instance->description)));
  $form_state['redirect'] = 'admin/structure/feeds/' . $instance->importer . '/tamper';
}

/**
 * Delete plugin form.
 */
function feeds_tamper_ui_delete_form($form, &$form_state, $instance) {
  if (!$instance) {
    drupal_set_message(t('Invalid plugin id.'), 'error');
    return;
  }

  $form_state['instance'] = $instance;

  if ($instance->export_type & EXPORT_IN_CODE) {
    $question = t('Would you really like to revert the plugin @instance?', array('@instance' => $instance->description));
    $button_label = t('Revert');
  }
  else {
    $question = t('Would you really like to delete the plugin @instance?', array('@instance' => $instance->description));
    $button_label = t('Delete');
  }
  return confirm_form(
    $form,
    $question,
    'admin/structure/feeds/' . $instance->importer . '/tamper',
    NULL,
    $button_label
  );
}

/**
 * Delete plugin form submit handler.
 */
function feeds_tamper_ui_delete_form_submit($form, &$form_state) {
  $instance = $form_state['instance'];
  feeds_tamper_delete_instance($instance);
  $source_name = feeds_tamper_ui_source_name($instance);

  switch ($instance->export_type) {
    case EXPORT_IN_DATABASE:
      drupal_set_message(t('The plugin %plugin has been deleted from %source.', array('%plugin' => $instance->description, '%source' => $source_name)));
      break;
    case EXPORT_IN_CODE | EXPORT_IN_DATABASE:
      drupal_set_message(t('The plugin %plugin has been reverted.', array('%plugin' => $instance->description)));
      break;
  }
  $form_state['redirect'] = 'admin/structure/feeds/' . $instance->importer . '/tamper';
}

/**
 * Helper functions for ajax.
 */
function feeds_tamper_machine_name_callback($id, $form, $form_state) {
  // Importer id's are machine safe.
  $importer_id = $form_state['importer']->id;
  $source = feeds_tamper_make_machine($form_state['source']);
  return feeds_tamper_load_instance("$importer_id-$source-$id");
}

/**
 * Ajax callback for add plugin form.
 */
function feeds_tamper_ajax_callback($form, $form_state) {
  // The form has already been submitted and updated. We can return the replaced
  // item as it is.
  return $form['plugin'];
}

/**
 * Export form.
 */
function feeds_tamper_ui_export_form($form, &$form_state, $importer) {
  $code = feeds_tamper_export($importer->id);

  $form['export'] = array(
    '#title' => t('Export Feeds Tamper plugins'),
    '#type' => 'textarea',
    '#value' => $code,
    '#rows' => substr_count($code, "\n") + 2,
  );
  return $form;
}
