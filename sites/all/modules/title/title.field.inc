<?php

/**
 * @file
 * Implement a title field formater.
 */


/**
 * Implements hook_field_formatter_info().
 */
function title_field_formatter_info() {
  return array(
    'title_linked' => array(
      'label' => t('Linked and wrapped'),
      'field types' => array('text'),
      'settings' => array('title_style' => '', 'title_link' => '', 'title_class' => ''),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function title_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $settings = $instance['display'][$view_mode]['settings'];
  $element = array();

  $wrap_tags = array(
    '_none' => t('- None -'),
    'div' => t('DIV'),
    'h1' => t('H1'),
    'h2' => t('H2'),
    'h3' => t('H3'),
    'h4' => t('H4'),
    'h5' => t('H5'),
    'h6' => t('H6'),
    'span' => t('SPAN'),
  );
  $element['title_style'] = array(
    '#title' => t('Wrap title in tag'),
    '#type' => 'select',
    '#default_value' => !empty($settings['title_style']) ? $settings['title_style'] : '_none',
    '#options' => $wrap_tags,
  );

  $link_types = array(
    'content' => t('Content'),
  );
  $element['title_link'] = array(
    '#title' => t('Link title to'),
    '#type' => 'select',
    '#default_value' => $settings['title_link'],
    '#empty_option' => t('Nothing'),
    '#options' => $link_types,
  );

  $element['title_class'] = array(
    '#title' => t('Tag classes'),
    '#type' => 'textfield',
    '#description' => t('A CSS class to use in the wrapper tag for the title.'),
    '#default_value' => $settings['title_class'],
    '#element_validate' => array('_title_validate_class'),
  );

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function title_field_formatter_settings_summary($field, $instance, $view_mode) {
  $settings = $instance['display'][$view_mode]['settings'];
  $summary = array();

  $tag = isset($settings['title_style']) && $settings['title_style'] != '' && $settings['title_style'] != '_none' ? $settings['title_style'] : t('- None -');
  $summary[] = t('Title wrap tag: @tag', array('@tag' => $tag));

  $link_types = array(
    'content' => t('Linked to content'),
  );
  // Display this setting only if field is linked.
  if (isset($link_types[$settings['title_link']])) {
    $summary[] = $link_types[$settings['title_link']];
  }

  // Display this setting only if wrapper has a class.
  if (isset($settings['title_class']) && $settings['title_class'] != '_none' && $settings['title_class'] != '') {
    $summary[] = t('Wrap tag classes: @classes', array('@classes' => $settings['title_class']));
  }

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function title_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $settings = $display['settings'];
  $output = '';
  if (isset($items[0]['safe_value'])) {
    $output = $items[0]['safe_value'];
  }
  elseif (isset($items[0]['value'])) {
    $output = _text_sanitize($instance, $langcode, $items[0], 'value');
  }
  $element = array();

  if (!empty($output)) {
    if ($settings['title_link'] == 'content') {
      $uri = entity_uri($entity_type, $entity);
      $options = array('html' => TRUE);
      if (!empty($uri['options'])) {
        $options = array_merge($options, $uri['options']);
      }
      $output = l($output, $uri['path'], $options);
    }

    $wrap_tag = empty($settings['title_style']) ? '_none' : $settings['title_style'];

    if ($wrap_tag != '_none') {
      $variables = array(
        'element' => array(
          '#tag' => $wrap_tag,
          '#value' => $output,
        ),
      );

      if (!empty($settings['title_class'])) {
        $variables['element']['#attributes'] = array('class' => $settings['title_class']);
      }

      $output = theme('html_tag', $variables);
    }

    $element = array(
      array(
        '#markup' => $output,
      ),
    );
  }
  return $element;
}

/**
 * Validate that a space-separated list of values are lowercase and appropriate for use as HTML classes.
 *
 * @see title_field_formatter_settings_form()
 */
function _title_validate_class($element, &$form_state) {
  $value = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  $classes = explode(' ', $value);

  foreach ($classes as $class) {
    if ($class != drupal_html_class($class)) {
      form_error($element, t('Wrapper classes contain illegal characters; classes should be lowercase and may contain letters, numbers, and dashes.'));
      return;
    }
  }
}
