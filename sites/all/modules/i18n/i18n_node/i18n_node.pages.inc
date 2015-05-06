<?php

/**
 * @file
 * User page callbacks for the translation module.
 */


/**
 * Replacement for node_add_page.
 */
function i18n_node_add_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  // Bypass the node/add listing if only one content type is available.
  if (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }
  foreach ($content as &$item) {
    // Type machine name will be the first page argument
    $page_arguments = unserialize($item['page_arguments']);
    // Check whether this has a node type, other items may be here too, see #1264662
    $type = isset($page_arguments[0]) ? $page_arguments[0] : NULL;
    if ($type) {
      // We just need to translate the description, the title is translated by the menu system
      // The string will be filtered (xss_admin) on the theme layer
      $item['description'] = i18n_node_translate_type($type, 'description', $item['description'], array('sanitize' => FALSE));
    }
  }
  return theme('node_add_list', array('content' => $content));
}

/**
 * Overview page for a node's translations.
 *
 * @param $node
 *   Node object.
 */
function i18n_node_translation_overview($node) {
  include_once DRUPAL_ROOT . '/includes/language.inc';

  if (!empty($node->tnid)) {
    // Already part of a set, grab that set.
    $tnid = $node->tnid;
    $translations = translation_node_get_translations($node->tnid);
  }
  else {
    // We have no translation source nid, this could be a new set, emulate that.
    $tnid = $node->nid;
    $translations = array($node->language => $node);
  }

  $type = variable_get('translation_language_type', LANGUAGE_TYPE_INTERFACE);
  $header = array(t('Language'), t('Title'), t('Status'), t('Operations'));

  // Modes have different allowed languages
  foreach (i18n_node_language_list($node) as $langcode => $language_name) {
    if ($langcode == LANGUAGE_NONE) {
      // Never show language neutral on the overview.
      continue;
    }
    $options = array();
    if (isset($translations[$langcode])) {
      // Existing translation in the translation set: display status.
      // We load the full node to check whether the user can edit it.
      $translation_node = node_load($translations[$langcode]->nid);
      $path = 'node/' . $translation_node->nid;
      $title = i18n_node_translation_link($translation_node->title, $path, $langcode);
      if (node_access('update', $translation_node)) {
        $text = t('edit');
        $path = 'node/' . $translation_node->nid . '/edit';
        $options[] = i18n_node_translation_link($text, $path, $langcode);
      }
      $status = $translation_node->status ? t('Published') : t('Not published');
      $status .= $translation_node->translate ? ' - <span class="marker">' . t('outdated') . '</span>' : '';
      if ($translation_node->nid == $tnid) {
        $language_name = t('<strong>@language_name</strong> (source)', array('@language_name' => $language_name));
      }
    }
    else {
      // No such translation in the set yet: help user to create it.
      $title = t('n/a');
      if (node_access('create', $node->type)) {
        $text = t('add translation');
        $path = 'node/add/' . str_replace('_', '-', $node->type);
        $query = array('query' => array('translation' => $node->nid, 'target' => $langcode));
        $options[] = i18n_node_translation_link($text, $path, $langcode, $query);
      }
      $status = t('Not translated');
    }
    $rows[] = array($language_name, $title, $status, implode(" | ", $options));
  }

  drupal_set_title(t('Translations of %title', array('%title' => $node->title)), PASS_THROUGH);

  $build['translation_node_overview'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  if (user_access('administer content translations')) {
    $build['translation_node_select'] = drupal_get_form('i18n_node_select_translation', $node, $translations);
  }
  return $build;
}

/**
 * Create link for node translation. This may be a 'edit' or a 'add translation' link.
 */
function i18n_node_translation_link($text, $path, $langcode, $options = array()) {
  $type = variable_get('translation_language_type', LANGUAGE_TYPE_INTERFACE);
  $links = language_negotiation_get_switch_links($type, $path);
  // When node not published, links don't have href so we use path instead
  // Note: this is a bug in Core translation module, see http://drupal.org/node/1137074
  if (!empty($links->links[$langcode]) && !empty($links->links[$langcode]['href'])) {
    $options += array('attributes' => array(), 'html' => FALSE);
    $options['attributes'] += $links->links[$langcode]['attributes'];
    $options += $links->links[$langcode];
    $path = $links->links[$langcode]['href'];
  }
  return l($text, $path, $options);
}

/**
 * Form to select existing nodes as translation
 *
 * This one uses autocomplete fields for all languages
 */
function i18n_node_select_translation($form, &$form_state, $node, $translations) {
  $form['node'] = array('#type' => 'value', '#value' => $node);
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select translations for %title', array('%title' => $node->title)),
    '#tree' => TRUE,
    '#theme' => 'i18n_node_select_translation',
    '#description' => t("Alternatively, you can select existing nodes as translations of this one or remove nodes from this translation set. Only nodes that have the right language and don't belong to other translation set will be available here.")
  );
  foreach (i18n_node_language_list($node) as $langcode => $language_name) {
    if ($langcode != $node->language && $langcode != LANGUAGE_NONE) {
      $nid = isset($translations[$langcode]) ? $translations[$langcode]->nid : 0;
      $form['translations']['nid'][$langcode] = array(
        '#type' => 'value',
        '#value' => $nid,
      );
      $form['translations']['language'][$langcode] = array(
        '#type' => 'value',
        '#value' => $language_name,
      );
      $form['translations']['node'][$langcode] = array(
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#autocomplete_path' => 'i18n/node/autocomplete/' . $node->type . '/' . $langcode,
        '#default_value' => $nid ? i18n_node_nid2autocomplete($nid) : '',
      );
    }
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Update translations'),
  );
  //$form['buttons']['clean'] = array('#type' => 'submit', '#value' => t('Delete translation set'));
  return $form;
}

