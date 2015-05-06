<?php
/**
 * @file
 * registry.inc
 *
 * Contains functions specific to the theme registry.
 *
 * Theme templates, functions and [pre]process functions
 *
 * Theme templates `*.tpl.php` files are stored in the `theme` directory along
 * with `*.func.php` and `*.vars.php` files. The latter two are enabled by the
 * processing done below. The three types of files can be grouped into
 * sub-directories. It is recommended that they are grouped by the modules
 * they originate from. Theme specific hooks should be grouped into a folder
 * named after the theme itself.
 *
 * Function files store theme functions directly related to the base file name
 * or "theme hook", e.g., the hook `foo` would be handled by the
 * `bootstrap_foo` function which would be stored in `foo.func.php`.
 *
 * Variable files store [pre]process functions directly related to the base file
 * file name or "theme hook" just like function files, e.g., the hook `html`
 * would be handled by the `bootstrap_preprocess_html` function held in
 * `html.vars.php`.
 *
 * The two types of files mentioned above should also be used to hold support
 * functions specific to the theming hook. When a function grows too large,
 * break it apart so it becomes more readable.
 *
 * Files specific to theme functions and variable preprocess/process functions
 * should be named after the `base hook`. Alternate hooks (or what is commonly
 * referred to as "hook suggestions") should not be used for the file name.
 * The contents of the theme functions file can hold hook suggestions and it
 * should be related to the base hook. [Pre]process functions for hook
 * suggestions are not supported in Drupal 7 core.
 *
 * Using these files will keep the `template.php` file clean and grouping them
 * within the theme directory will make it easier to discover and work with.
 *
 * Warning: Using core's `path_to_theme` function may return the wrong path.
 * It is difficult to predict to begin with and the changes made here will
 * make it even worse. Use `drupal_get_path('theme', 'bootstrap')` instead.
 */

/**
 * Stub implementation for hook_theme().
 *
 * @see bootstrap_theme()
 * @see hook_theme()
 */
function _bootstrap_theme(&$existing, $type, $theme, $path) {
  // If we are auto-rebuilding the theme registry, warn about the feature.
  if (
    // Only display for site config admins.
    isset($GLOBALS['user']) && function_exists('user_access') && user_access('administer site configuration')
    && theme_get_setting('bootstrap_rebuild_registry')
    // Always display in the admin section, otherwise limit to three per hour.
    && (arg(0) == 'admin' || flood_is_allowed($GLOBALS['theme'] . '_rebuild_registry_warning', 3))
  ) {
    flood_register_event($GLOBALS['theme'] . '_rebuild_registry_warning');
    drupal_set_message(t('For easier theme development, the theme registry is being rebuilt on every page request. It is <em>extremely</em> important to <a href="!link">turn off this feature</a> on production websites.', array('!link' => url('admin/appearance/settings/' . $GLOBALS['theme']))), 'warning', FALSE);
  }

  // Custom theme hooks:
  // Do not define the `path` or `template`.
  $hook_theme = array(
    'bootstrap_links' => array(
      'variables' => array(
        'links' => array(),
        'attributes' => array(),
        'heading' => NULL,
      ),
    ),
    'bootstrap_btn_dropdown' => array(
      'variables' => array(
        'links' => array(),
        'attributes' => array(),
        'type' => NULL,
      ),
    ),
    'bootstrap_modal' => array(
      'variables' => array(
        'heading' => '',
        'body' => '',
        'footer' => '',
        'attributes' => array(),
        'html_heading' => FALSE,
      ),
    ),
    'bootstrap_accordion' => array(
      'variables' => array(
        'id' => '',
        'elements' => array(),
      ),
    ),
    'bootstrap_search_form_wrapper' => array(
      'render element' => 'element',
    ),
    'bootstrap_panel' => array(
      'render element' => 'element',
    ),
  );

  // Process custom. This should be used again for any sub-themes.
  bootstrap_hook_theme_complete($hook_theme, $theme, $path . '/theme');

  // Process existing registry. Only invoke once from base theme.
  bootstrap_hook_theme_complete($existing, $theme, $path . '/theme');

  return $hook_theme;
}

/**
 * Discovers and fills missing elements in the theme registry.
 *
 * Adds the following:
 *  - `includes` `*.vars.php` if variables file is found.
 *  - `includes` `*.func.php` if function file is found.
 *  - `function` if the function for $theme is found.
 *  - `path` if template file is found.
 *  - `template` if template file is found.
 */
function bootstrap_hook_theme_complete(&$registry, $theme, $path) {
  $registry = drupal_array_merge_deep(
    $registry,
    bootstrap_find_theme_includes($registry, '.vars.php', $path),
    bootstrap_find_theme_includes($registry, '.func.php', $path),
    drupal_find_theme_functions($registry, array($theme)),
    drupal_find_theme_templates($registry, '.tpl.php', $path)
  );
  // Post-process the theme registry.
  foreach ($registry as $hook => $info) {
    // Core find functions above does not carry over the base `theme path` when
    // finding suggestions. Add it to prevent notices for `theme` calls.
    if (!isset($info['theme path']) && isset($info['base hook'])) {
      $registry[$hook]['theme path'] = $path;
    }
    // Setup a default "context" variable. This allows #context to be passed
    // to every template and theme function.
    // @see https://drupal.org/node/2035055
    if (isset($info['variables']) && !isset($info['variables']['context'])) {
      $registry[$hook]['variables'] += array(
        'context' => array(),
      );
    }
  }
}

/**
 * Discovers and sets the path to each `THEME-HOOK.$extension` file.
 */
function bootstrap_find_theme_includes($registry, $extension, $path) {
  $regex = '/' . str_replace('.', '\.', $extension) . '$/';
  $files = drupal_system_listing($regex, $path, 'name', 0);

  $hook_includes = array();
  foreach ($files as $name => $file) {
    // Chop off the remaining extension.
    if (($pos = strpos($name, '.')) !== FALSE) {
      $name = substr($name, 0, $pos);
    }
    // Transform "-" in filenames to "_" to match theme hook naming scheme.
    $hook = strtr($name, '-', '_');
    // File to be included by core's theme function when the hook is invoked.
    // This only applies to base hooks. When hook derivatives are called
    // (those with a double "__"), it checks for the base hook, calls its
    // variable processors and ignores anything specific to the derivative.
    // Due to the way it works, It becomes redundant to give it a path that is
    // not a base hook.
    // @see https://drupal.org/node/939462
    if (isset($registry[$hook]) && !isset($registry[$hook]['base hook'])) {
      // Include the file so core can discover any containing functions.
      include_once DRUPAL_ROOT . '/' . $file->uri;
      $hook_includes[$hook]['includes'][] = $file->uri;
    }
  }

  return $hook_includes;
}
