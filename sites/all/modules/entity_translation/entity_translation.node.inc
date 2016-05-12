<?php

/**
 * @file
 * The node specific translation functions and hook implementations.
 */

/**
 * Identifies a content type which has translation support enabled.
 */
define('ENTITY_TRANSLATION_ENABLED', 4);

/**
 * Hides translation metadata.
 */
define('ENTITY_TRANSLATION_METADATA_HIDE', 0);

/**
 * Adds translation metadata to the original authoring information.
 */
define('ENTITY_TRANSLATION_METADATA_SHOW', 1);

/**
 * Replaces the original authoring information with translation metadata.
 */
define('ENTITY_TRANSLATION_METADATA_REPLACE', 2);

/**
 * Checks if the given entity has node translation enabled.
 */
function entity_translation_node($entity_type, $node) {
  return $entity_type == 'node' && function_exists('translation_supported_type') && translation_supported_type($node->type);
}

/**
 * Node-specific menu alterations.
 */
function entity_translation_node_menu_alter(&$items, $backup) {
  if (isset($backup['node'])) {
    $item = $backup['node'];
    // Preserve the menu router item defined by other modules.
    $callback['page callback'] = $item['page callback'];
    $callback['file'] = $item['file'];
    $callback['module'] = $item['module'];
    $access_arguments = array_merge(array(1, $item['access callback']), $item['access arguments']);
  }
  else {
    $callback = FALSE;
    $access_arguments = array(1);
  }

  $items['node/%node/translate']['page callback'] = 'entity_translation_overview';
  $items['node/%node/translate']['page arguments'] = array('node', 1, $callback);
  $items['node/%node/translate']['access arguments'] = $access_arguments;
  $items['node/%node/translate']['access callback'] = 'entity_translation_node_tab_access';
  $items['node/%node/translate']['file'] = 'entity_translation.admin.inc';
  $items['node/%node/translate']['module'] = 'entity_translation';
}

/**
 * Node specific access callback.
 */
function entity_translation_node_tab_access() {
  $args = func_get_args();
  $node = array_shift($args);
  if (entity_translation_node('node', $node)) {
    $function = array_shift($args);
    return call_user_func_array($function, $args);
  }
  else {
    return entity_translation_tab_access('node', $node);
  }
}

/**
 * Returns whether the given node type has support for translations.
 *
 * @return
 *   Boolean value.
 */
function entity_translation_node_supported_type($type) {
  return variable_get('language_content_type_' . $type, 0) == ENTITY_TRANSLATION_ENABLED;
}

/**
 * Implements hook_node_view().
 *
 * Provides content language switcher links to navigate among node translations.
 */
