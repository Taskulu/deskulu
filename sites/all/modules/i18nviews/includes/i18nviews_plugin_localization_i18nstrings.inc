<?php
/**
 * @file
 * Contains the i18nstrings localization plugin.
 */

/**
 * Localization plugin to pass translatable strings through i18n_string().
 *
 * @ingroup views_localization_plugins
 */
class i18nviews_plugin_localization_i18nstrings extends views_plugin_localization {

  /**
   * Translate a string.
   *
   * @param $string
   *   The string to be translated.
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   * @param $format
   *   The input format of the string. This is optional.
   */
  function translate_string($string, $keys = array(), $format = '') {
    $options = array();
    $options['format'] = $format ? $format : I18N_STRING_FILTER_XSS_ADMIN;
    return i18n_string($this->stringid($keys), $string, $options);
  }

  /**
   * Save a string for translation
   *
   * @param $string
   *   The string to be translated.
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   * @param $format
   *   The input format of the string. This is optional.
   */
  function save_string($string, $keys = array(), $format = '') {
    $options = array();
    $options['format'] = $format ? $format : I18N_STRING_FILTER_XSS_ADMIN;
    i18n_string_update($this->stringid($keys), $string, $options);
  }

  /**
   * Delete a string.
   *
   * @param $source
   *   Full data for the string to be translated.
   */
  function delete($source) {
    i18n_string_remove($this->stringid($source['keys']), $source['value']);
    return TRUE;
  }

  /**
   * Get string id for i18n
   *
   * @param $keys
   *   Array of keys for the string to be translated.
   */
  function stringid($keys) {

    foreach ($keys as $key => $value) {
      if (is_array($value)) {
        $keys[$key] = implode(':', $value);
      }
    }

    return 'views:' . implode(':', $keys);
  }
}
