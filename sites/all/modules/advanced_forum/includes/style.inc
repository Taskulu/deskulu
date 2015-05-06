<?php

/**
 * @file
 * Functions relating to the style system, not including core hooks and
 * preprocess / theme functions.
 */

/**
 * Returns the name of the forum style to use.
 */
function advanced_forum_get_current_style() {
  return variable_get('advanced_forum_style', 'silver_bells');
}

/**
 * Returns style.
 */
function advanced_forum_get_style($style) {
  ctools_include('plugins');
  return ctools_get_plugins('advanced_forum', 'styles', $style);
}

/**
 * Returns all styles.
 */
function advanced_forum_get_all_styles() {
  ctools_include('plugins');
  return ctools_get_plugins('advanced_forum', 'styles');
}

/**
 * Returns the path to the advanced forum style.
 */
function advanced_forum_path_to_style($requested_style = NULL) {
  // Set a static variable so this is cached after the first call.
  static $style_paths = array();

  if (empty($requested_style)) {
    // If no style is passed in, assume the current style is wanted.
    $requested_style = advanced_forum_get_current_style();
  }

  if (empty($style_path)) {
    // Get the path information.
    $styles = advanced_forum_get_all_styles();
    foreach ($styles as $key => $style) {
      $style_paths[$key] = $style['path'];
    }
  }
  return $style_paths[$requested_style];
}

/**
 * Starting at a given style, return paths of it and all ancestor styles.
 */
function advanced_forum_style_lineage($style = NULL) {
  static $lineages = array();
  if (empty($style)) {
    // If no style is passed in, assume the current style is wanted.
    $style = advanced_forum_get_current_style();
  }

  if (!array_key_exists($style, $lineages)) {
    $lineage = array();

    // Get an array with information from all styles.
    $all_styles = advanced_forum_get_all_styles();

    // Fallback.
    if (empty($all_styles[$style])) {
      $style = "silver_bells";
    }

    // Add the current style to the lineage first.
    $lineage[$style] = $all_styles[$style]['path'];

    // Check if there is an "extra style" listed. This allows you to grab the
    // CSS of one other style and toss it in as a pseudo parent. We do not
    // follow the path up its parent. The primary use is for adding in the
    // "stacked" CSS but could be used for other purposes as well.
    if (!empty($all_styles[$style]['extra style']) && !empty($all_styles[$all_styles[$style]['extra style']]['path'])) {
      $extra_style = $all_styles[$style]['extra style'];
      $lineage[$extra_style] = $all_styles[$extra_style]['path'];
    }

    // Grab the style we are looking at. This variable starts out as the current
    // style in use on the page but will change as we move up the chain.
    $current_style = $style;

    while (!empty($all_styles[$current_style]['base style'])) {
      // There is a parent style, so move up to it.
      $current_style = $all_styles[$current_style]['base style'];

      // Make sure the listed parent style actually exists.
      if (!empty($all_styles[$current_style]['path'])) {
        // Add the style to our list.
        $lineage[$current_style] = $all_styles[$current_style]['path'];
      }
    }

    $lineages[$style] = $lineage;
  }

  return $lineages[$style];
}

/**
 * Starting at a given style, return path of template file.
 */
function advanced_forum_style_template_path($template_name, $style = NULL) {
  static $styles_files = array();

  if (empty($styles_files)) {
    $styles_files = file_scan_directory(drupal_get_path('module', 'advanced_forum') . '/styles', '/(.*)/', array(), 2);
  }

  $lineage = advanced_forum_style_lineage($style);

  $path = '';
  foreach ($lineage as $key => $path) {
    $template_path = "";
  }

  return $path;
}

/**
 * Add basename of file.
 */
function advanced_forum_template_basename(&$file, $key, $extension) {
  $file->name = basename($file->uri, $extension);
}

/**
 * Find style template.
 */
