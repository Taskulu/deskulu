<?php

/**
 * @file
 * User page callbacks for the token module.
 */

/**
 * Theme a link to a token tree either as a regular link or a dialog.
 */
function theme_token_tree_link($variables) {
  if (empty($variables['text'])) {
    $variables['text'] = t('Browse available tokens.');
  }

  if (!empty($variables['dialog'])) {
    drupal_add_library('token', 'dialog');
    $variables['options']['attributes']['class'][] = 'token-dialog';
  }

  $info = token_theme();
  $tree_variables = array_intersect_key($variables, $info['token_tree']['variables']);
  $tree_variables = drupal_array_diff_assoc_recursive($tree_variables, $info['token_tree']['variables']);
  if (!isset($variables['options']['query']['options'])) {
    $variables['options']['query']['options'] = array();
  }
  $variables['options']['query']['options'] += $tree_variables;

  // We should never pass the dialog option to theme_token_tree(). It is only
  // used for this function.
  unset($variables['options']['query']['options']['dialog']);

  // Add a security token so that the tree page should only work when used
  // when the dialog link is output with theme('token_tree_link').
  $variables['options']['query']['token'] = drupal_get_token('token-tree:' . serialize($variables['options']['query']['options']));

  // Because PHP converts query strings with arrays into a different syntax on
  // the next request, the options have to be encoded with JSON in the query
  // string so that we can reliably decode it for token comparison.
  $variables['options']['query']['options'] = drupal_json_encode($variables['options']['query']['options']);

  // Set the token tree to open in a separate window.
  $variables['options']['attributes'] + array('target' => '_blank');

  return l($variables['text'], 'token/tree', $variables['options']);
}

/**
 * Page callback to output a token tree as an empty page.
 */
function token_page_output_tree() {
  $options = isset($_GET['options']) ? drupal_json_decode($_GET['options']) : array();

  // Check the token against the serialized options to prevent random access to
  // the token browser page.
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], 'token-tree:' . serialize($options))) {
    return MENU_ACCESS_DENIED;
  }

  // Force the dialog option to be false so we're not creating a dialog within
  // a dialog.
  $options['dialog'] = FALSE;

  $output = theme('token_tree', $options);
  print '<html><head>' . drupal_get_css() . drupal_get_js() . '</head>';
  print '<body class="token-tree">' . $output . '</body></html>';
  drupal_exit();
}

/**
 * Theme a tree table.
 *
 * @ingroup themeable
 */
function theme_tree_table($variables) {
  foreach ($variables['rows'] as &$row) {
    $row += array('class' => array());
    if (!empty($row['parent'])) {
      $row['class'][] = 'child-of-' . $row['parent'];
      unset($row['parent']);
    }
  }

  if (!empty($variables['rows'])) {
    drupal_add_library('token', 'treeTable');
  }

  return theme('table', $variables);
}

/**
 * Provide a 'tree' display of nested tokens.
 *
 * @ingroup themeable
 */
function theme_token_tree($variables) {
  if (!empty($variables['dialog'])) {
    return theme_token_tree_link($variables);
  }

  $token_types = $variables['token_types'];
  $info = token_get_info();

  if ($token_types == 'all') {
    $token_types = array_keys($info['types']);
  }
  elseif ($variables['global_types']) {
    $token_types = array_merge($token_types, token_get_global_token_types());
  }

  $element = array(
    '#cache' => array(
      'cid' => 'tree-rendered:' . hash('sha256', serialize(array('token_types' => $token_types, 'global_types' => NULL) + $variables)),
      'bin' => 'cache_token',
    ),
  );
  if ($cached_output = token_render_cache_get($element)) {
    return $cached_output;
  }

  $options = array(
    'flat' => TRUE,
    'restricted' => $variables['show_restricted'],
    'depth' => $variables['recursion_limit'],
  );
  $multiple_token_types = (count($token_types) > 1);
  $rows = array();

  foreach ($info['types'] as $type => $type_info) {
    if (!in_array($type, $token_types)) {
      continue;
    }

    if ($multiple_token_types) {
      $row = _token_token_tree_format_row($type, $type_info, TRUE);
      unset($row['data']['value']);
      $rows[] = $row;
    }

    $tree = token_build_tree($type, $options);
    foreach ($tree as $token => $token_info) {
      if (!empty($token_info['restricted']) && empty($variables['show_restricted'])) {
        continue;
      }
      if ($multiple_token_types && !isset($token_info['parent'])) {
        $token_info['parent'] = $type;
      }
      $row = _token_token_tree_format_row($token, $token_info);
      unset($row['data']['value']);
      $rows[] = $row;
    }
  }

  $element += array(
    '#theme' => 'tree_table',
    '#header' => array(
      t('Name'),
      t('Token'),
      t('Description'),
    ),
    '#rows' => $rows,
    '#attributes' => array('class' => array('token-tree')),
    '#empty' => t('No tokens available'),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'token') . '/token.js'),
      'css' => array(drupal_get_path('module', 'token') . '/token.css'),
      'library' => array(array('token', 'treeTable')),
    ),
  );

  if ($variables['click_insert']) {
    $element['#caption'] = t('Click a token to insert it into the field you\'ve last clicked.');
    $element['#attributes']['class'][] = 'token-click-insert';
  }

  $output = drupal_render($element);
  token_render_cache_set($output, $element);
  return $output;
}

