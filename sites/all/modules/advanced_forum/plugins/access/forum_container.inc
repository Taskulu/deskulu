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
  'title' => t("Forum: container"),
  'description' => t('Control access by whether or not the forum is a container.'),
  'callback' => 'advanced_forum_forum_container_ctools_access_check',
  'default' => array('container' => 0),
  'settings form' => 'advanced_forum_forum_container_ctools_access_settings',
  'settings form submit' => 'advanced_forum_forum_container_ctools_access_settings_submit',
  'summary' => 'advanced_forum_forum_container_ctools_acesss_summary',
  'required context' => new ctools_context_required(t('Forum'), array('forum')),
);

/**
 * Settings form for the 'by term_vocabulary' access plugin.
 */
function advanced_forum_forum_container_ctools_access_settings($form, &$form_state, $conf) {
  $form['settings']['container'] = array(
    '#type' => 'select',
    '#title' => t('Container'),
    '#options' => array(
      0 => t('Pass if forum is a container'),
      1 => t('Pass if forum is not a container'),
    ),
    '#default_value' => $conf['container'],
  );

  return $form;
}

/**
 * Compress the term_vocabularys allowed to the minimum.
 */
function advanced_forum_forum_container_ctools_access_settings_submit($form, &$form_state) {
  $form_state['values']['settings']['rids'] = array_keys(array_filter($form_state['values']['settings']['rids']));
}

/**
 * Check for access.
 */
function advanced_forum_forum_container_ctools_access_check($conf, $context) {
  // As far as I know there should always be a context at this point, but this
  // is safe.
  if (empty($context) || empty($context->data)) {
    return FALSE;
  }

  // Xor returns false if the two bools are the same, and true if they are not.
  return $context->data->container xor !empty($conf['container']);
}

/**
 * Provide a summary description based upon the checked term_vocabularys.
 */
function advanced_forum_forum_container_ctools_acesss_summary($conf, $context) {
  $comparison = empty($conf['container']) ? "is" : 'is not';

  return t('@id1 @comp a forum container', array('@comp' => $comparison, '@id1' => $context->identifier));
}
