<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for locale.module.
 */

/**
 * Implements hook_feeds_processor_targets_alter().
 */
function locale_feeds_processor_targets_alter(array &$targets, $entity_type, $bundle_name) {
  foreach (array_keys($targets) as $target) {
    $targets[$target]['preprocess_callbacks'][] = 'locale_feeds_preprocess_callback';
    $targets[$target]['summary_callbacks'][] = 'locale_feeds_summary_callback';
    $targets[$target]['form_callbacks'][] = 'locale_feeds_form_callback';
  }
}

/**
 * Preprocess callback that sets the configured mapping language.
 */
function locale_feeds_preprocess_callback(array $target, array &$mapping) {
  if (empty($mapping['field_language'])) {
    return;
  }

  $mapping['language'] = $mapping['field_language'];
}

/**
 * Summary callback.
 */
function locale_feeds_summary_callback(array $mapping, array $target, array $form, array $form_state) {
  $entity_type = $form_state['build_info']['args'][0]->processor->entityType();
  $translatable = _locale_feeds_target_is_translatable($entity_type, $mapping['target']);

  $mapping += array('field_language' => LANGUAGE_NONE);

  $language_options = array(LANGUAGE_NONE => t('Language neutral')) + locale_language_list('name');

  $error = NULL;
  if ($mapping['field_language'] !== LANGUAGE_NONE && !$translatable) {
    // This is an invalid configuration that can come from disabling
    // entity_translation.
    $error = t('Field not translatable');
  }
  if (!isset($language_options[$mapping['field_language']])) {
    // This is an invalid configuration that can be caused by disabling or
    // removing the language in question.
    $error = t('Language \'@lang\' not available', array('@lang' => $mapping['field_language']));
  }

  // Nothing to see here.
  if (!$error && !$translatable) {
    return;
  }

  if ($error) {
    return t('Language: <strong>Error: @error</strong>', array('@error' => $error));
  }

  return t('Language: %lang', array('%lang' => $language_options[$mapping['field_language']]));
}

/**
 * Form callback.
 */
function locale_feeds_form_callback(array $mapping, array $target, array $form, array $form_state) {
  $form = array();

  $entity_type = $form_state['build_info']['args'][0]->processor->entityType();

  $translatable = _locale_feeds_target_is_translatable($entity_type, $mapping['target']);
  $mapping += array('field_language' => LANGUAGE_NONE);

  // This is an invalid configuration that can come from disabling
  // entity_translation.
  $error = $mapping['field_language'] !== LANGUAGE_NONE && !$translatable;

  // Nothing to see here.
  if (!$error && !$translatable) {
    return $form;
  }

  $language_options = array(LANGUAGE_NONE => t('Language neutral'));

  if (!$error) {
    $language_options += locale_language_list('name');
  }

  $form['field_language'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#options' => $language_options,
    '#default_value' => $mapping['field_language'],
  );

  return $form;
}

/**
 * Determines if a target is translatable.
 *
 * @param string $entity_type
 *   The entity type.
 * @param string $target
 *   The target.
 *
 * @return bool
 *   Returns true if the target is translatable, false if not.
 */
function _locale_feeds_target_is_translatable($entity_type, $target) {
  list($field_name) = explode(':', $target, 2);

  $info = field_info_field($field_name);

  return !empty($info) && field_is_translatable($entity_type, $info);
}
