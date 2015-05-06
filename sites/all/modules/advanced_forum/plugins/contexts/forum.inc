<?php

/**
 * @file
 * Plugin to provide a user context
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Forum"),
  'description' => t('A single forum object.'),
  'context' => 'advanced_forum_forum_context_create_forum',
  'settings form' => 'advanced_forum_forum_context_settings_form',
  'settings form validate' => 'advanced_forum_forum_context_settings_form_validate',
  'keyword' => 'forum',
  'context name' => 'forum',
  'convert list' => array(
    'tid' => t('Forum ID'),
    'name' => t('Forum name'),
  ),
  'convert' => 'advanced_forum_forum_context_convert',
  'defaults' => array('tid' => 0),
);

/**
 * Create a context, either from manual configuration or from an argument on the URL.
 *
 * @param bool $empty
 *   If true, just return an empty context.
 *
 * @param array|null $data
 *   If from settings form, an array as from a form. If from argument, a string.
 *
 * @param array|bool $conf
 *   TRUE if the $data is coming from admin configuration, FALSE if it's from a URL arg.
 *
 * @return object.
 *   a Context object.
 */
function advanced_forum_forum_context_create_forum($empty, $data = NULL, $conf = FALSE) {
  $context = new ctools_context(array('forum', 'term'));
  $context->plugin = 'forum';

  if ($empty) {
    return $context;
  }

  if ($conf) {
    if (!empty($data['tid'])) {
      $data = taxonomy_term_load($data['tid']);
    }
    else {
      $data = taxonomy_vocabulary_load(variable_get('forum_nav_vocabulary', 0));
      $data->tid = 0;
    }
  }

  if (!empty($data)) {
    $data->container = (!$data->tid || in_array($data->tid, variable_get('forum_containers', array())));
    $context->data = clone $data;
    $context->title = $data->name;
    $context->argument = $data->tid;
    $context->vid = variable_get('forum_nav_vocabulary', '');
    $context->vocabulary = taxonomy_vocabulary_load($context->vid);
    if ($data->tid) {
      $context->parents = taxonomy_get_parents_all($data->tid);
    }

    return $context;
  }
}

/**
 * Context settings form.
 */
function advanced_forum_forum_context_settings_form($form, &$form_state) {
  $conf = $form_state['conf'];

  if (empty($conf)) {
    $conf = array('tid' => 0);
  }

  $options = array();
  $vocabulary = taxonomy_vocabulary_load(variable_get('forum_nav_vocabulary', 0));
  $options[$vocabulary->vid] = $vocabulary->name;

  $tree = taxonomy_get_tree($vocabulary->vid);
  if ($tree) {
    foreach ($tree as $term) {
      $choice = new stdClass();
      $choice->option = array($term->tid => str_repeat('-', $term->depth + 1) . $term->name);
      $options[] = $choice;
    }
  }

  $form['tid'] = array(
    '#type' => 'select',
    '#title' => t('Forum'),
    '#default_value' => $conf['tid'],
    '#options' => $options,
  );

  return $form;
}

/**
 * Convert a context into a string.
 */
function advanced_forum_forum_context_convert($context, $type) {
  switch ($type) {
    case 'tid':
      return $context->data->tid;

    case 'name':
      $forum_term = $context->data;

      if (module_exists('i18n_taxonomy')) {
        $forum_term = i18n_taxonomy_localize_terms($forum_term);
      }

      return $forum_term->name;
  }
}
