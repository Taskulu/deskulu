<?php

/**
 * @file
 * Provides locale-related properties.
 */

/**
 * Implements hook_entity_property_info_alter() on top of locale module.
 *
 * @see entity_entity_property_info_alter()
 */
function entity_metadata_locale_entity_property_info_alter(&$info) {

  $info['user']['properties']['language'] = array(
    'label' => t("Language"),
    'description' => t("This account's default language for e-mails, and preferred language for site presentation."),
    'type' => 'token',
    'getter callback' => 'entity_metadata_locale_get_user_language',
    'setter callback' => 'entity_property_verbatim_set',
    'options list' => 'entity_metadata_language_list',
    'schema field' => 'language',
    'setter permission' => 'administer users',
  );

  $info['site']['properties']['current_page']['property info']['language'] = array(
    'label' => t("Interface language"),
    'description' => t("The language code of the current user interface language."),
    'type' => 'token',
    'getter callback' => 'entity_metadata_locale_get_languages',
    'options list' => 'entity_metadata_language_list',
  );
  $info['site']['properties']['current_page']['property info']['language_content'] = array(
    'label' => t("Content language"),
    'description' => t("The language code of the current content language."),
    'type' => 'token',
    'getter callback' => 'entity_metadata_locale_get_languages',
    'options list' => 'entity_metadata_language_list',
  );
}
