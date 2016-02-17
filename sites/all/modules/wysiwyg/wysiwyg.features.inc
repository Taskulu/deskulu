<?php

/**
 * Implements hook_features_export_options().
 */
function wysiwyg_features_export_options() {
  $profiles = array();

  // Get human-readable name from filter module.
  $formats = filter_formats();

  foreach (array_keys(wysiwyg_profile_load_all()) as $format) {
    // Text format may vanish without deleting the wysiwyg profile.
    if (isset($formats[$format])) {
      $profiles[$format] = $formats[$format]->name;
    }
  }
  return $profiles;
}

/**
 * Implements hook_features_export().
 */
function wysiwyg_features_export($data, &$export, $module_name = '') {
  $pipe = array();

  // The wysiwyg_default_formats() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['wysiwyg'] = 'wysiwyg';

  foreach ($data as $name) {
    if ($profile = wysiwyg_get_profile($name)) {
      // Add profile to exports.
      $export['features']['wysiwyg'][$profile->format] = $profile->format;

      // Chain filter format for export.
      $pipe['filter'][] = $profile->format;
    }
  }

  return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function wysiwyg_features_export_render($module, $data, $export = NULL) {
  $code = array();
  $code[] = '  $profiles = array();';
  $code[] = '';

  foreach ($data as $name) {
    if ($profile = wysiwyg_get_profile($name)) {
      $profile_export = features_var_export($profile, '  ');
      $profile_identifier = features_var_export($profile->format);
      $code[] = "  // Exported profile: {$profile->format}";
      $code[] = "  \$profiles[{$profile_identifier}] = {$profile_export};";
      $code[] = "";
    }
  }

  $code[] = '  return $profiles;';
  $code = implode("\n", $code);
  return array('wysiwyg_default_profiles' => $code);
}

/**
 * Implements hook_features_revert().
 */
function wysiwyg_features_revert($module) {
  return wysiwyg_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 */
function wysiwyg_features_rebuild($module) {
  if ($defaults = features_get_default('wysiwyg', $module)) {
    foreach ($defaults as $profile) {
      db_merge('wysiwyg')
        ->key(array('format' => $profile['format']))
        ->fields(array(
          'editor' => $profile['editor'],
          'settings' => serialize($profile['settings']),
        ))
        ->execute();
    }
    wysiwyg_profile_cache_clear();
  }
}

