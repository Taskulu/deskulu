<?php

/**
 * @file
 * Page callbacks for the taxonomy module, i18n remake.
 */

/**
 * Menu callback; displays all nodes associated with a term.
 *
 * @param $term
 *   The taxonomy term.
 * @return
 *   The page content.
 */
function i18n_taxonomy_term_page($term) {
  $term = i18n_taxonomy_localize_terms($term);

  // Assign the term name as the page title.
  drupal_set_title($term->name);

  // Build breadcrumb based on the hierarchy of the term.
  $current = (object) array(
    'tid' => $term->tid,
  );
  // @todo This overrides any other possible breadcrumb and is a pure hard-coded
  //   presumption. Make this behavior configurable per vocabulary or term.
  $breadcrumb = array();
  while ($parents = taxonomy_get_parents($current->tid)) {
    $parents = i18n_taxonomy_localize_terms($parents);
    $current = array_shift($parents);
    $breadcrumb[] = l($current->name, 'taxonomy/term/' . $current->tid);
  }
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb = array_reverse($breadcrumb);
  drupal_set_breadcrumb($breadcrumb);
  drupal_add_feed('taxonomy/term/' . $term->tid . '/feed', 'RSS - ' . $term->name);

  $build = array();

  $build['term_heading'] = array(
    '#prefix' => '<div class="term-listing-heading">',
    '#suffix' => '</div>',
    'term' => taxonomy_term_view($term, 'full'),
  );

  if ($nids = taxonomy_select_nodes($term->tid, TRUE, variable_get('default_nodes_main', 10))) {
    $nodes = node_load_multiple($nids);
    $build += node_view_multiple($nodes);
    $build['pager'] = array(
      '#theme' => 'pager',
      '#weight' => 5,
    );
  }
  else {
    $build['no_content'] = array(
      '#prefix' => '<p>',
      '#markup' => t('There is currently no content classified with this term.'),
      '#suffix' => '</p>',
    );
  }
  return $build;
}

/**
 * Helper function for autocompletion. Replacement for taxonomy_autocomplete
 */
function i18n_taxonomy_autocomplete_field($field_name, $tags_typed = '') {
  // Part of the criteria for the query come from the field's own settings.
  $field = field_info_field($field_name);
  $vids = array();
  $vocabularies = taxonomy_vocabulary_get_names();
  foreach ($field['settings']['allowed_values'] as $tree) {
    $vids[] = $vocabularies[$tree['vocabulary']]->vid;
  }
  // This has been redirected from taxonomy module so we add current language and no language
  // Because some of the vocabularies may not have language
  $langcode = array(i18n_langcode(), LANGUAGE_NONE);
  return _i18n_taxonomy_autocomplete($langcode, $vids, $tags_typed);
}

/**
 * Helper function for autocompletion. Select by language
 */
function i18n_taxonomy_autocomplete_language($langcode, $vocabulary, $tags_typed = '') {
  $vids = $vocabulary ? array($vocabulary->vid) : NULL;
  return _i18n_taxonomy_autocomplete($langcode, $vids, $tags_typed);
}

/**
 * Helper function for autocompletion
 */
function _i18n_taxonomy_autocomplete($langcode, $vids, $tags_typed = '') {
  // The user enters a comma-separated list of tags. We only autocomplete the last tag.
  $tags_typed = drupal_explode_tags($tags_typed);
  $tag_last = drupal_strtolower(array_pop($tags_typed));

  $matches = array();
  if ($langcode && $tag_last != '') {
    $query = db_select('taxonomy_term_data', 't')
      ->fields('t', array('tid', 'name'));
    $query->addTag('translatable');
    $query->addTag('term_access');
    // Disable i18n_select for this query
    $query->addTag('i18n_select');
    // Add language condition
    $query->condition('t.language', $langcode);

    // Do not select already entered terms.
    if (!empty($tags_typed)) {
      $query->condition('t.name', $tags_typed, 'NOT IN');
    }
    // There may be vocabulary restrictions
    if ($vids) {
      $query->condition('t.vid', $vids);
    }
    // Select rows that match by term name.
    $tags_return = $query
      ->condition('t.name', '%' . db_like($tag_last) . '%', 'LIKE')
      ->range(0, 10)
      ->execute()
      ->fetchAllKeyed();

    $prefix = count($tags_typed) ? drupal_implode_tags($tags_typed) . ', ' : '';

    $term_matches = array();
    foreach ($tags_return as $tid => $name) {
      $n = $name;
      // Term names containing commas or quotes must be wrapped in quotes.
      if (strpos($name, ',') !== FALSE || strpos($name, '"') !== FALSE) {
        $n = '"' . str_replace('"', '""', $name) . '"';
      }
      $term_matches[$prefix . $n] = check_plain($name);
    }
  }

  drupal_json_output($term_matches);
}
