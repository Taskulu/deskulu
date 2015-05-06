<?php

/**
 * @file
 * This file contains a field handler for entity translation that shows field
 * content translated into a specified language.
 */

class entity_translation_handler_field_field extends views_handler_field_field {

  function option_definition() {
    $options = parent::option_definition();

    $options['language'] = array(
      'default' => '***CURRENT_LANGUAGE***',
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $languages = array(
      '***CURRENT_LANGUAGE***' => t("Current user's language"),
      '***DEFAULT_LANGUAGE***' => t("Default site language"),
      LANGUAGE_NONE => t('Language neutral'),
    );

    $languages = array_merge($languages, locale_language_list());

    $form['language'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#options' => $languages,
      '#default_value' => $this->options['language'],
      '#description' => t('Select the language to display this field in')
    );
  }

  /**
   * Overrides parent::field_language, retrieving the language from the handler
   * options.
   */
  function field_language($entity_type, $entity) {
    global $language_content;

    if (field_is_translatable($entity_type, $this->field_info)) {
      $default_language = language_default('language');
      $language = str_replace(array('***CURRENT_LANGUAGE***', '***DEFAULT_LANGUAGE***'),
                              array($language_content->language, $default_language),
                              $this->options['language']);

      // Give the Field Language API a chance to fallback to a different language
      // (or LANGUAGE_NONE), in case the field has no data for the selected language.
      // field_view_field() does this as well, but since the returned language code
      // is used before calling it, the fallback needs to happen explicitly.
      $language = field_language($entity_type, $entity, $this->field_info['field_name'], $language);

      return $language;
    }
    else {
      return LANGUAGE_NONE;
    }
  }
}