/**
 * Form validation
 */
function i18n_node_select_translation_validate($form, &$form_state) {
  foreach ($form_state['values']['translations']['node'] as $lang => $title) {
    if (!$title) {
      $nid = 0;
    }
    else {
      $nid = i18n_node_autocomplete2nid($title, "translations][node][$lang", array($form_state['values']['node']->type), array($lang));
    }
    $form_state['values']['translations']['nid'][$lang] = $nid;
  }
}

/**
 * Form submission: update / delete the translation set
 */
function i18n_node_select_translation_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : NULL;
  $node = $form_state['values']['node'];
  $translations = $node->tnid ? translation_node_get_translations($node->tnid) : array($node->language => $node);
  foreach ($translations as $trans) {
    $current[$trans->language] = $trans->nid;
  }
  $update = array($node->language => $node->nid) + array_filter($form_state['values']['translations']['nid']);
  // Compute the difference to see which are the new translations and which ones to remove
  $new = array_diff_assoc($update, $current);
  $remove = array_diff_assoc($current, $update);

  // The tricky part: If the existing source is not in the new set, we need to create a new tnid
  if ($node->tnid && in_array($node->tnid, $update)) {
    $tnid = $node->tnid;
    $add = $new;
  }
  else {
    // Create new tnid, which is the source node
    $tnid = $node->nid;
    $add = $update;
  }
  // Now update values for all nodes
  if ($add) {
    db_update('node')
      ->fields(array(
        'tnid' => $tnid,
      ))
      ->condition('nid', $add)
      ->execute();
    entity_get_controller('node')->resetCache($add);
    if (count($new)) {
      drupal_set_message(format_plural(count($new), 'Added a node to the translation set.', 'Added @count nodes to the translation set.'));
    }
  }
  if ($remove) {
    db_update('node')
      ->fields(array(
        'tnid' => 0,
      ))
      ->condition('nid', $remove)
      ->execute();
    entity_get_controller('node')->resetCache($remove);
    drupal_set_message(format_plural(count($remove), 'Removed a node from the translation set.', 'Removed @count nodes from the translation set.'));
  }
}