function entity_translation_node_view($node, $build_mode, $langcode) {
  if (!empty($node->translations) && drupal_multilingual() && entity_translation_node_supported_type($node->type) && !variable_get("entity_translation_hide_translation_links_{$node->type}", FALSE)) {
    $path = 'node/' . $node->nid;
    $links = EntityTranslationDefaultHandler::languageSwitchLinks($path);

    if (is_object($links) && !empty($links->links)) {
      $handler = entity_translation_get_handler('node', $node);
      $translations = $handler->getTranslations()->data;

      // Remove the link for the current language.
      unset($links->links[$langcode]);

      // Remove links to unavailable translations.
      foreach ($links->links as $langcode => $link) {
        if (!isset($translations[$langcode]) || !entity_translation_access('node', $translations[$langcode])) {
          unset($links->links[$langcode]);
        }
      }

      $node->content['links']['translation'] = array(
        '#theme' => 'links',
        '#links' => $links->links,
        '#attributes' => array('class' => 'links inline'),
      );
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Provides settings into the node content type form to choose for entity
 * translation metadata and comment filtering.
 */
function entity_translation_form_node_type_form_alter(&$form, &$form_state) {
  if (entity_translation_enabled('node')) {
    $type = $form['#node_type']->type;

    $t_args = array('!url' => url('admin/config/regional/entity_translation'));
    $form['workflow']['language_content_type']['#options'][ENTITY_TRANSLATION_ENABLED] = t('Enabled, with field translation');
    $form['workflow']['language_content_type']['#description'] .= ' <p>' . t('If field translation is selected you can have per-field translation for each available language. You can find more options in the <a href="!url">entity translation settings</a>.', $t_args) . '</p>';

    // Hide settings when entity translation is disabled for this content type.
    $states = array(
      'visible' => array(
        ':input[name="language_content_type"]' => array('value' => ENTITY_TRANSLATION_ENABLED),
      ),
    );

    $form['workflow']['entity_translation_hide_translation_links'] = array(
      '#type' => 'checkbox',
      '#default_value' => variable_get("entity_translation_hide_translation_links_$type", FALSE),
      '#title' => t('Hide content translation links'),
      '#description' => t('Hide the links to translations in content body and teasers. If you choose this option, switching language will only be available from the language switcher block.'),
      '#states' => $states,
    );

    $form['display']['entity_translation_node_metadata'] = array(
      '#type' => 'radios',
      '#title' => t('Translation post information'),
      '#description' => t('Whether the translation authoring information should be hidden, shown, or replace the node\'s authoring information.'),
      '#default_value' => variable_get("entity_translation_node_metadata_$type", ENTITY_TRANSLATION_METADATA_HIDE),
      '#options' => array(t('Hidden'), t('Shown'), t('Replacing post information')),
      '#states' => $states,
    );

    if (isset($form['comment'])) {
      $form['comment']['entity_translation_comment_filter'] = array(
        '#type' => 'checkbox',
        '#title' => t('Filter comments per language'),
        '#default_value' => variable_get("entity_translation_comment_filter_$type", FALSE),
        '#description' => t('Show only comments whose language matches content language.'),
        '#states' => $states,
      );
    }
  }
}

/**
 * Implements hook_preprocess_node().
 *
 * Alters node template variables to show/replace entity translation metadata.
 */
function entity_translation_preprocess_node(&$variables) {
  $node = $variables['node'];
  $submitted = variable_get("node_submitted_{$node->type}", TRUE);
  $mode = variable_get("entity_translation_node_metadata_{$node->type}", ENTITY_TRANSLATION_METADATA_HIDE);

  if ($submitted && $mode != ENTITY_TRANSLATION_METADATA_HIDE) {
    global $language_content, $user;

    $handler = entity_translation_get_handler('node', $node);
    $translations = $handler->getTranslations();
    $langcode = $language_content->language;

    if (isset($translations->data[$langcode]) && $langcode != $translations->original) {
      $translation = $translations->data[$langcode];
      $date = format_date($translation['created']);
      $name = FALSE;

      if ($node->uid != $translation['uid']) {
        $account = $user->uid != $translation['uid'] ? user_load($translation['uid']) : $user;
        $name = theme('username', array('account' => $account));
      }

      switch ($mode) {
        case ENTITY_TRANSLATION_METADATA_SHOW:
          $variables['date'] .= ' (' . t('translated on <em>!date</em>', array('!date' => $date)) . ')';
          if ($name) {
            $variables['name'] .= ' (' . t('translated by !name', array('!name' => $name)) . ')';
          }
          break;

        case ENTITY_TRANSLATION_METADATA_REPLACE:
          $variables['date'] = $date;
          if ($name) {
            $variables['name'] = $name;
          }
          break;
      }
    }
  }
}

/**
 * Returns whether the given comment type has support for translations.
 *
 * @return
 *   Boolean value.
 */
function entity_translation_comment_supported_type($comment_type) {
  $type = str_replace('comment_node_', '', $comment_type);
  return entity_translation_node_supported_type($type);
}

/**
 * Implements hook_query_TAG_alter().
 *
 * Filters out node comments by content language.
 *
 * @todo Find a way to track node comment statistics per language.
 */
function entity_translation_query_comment_filter_alter(QueryAlterableInterface $query) {
  $node = $query->getMetaData('node');
  if (!empty($node->type) && variable_get("entity_translation_comment_filter_{$node->type}", FALSE)) {
    // Determine alias for "comment" table.
    $comment_alias = FALSE;
    foreach ($query->getTables() as $table) {
      if (is_string($table['table']) && $table['table'] == 'comment') {
        $comment_alias = $table['alias'];
        break;
      }
    }
    // Only show comments without language or matching the current content language.
    if ($comment_alias) {
      $query->condition(db_or()
        ->condition($comment_alias . '.language', $GLOBALS['language_content']->language)
        ->condition($comment_alias . '.language', LANGUAGE_NONE)
      );
    }
  }
}

/**
 * Implements hook_node_update_index().
 *
 * Add translated node content to search index.
 */
function entity_translation_node_update_index($node) {
  $text = '';
  $langcodes = array_keys(language_list());
  $translations = entity_translation_get_handler('node', $node)->getTranslations();
  foreach ($langcodes as $langcode) {
    // Skip the default language (already indexed by search.module).
    if ($GLOBALS[LANGUAGE_TYPE_CONTENT]->language != $langcode && isset($translations->data[$langcode])) {
      // Render the node in each language.
      node_build_content($node, 'search_index', $langcode);
      $rendered = drupal_render($node->content);
      $text .= '<h1>' . check_plain($node->title) . '</h1>' . $rendered;
    }
  }
  return $text;
}
