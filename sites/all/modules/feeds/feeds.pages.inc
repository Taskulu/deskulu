<?php

/**
 * @file
 * Menu callbacks, form callbacks and helpers.
 */

/**
 * Render a page of available importers.
 */
function feeds_page() {
  $rows = array();
  if ($importers = feeds_importer_load_all()) {
    foreach ($importers as $importer) {
      if ($importer->disabled) {
        continue;
      }
      if (!(user_access('import ' . $importer->id . ' feeds') || user_access('administer feeds'))) {
        continue;
      }
      if (empty($importer->config['content_type'])) {
        $link = 'import/' . $importer->id;
        $title = $importer->config['name'];
      }
      elseif (node_access('create', $importer->config['content_type'])) {
        $link = 'node/add/' . str_replace('_', '-', $importer->config['content_type']);
        $title = node_type_get_name($importer->config['content_type']);
      }
      else {
        continue;
      }
      $rows[] = array(
        l($title, $link),
        check_plain($importer->config['description']),
      );
    }
  }
  if (empty($rows)) {
    // The feeds_ui module is enabled.
    if (module_exists('feeds_ui') && user_access('administer feeds')) {
      drupal_set_message(t('There are no importers, go to <a href="@importers">Feed importers</a> to create one or enable an existing one.', array('@importers' => url('admin/structure/feeds'))));
    }
    else {
      // The feeds_ui module is not enabled but the current user has access to
      // Modules to enable it.
      if (user_access('administer modules')) {
        drupal_set_message(t('The Feeds UI Admin module is not enabled and there are no importers, go to <a href="@modules">Modules</a> and enable Feeds Admin UI. Then go to <a href="@importers">Feed importers</a> to create one or enable an existing one.', array('@modules' => url('admin/modules'), '@importers' => url('admin/structure/feeds'))));
      }
      else {
        // The feeds_ui module is not enabled and the current user cannot
        // enable it.
        drupal_set_message(t("The Feeds UI Admin module is not enabled. Please contact the Administrator for your site and ask them to enable it."));
      }
    }
  }
  $header = array(
    t('Import'),
    t('Description'),
  );
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Render a feeds import form on import/[config] pages.
 */
function feeds_import_form(array $form, array &$form_state, FeedsImporter $importer) {
  $source = feeds_source($importer->id);

  $form['#importer_id'] = $importer->id;
  // @todo Move this into fetcher?
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['source_status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#tree' => TRUE,
    '#value' => feeds_source_status($source),
  );

  $source_form = $source->configForm($form_state);
  if (!empty($source_form)) {
    $form['feeds'] = array(
      '#type' => 'fieldset',
      '#title' => t('Import'),
      '#tree' => TRUE,
    ) + $source_form;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );
  $progress = $source->progressImporting();
  if ($progress !== FEEDS_BATCH_COMPLETE) {
    $form['submit']['#disabled'] = TRUE;
    $form['submit']['#value'] =
      t('Importing (@progress %)', array('@progress' => number_format(100 * $progress, 0)));
  }
  return $form;
}

/**
 * Validation handler for node forms and feeds_import_form().
 */
function feeds_import_form_validate($form, &$form_state) {
  // @todo This may be a problem here, as we don't have a feed_nid at this point.
  feeds_source($form['#importer_id'])->configFormValidate($form_state['values']['feeds']);
}

/**
 * Submit handler for feeds_import_form().
 */
function feeds_import_form_submit($form, &$form_state) {
  // Save source and import.
  $source = feeds_source($form['#importer_id']);

  if (!empty($form_state['values']['feeds']) && is_array($form_state['values']['feeds'])) {
    $source->addConfig($form_state['values']['feeds']);
    $source->save();
  }

  // Refresh feed if import on create is selected.
  if ($source->importer->config['import_on_create']) {
    $source->startImport();
  }

  // Add to schedule, make sure importer is scheduled, too.
  $source->schedule();
}

/**
 * Render a feeds import form on node/id/import pages.
 */
function feeds_import_tab_form($form, &$form_state, $node) {
  $importer_id = feeds_get_importer_id($node->type);
  $source = feeds_source($importer_id, $node->nid);

  $form = array();
  $form['#feed_nid'] = $node->nid;
  $form['#importer_id'] = $importer_id;
  $form['#redirect'] = 'node/' . $node->nid;
  $form['source_status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#tree' => TRUE,
    '#value' => feeds_source_status($source),
  );
  $form = confirm_form($form, t('Import all content from source?'), 'node/' . $node->nid, '', t('Import'), t('Cancel'), 'confirm feeds update');
  $progress = $source->progressImporting();
  if ($progress !== FEEDS_BATCH_COMPLETE) {
    $form['actions']['submit']['#disabled'] = TRUE;
    $form['actions']['submit']['#value'] =
      t('Importing (@progress %)', array('@progress' => number_format(100 * $progress, 0)));
  }
  return $form;
}

/**
 * Submit handler for feeds_import_tab_form().
 */
function feeds_import_tab_form_submit($form, &$form_state) {
  $form_state['redirect'] = $form['#redirect'];
  feeds_source($form['#importer_id'], $form['#feed_nid'])->startImport();
}

/**
 * Render a feeds delete form.
 *
 * Used on both node pages and configuration pages.
 * Therefore $node may be missing.
 */
function feeds_delete_tab_form(array $form, array &$form_state, FeedsImporter $importer = NULL, $node = NULL) {
  if (empty($node)) {
    $source = feeds_source($importer->id);
    $form['#redirect'] = 'import/' . $source->id;
  }
  else {
    $importer_id = feeds_get_importer_id($node->type);
    $source = feeds_source($importer_id, $node->nid);
    $form['#redirect'] = 'node/' . $source->feed_nid;
  }
  // Form cannot pass on source object.
  $form['#importer_id'] = $source->id;
  $form['#feed_nid'] = $source->feed_nid;
  $form['source_status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#tree' => TRUE,
    '#value' => feeds_source_status($source),
  );
  $form = confirm_form($form, t('Delete all items from source?'), $form['#redirect'], '', t('Delete'), t('Cancel'), 'confirm feeds update');
  $progress = $source->progressClearing();
  if ($progress !== FEEDS_BATCH_COMPLETE) {
    $form['actions']['submit']['#disabled'] = TRUE;
    $form['actions']['submit']['#value'] =
      t('Deleting (@progress %)', array('@progress' => number_format(100 * $progress, 0)));
  }
  return $form;
}

/**
 * Submit handler for feeds_delete_tab_form().
 */
function feeds_delete_tab_form_submit($form, &$form_state) {
  $form_state['redirect'] = $form['#redirect'];
  $feed_nid = empty($form['#feed_nid']) ? 0 : $form['#feed_nid'];
  feeds_source($form['#importer_id'], $feed_nid)->startClear();
}

/**
 * Render a feeds unlock form.
 *
 * Used on both node pages and configuration pages.
 * Therefore $node may be missing.
 */
function feeds_unlock_tab_form($form, &$form_state, FeedsImporter $importer = NULL, $node = NULL) {
  if (empty($node)) {
    $source = feeds_source($importer->id);
    $form['#redirect'] = 'import/' . $source->id;
  }
  else {
    $importer_id = feeds_get_importer_id($node->type);
    $source = feeds_source($importer_id, $node->nid);
    $form['#redirect'] = 'node/' . $source->feed_nid;
  }
  // Form cannot pass on source object.
  $form['#importer_id'] = $source->id;
  $form['#feed_nid'] = $source->feed_nid;
  $form['source_status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#tree' => TRUE,
    '#value' => feeds_source_status($source),
  );
  $form = confirm_form($form, t('Unlock this importer?'), $form['#redirect'], '', t('Delete'), t('Cancel'), 'confirm feeds update');
  if ($source->progressImporting() == FEEDS_BATCH_COMPLETE && $source->progressClearing() == FEEDS_BATCH_COMPLETE) {
    $form['source_locked'] = array(
      '#type' => 'markup',
      '#title' => t('Not Locked'),
      '#tree' => TRUE,
      '#markup' => t('This importer is not locked, therefore it cannot be unlocked.'),
    );
    $form['actions']['submit']['#disabled'] = TRUE;
    $form['actions']['submit']['#value'] = t('Unlock (disabled)');
  }
  else {
    $form['actions']['submit']['#value'] = t('Unlock');
  }
  return $form;
}

/**
 * Form submit handler. Resets all feeds state.
 */
function feeds_unlock_tab_form_submit($form, &$form_state) {
  drupal_set_message(t('Import Unlocked'));
  $form_state['redirect'] = $form['#redirect'];
  $feed_nid = empty($form['#feed_nid']) ? 0 : $form['#feed_nid'];
  $importer_id = $form['#importer_id'];

  //Is there a more API-friendly way to set the state?
  db_update('feeds_source')
    ->condition('id', $importer_id)
    ->condition('feed_nid', $feed_nid)
    ->fields(array('state' => FALSE))
    ->execute();
}

/**
 * Handle a fetcher callback.
 */
function feeds_fetcher_callback($importer, $feed_nid = 0) {
  if ($importer instanceof FeedsImporter) {
    try {
      return $importer->fetcher->request($feed_nid);
    }
    catch (Exception $e) {
      // Do nothing.
    }
  }
  drupal_access_denied();
}

/**
 * Template generation
 */
function feeds_importer_template(FeedsImporter $importer) {
  if ($importer->parser instanceof FeedsCSVParser) {
    return $importer->parser->getTemplate();
  }
  return drupal_not_found();
}

/**
 * Renders a status display for a source.
 */
function feeds_source_status($source) {
  $progress_importing = $source->progressImporting();
  $v = array();
  if ($progress_importing != FEEDS_BATCH_COMPLETE) {
    $v['progress_importing'] = $progress_importing;
  }
  $progress_clearing = $source->progressClearing();
  if ($progress_clearing != FEEDS_BATCH_COMPLETE) {
    $v['progress_clearing'] = $progress_clearing;
  }
  $v['imported'] = $source->imported;
  $v['count'] = $source->itemCount();
  if (!empty($v)) {
    return theme('feeds_source_status', $v);
  }
}

/**
 * Themes a status display for a source.
 */
function theme_feeds_source_status($v) {
  $output = '<div class="info-box feeds-source-status">';
  $items = array();
  if ($v['progress_importing']) {
    $progress = number_format(100.0 * $v['progress_importing'], 0);
    $items[] = t('Importing - @progress % complete.', array('@progress' => $progress));
  }
  if ($v['progress_clearing']) {
    $progress = number_format(100.0 * $v['progress_clearing'], 0);
    $items[] = t('Deleting items - @progress % complete.', array('@progress' => $progress));
  }
  if (!count($items)) {
    if ($v['count']) {
      if ($v['imported']) {
        $items[] = t('Last import: @ago ago.', array('@ago' => format_interval(REQUEST_TIME - $v['imported'], 1)));
      }
      $items[] = t('@count imported items total.', array('@count' => $v['count']));
    }
    else {
      $items[] = t('No imported items.');
    }
  }
  $output .= theme('item_list', array('items' => $items));
  $output .= '</div>';
  return $output;
}

/**
 * Theme upload widget.
 */
function theme_feeds_upload($variables) {
  $element = $variables['element'];
  drupal_add_css(drupal_get_path('module', 'feeds') . '/feeds.css');
  _form_set_class($element, array('form-file'));
  $summary = '';
  if (!empty($element['#file_info'])) {
    $file = $element['#file_info'];
    $wrapper = file_stream_wrapper_get_instance_by_uri($file->uri);
    $summary .= '<div class="feeds-file-info">';
    $summary .= '<div class="feeds-file-name">';
    if ($wrapper) {
      $summary .= l(check_plain($file->filename), $wrapper->getExternalUrl());
    }
    else {
      $summary .= t('URI scheme %scheme not available.', array('%scheme' =>  file_uri_scheme($uri)));
    }
    $summary .= '</div>';
    $summary .= '<div class="file-size">';
    $summary .= format_size($file->filesize);
    $summary .= '</div>';
    $summary .= '<div class="feeds-file-mime">';
    $summary .= check_plain($file->filemime);
    $summary .= '</div>';
    $summary .= '</div>';
  }
  // Prepend the summary to the form field.
  $element['#children'] = '<div class="feeds-file">' . $summary . '<div class="feeds-file-upload">' . $element['#children'];
  // Render file upload field using theme_form_element().
  $output = theme('form_element', $element);
  // Close "feeds-file" and "feeds-file-upload" divs.
  $output .= '</div></div>';

  return $output;
}
