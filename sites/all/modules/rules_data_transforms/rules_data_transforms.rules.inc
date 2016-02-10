<?php

/**
 * @file
 * A plugin for transforming data in rules.
 */

/**
 * Implements hook_rules_action_info().
 */
function rules_data_transforms_rules_action_info() {
  $defaults = array(
    'group' => t('Data Transforms'),
    'parameter' => array(
      'input_text' => array(
        'type' => 'text',
        'label' => t('Input text'),
      ),
    ),
    'provides' => array(
      'output_text' => array(
        'type' => 'text',
        'label' => t('Output text'),
      ),
    ),
  );
  return array(
    'uppercase' => array(
      'label' => t('Make uppercase'),
      'base' => 'rules_data_transforms_action_uppercase',
      'provides' => array(
        'uppercase_text' => array(
          'type' => 'text',
          'label' => t('Uppercase text'),
        ),
      ),
    ) + $defaults,
    'lowercase' => array(
      'label' => t('Make lowercase'),
      'base' => 'rules_data_transforms_action_lowercase',
      'provides' => array(
        'lowercase_text' => array(
          'type' => 'text',
          'label' => t('Lowercase text'),
        ),
      ),
    ) + $defaults,
    'capitalize' => array(
      'label' => t('Capitalize first word of string'),
      'base' => 'rules_data_transforms_action_capitalize',
      'provides' => array(
        'capitalize_text' => array(
          'type' => 'text',
          'label' => t('Capitalize first'),
        ),
      ),
    ) + $defaults,
    'capitalizeall' => array(
      'label' => t('Capitalize all words in a string'),
      'base' => 'rules_data_transforms_action_capitalizeall',
      'provides' => array(
        'capitalizeall_text' => array(
          'type' => 'text',
          'label' => t('Capitalize all'),
        ),
      ),
    ) + $defaults,
    'decode_html_entities' => array(
      'label' => t('Decode HTML entities'),
      'base' => 'rules_data_transforms_action_decode_html_entities',
      'provides' => array(
        'decoded_text' => array(
          'type' => 'text',
          'label' => t('Decoded text'),
        ),
      ),
    ) + $defaults,
    'encode_html_entities' => array(
      'label' => t('Encode HTML entities'),
      'base' => 'rules_data_transforms_action_encode_html_entities',
      'provides' => array(
        'encoded_text' => array(
          'type' => 'text',
          'label' => t('Encoded text'),
        ),
      ),
    ) + $defaults,
    'strtotime' => array(
      'label' => t('String to time'),
      'base' => 'rules_data_transforms_action_strtotime',
      'provides' => array(
        'parsed_date' => array(
          'type' => 'text',
          'label' => t('Date'),
        ),
      ),
    ) + $defaults,
    'implode' => array(
      'label' => t('Combine list into string'),
      'base' => 'rules_data_transforms_action_implode',
      'parameter' => array(
        'input_list' => array(
          'type' => 'list<text>',
          'label' => t('Input list'),
        ),
        'gluez' => array(
          'type' => 'text',
          'optional' => TRUE,
          'label' => t('Glue text'),
        ),
      ),
      'provides' => array(
        'combined_text' => array(
          'type' => 'text',
          'label' => t('Combined text'),
        ),
      ),
    ) + $defaults,
    'filter_elements' => array(
      'label' => t('Filter empty elements from list'),
      'base' => 'rules_data_transforms_action_filter_elements',
      'parameter' => array(
        'input_list' => array(
          'type' => 'list<text>',
          'label' => t('Input list'),
        ),
      ),
      'provides' => array(
        'filtered_list' => array(
          'type' => 'list<text>',
          'label' => t('Filtered list'),
        ),
      ),
    ) + $defaults,
    'explode' => array(
      'label' => t('Explode text'),
      'base' => 'rules_data_transforms_action_explode',
      'parameter' => array(
        'input_text' => array(
          'type' => 'text',
          'label' => t('Input text'),
        ),
        'delimiter' => array(
          'type' => 'text',
          'label' => t('Delimiter'),
        ),
        'limit' => array(
          'type' => 'integer',
          'label' => t('Limit'),
          'optional' => TRUE,
        ),
      ),
      'provides' => array(
        'output_list' => array(
          'type' => 'list<text>',
          'label' => t('Output list'),
        ),
      ),
    ) + $defaults,
    'preg_split' => array(
      'label' => t('Split text by a regular expression'),
      'base' => 'rules_data_transforms_action_preg_split',
      'parameter' => array(
        'input_text' => array(
          'type' => 'text',
          'label' => t('Input text'),
        ),
        'pattern' => array(
          'type' => 'text',
          'label' => t('Pattern'),
          'description' => t('For example write "[\n]" if you need split at line break.'),
        ),
        'limit' => array(
          'type' => 'integer',
          'label' => t('Limit'),
          'optional' => TRUE,
        ),
      ),
      'provides' => array(
        'output_list' => array(
          'type' => 'list<text>',
          'label' => t('Output list'),
        ),
      ),
    ) + $defaults,
    'trim' => array(
      'label' => t('Trim text'),
      'base' => 'rules_data_transforms_action_trim',
      'parameter' => array(
        'input_text' => array(
          'type' => 'text',
          'label' => t('Input text'),
        ),
        'side' => array(
          'type' => 'text',
          'label' => t('Side'),
          'options list' => '_rules_data_transforms_side_options',
          'default value' => 'both',
        ),
        'mask' => array(
          'type' => 'text',
          'label' => t('mask'),
          'description' => t('Leave blank to remove all types of whitespace.'),
          'optional' => TRUE,
        ),
      ),
      'provides' => array(
        'trimmed_text' => array(
          'type' => 'text',
          'label' => t('Trimmed text'),
        ),
      ),
    ) + $defaults,
    'strip_tags' => array(
      'label' => t('Remove HTML tags'),
      'base' => 'rules_data_transforms_action_strip_tags',
      'parameter' => array(
        'input_text' => array(
          'type' => 'text',
          'label' => t('Input text'),
        ),
        'allowed' => array(
          'type' => 'text',
          'label' => t('Allowed tags'),
          'optional' => TRUE,
        ),
      ),
      'provides' => array(
        'stripped_text' => array(
          'type' => 'text',
          'label' => t('Stripped text'),
        ),
      ),
    ) + $defaults,
    'reverse_text' => array(
      'label' => t('Reverse a text string'),
      'base' => 'rules_data_transforms_action_reverse_text',
      'provides' => array(
        'reversed_text' => array(
          'type' => 'text',
          'label' => t('Reversed text'),
        ),
      ),
    ) + $defaults,
    'reverse_list' => array(
      'label' => t('Reverse a list'),
      'base' => 'rules_data_transforms_action_reverse_list',
      'parameter' => array(
        'input_list' => array(
          'type' => 'list',
          'label' => t('Input list'),
        ),
      ),
      'provides' => array(
        'reversed_list' => array(
          'type' => 'list',
          'label' => t('Reversed list'),
        ),
      ),
      'callbacks' => array(
        'info_alter' => 'rules_data_transforms_action_list_info_alter',
      ),
    ) + $defaults,
    'sort_list' => array(
      'label' => t('Sort a list'),
      'base' => 'rules_data_transforms_action_sort_list',
      'parameter' => array(
        'input_list' => array(
          'type' => 'list',
          'label' => t('Input list'),
        ),
        'sort_order' => array(
          'type' => 'text',
          'label' => t('Sort order'),
          'options list' => '_rules_data_transforms_sort_options',
          'default value' => SORT_REGULAR,
        ),
      ),
      'provides' => array(
        'sorted_list' => array(
          'type' => 'list',
          'label' => t('Sorted list'),
        ),
      ),
      'callbacks' => array(
        'info_alter' => 'rules_data_transforms_action_list_info_alter',
      ),
    ) + $defaults,
    'rsort_list' => array(
      'label' => t('Reverse sort a list'),
      'base' => 'rules_data_transforms_action_rsort_list',
      'parameter' => array(
        'input_list' => array(
          'type' => 'list',
          'label' => t('Input list'),
        ),
        'sort_order' => array(
          'type' => 'text',
          'label' => t('Sort order'),
          'options list' => '_rules_data_transforms_sort_options',
          'default value' => SORT_REGULAR,
        ),
      ),
      'provides' => array(
        'reverse_sorted_list' => array(
          'type' => 'list',
          'label' => t('Reverse sorted list'),
        ),
      ),
      'callbacks' => array(
        'info_alter' => 'rules_data_transforms_action_list_info_alter',
      ),
    ) + $defaults,
    'combine_lists' => array(
      'label' => t('Combine two lists'),
      'base' => 'rules_data_transforms_action_combine_lists',
      'parameter' => array(
        'input_list_1' => array(
          'type' => 'list',
          'label' => t('Input list 1'),
        ),
        'input_list_2' => array(
          'type' => 'list',
          'label' => t('Input list 2'),
          'description' => t('List 2 will be added to the end of list 1.'),
        ),
      ),
      'provides' => array(
        'combined_list' => array(
          'type' => 'list',
          'label' => t('Combined list'),
        ),
      ),
      'callbacks' => array(
        'info_alter' => 'rules_data_transforms_action_list_info_alter',
      ),
    ) + $defaults,
    'permute_text' => array(
      'label' => t('Get permutations of string'),
      'base' => 'rules_data_transforms_action_permute_text',
      'provides' => array(
        'permutation_list' => array(
          'type' => 'list<text>',
          'label' => t('Permutation list'),
        ),
      ),
    ) + $defaults,
    'fix_nbsp' => array(
      'label' => t('Replace non break space character with space character'),
      'base' => 'rules_data_transforms_action_fix_nbsp',
      'provides' => array(
        'nbsp_fixed_text' => array(
          'type' => 'text',
          'label' => t('Fixed text'),
        ),
      ),
    ) + $defaults,
    'get_string_url_arg' => array(
      'label' => t('Get string argument from URL'),
      'base' => 'rules_data_transforms_get_string_url_arg',
      'parameter' => array(
        'number' => array(
          'type' => 'integer',
          'label' => t('Argument number'),
          'description' => t('For the page @path use number @number to get @result as the result.', array('@path' => '"node/1000"', '@number' => '0', '@result' => '"node"')),
        ),
      ),
      'provides' => array(
        'arg' => array(
          'type' => 'text',
          'label' => t('Argument value as string'),
        ),
      ),
    ) + $defaults,
    'get_integer_url_arg' => array(
      'label' => t('Get integer argument from URL'),
      'base' => 'rules_data_transforms_get_integer_url_arg',
      'parameter' => array(
        'number' => array(
          'type' => 'integer',
          'label' => t('Argument number'),
          'description' => t('For the page @path use number @number to get @result as the result.', array('@path' => '"node/1000"', '@number' => '1', '@result' => '1000')),
        ),
      ),
      'provides' => array(
        'arg' => array(
          'type' => 'integer',
          'label' => t('Argument value as integer'),
        ),
      ),
    ) + $defaults,
  );
}