/**
 * Build a row in the token tree.
 */
function _token_token_tree_format_row($token, array $token_info, $is_group = FALSE) {
  // Build a statically cached array of default values. This is around four
  // times as efficient as building the base array from scratch each time this
  // function is called.
  static $defaults = array(
    'id' => '',
    'class' => array(),
    'data' => array(
      'name' => '',
      'token' => '',
      'value' => '',
      'description' => '',
    ),
  );

  $row = $defaults;
  $row['id'] = _token_clean_css_identifier($token);
  $row['data']['name'] = $token_info['name'];
  $row['data']['description'] = $token_info['description'];

  if ($is_group) {
    // This is a token type/group.
    $row['class'][] = 'token-group';
  }
  else {
    // This is a token.
    $row['data']['token']['data'] = $token;
    $row['data']['token']['class'][] = 'token-key';
    if (isset($token_info['value'])) {
      $row['data']['value'] = $token_info['value'];
    }
    if (!empty($token_info['parent'])) {
      $row['parent'] = _token_clean_css_identifier($token_info['parent']);
    }
  }

  return $row;
}

/**
 * Wrapper function for drupal_clean_css_identifier() for use with tokens.
 *
 * This trims any brackets from the token and also cleans the colon character
 * to a hyphen.
 *
 * @see drupal_clean_css_identifier()
 */
function _token_clean_css_identifier($id) {
  static $replacements = array(' ' => '-', '_' => '-', '/' => '-', '[' => '-', ']' => '', ':' => '--', '?' => '', '<' => '-', '>' => '-');
  return 'token-' . rtrim(strtr(trim($id, '[]'), $replacements), '-');
}

/**
 * Menu callback; prints the available tokens and values for an object.
 */
function token_devel_token_object($entity_type, $entity, $token_type = NULL) {
  $header = array(
    t('Token'),
    t('Value'),
  );
  $rows = array();

  $options = array(
    'flat' => TRUE,
    'values' => TRUE,
    'data' => array($entity_type => $entity),
  );
  if (!isset($token_type)) {
    $token_type = $entity_type;
  }
  $tree = token_build_tree($token_type, $options);
  foreach ($tree as $token => $token_info) {
    if (!empty($token_info['restricted'])) {
      continue;
    }
    if (!isset($token_info['value']) && !empty($token_info['parent']) && !isset($tree[$token_info['parent']]['value'])) {
      continue;
    }
    $row = _token_token_tree_format_row($token, $token_info);
    unset($row['data']['description']);
    unset($row['data']['name']);
    $rows[] = $row;
  }

  $build['tokens'] = array(
    '#theme' => 'tree_table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('class' => array('token-tree')),
    '#empty' => t('No tokens available.'),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'token') . '/token.js'),
      'css' => array(drupal_get_path('module', 'token') . '/token.css'),
    ),
  );

  return $build;
}

/**
 * Page callback to clear the token registry caches.
 */
function token_flush_cache_callback() {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], current_path())) {
    return MENU_NOT_FOUND;
  }

  token_clear_cache();
  drupal_set_message(t('Token registry caches cleared.'));
  drupal_goto();
}

function token_autocomplete() {
  $args = func_get_args();
  $string = implode('/', $args);

  $token_info = token_info();

  preg_match_all('/\[([^\s\]:]*):?([^\s\]]*)?\]?/', $string, $matches);
  $types = $matches[1];
  $tokens = $matches[2];

  foreach ($types as $index => $type) {
    if (!empty($tokens[$index]) || isset($token_info['types'][$type])) {
      token_autocomplete_token($type, $tokens[$index]);
    }
    else {
      token_autocomplete_type($type);
    }
  }

}

function token_autocomplete_type($string = '') {
  $token_info = token_info();
  $types = $token_info['types'];
  $matches = array();

  foreach ($types as $type => $info) {
    if (!$string || strpos($type, $string) === 0) {
      $type_key = "[{$type}:";
      $matches[$type_key] = levenshtein($type, $string);
    }
  }

  if ($string) {
    asort($matches);
  }
  else {
    ksort($matches);
  }

  $matches = drupal_map_assoc(array_keys($matches));
  drupal_json_output($matches);
}

function token_autocomplete_token($token_type) {
  $args = func_get_args();
  array_shift($args);
  $string = trim(implode('/', $args));
  $string = substr($string, strrpos($string, '['));

  $token_type = $token_type['type'];
  $matches = array();

  if (!drupal_strlen($string)) {
    $matches["[{$token_type}:"] = 0;
  }
  else {
    $depth = max(1, substr_count($string, ':'));
    $tree = token_build_tree($token_type, array('flat' => TRUE, 'depth' => $depth));
    foreach (array_keys($tree) as $token) {
      if (strpos($token, $string) === 0) {
        $matches[$token] = levenshtein($token, $string);
        if (isset($tree[$token]['children'])) {
          $token = rtrim($token, ':]') . ':';
          $matches[$token] = levenshtein($token, $string);
        }
      }
    }
  }

  asort($matches);
  $matches = drupal_map_assoc(array_keys($matches));
  drupal_json_output($matches);
}