function advanced_forum_find_style_templates($style = NULL) {
  // Get theme engine extension.
  global $theme_engine;
  $extension = '.tpl.php';
  if (isset($theme_engine)) {
    $extension_function = $theme_engine . '_extension';
    if (function_exists($extension_function)) {
      $extension = $extension_function();
    }
  }

  // Most significant style template comes first in lineage array.
  $lineage = array_reverse(advanced_forum_style_lineage($style), TRUE);

  $templates = array();

  // Loop through each style folder looking for templates.
  foreach ($lineage as $key => $path) {
    $styles_files = file_scan_directory($path, '/' . str_replace('.', '\.', $extension) . '$/', array('key' => 'filename'));
    foreach ($styles_files as $file) {
      $file->name = basename($file->uri, $extension);
      $theme_file = $file->name;
      $theme_path = advanced_forum_path_to_theme();

      // Remove style name from file name.
      $theme_base_file = str_replace(".$key.", "-", $file->name);

      // Transform - in filenames to _ to match function naming scheme
      // and create proper hook name.
      $hook = strtr($theme_base_file, '-', '_');

      // Check for user theme template override
      // 1. check for <theme>/<style>/advanced-forum.<style>.<template>.tpl.php
      if (file_exists($theme_path . "/$key/" . $file->name . $extension)) {
        $theme_path = $theme_path . "/$key";
      }
      // 2. check for <theme>/advanced-forum-<template>.tpl.php
      elseif (file_exists($theme_path . "/" . $theme_base_file . $extension)) {
        $theme_file = $theme_base_file;
      }
      // 3. check for advanced_forum/styles/<style>/... template.
      else {
        $theme_path = dirname($file->uri);
      }

      $templates[$hook] = array(
        'template' => $theme_file,
        'path' => $theme_path,
      );
    }
  }
  return $templates;
}

/**
 * Load any .inc files related to the style so that preprocess functions can run as appropriate.
 */
function advanced_forum_load_style_includes($style = NULL) {
  $lineage = advanced_forum_style_lineage($style);
  foreach ($lineage as $key => $path) {
    if (file_exists("$path/$key.inc")) {
      require_once "$path/$key.inc";
    }
  }
}

/**
 * Get the info for a style, using an additive notation to include all items from the parent lineage.
 */
function advanced_forum_style_info($style = NULL) {
  static $info = array();
  if (empty($style)) {
    // If no style is passed in, assume the current style is wanted.
    $style = advanced_forum_get_current_style();
  }

  if (!array_key_exists($style, $info)) {
    $info[$style] = array();
    $lineage = advanced_forum_style_lineage();
    foreach ($lineage as $key => $dir) {
      // Using the + operator is additive but will not overwrite.
      // $lineage comes out as the actual style with each
      // successive style after it, so simply adding the arrays
      // together means that all info will be combined and keys
      // in the parent info will be defaults passed down to children
      // only if they do not override them.
      $info[$style] += advanced_forum_get_style($key);
    }
  }

  return $info[$style];
}

/**
 * Returns whether forum is styled.
 */
function advanced_forum_is_styled($content, $teaser = FALSE, $type = 'node') {
  // This is the ID of the topic starting node.
  static $master_topic_id;

  // Give other modules a first crack at making the decision. To keep this
  // simple, he last one in the array wins.
  $topic_ids = module_invoke_all('advanced_forum_is_styled_alter', $content, $teaser, $type);
  $topic_id = end($topic_ids);

  if (!$topic_id) {
    // If no other modules made a decision on this, we look at it.
    switch ($type) {
      case 'node':
        // See if the node should be styled.
        $topic_id = advanced_forum_node_is_styled($content, $teaser);
        break;

      case 'comment':
        $topic_id = advanced_forum_comment_is_styled($content, $master_topic_id);
        break;

      case 'comment-wrapper':
        if ($content->nid == $master_topic_id) {
          // Use our comment wrapper only if we are on a styled node.
          $topic_id = $master_topic_id;
        }
        break;
    }
  }

  if ($topic_id > 0) {
    // If we have a positive number for the topic ID, then this item is styled.
    // We need to update the master ID, add the CSS/JS files, and tell the caller
    // our decision.
    $master_topic_id = $topic_id;
    _advanced_forum_add_files();
    return TRUE;
  }
  elseif ($topic_id == -42) {
    // A -42 means all the tests passed but the node is being previewed so there
    // is no node id, yet. Add the files and return true but don't update the
    // master topic ID.
    _advanced_forum_add_files();
    return TRUE;
  }

}

/**
 * Returns whether node is styled.
 */
function advanced_forum_node_is_styled($node, $teaser = FALSE) {
  // Get the list of types that should have the style applied.
  $styled_node_types = variable_get('advanced_forum_styled_node_types', array('forum'));
  // If this type is in the list of types that should be styled...
  if (in_array($node->type, $styled_node_types)) {
    // ...and if we are styling teasers or this isn't a teaser...
    if (variable_get('advanced_forum_style_teasers', FALSE) || !$teaser) {
      // ...and if this is not a new node being previewed...
      if (!empty($node->nid)) {
        // ...and if we are styling non forum tagged nodes or this is forum tagged...
        if (!variable_get('advanced_forum_style_only_forum_tagged', TRUE) || advanced_forum_is_forum_tagged($node)) {
          // ... then return the node ID.
          return $node->nid;
        }
      }
      // ...or if this _is_ a new node being previewed...
      else {
        // ...and if we are styling non forum tagged nodes or this is forum tagged...
        if (!variable_get('advanced_forum_style_only_forum_tagged', TRUE) || advanced_forum_is_forum_tagged($node, TRUE)) {
          // ...Send back -42 as a special code to say that this should be
          // styled but that it isn't the actual node id.
          return -42;
        }
      }
    }
  }
}

