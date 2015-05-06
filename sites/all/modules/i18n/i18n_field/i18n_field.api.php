<?php

/**
 * @file
 * API documentation file for Field translation module.
 *
 * This module takes care of translating common field elements like title and
 * description for all fields, plus some field specific values (default, options)
 * for field types defined by Drupal core.
 *
 * Before implementing any of these hooks, consider whether you would be better
 * off implementing Drupal core's hook_field_widget_form_alter().
 *
 * @see i18n_field_field_widget_form_alter()
 */

/**
 * Provide information about callbacks for translating specific field types.
 *
 * This information can be retrieved using i18n_field_type_info().
 * @return
 *   Array of values indexed by field type. Valid keys are:
 *   - 'translate_default', Callback for translating the default value for this field type.
 *   - 'translate_options', Callback for translating options for this field type.
 *
 * @see i18n_field_type_info()
 * @see i18n_field_i18n_field_info()
 *
 * For examples of both callback types:
 *
 * @see i18n_field_translate_allowed_values()
 * @see i18n_field_translate_default()
 *
 */
function hook_i18n_field_info() {
  $info['text'] = $info['text_long'] = $info['text_with_summary'] = array(
    'translate_default' => 'i18n_field_translate_default',
  );
  $info['list_text'] = $info['list_boolean'] = $info['list_integer'] = array(
    'translate_options' => 'i18n_field_translate_allowed_values',
  );
  return $info;
}

/**
 * Alter information provided by hook_i18n_field_info().
 *
 * @see i18n_field_type_info()
 */
function hook_i18n_field_info_alter(&$info) {
  // Unset the default callback for text fields.
  unset($info['text']['translate_default']);
}