function rules_data_transforms_action_uppercase($input_text) {
  return array('uppercase_text' => drupal_strtoupper($input_text));
}

function rules_data_transforms_action_capitalize($input_text) {
  return array('capitalize_text' => drupal_ucfirst($input_text));
}

function rules_data_transforms_action_capitalizeall($input_text) {
  return array('capitalizeall_text' => preg_replace('/\b(\w)/e', 'drupal_strtoupper("$1")', $input_text));
}

function rules_data_transforms_action_lowercase($input_text) {
  return array('lowercase_text' => drupal_strtolower($input_text));
}

function rules_data_transforms_action_decode_html_entities($input_text) {
  return array('decoded_text' => decode_entities($input_text));
}

function rules_data_transforms_action_encode_html_entities($input_text) {
  return array('encoded_text' => check_plain($input_text));
}

function rules_data_transforms_action_strtotime($input_text) {
  return array('parsed_date' => strtotime($input_text));
}

function rules_data_transforms_action_implode($input_list, $glue) {
  return array('combined_text' => is_array($input_list) ? implode($glue, $input_list) : '');
}

function rules_data_transforms_action_explode($input_text, $delimiter, $limit) {
  if (isset($limit)) {
    return array('output_list' => explode($delimiter, $input_text, $limit));
  }
  return array('output_list' => explode($delimiter, $input_text));
}

