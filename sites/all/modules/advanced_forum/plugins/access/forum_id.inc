<?php

/**
 * @file
 * Plugin to provide access control based upon term vocabulary
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Forum: forum"),
  'description' => t('Control access by which forum is in use.'),
  'callback' => 'advanced_forum_forum_id_ctools_access_check',
  'default' => array('tids' => array()),
  'settings form' => 'advanced_forum_forum_id_ctools_access_settings',
  'settings form submit' => 'advanced_forum_forum_id_ctools_access_settings_submit',
  'summary' => 'advanced_forum_forum_id_ctools_acesss_summary',
  'required context' => new ctools_context_required(t('Forum'), array('forum')),
);

/**
 * Settings form for the 'by term_vocabulary' access plugin.
 */
function advanced_forum_forum_id_ctools_access_settings($form, &$form_state, $conf) {
  $options = array();
  $vocabulary = taxonomy_vocabulary_load(variable_get('forum_nav_vocabulary', ''));
  $options[0] = $vocabulary->name;

  $tree = taxonomy_get_tree($vocabulary->vid);
  if ($tree) {
    foreach ($tree as $term) {
      $choice = new stdClass();
      $choice->option = array($term->tid => str_repeat('-', $term->depth + 1) . $term->name);
      $options[] = $choice;
    }
  }

  $form['settings']['tids'] = array(
    '#type' => 'select',
    '#title' => t('Forums'),
    '#default_value' => $conf['tids'],
    '#options' => $options,
    '#multiple' => TRUE,
    '#size' => $multiple ? min(9, count($options)) : 0,
  );

  return $form;
}

/**
 * Compress the term_vocabularys allowed to the minimum.
 */
function advanced_forum_forum_id_ctools_access_settings_submit($form, &$form_state) {
  $form_state['values']['settings']['rids'] = array_keys(array_filter($form_state['values']['settings']['rids']));
}

/**
 * Check for access.
 */
function advanced_forum_forum_id_ctools_access_check($conf, $context) {
  // As far as I know there should always be a context at this point, but this
  // is safe.
  if (empty($context) || empty($context->data) || !isset($context->data->tid)) {
    return FALSE;
  }

  if (!empty($conf['tids']) && !isset($conf['tids'][$context->data->tid])) {
    return FALSE;
  }

  return TRUE;
}

/**
 * Provide a summary description based upon the checked term_vocabularys.
 */
function advanced_forum_forum_id_ctools_acesss_summary($conf, $context) {
  $vocabulary = taxonomy_vocabulary_load(variable_get('forum_nav_vocabulary', ''));
  if (empty($conf['tids'])) {
    return t('@identifier is any forum', array('@identifier' => $context->identifier));
  }

  $names = array();
  if (in_array(0, $conf['tids'])) {
    $names[] = check_plain($vocabulary->name);
  }
  $result = db_query("SELECT name FROM {taxonomy_term_data} WHERE tid IN (" . implode(',', $conf['tids']) . ")");

  while ($term = $result->fetchObject()) {
    $names[] = check_plain($term->name);
  }

  if (empty($names)) {
    return t('@identifier is any forum', array('@identifier' => $context->identifier));
  }

  return format_plural(count($names), '@identifier is "@tids"', '@identifier is one of "@tids"', array(
      '@tids' => implode(', ', $names),
      '@identifier' => $context->identifier,
    )
  );
}
