<?php

/**
 * @file
 * Integrates Features for @font-your-face.
 */

/**
 * Implements hook_features_export_options. [component_hook]
 *
 * This hook will alert features of which specific items of this component may
 * be exported. For instances, in this case, we want to make available all the
 * existing items.  If there are no items to be exported, this component will
 * not be made available in the features export page.
 *
 * @return array
 *   A keyed array of items, suitable for use with a FormAPI select or
 *   checkboxes element.
 */
function fontyourface_features_export_options() {
  $fonts = array();

  foreach (fontyourface_get_fonts('enabled = 1') as $font) {
    $fonts[$font->name] = $font->name;
  }

  return $fonts;
}

/**
 * Implements hook_features_export [component hook]
 *
 * This is a component hook, rather then a module hook, therefore this is the
 * callback from hook_features_api which relates to the specific component we
 * are looking to export.  When a specific instance of the component we are
 * looking to export is selected, this will include the necessariy item, plus
 * any dependencies into our export array.
 *
 * @param array $data
 *   this is the machine name for the component in question
 * @param array &$export
 *   array of all components to be exported
 * @param string $module_name
 *   The name of the feature module to be generated.
 * @return array
 *   The pipe array of further processors that should be called
 */
function fontyourface_features_export($data, &$export, $module_name = '') {

  // fontyourface_default_fonts integration is provided by Features.
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['fontyourface'] = 'fontyourface';

  // Add dependencies for each font.
  $fonts = fontyourface_get_fonts('enabled = 1');

  foreach ($fonts as $font) {
    if (in_array($font->name, $data)) {

      // Make the font provider required
      $export['dependencies'][$font->provider] = $font->provider;

      $export['features']['fontyourface'][$font->name] = $font->name;
    }
  }

  return $export;
}

/**
 * Implements hook_features_export_render. [component hook]
 *
 * This hook will be invoked in order to export
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * Render one or more component objects to code.
 *
 * @param string $module_name
 *   The name of the feature module to be exported.
 * @param array $data
 *   An array of machine name identifiers for the objects to be rendered.
 * @param array $export
 *   The full export array of the current feature being exported. This is only
 *   passed when hook_features_export_render() is invoked for an actual feature
 *   update or recreate, not during state checks or other operations.
 * @return array
 *   An associative array of rendered PHP code where the key is the name of the
 *   hook that should wrap the PHP code. The hook should not include the name
 *   of the module, e.g. the key for `hook_example` should simply be `example`.
 */
function fontyourface_features_export_render($module, $data) {
  $fonts = fontyourface_get_fonts('enabled = 1');
  $code = array();
  foreach ($data as $name) {
    foreach ($fonts as $font) {
      if ($font->name == $name) {
        // We don't want to break the entity cache, so we need to clone the
        // font before unsetting the id.
        $font = clone $font;
        unset($font->fid);
        unset($font->tags);
        $code[$name] = $font;
      }
    }
  }
  $code = "  return " . features_var_export($code, '  ') . ";";
  return array('fontyourface_features_default_font' => $code);
}

/**
 * Implements hook_features_revert().
 */
function fontyourface_features_revert($module) {
  fontyourface_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 *
 * Rebuilds @font-your-face fonts from code defaults.
 */
function fontyourface_features_rebuild($module) {
  $saved_fonts = module_invoke($module, 'fontyourface_features_default_font');

  foreach ($saved_fonts as $key => $font) {
    $font = (object) $font;
    $font->tags = array();
    $saved = fontyourface_save_font($font, TRUE);
  }
}
