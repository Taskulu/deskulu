<?php

/**
 * @file
 * Drush integration for @font-your-face.
 */

/**
 * Implements hook_drush_help().
 */
function fontyourface_drush_help($section) {
  switch ($section) {
    case 'drush:fyf-status':
      $help = dt('Displays @font-your-face status');
      return $help;
    case 'drush:fyf-import':
      $help = dt('Imports @font-your-face fonts');
      return $help;
    case 'meta:fontyourface:title':
      $help = dt('@font-your-face commands');
      return $help;
  }
}

/**
 * Implements hook_drush_command().
 */
function fontyourface_drush_command() {
  $items = array();
  $items['fyf-status'] = array(
    'callback' => 'fontyourface_fyf_status',
    'drupal dependencies' => array('fontyourface'),
    'description' => 'Displays general @font-your-face status information',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('fyfs'),
  );
  $items['fyf-import'] = array(
    'callback' => 'fontyourface_fyf_import',
    'drupal dependencies' => array('fontyourface'),
    'description' => 'Imports fonts from one or more providers',
    'arguments' => array(
      'providers' => '(optional) A space delimited list of providers.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('fyfi'),
    'examples' => array(
      'drush fyfi' => 'Import fonts from all enabled providers.',
      'drush fyfi google_fonts_api fontdeck' => 'Import fonts from Google Fonts API and Fontdeck.',
    ),
  );
  return $items;
}

/**
 * Callback for Drush command 'fyf-status'.
 */
function fontyourface_fyf_status() {
  // Provider info.
  $modules = module_implements('fontyourface_info', TRUE);
  if (count($modules) > 0) {
    drush_print(dt('Providers enabled (@count):', array('@count' => count($modules))));
    $rows = array();
    $rows[] = array(
      dt('Provider'),
      dt('Total fonts'),
      dt('Enabled fonts'),
    );
    foreach ($modules as $module) {
      $row = array();
      $row[] = $module;
      $row[] = fontyourface_count_fonts($module);
      $row[] = fontyourface_count_fonts($module, TRUE);
      $rows[] = $row;
    }
    drush_print_table($rows, TRUE);
  }
  else {
    drush_print(dt('No font providers are enabled.'));
    // If there are no providers, we can call it quits as there will be no
    // fonts either.
    return;
  }
  // Font info.
  $fonts = fontyourface_get_fonts('enabled = 1');
  if (count($fonts) > 0) {
    drush_print(dt('Fonts enabled (@count):', array('@count' => count($fonts))));
    $rows = array();
    $rows[] = array(
      dt('Font'),
      dt('Provider'),
      dt('CSS selector'),
    );
    foreach ($fonts as $font) {
      $row = array();
      $row[] = $font->name;
      $row[] = $font->provider;
      $row[] = $font->css_selector;
      $rows[] = $row;
    }
    drush_print_table($rows, TRUE);
  }
  else {
    drush_print(dt('No fonts are enabled.'));
  }
}

/**
 * Callback for Drush command 'fyf-import'.
 */
function fontyourface_fyf_import() {
  $providers = _convert_csv_to_array(func_get_args());
  if (empty($providers)) {
    // If no providers were specified, get all existing import functions.
    $providers = module_implements('fontyourface_import');
  }
  foreach ($providers as $provider) {
    $import_function = $provider . '_fontyourface_import';
    if ($provider == 'fontsquirrel') {
      drush_set_error(dt("Font Squirrel import is not supported yet. We're working on that."));
    }
    elseif (!function_exists($import_function)) {
      drush_set_error(dt("Unknown provider '@provider', or there is no import function for it; run 'drush fyf-status' to see enabled providers", array('@provider' => $provider)));
    }
    else {
      $before = fontyourface_count_fonts($provider);
      if ($import_function()) {
        $message = dt('Imported fonts from @provider.', array('@provider' => $provider));
        drush_log($message, 'success');
        $after = fontyourface_count_fonts($provider);
        drush_print(dt('@provider now has @fonts fonts (+@diff).', array('@provider' => $provider, '@fonts' => $after, '@diff' => $after - $before)));
      }
      else {
        drush_set_error(dt('Font import from @provider failed.', array('@provider' => $provider)));
      }
    }
  }
}
