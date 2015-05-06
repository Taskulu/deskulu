<?php

/**
 * @file
 * Plugin to provide an argument handler for a user id
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Forum: ID"),
  // Keyword to use for %substitution.
  'keyword' => 'forum',
  'description' => t('Creates a forum context from a forum ID argument.'),
  'context' => 'advanced_forum_argument_forum_id_context',
  'placeholder form' => array(
    '#type' => 'textfield',
    '#description' => t('Enter the forum ID of a form for this argument'),
  ),
  'settings form' => 'advanced_forum_forum_id_settings_form',
  'breadcrumb' => 'advanced_forum_forum_id_breadcrumb',
  'default' => array('breadcrumb' => TRUE),
);

/**
 * Discover if this argument gives us the user we crave.
 */
function advanced_forum_argument_forum_id_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return ctools_context_create_empty('forum');
  }

  if (!is_numeric($arg)) {
    return NULL;
  }

  if ($arg != 0) {
    $term = taxonomy_term_load($arg);
  }

  if ($arg == 0 || empty($term) || $term->vid != variable_get('forum_nav_vocabulary', 0)) {
    $term = taxonomy_vocabulary_load(variable_get('forum_nav_vocabulary', 0));
    $term->tid = 0;
  }

  if (!$term) {
    return NULL;
  }

  return ctools_context_create('forum', $term);
}

/**
 * Settings form for the argument.
 */
function advanced_forum_forum_id_settings_form(&$form, &$form_state, $conf) {
  $form['settings']['breadcrumb'] = array(
    '#title' => t('Inject hierarchy into breadcrumb trail'),
    '#type' => 'checkbox',
    '#default_value' => !empty($conf['breadcrumb']),
    '#description' => t('If checked, forum parents will appear in the breadcrumb trail.'),
  );
}

/**
 * Inject the breadcrumb trail if necessary.
 */
function advanced_forum_forum_id_breadcrumb($conf, $context) {
  if (empty($conf['breadcrumb'])) {
    return;
  }

  $breadcrumb = array();
  if (isset($context->data->parents)) {
    $parents = array_reverse($context->data->parents);
    if (!empty($parents)) {
      foreach ($parents as $p) {
        if ($p->tid != $context->data->tid) {
          $breadcrumb[] = l($p->name, 'forum/' . $p->tid);
        }
      }
    }
  }

  $breadcrumb = array_merge(drupal_get_breadcrumb(), $breadcrumb);
  drupal_set_breadcrumb($breadcrumb);
}
