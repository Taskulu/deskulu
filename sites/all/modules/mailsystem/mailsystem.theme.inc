<?php

/**
* @file
* The theme system, which controls the output of email messages.
*/

/**
* Implements hook_theme_registry_alter().
*/
function mailsystem_theme_theme_registry_alter(&$theme_registry) {
  global $theme_key;
  static $recursion_prevention = FALSE;

  // Prevent recursive execution.
  if ($recursion_prevention) {
    return;
  }
  $recursion_prevention = TRUE;
  $mailsystem_theme = mailsystem_get_mail_theme();

  // Only take action if the mailsystem theme is not the current theme.
  if ($mailsystem_theme != $theme_key) {
    $themes = list_themes();
    // Get the mailsystem theme to be used for rendering emails.
    if (isset($themes[$mailsystem_theme])) {
      $theme = clone $themes[$mailsystem_theme];
      if (isset($theme)) {
        // Swap to the mailsystem theme
        $old_theme = $theme_key;
        mailsystem_theme_swap_theme($mailsystem_theme);

        // Establish variables for further processing.
        $base_theme = array();
        if (isset($theme->base_themes)) {
          foreach (array_keys($theme->base_themes) as $base) {
            $base_theme[$base] = clone $themes[$base];
          }
        }
        if (isset($theme->base_theme) && !isset($base_theme[$theme->base_theme])) {
          $base_theme[$theme->base_theme] = clone $themes[$theme->base_theme];
        }
        if (isset($theme->engine)) {
          $theme_engine = $theme->engine;
        }

        // Include template files to let _theme_load_registry add preprocess
        // functions.
        foreach ($base_theme as $base) {
          include_once(drupal_get_path('theme', $base->name) . '/template.php');
        }
        include_once(drupal_get_path('theme', $theme->name) . '/template.php');

        // Get the theme_registry cache.
        $cache = _theme_load_registry($theme, $base_theme, $theme_engine);

        // Change the registry for hooks with a 'mail theme' element.
        foreach ($theme_registry as $name => $hook) {
          if (!empty($hook['mail theme'])) {
            if (isset($cache[$name])) {
              $cache[$name]['includes'][] = drupal_get_path('theme', $theme->name) . '/template.php';
              foreach ($base_theme as $base) {
                $cache[$name]['includes'][] = drupal_get_path('theme', $base->name) . '/template.php';
              }
              // Change the current registry for the new record.
              $theme_registry[$name] = $cache[$name];
            }

            // Look for template suggestions.
            foreach ($cache as $cache_name => $cache_hook) {
              if (strpos($cache_name, $name . '__') !== FALSE) {
                $cache_hook['includes'][] = drupal_get_path('theme', $theme->name) . '/template.php';
                foreach ($base_theme as $base) {
                  $cache_hook['includes'][] = drupal_get_path('theme', $base->name) . '/template.php';
                }
                // Change the current registry for the new record.
                $theme_registry[$cache_name] = $cache_hook;
              }
            }
          }
        }

        // Swap back to the original theme
        mailsystem_theme_swap_theme($old_theme);
      }
    }
  }
  $recursion_prevention = FALSE;
}

/**
 * Helper to swap themes safely for use by mailsystem_theme_theme_registry_alter().
 */
function mailsystem_theme_swap_theme($new_theme) {
  // Make sure the theme exists
  $themes = list_themes();
  if (empty($themes[$new_theme])) {
    return;
  }

  // Both theme/theme_key are set to the new theme
  global $theme, $theme_key;
  $theme = $theme_key = $new_theme;

  // Create the base_theme_info
  global $base_theme_info;
  $base_theme = array();
  $ancestor = $theme;
  while ($ancestor && isset($themes[$ancestor]->base_theme)) {
    $ancestor = $themes[$ancestor]->base_theme;
    $base_theme[] = $themes[$ancestor];
  }
  $base_theme_info = array_reverse($base_theme);

  // Some other theme related globals
  global $theme_engine, $theme_info, $theme_path;
  $theme_engine = $themes[$theme]->engine;
  $theme_info = $themes[$theme];
  $theme_path = dirname($themes[$theme]->filename);

  // We need to reset the drupal_alter and module_implements statics
  drupal_static_reset('drupal_alter');
  drupal_static_reset('module_implements');
}
