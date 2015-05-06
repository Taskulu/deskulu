<?php
/**
 * @file
 * Plugin to provide access control on entities based on translations.
 */

/**
 * CTools Plugin definition.
 */
$plugin = array(
  'title' => t("Entity translation: translation exists"),
  'description' => t('Control access by checking if a translation exists.'),
  'callback' => 'entity_translation_translation_exists_ctools_access_check',
  'default' => array('language' => array()),
  'settings form' => 'entity_translation_translation_exists_ctools_access_settings',
  'settings form submit' => 'entity_translation_translation_exists_ctools_access_settings_submit',
  'summary' => 'entity_translation_translation_exists_ctools_access_summary',
  'get child' => 'entity_translation_translation_exists_ctools_access_get_child',
  'get children' => 'entity_translation_translation_exists_ctools_access_get_children',
);

/**
 * Get a particular instance of this plugin.
 */
function entity_translation_translation_exists_ctools_access_get_child($plugin, $parent, $child) {

  $plugins = &drupal_static(__FUNCTION__, array());
  if (empty($plugins[$parent . ':' . $child])) {
    $plugins[$parent . ':' . $child] = _entity_translation_translation_exists_ctools_access_definition($plugin, $parent, $child);
  }
  return $plugins[$parent . ':' . $child];
}

/**
 * Get all children of this plugin.
 */
function entity_translation_translation_exists_ctools_access_get_children($plugin, $parent) {
  $plugins = &drupal_static(__FUNCTION__, array());
  if (!empty($plugins)) {
    return $plugins;
  }
  $entities = entity_get_info();
  foreach ($entities as $entity_type => $entity) {
    $plugin = _entity_translation_translation_exists_ctools_access_definition($plugin, $parent, $entity_type);
    $plugins[$parent . ':' . $entity_type] = $plugin;
  }
  return $plugins;
}

/**
 * Plugin definition for one particular plugin child.
 */
function _entity_translation_translation_exists_ctools_access_definition($plugin, $parent, $entity_type) {
  $entity = entity_get_info($entity_type);

  $plugin['title'] = t('@entity: entity translation exists', array('@entity' => $entity['label']));
  $plugin['keyword'] = $entity_type;
  $plugin['description'] = t('Control access by @entity language', array('@entity' => $entity['label']));
  $plugin['name'] = $parent . ':' . $entity_type;
  $plugin['required context'] = new ctools_context_required(t(ucfirst($entity_type)), $entity_type);

  return $plugin;
}

/**
 * Settings form for the 'by node_language' access plugin
 */
function entity_translation_translation_exists_ctools_access_settings($form, &$form_state, $conf) {
  $options = array(
    ENTITY_TRANSLATION_LANGUAGE_CURRENT => t('Current content language'),
    ENTITY_TRANSLATION_LANGUAGE_DEFAULT => t('Default site language'),
    LANGUAGE_NONE => t('Language neutral'),
  );
  $options = array_merge($options, locale_language_list());
  $form['settings']['language'] = array(
    '#title' => t('Language'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('This rule will pass if any of these languages are present.'),
    '#default_value' => $conf['language'],
  );
  return $form;
}

/**
 * Grant access based on the which translations are available.
 */
function entity_translation_translation_exists_ctools_access_check($conf, $context) {
  // Check that the context exists.
  if (empty($context) || empty($context->data)) {
    return FALSE;
  }
  $entity = $context->data;
  $handler = entity_translation_get_handler($context->keyword, $entity);
  if (!empty($handler)) {
    $translations = $handler->getTranslations();
    global $language_content;
    foreach ($conf['language'] as $lang) {
      if ($lang) {
        switch ($lang) {
          case ENTITY_TRANSLATION_LANGUAGE_CURRENT:
            $lang = $language_content->language;
            break;
          case ENTITY_TRANSLATION_LANGUAGE_DEFAULT:
            $lang = language_default('language');
            break;
        }
        if (isset($translations->data[$lang]) && $translations->data[$lang]['status']) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/**
 * Provide a summary description based upon the checked node_languages.
 */
function entity_translation_translation_exists_ctools_access_summary($conf, $context) {
  $languages = array(
    ENTITY_TRANSLATION_LANGUAGE_CURRENT => t('Current site content language'),
    ENTITY_TRANSLATION_LANGUAGE_DEFAULT => t('Default site language'),
    LANGUAGE_NONE => t('Language neutral'),
  );
  $languages = array_merge($languages, locale_language_list());

  if (!isset($conf['language'])) {
    $conf['language'] = array();
  }

  $names = array();
  foreach (array_filter($conf['language']) as $language) {
    $names[] = $languages[$language];
  }

  if (empty($names)) {
    return t('@identifier is in any language', array('@identifier' => $context->identifier));
  }

  return format_plural(count($names), '@languages translation exists for @identifier', '@languages translations exists for identifier', array('@languages' => implode(', ', $names), '@identifier' => $context->identifier));
}
