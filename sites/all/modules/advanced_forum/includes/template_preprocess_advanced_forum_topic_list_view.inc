<?php

/**
 * @file
 * Holds the contents of a preprocess function moved into its own file
 * to ease memory requirements and having too much code in one file.
 */

/**
 * Preprocess advanced forum topic list.
 */
function _template_preprocess_advanced_forum_topic_list_view(&$variables) {

  // Even though this isn't needed for the actual topic list view, the other
  // views use this same style and need the AF style files added.
  _advanced_forum_add_files();
  advanced_forum_add_template_suggestions("topic-list-view", $variables);

  $view = $variables['view'];

  // We need the raw data for this grouping, which is passed in as $variables['rows'].
  // However, the template also needs to use for the rendered fields.  We
  // therefore swap the raw data out to a new variable and reset $variables['rows']
  // so that it can get rebuilt.
  $result = $variables['rows'];
  $variables['rows'] = array();
  $variables['field_classes'] = array();

  $options = $view->style_plugin->options;
  $handler = $view->style_plugin;

  $fields = &$view->field;
  $columns = $handler->sanitize_columns($options['columns'], $fields);

  $active = !empty($handler->active) ? $handler->active : '';
  $order = !empty($handler->order) ? $handler->order : 'asc';

  $query = tablesort_get_query_parameters();
  if (isset($view->exposed_raw_input)) {
    $query += $view->exposed_raw_input;
  }

  $header = array();

  $forum = $view->style_plugin->get_forum_ids();

  // Fields must be rendered in order as of Views 2.3, so we will pre-render
  // everything.
  $renders = $handler->render_fields($result);

  $keys = array_keys($view->field);
  $stickies_done = FALSE;
  $sticky_count = 0;

  foreach ($result as $count => $row) {
    // Grab the teaser so it can be used on the title of the td as a popup
    // preview. By default, the view provides the teaser as a popup on the
    // title link.
    if (!empty($renders[$count]['teaser'])) {
      $teaser = $renders[$count]['teaser'];
      $teaser = htmlentities($teaser);
      $teasers[$count] = $teaser;
    }

    // Determine stickiness.
    if (!$stickies_done) {
      $sticky[$count] = !empty($row->topic_is_sticky);
      if ($sticky[$count]) {
        $sticky_count++;
        $variables['row_classes'][$count][] = 'sticky';
      }
      else {
        $stickies_done = TRUE;
        if ($sticky_count) {
          // This ends up appearing only if we have both sticky and non sticky posts.
          // Otherwise it will never show up.
          $variables['row_classes'][$count - 1][] = 'forum-last-sticky';
        }
      }
    }

    // Shadow (moved) topics.
    if ($forum && !in_array($row->topic_actual_forum, $forum)) {
      $term = taxonomy_term_load($row->topic_actual_forum);
      $variables['shadow'][$count] = theme('advanced_forum_shadow_topic', array(
        'title' => $row->node_title,
        'nid' => $row->nid,
        'new_forum' => empty($term->name) ? "" : $term->name,
          ));
    }
    else {
      $variables['shadow'][$count] = FALSE;
    }
  }

  // Make the sticky variable available to the template.
  $variables['sticky'] = $sticky;
  $variables['teasers'] = empty($teasers) ? NULL : $teasers;

  foreach ($columns as $field => $column) {
    // Create a second variable so we can easily find what fields we have and what the
    // CSS classes should be.
    $variables['fields'][$field] = drupal_clean_css_identifier($field);
    if ($active == $field) {
      $variables['fields'][$field] .= ' active';
    }

    // Render the header labels.
    if ($field == $column && empty($fields[$field]->options['exclude'])) {
      $label = check_plain(!empty($fields[$field]) ? $fields[$field]->label() : '');
      if (empty($options['info'][$field]['sortable']) || !$fields[$field]->click_sortable()) {
        $variables['header'][$field] = $label;
      }
      else {
        $initial = !empty($options['info'][$field]['default_sort_order']) ? $options['info'][$field]['default_sort_order'] : 'asc';

        if ($active == $field) {
          $initial = ($order == 'asc') ? 'desc' : 'asc';
        }

        $title = t('sort by @s', array('@s' => $label));
        if ($active == $field) {
          $label .= theme('tablesort_indicator', array('style' => $initial));
        }

        $query['order'] = $field;
        $query['sort'] = $initial;
        $link_options = array(
          'html' => TRUE,
          'attributes' => array('title' => $title),
          'query' => $query,
        );
        $variables['header'][$field] = l($label, $_GET['q'], $link_options);
      }

      $variables['header_classes'][$field] = '';
      // Set up the header label class.
      if ($fields[$field]->options['element_default_classes']) {
        $variables['header_classes'][$field] .= "views-field views-field-" . $variables['fields'][$field];
      }
      $class = $fields[$field]->element_label_classes(0);
      if ($class) {
        if ($variables['header_classes'][$field]) {
          $variables['header_classes'][$field] .= ' ';
        }
        $variables['header_classes'][$field] .= $class;
      }

      // Add a header label wrapper if one was selected.
      if ($variables['header'][$field]) {
        $element_label_type = $fields[$field]->element_label_type(TRUE, TRUE);
        if ($element_label_type) {
          $variables['header'][$field] = '<' . $element_label_type . '>' . $variables['header'][$field] . '</' . $element_label_type . '>';
        }
      }
    }

    // Add a CSS align class to each field if one was set.
    if (!empty($options['info'][$field]['align'])) {
      $variables['fields'][$field] .= ' ' . drupal_clean_css_identifier($options['info'][$field]['align']);
    }

    // Render each field into its appropriate column.
    foreach ($result as $num => $row) {
      // Add field classes.
      $variables['field_classes'][$field][$num] = '';
      if ($fields[$field]->options['element_default_classes']) {
        $variables['field_classes'][$field][$num] = "views-field views-field-" . $variables['fields'][$field];
      }
      if ($classes = $fields[$field]->element_classes($num)) {
        if ($variables['field_classes'][$field][$num]) {
          $variables['field_classes'][$field][$num] .= ' ';
        }

        $variables['field_classes'][$field][$num] .= $classes;
      }
      $variables['field_attributes'][$field][$num] = array();

      if (!empty($fields[$field]) && empty($fields[$field]->options['exclude'])) {
        $field_output = $renders[$num][$field];
        $element_type = $fields[$field]->element_type(TRUE, TRUE);
        if ($element_type) {
          $field_output = '<' . $element_type . '>' . $field_output . '</' . $element_type . '>';
        }

        // Don't bother with separators and stuff if the field does not show up.
        if (empty($field_output) && !empty($variables['rows'][$num][$column])) {
          continue;
        }

        // Place the field into the column, along with an optional separator.
        if (!empty($variables['rows'][$num][$column])) {
          if (!empty($options['info'][$column]['separator'])) {
            $variables['rows'][$num][$column] .= filter_xss_admin($options['info'][$column]['separator']);
          }
        }
        else {
          $variables['rows'][$num][$column] = '';
        }

        $variables['rows'][$num][$column] .= $field_output;
      }
    }
  }

  $count = 0;
  foreach ($variables['rows'] as $num => $row) {
    $variables['row_classes'][$num][] = ($count++ % 2 == 0) ? 'odd' : 'even';
    if ($row_class = $handler->get_row_class($num)) {
      $variables['row_classes'][$num][] = $row_class;
    }
  }

  $variables['row_classes'][0][] = 'views-row-first';
  $variables['row_classes'][count($variables['row_classes']) - 1][] = 'views-row-last';

  if (!variable_get('advanced_forum_keep_classes', FALSE) || !is_array($variables['classes_array'])) {
    // Add in our classes overwriting existing.
    $variables['classes_array'] = array();
  }

  $variables['classes_array'][] = 'forum-table forum-table-topics';
  if (empty($variables['rows']) && !empty($options['empty_table'])) {
    $variables['rows'][0][0] = $view->display_handler->render_area('empty');
    // Calculate the amounts of rows with output.
    $variables['field_attributes'][0][0]['colspan'] = count($variables['header']);
    $variables['field_classes'][0][0] = 'views-empty';
  }

  if (!empty($options['sticky'])) {
    drupal_add_js('misc/tableheader.js');
    $variables['classes_array'][] = "sticky-enabled";
  }
  $variables['classes_array'][] = 'cols-' . count($variables['header']);

  if (!empty($handler->options['summary'])) {
    $variables['attributes_array'] = array('summary' => $handler->options['summary']);
  }
}
