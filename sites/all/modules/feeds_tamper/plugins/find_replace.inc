<?php

/**
 * @file
 * Find/replace text.
 */

$plugin = array(
  'form' => 'feeds_tamper_find_replace_form',
  'callback' => 'feeds_tamper_find_replace_callback',
  'validate' => 'feeds_tamper_find_replace_validate',
  'name' => 'Find replace',
  'multi' => 'loop',
  'category' => 'Text',
);

function feeds_tamper_find_replace_form($importer, $element_key, $settings) {
  $form = array();
  $form['find'] = array(
    '#type' => 'textfield',
    '#title' => t('Text to find'),
    '#default_value' => isset($settings['find']) ? $settings['find'] : '',
  );
  $form['replace'] = array(
    '#type' => 'textfield',
    '#title' => t('Text to replace'),
    '#default_value' => isset($settings['replace']) ? $settings['replace'] : '',
  );
  $form['case_sensitive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Case sensitive'),
    '#default_value' => isset($settings['case_sensitive']) ? $settings['case_sensitive'] : FALSE,
    '#description' => t('If checked, "book" will match "book" but not "Book" or "BOOK".'),
  );
  $form['word_boundaries'] = array(
    '#type' => 'checkbox',
    '#title' => t('Respect word boundaries'),
    '#default_value' => isset($settings['word_boundaries']) ? $settings['word_boundaries'] : FALSE,
    '#description' => t('If checked, "book" will match "book" but not "bookcase".'),
  );
  $form['whole'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match whole word/phrase'),
    '#default_value' => isset($settings['whole']) ? $settings['whole'] : FALSE,
    '#description' => t('If checked, then the whole word or phrase will be matched, e.g. "book" will match "book" but not "the book".<br />If this option is selected then "Respect word boundaries" above will be ignored.'),
  );
  return $form;
}

function feeds_tamper_find_replace_validate(&$settings) {
  $settings['regex'] = FALSE;

  if (!$settings['word_boundaries'] && !$settings['whole'] && $settings['case_sensitive']) {
    $settings['func'] = 'str_replace';
  }
  elseif (!$settings['word_boundaries'] && !$settings['whole'] && !$settings['case_sensitive']) {
    $settings['func'] = 'str_ireplace';
  }
  else {
    $settings['regex'] = TRUE;

    if ($settings['whole']) {
      $regex = '/^' . preg_quote($settings['find'], '/') . '$/u';
    }
    else {
      // Word boundaries can only match a word with letters at the end.
      if (!preg_match('/^\w(.*\w)?$/u', $settings['find'])) {
        form_set_error('settings][find', t('Search text must begin and end with a letter, number, or underscore to use the %option option.', array('%option' => t('Respect word boundaries'))));
      }
      $regex = '/\b' . preg_quote($settings['find'], '/') . '\b/u';
    }
    if (!$settings['case_sensitive']) {
      $regex .= 'i';
    }
    $settings['regex_find'] = $regex;
  }
}

function feeds_tamper_find_replace_callback($result, $item_key, $element_key, &$field, $settings, $source) {
  if ($settings['regex']) {
    $field = preg_replace($settings['regex_find'], $settings['replace'], $field);
  }
  else {
    $field = $settings['func']($settings['find'], $settings['replace'], $field);
  }
}
