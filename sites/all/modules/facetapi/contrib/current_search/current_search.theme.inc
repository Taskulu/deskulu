<?php

/**
 * @file
 * Theme functions for the Current Search Blocks module.
 */

/**
 * Returns HTML for the inactive facet item's count.
 *
 * @param $variables
 *   An associative array containing:
 *   - text: The text being displayed.
 *   - wrapper: A boolean flagging whether wrapper markup should be added.
 *   - element: The HTML element the text is wrapped in.
 *   - css: A boolean flagging whether a CSS class should be added to the
 *     wrapper element.
 *   - class: An array of CSS classes.
 *   - options: An associative array of options containing:
 *     - html: Whether or not "text" is rendered HTML, otherwise the string is
 *       passed through check_plain(). Defaults to FALSE.
 *
 * @ingroup themeable
 */
function theme_current_search_text(array $variables) {
  // Initializes output, sanitizes text if necessary.
  $sanitize = empty($variables['options']['html']);
  $output = ($sanitize) ? check_plain($variables['text']) : $variables['text'];

  // Adds wrapper markup and CSS classes.
  if ($variables['wrapper'] && $variables['element']) {
    $attributes = array('class' => $variables['class']);
    $element = check_plain($variables['element']);
    $output = '<' . $element . drupal_attributes($attributes) . '>' . $output . '</' . $element . '>';
  }

  return $output;
}

/**
 * Returns HTML for the group list title.
 *
 * @param $variables
 *   An associative array containing:
 *   - title: The title of the group list.
 *
 * @ingroup themeable
 */
function theme_current_search_group_title(array $variables) {
  return '<h4 class="current-search-group-title">' . $variables['title'] . '</h4>';
}

/**
 * Adds wrapper markup around the current search item.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: The render array for the current search item.
 *
 * @ingroup themeable
 */
function theme_current_search_item_wrapper(array $variables) {
  $element = $variables['element'];
  $attributes = array(
    'class' => array(
      'current-search-item',
      drupal_html_class('current-search-item-' . $element['#current_search_id']),
      drupal_html_class('current-search-item-' . $element['#current_search_name']),
    ),
  );
  return '<div' . drupal_attributes($attributes) . '>' . $element['#children'] . '</div>';
}

/**
 * Adds wrapper markup around the group.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: The render array for the current search group.
 *
 * @ingroup themeable
 */
function theme_current_search_group_wrapper(array $variables) {
  $element = $variables['element'];
  $attributes = array('class' => array('current-search-group current-search-group-' . drupal_html_class($element['#facet_name'])), 'id' => $element['#id']);
  return '<div' . drupal_attributes($attributes) . '>' . $element['#children'] . '</div>';
}

/**
 * Returns HTML for a grouped active facet item.
 *
 * @param $variables
 *   An associative array containing the keys 'text', 'path', 'options', and
 *   'count'.
 *
 * @ingroup themeable
 */
function theme_current_search_link_active($variables) {
  // Builds accessible markup.
  // @see http://drupal.org/node/1316580
  $accessible_vars = array(
    'text' => $variables['text'],
    'active' => TRUE,
  );
  $accessible_markup = theme('facetapi_accessible_markup', $accessible_vars);

  // Sanitizes the link text if necessary.
  $sanitize = empty($variables['options']['html']);
  $variables['text'] = ($sanitize) ? check_plain($variables['text']) : $variables['text'];

  // Adds the deactivation widget.
  $variables['text'] .= theme('current_search_deactivate_widget');

  // Resets link text, sets to options to HTML since we already sanitized the
  // link text and are providing additional markup for accessibility.
  $variables['text'] .= ' ' . $accessible_markup;
  $variables['options']['html'] = TRUE;
  return theme_link($variables);
}

/**
 * Returns HTML for a search keys facet item.
 *
 * @param $variables
 *   An associative array containing the keys 'keys' and 'adapter'.
 *
 * @ingroup themeable
 */
function theme_current_search_keys($variables) {
  return check_plain($variables['keys']);
}

/**
 * Returns HTML for the deactivation widget.
 *
 * @param $variables
 *   An associative array containing the keys 'text', 'path', 'options', and
 *   'count'.
 *
 * @ingroup themeable
 */
function theme_current_search_deactivate_widget($variables) {
  return ' [X]';
}

/**
 * Returns the sort table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_current_search_sort_settings_table($variables) {
  $output = '';

  // Builds table rows.
  $rows = array();
  foreach ($variables['element']['#current_search']['items'] as $name => $settings) {
    $rows[$name] = array(
      'class' => array('draggable'),
      'data' => array(
        drupal_render($variables['element'][$name]['item']),
        drupal_render($variables['element'][$name]['weight']),
        array(
          'data' => drupal_render($variables['element'][$name]['remove']),
          'class' => 'current-search-remove-link',
        ),
      ),
    );
  }

  // Builds table with drabble rows, returns output.
  $table_id = 'current-search-sort-settings';
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'current-search-sort-weight');
  $output .= drupal_render_children($variables['element']);
  $output .= theme('table', array('rows' => $rows, 'attributes' => array('id' => $table_id)));
  return $output;
}