/**
 * Returns whether comment is styled.
 */
function advanced_forum_comment_is_styled($comment, $master_topic_id) {
  if (isset($master_topic_id) && ($comment->nid == $master_topic_id)) {
    // This comment is on a node we already know is styled, or a previous
    // comment on this node is styled, so the comment is styled.
    return $master_topic_id;
  }

  if (variable_get("advanced_forum_style_all_comments", 0)) {
    // All comments on this site should be styled.
    return $comment->nid;
  }

  // Load up the node this comment is attached to and see if it is styled.
  // This should not happen often, only in a case where the comment is
  // displayed seperately from the node (such as a reply preview).
  $node = node_load($comment->nid);
  if (advanced_forum_node_is_styled($node, FALSE)) {
    return $comment->nid;
  }

  // Comment doesn't pass any styling test.
  return -1;
}

/**
 * Returns whether forum is tagged.
 */
function advanced_forum_is_forum_tagged($node, $preview = FALSE) {
  // Is this a forum node at all?
  if (empty($node->taxonomy_forums)) {
    return FALSE;
  }
  $vid = variable_get('forum_nav_vocabulary');

  // Check for language used.
  if (empty($node->taxonomy_forums[$node->language])) {
    $langcode = LANGUAGE_NONE;
  }
  else {
    $langcode = $node->language;
  }

  if ($preview) {
    foreach ($node->taxonomy_forums[$langcode] as $tforum) {
      if ($tforum['taxonomy_term']->vid == $vid) {
        return TRUE;
      }
    }
  }
  else {
    // Get a list of the terms attached to this node.
    $terms = $node->taxonomy_forums[$langcode];
    if (count($terms) > 0) {
      return TRUE;
    }
  }
}

/**
 * Returns the path to actual site theme in use because path_to_theme is flaky.
 */
function advanced_forum_path_to_theme() {
  global $theme;

  if (!empty($theme)) {
    // Normally, the global theme variable is set, so we use that.
    return drupal_get_path('theme', $theme);
  }

  // For some reason I've never figured out, some users are reporting
  // that the global theme variable is not set when this is called.
  // As a band-aid solution, this will pull the default theme out of the
  // database and use that.
  $default_theme = variable_get('theme_default', 'garland');
  return drupal_get_path('theme', $default_theme);
}

/**
 * Manipulate the template suggestions to add in one for each style as well as the default.
 */
function advanced_forum_add_template_suggestions($template, &$variables) {
  // Adjust template name.
  $template = strtr($template, '-', '_');
  if (strpos($template, 'advanced_forum') === FALSE) {
    $template = "advanced_forum_" . $template;
  }
  // Set AF template as most significant suggestion.
  $variables['theme_hook_suggestion'] = $template;
  $variables['theme_hook_suggestions'][] = $template;
  return;
}

/**
 * Adds extra files needed for styling.
 *
 * This is currently just CSS files but was made generic to allow adding
 * javascript in the future.
 */
function _advanced_forum_add_files() {
  // This could get called more than once on a page and we only need to do it once.
  static $added;

  if (empty($added)) {
    $added = TRUE;
    $lineage = advanced_forum_style_lineage();
    $lineage = array_reverse($lineage, TRUE);
    $theme_path = advanced_forum_path_to_theme();

    foreach (array('structure.css', 'style.css', 'images.css') as $css_type) {
      $css_file = "$theme_path/advanced-forum.$css_type";
      if (file_exists($css_file)) {
        // CSS files with no style name in the theme directory trump all
        // to provide a theme specific style override.
        drupal_add_css($css_file);
      }
      else {
        // For each style from the current style on up through each parent
        // style, look for the style specific CSS file first in the active
        // theme and then in the style directory.
        foreach ($lineage as $key => $path) {
          $css_file = "advanced-forum.$key.$css_type";
          if (file_exists("$theme_path/$css_file")) {
            // If the style specific file is in the theme, use that.
            drupal_add_css("$theme_path/$css_file");
          }
          elseif (file_exists("$path/$css_file")) {
            // Otherwise look in the style for it.
            drupal_add_css("$path/$css_file");
          }
        }
      }
    }

    advanced_forum_load_style_includes();
  }
}

/**
 * Ctools integration for styles.
 */
function advanced_forum_ctools_plugin_type() {
  return array(
    'styles' => array(
      'info file' => TRUE,
      'load themes' => TRUE,
    ),
  );
}