/**
 * Node title autocomplete callback
 */
function i18n_node_autocomplete($type, $language, $string = '') {
  $params = array('type' => $type, 'language' => $language, 'tnid' => 0);
  $matches = array();
  foreach (_i18n_node_references($string, 'contains', $params) as $id => $row) {
    // Add a class wrapper for a few required CSS overrides.
    $matches[$row['title'] . " [nid:$id]"] = '<div class="reference-autocomplete">' . $row['rendered'] . '</div>';
  }
  drupal_json_output($matches);
}

/**
 * Generates 'title [nid:$nid]' for the autocomplete field
 */
function i18n_node_nid2autocomplete($nid) {
  if ($node = node_load($nid)) {
    return $node->title . ' [nid:' . $nid . ']';
  }
  else {
    return t('Not found');
  }
}

/**
 * Reverse mapping from node title to nid
 *
 * We also handle autocomplete values (title [nid:x]) and validate the form
 */
function i18n_node_autocomplete2nid($name, $field = NULL, $type, $language) {
  if (!empty($name)) {
    preg_match('/^(?:\s*|(.*) )?\[\s*nid\s*:\s*(\d+)\s*\]$/', $name, $matches);
    if (!empty($matches)) {
      // Explicit [nid:n].
      list(, $title, $nid) = $matches;
      if (!empty($title) && ($node = node_load($nid)) && $title != $node->title) {
        if ($field) {
          form_set_error($field, t('Node title mismatch. Please check your selection.'));
        }
        $nid = NULL;
      }
    }
    else {
      // No explicit nid.
      $reference = _i18n_node_references($name, 'equals', array('type' => $type, 'language' => $language), 1);
      if (!empty($reference)) {
        $nid = key($reference);
      }
      elseif ($field) {
        form_set_error($field, t('Found no valid post with that title: %title', array('%title' => $name)));
      }
    }
  }
  return !empty($nid) ? $nid : NULL;
}

/**
 * Find node title matches.
 *
 * @param $string
 *   String to match against node title
 * @param $match
 *   Match mode: 'contains', 'equals', 'starts_with'
 * @param $params
 *   Other query arguments: type, language or numeric ones
 */
function _i18n_node_references($string, $match = 'contains', $params = array(), $limit = 10) {
  $query = db_select('node', 'n')
    ->fields('n', array('nid', 'title' , 'type'))
    ->orderBy('n.title')
    ->orderBy('n.type')
    ->range(0, $limit);

  foreach ($params as $key => $value) {
    $query->condition($key, $value);
  }

  switch ($match) {
    case 'equals':
      $query->condition('n.title', $string);
      break;

    case 'starts_with':
      $query->condition('n.title', $string . '%', 'LIKE');
      break;

    case 'contains':
    default:
      $query->condition('n.title', '%' . $string . '%', 'LIKE');
      break;
  }

  // Disable and reenable i18n selection mode so no language conditions are inserted
  i18n_select(false);
  $references = array();
  foreach ($query->execute() as $node) {
    $references[$node->nid] = array(
      'title' => $node->title,
      'rendered' => check_plain($node->title),
    );
  }
  i18n_select(true);
  return $references;
}

/**
 * Theme select translation form
 * @ingroup themeable
 */
function theme_i18n_node_select_translation($variables) {
  $elements = $variables['element'];
  $output = '';
  if (isset($elements['nid'])) {
    $rows = array();
    foreach (element_children($elements['nid']) as $lang) {
      $rows[] = array(
        $elements['language'][$lang]['#value'],
        drupal_render($elements['node'][$lang]),
      );
    }
    $output .= theme('table', array('rows' => $rows));
    $output .= drupal_render_children($elements);
  }
  return $output;
}