function rules_data_transforms_action_preg_split($input_text, $pattern, $limit) {
  return array(
    'output_list' => isset($limit) ? preg_split('/' . $pattern . '/', $input_text, $limit) : preg_split('/' . $pattern . '/', $input_text),
  );
}

function _rules_data_transforms_side_options() {
  return array('left' => t('Left'), 'right' => t('Right'), 'both' => t('Both'));
}

function rules_data_transforms_action_trim($input_text, $side, $mask) {
  $func = 'trim';

  switch ($side) {
    case 'left':
      $func = 'ltrim';
      break;

    case 'right':
      $func = 'rtrim';
      break;
  }
  if (isset($mask)) {
    return array('trimmed_text' => $func($input_text, $mask));
  }
  return array('trimmed_text' => $func($input_text));
}

function rules_data_transforms_action_strip_tags($input_text, $allowed) {
  if (isset($allowed)) {
    return array('stripped_text' => strip_tags($input_text, $allowed));
  }
  return array('stripped_text' => strip_tags($input_text));
}

function rules_data_transforms_action_reverse_text($input_text) {
  return array('reversed_text' => strrev($input_text));
}

function rules_data_transforms_action_list_info_alter(&$element_info, $element) {
  $info = $element->getArgumentInfo('input_list');
  foreach ($element_info['provides'] as &$output) {
    if ($output['type'] == 'list') {
      $output['type'] = $info['type'];
    }
  }
// $element_info['provides']['reversed_list']['type'] = $info['type'];
}

