<?php

/**
 * @file
 * Internationalization (i18n) module. Translation sets admin
 */

/**
 * Overview page for translation sets
 * 
 * @param $type
 *   Translation set type to get a listing for this type only
 * @param $query
 *   Base query to build upon
 */
function i18n_translation_admin_overview($type = NULL, $query = NULL) {
  // Build the sortable table header.
  $header['title'] = array('data' => t('Title'), 'field' => 't.title');
  if (!$type) {
    $header['type'] = array('data' => t('Type'), 'field' => 't.type');
  }
  $header['items'] = t('Items');
  $header['created'] = array('data' => t('Created'), 'field' => 't.created');
  $header['changed'] = array('data' => t('Updated'), 'field' => 't.changed', 'sort' => 'desc');
  $header['operations'] = array('data' => t('Operations'));

  // Get the translation sets for this form
  $query = $query ? $query : db_select('i18n_translation_set', 't');
  $query = $query->extend('PagerDefault')->extend('TableSort');
  if ($type) {
    $query->condition('t.type', $type);
  }
  $tsids = $query
    ->fields('t', array('tsid'))
    ->limit(20)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();
  $translations = $tsids ? entity_load('i18n_translation', $tsids) : array();
  
  $form = drupal_get_form('i18n_translation_admin_form', $translations, $header);

  $form['pager'] = array('#markup' => theme('pager'));  
  return $form;  
}

/**
 * Admin form
 */
function i18n_translation_admin_form($form, &$form_state, $translations, $header) {
  $destination = drupal_get_destination();
  $rows = array();
  foreach ($translations as $set) {
    $info = i18n_object_info($set->type);
    $rows[$set->tsid]['title'] = check_plain($set->get_title());
    if (isset($header['type'])) {
      $rows[$set->tsid]['type'] = isset($info['title']) ? $info['title'] : t('Unknown');
    }
    $rows[$set->tsid]['items'] = ($items = $set->get_links()) ? theme('links', array('links' => $items)) : '';
    $rows[$set->tsid] += array(
      'created' => format_date($set->created, 'short'),
      'changed' => format_date($set->changed, 'short'),
      'operations' => '',
    );

    // Build a list of all the accessible operations for the current set.
    $operations = $set->get_operations();
    if (count($operations) > 1) {
      // Render an unordered list of operations links.
      $rows[$set->tsid]['operations'] = array(
        'data' => array(
          '#theme' => 'links__node_operations',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $rows[$set->tsid]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }
  $form['translations'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No translation sets available.'),
  );
  return $form;
}