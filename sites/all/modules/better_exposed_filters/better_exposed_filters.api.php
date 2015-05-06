<?php
/**
 * @file
 * Hooks provided by the Better Expoosed Filters module.
 */

/**
 * Alters Better Exposed Filters settings before the exposed form widgets are
 * built.
 *
 * @param $settings
 *   The settings array.
 * @param $context
 *   The view and display to which the settings apply.
 */
function hook_better_exposed_filters_settings_alter(&$settings, $context) {
  // Set the min/max value of a slider.
  $settings['field_price_value']['slider_options']['bef_slider_min'] = 500;
  $settings['field_price_value']['slider_options']['bef_slider_max'] = 5000;
}