function rules_data_transforms_action_reverse_list($input_list) {
  return array('reversed_list' => is_array($input_list) ? array_reverse($input_list) : '');
}

function _rules_data_transforms_sort_options() {
  return array(
    SORT_REGULAR => t('Regular'),
    SORT_NUMERIC => t('Numeric'),
    SORT_STRING => t('String'),
    SORT_LOCALE_STRING => t('Locale'),
  );
}

function rules_data_transforms_action_sort_list($input_list, $sort) {
  if (is_array($input_list)) {
    sort($input_list, $sort);
    return array('sorted_list' => $input_list);
  }
  return array('sorted_list' => array());
}

function rules_data_transforms_action_rsort_list($input_list, $sort) {
  if (is_array($input_list)) {
    rsort($input_list, $sort);
    return array('reverse_sorted_list' => $input_list);
  }
  return array('reverse_sorted_list' => array());
}

function rules_data_transforms_action_combine_lists($input_list_1, $input_list_2) {
  if (is_array($input_list_1) && is_array($input_list_2)) {
    foreach ($input_list_2 as $i) {
      $input_list_1[] = $i;
    }
    return array('combined_list' => $input_list_1);
  }
  return array('combined_list' => array());
}

function rules_data_transforms_action_filter_elements($input_list) {
  return array('filtered_list' => array_filter($input_list));
}

function _rules_data_transforms_permute($string) {
  if (drupal_strlen($string) < 2) {
    return array($string);
  }
  $permutations = array();

  $tail = drupal_substr($string, 1);

  foreach (_rules_data_transforms_permute($tail) as $permutation) {
    $length = drupal_strlen($permutation);

    for ($i = 0; $i <= $length; $i++) {
      $permutations[] = drupal_substr($permutation, 0, $i) . $string[0] . drupal_substr($permutation, $i);
    }
  }
  return $permutations;
}

function rules_data_transforms_action_permute_text($input_text) {
  return array('permutation_list' => array_unique(_rules_data_transforms_permute($input_text)));
}

function rules_data_transforms_action_fix_nbsp($input_text) {
  return array('nbsp_fixed_text' => preg_replace("/\xC2\xA0/", ' ', $input_text));
}

function rules_data_transforms_get_string_url_arg($number) {
  return array('arg' => arg($number));
}

function rules_data_transforms_get_integer_url_arg($number) {
  return array('arg' => intval(arg($number)));
}
