<?php

/**
 * @file
 * The entity translation user interface.
 */

/**
 * Builder function for the entity translation settings form.
 */
function entity_translation_admin_form($form, $form_state) {
  $options = array();

  $form['locale_field_language_fallback'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable language fallback'),
    '#description' => t('When language fallback is enabled, if a translation is not available for the requested language an existing one will be displayed.'),
    '#default_value' => variable_get('locale_field_language_fallback', TRUE),
  );

  $form['entity_translation_show_fallback_on_overview_pages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show fallback statuses on overview pages'),
    '#description' => t('Enable to the show fallback information on the entity overview pages.'),
    '#default_value' => variable_get('entity_translation_show_fallback_on_overview_pages', FALSE),
    '#states' => array(
      'visible' => array(
        ':input[name="locale_field_language_fallback"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['entity_translation_shared_labels'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display shared labels'),
    '#description' => t('Append an "all languages" hint to entity form widgets shared across translations.'),
    '#default_value' => variable_get('entity_translation_shared_labels', TRUE),
  );

  $form['entity_translation_workflow_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable translation workflow permissions'),
    '#description' => t('By enabling the translation workflow permissions it will be possible to limit the access to the entity form elements. Once this is active every role previously allowed to access the entity form will need to be granted the <em>Edit original values</em> permission to edit the entity in the original language. Moreover, form elements dealing with values shared across the translations will be visible only to roles having been granted the <em>Edit shared values</em> permission.'),
    '#default_value' => entity_translation_workflow_enabled(),
  );

  $_null = NULL;
  list($items, ) = menu_router_build();
  _entity_translation_validate_path_schemes($_null, FALSE, $items);

  foreach (entity_get_info() as $entity_type => $info) {
    if ($info['fieldable']) {
      $et_info = &$info['translation']['entity_translation'];
      _entity_translation_process_path_schemes($entity_type, $et_info);
      _entity_translation_validate_path_schemes($et_info['path schemes'], $info['label']);

      // Translation can be enabled for the current entity only if it defines at
      // least one valid base path.
      foreach ($et_info['path schemes'] as $delta => $scheme) {
        if (!empty($scheme['base path'])) {
          $options[$entity_type] = !empty($info['label']) ? t($info['label']) : $entity_type;
          break;
        }
      }
    }
  }

  // Avoid bloating memory with unused data.
  drupal_static_reset('_entity_translation_validate_path_schemes');
  $enabled_types = array_filter(variable_get('entity_translation_entity_types', array()));

  $form['enabled'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translatable entity types'),
    '#description' => t('Select which entities can be translated.'),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($enabled_types),
  );

  $form['enabled']['entity_translation_entity_types'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $enabled_types,
  );

  $form['tabs'] = array(
    '#type' => 'vertical_tabs',
  );

  $languages = array(
    ENTITY_TRANSLATION_LANGUAGE_DEFAULT => t('Default language'),
    ENTITY_TRANSLATION_LANGUAGE_CURRENT => t('Current language'),
    ENTITY_TRANSLATION_LANGUAGE_AUTHOR => t('Author language'),
    LANGUAGE_NONE => t('Language neutral'),
  );
  foreach (entity_translation_languages($entity_type) as $langcode => $language) {
    $languages[$langcode] = t($language->name);
  }

  foreach ($enabled_types as $entity_type) {
    $label = $options[$entity_type];

    $entity_info = entity_get_info($entity_type);
    $bundles = !empty($entity_info['bundles']) ? $entity_info['bundles'] : array($entity_type => array('label' => $label));
    $enabled_bundles = 0;

    foreach ($bundles as $bundle => $info) {
      if (entity_translation_enabled_bundle($entity_type, $bundle)) {
        $enabled_bundles++;
        $settings = entity_translation_settings($entity_type, $bundle);
        $settings_key = 'entity_translation_settings_' . $entity_type . '__' . $bundle;
        $form['settings'][$entity_type][$settings_key] = array('#tree' => TRUE);

        // If the entity defines no bundle we do not need the fieldset.
        if (count($bundles) > 1 || $bundle != $entity_type) {
          $form['settings'][$entity_type][$settings_key] += array(
            '#type' => 'fieldset',
            '#title' => $info['label'],
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
          );
        }

        $form['settings'][$entity_type][$settings_key]['default_language'] = array(
          '#type' => 'select',
          '#title' => t('Default language'),
          '#options' => $languages,
          '#default_value' => $settings['default_language'],
        );

        $form['settings'][$entity_type][$settings_key]['hide_language_selector'] = array(
          '#type' => 'checkbox',
          '#title' => t('Hide language selector'),
          '#default_value' => $settings['hide_language_selector'],
        );

        $form['settings'][$entity_type][$settings_key]['exclude_language_none'] = array(
          '#type' => 'checkbox',
          '#title' => t('Exclude <em>Language neutral</em> from the available languages'),
          '#default_value' => $settings['exclude_language_none'],
        );

        $form['settings'][$entity_type][$settings_key]['lock_language'] = array(
          '#type' => 'checkbox',
          '#title' => t('Prevent language from being changed once the entity has been created'),
          '#default_value' => $settings['lock_language'],
        );

        $form['settings'][$entity_type][$settings_key]['shared_fields_original_only'] = array(
          '#type' => 'checkbox',
          '#title' => t('Hide shared elements on translation forms'),
          '#description' => t('Display form elements shared across translations only on entity forms for the original language.'),
          '#default_value' => $settings['shared_fields_original_only'],
        );
      }
    }

    if ($enabled_bundles > 0) {
      $form['settings'][$entity_type][$settings_key]['#collapsed'] = $enabled_bundles > 1;
      $form['settings'][$entity_type] += array(
        '#type' => 'fieldset',
        '#group' => 'tabs',
        '#title' => $label,
      );
    }
  }

  $form = system_settings_form($form);

  // Menu rebuilding needs to be performed after the system settings are saved.
  $form['#submit'][] = 'entity_translation_admin_form_submit';

  return $form;
}

/**
 * Submit handler for the entity translation settings form.
 */
function entity_translation_admin_form_submit($form, $form_state) {
  // Clear the entity info cache for the new entity translation settings.
  entity_info_cache_clear();
  menu_rebuild();
}

/**
 * Applies the given settings to every defined bundle.
 *
 * @param $entity_type
 *   The entity type the settings refer to.
 * @param $settings
 *   (optional) The settings to be applied. Defaults to the entity default
 *   settings.
 */
function entity_translation_settings_init($entity_type, $settings = array()) {
  if (entity_translation_enabled($entity_type)) {
    $info = entity_get_info($entity_type);
    $bundles = !empty($info['bundles']) ? array_keys($info['bundles']) : array($entity_type);

    foreach ($bundles as $bundle) {
      if (entity_translation_enabled_bundle($entity_type, $bundle)) {
        $settings += entity_translation_settings($entity_type, $bundle);
      }
    }

    variable_set('entity_translation_settings_' . $entity_type . '__' . $bundle, $settings);
  }
}

/**
 * Translations overview page callback.
 */
function entity_translation_overview($entity_type, $entity, $callback = NULL) {
  // Entity translation and node translation share the same system path.
  if ($callback && entity_translation_node($entity_type, $entity)) {
    return entity_translation_overview_callback($callback, $entity);
  }

  $handler = entity_translation_get_handler($entity_type, $entity);
  // Ensure $entity holds an entity object and not an id.
  $entity = $handler->getEntity();
  $handler->initPathScheme();

  // Initialize translations if they are empty.
  $translations = $handler->getTranslations();
  if (empty($translations->original)) {
    $handler->initTranslations();
    $handler->saveTranslations();
  }

  // Ensure that we have a coherent status between entity language and field
  // languages.
  if ($handler->initOriginalTranslation()) {
    // FIXME!
    entity_translation_entity_save($entity_type, $entity);
  }

  $header = array(t('Language'), t('Source language'), t('Translation'), t('Status'), t('Operations'));
  $languages = entity_translation_languages();
  $source = $translations->original;
  $path = $handler->getViewPath();
  $rows = array();

  if (drupal_multilingual()) {
    // If we have a view path defined for the current entity get the switch
    // links based on it.
    if ($path) {
      $links = EntityTranslationDefaultHandler::languageSwitchLinks($path);
    }

    $show_fallback = variable_get('locale_field_language_fallback', TRUE) && variable_get('entity_translation_show_fallback_on_overview_pages', FALSE);

    foreach ($languages as $language) {
      $classes = array();
      $options = array();
      $language_name = $language->name;
      $langcode = $language->language;
      $edit_path = $handler->getEditPath($langcode);
      $add_path = "{$handler->getEditPath()}/add/$source/$langcode";

      if ($edit_path) {
        $add_links = EntityTranslationDefaultHandler::languageSwitchLinks($add_path);
        $edit_links = EntityTranslationDefaultHandler::languageSwitchLinks($edit_path);
      }

      if (isset($translations->data[$langcode])) {
        list(, , $bundle) = entity_extract_ids($entity_type, $entity);

        // Existing translation in the translation set: display status.
        $is_original = $langcode == $translations->original;
        $translation = $translations->data[$langcode];
        $label = _entity_translation_label($entity_type, $entity, $langcode);
        $link = isset($links->links[$langcode]['href']) ? $links->links[$langcode] : array('href' => $path, 'language' => $language);
        $row_title = l($label, $link['href'], $link);

        if (empty($link['href'])) {
          $row_title = $is_original ? $label : t('n/a');
        }

        if ($edit_path && $handler->getAccess('update') && $handler->getTranslationAccess($langcode)) {
          $link = isset($edit_links->links[$langcode]['href']) ? $edit_links->links[$langcode] : array('href' => $edit_path, 'language' => $language);
          $link['query'] = isset($_GET['destination']) ? drupal_get_destination() : FALSE;
          $options[] = l(t('edit'), $link['href'], $link);
        }

        $status = $translation['status'] ? t('Published') : t('Not published');
        $classes[] = $translation['status'] ? 'published' : 'not-published';
        $status .= isset($translation['translate']) && $translation['translate'] ? theme('entity_translation_overview_outdated', array('message' => t('outdated'))) : '';
        $classes[] = isset($translation['translate']) && $translation['translate'] ? 'outdated' : '';

        if ($is_original) {
          $language_name = t('<strong>@language_name</strong>', array('@language_name' => $language_name));
          $source_name = t('(original content)');
        }
        else {
          $source_name = $languages[$translation['source']]->name;
        }
      }
      else {
        // No such translation in the set yet: help user to create it.
        $row_title = $source_name = t('n/a');

        if ($source != $langcode && $handler->getAccess('update') && $handler->getTranslationAccess($langcode)) {
          list(, , $bundle) = entity_extract_ids($entity_type, $entity);
          $translatable = FALSE;

          foreach (field_info_instances($entity_type, $bundle) as $instance) {
            $field_name = $instance['field_name'];
            $field = field_info_field($field_name);
            if ($field['translatable']) {
              $translatable = TRUE;
              break;
            }
          }

          $link = isset($add_links->links[$langcode]['href']) ? $add_links->links[$langcode] : array('href' => $add_path, 'language' => $language);
          $link['query'] = isset($_GET['destination']) ? drupal_get_destination() : FALSE;
          $options[] = $translatable ? l(t('add'), $link['href'], $link) : t('No translatable fields');
          $classes[] = $translatable ? '' : 'non-traslatable';
        }
        $status = t('Not translated');
        // Show fallback information if required.
        if ($show_fallback) {
          $language_fallback_candidates = _entity_translation_language_fallback_get_candidates($language);
          $fallback_candidates = array_intersect_key(drupal_map_assoc($language_fallback_candidates), $translations->data);
          $fallback_langcode = reset($fallback_candidates);
          if ($fallback_langcode !== FALSE) {
            $status = t('Fallback from @language', array('@language' => $languages[$fallback_langcode]->name));
            $label = _entity_translation_label($entity_type, $entity, $fallback_langcode);
            $link = isset($links->links[$langcode]['href']) ? $links->links[$langcode] : array('href' => $path, 'language' => $langcode);
            $row_title = l($label, $link['href'], $link);
          }
        }
      }
      $rows[] = array(
        'data' => array($language_name, $source_name, $row_title, $status, implode(" | ", $options)),
        'class' => $classes,
      );
    }
  }

  drupal_set_title(t('Translations of %label', array('%label' => $handler->getLabel())), PASS_THROUGH);

  // Add metadata to the build render allow to let other modules know about
  // which entity this is.
  $build['#entity_type'] = $entity_type;
  $build['#entity'] = $entity;
  $build['entity_translation_overview'] = array(
    '#theme' => 'entity_translation_overview',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Calls the appropriate translation overview callback.
 */
function entity_translation_overview_callback($callback, $entity) {
  if (module_exists($callback['module'])) {
    if (isset($callback['file'])) {
      $path = isset($callback['file path']) ? $callback['file path'] : drupal_get_path('module', $callback['module']);
      require_once DRUPAL_ROOT . '/' . $path . '/' . $callback['file'];
    }
    return $callback['page callback']($entity);
  }
}

/**
 * Returns the appropriate entity label for the given language.
 */
function _entity_translation_label($entity_type, $entity, $langcode = NULL) {
  if (function_exists('title_entity_label')) {
    list (, , $bundle) = entity_extract_ids($entity_type, $entity);
    $entity_info = entity_get_info($entity_type);
    if (!empty($entity_info['entity keys']['label'])) {
      $legacy_field = $entity_info['entity keys']['label'];
      if (title_field_replacement_enabled($entity_type, $bundle, $legacy_field)) {
        $title = title_entity_label($entity, $entity_type, $langcode);
        if (!empty($title)) {
          return $title;
        }
      }
    }
  }
  return t('view');
}
/**
 * Theme wrapper for the entity translation language page.
 */
function theme_entity_translation_overview($variables){
  $rows = $variables['rows'];
  return theme('table', array(
    'rows' => $rows,
    'header' => $variables['header'],
  ));
}

/**
 * Theme wrapper for the entity translation language outdated translation.
 */
function theme_entity_translation_overview_outdated($variables){
  $message = $variables['message'];
  return ' - <span class="marker">' . $message . '</span>';
}

/**
 * Translation deletion confirmation form.
 */
function entity_translation_delete_confirm($form, $form_state, $entity_type, $entity, $langcode) {
  $handler = entity_translation_get_handler($entity_type, $entity);
  $handler->setFormLanguage($langcode);
  $languages = language_list();

  $form = array(
    '#handler' => $handler,
    '#entity_type' => $entity_type,
    // Ensure $entity holds an entity object and not an id.
    '#entity' => $handler->getEntity(),
    '#language' => $langcode,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the @language translation of %label?', array('@language' => $languages[$langcode]->name, '%label' => $handler->getLabel())),
    $handler->getEditPath($langcode),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for the translation deletion confirmation.
 */
function entity_translation_delete_confirm_submit($form, &$form_state) {
  $handler = $form['#handler'];
  $entity_type = $form['#entity_type'];
  $entity = $form['#entity'];
  $langcode = $form['#language'];

  // Some modules expect the original values to be present when updating the
  // field values. Since we are deleting the translation no value has changed.
  $entity->original = $entity;

  // Remove the translation entry and the related fields.
  $handler->removeTranslation($langcode);
  entity_translation_entity_save($entity_type, $entity);

  $form_state['redirect'] = $handler->getTranslatePath();
}

/*
 * Confirm form for changing field translatability.
 */
function entity_translation_translatable_form($form, &$form_state, $field_name) {
  $field = field_info_field($field_name);

  if ($field['translatable']) {
    $title = t('Are you sure you want to disable translation for this field?');
    $text = t('All occurrences of this field will become <em>untranslatable</em>:');
    if (empty($form_state['input'])) {
      drupal_set_message(t('All the existing field translations will be deleted. This operation cannot be undone.'), 'warning');
    }
  }
  else {
    $title = t('Are you sure you want to enable translation for this field?');
    $text = t('All occurrences of this field will become <em>translatable</em>:');

    $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Migration settings'),
      '#weight' => -10,
    );
    $form['options']['copy_all_languages'] = array(
      '#title' => t('Copy translations'),
      '#description' => t('Copy field data into <em>all</em> existing translations, otherwise data will only be available in the original language.'),
      '#type' => 'checkbox',
      '#default_value' => TRUE,
    );
  }

  $text .= _entity_translation_field_desc($field);
  $text .= t('This operation may take a long time to complete.');

  // We need to keep some information for later processing.
  $form_state['field'] = $field;

  // Store the 'translatable' status on the client side to prevent outdated form
  // submits from toggling translatability.
  $form['translatable'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['translatable'],
  );

  return confirm_form($form, $title, '', $text);
}

/**
 * Submit handler for the field settings form.
 *
 * This submit handler maintains consistency between the translatability of an
 * entity and the language under which the field data is stored. When a field is
 * marked as translatable, all the data in $entity->{field_name}[LANGUAGE_NONE]
 * is moved to $entity->{field_name}[$entity_language]. When a field is marked
 * as untranslatable the opposite process occurs. Note that marking a field as
 * untranslatable will cause all of its translations to be permanently removed,
 * with the exception of the one corresponding to the entity language.
 */
function entity_translation_translatable_form_submit($form, $form_state) {
  // This is the current state that we want to reverse.
  $translatable = $form_state['values']['translatable'];
  $field_name = $form_state['field']['field_name'];
  $copy_all_languages = !empty($form_state['values']['copy_all_languages']);
  $field = field_info_field($field_name);

  if ($field['translatable'] !== $translatable) {
    // Field translatability has changed since form creation, abort.
    $t_args = array('%field_name' => $field_name, '!translatable' => $translatable ? t('untranslatable') : t('translatable'));
    drupal_set_message(t('The field %field_name is already !translatable. No change was performed.', $t_args), 'warning');
    return;
  }

  // If a field is untranslatable, it can have no data except under
  // LANGUAGE_NONE. Thus we need a field to be translatable before we convert
  // data to the entity language. Conversely we need to switch data back to
  // LANGUAGE_NONE before making a field untranslatable lest we lose
  // information.
  $operations = array(
    array('entity_translation_translatable_batch', array(!$translatable, $field_name, $copy_all_languages)),
    array('entity_translation_translatable_switch', array(!$translatable, $field_name)),
  );
  $operations = $translatable ? $operations : array_reverse($operations);

  $t_args = array('%field' => $field_name);
  $title = !$translatable ? t('Enabling translation for the %field field', $t_args) : t('Disabling translation for the %field field', $t_args);

  $batch = array(
    'title' => $title,
    'operations' => $operations,
    'finished' => 'entity_translation_translatable_batch_done',
    'file' => drupal_get_path('module', 'entity_translation') . '/entity_translation.admin.inc',
  );

  batch_set($batch);
}

/*
 * Toggle translatability of the given field.
 *
 * This is called from a batch operation, but should only run once per field.
 */
function entity_translation_translatable_switch($translatable, $field_name) {
  $field = field_info_field($field_name);

  if ($field['translatable'] === $translatable) {
    return;
  }

  $field['translatable'] = $translatable;
  field_update_field($field);

  // This is needed for versions of Drupal core 7.10 and lower.
  // See http://drupal.org/node/1380660 for details.
  drupal_static_reset('field_available_languages');
}

/**
 * Batch operation. Convert field data to or from LANGUAGE_NONE.
 */
function entity_translation_translatable_batch($translatable, $field_name, $copy_all_languages, &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;

    // How many entities will need processing?
    $query = new EntityFieldQuery();
    $count = $query
      ->fieldCondition($field_name)
      ->count()
      ->execute();

    if (intval($count) === 0) {
      // Nothing to do.
      $context['finished'] = 1;
      return;
    }
    $context['sandbox']['max'] = $count;
  }

  // Number of entities to be processed for each step.
  $limit = variable_get('entity_translation_translatable_batch_limit', 10);

  $offset = $context['sandbox']['progress'];
  $query = new EntityFieldQuery();
  $result = $query
    ->fieldCondition($field_name)
    ->entityOrderBy('entity_id')
    ->range($offset, $limit)
    ->execute();

  foreach ($result as $entity_type => $entities) {
    foreach (entity_load($entity_type, array_keys($entities)) as $entity) {
      $context['sandbox']['progress']++;
      $handler = entity_translation_get_handler($entity_type, $entity);
      $langcode = $handler->getLanguage();

      // Skip process for language neutral entities.
      if ($langcode == LANGUAGE_NONE) {
        continue;
      }

      // We need a two-steps approach while updating field translations: given
      // that field-specific update functions might rely on the stored values to
      // perform their processing, see for instance file_field_update(), first
      // we need to store the new translations and only after we can remove the
      // old ones. Otherwise we might have data loss, since the removal of the
      // old translations might occur before the new ones are stored.
      if ($translatable && isset($entity->{$field_name}[LANGUAGE_NONE])) {
        // If the field is being switched to translatable and has data for
        // LANGUAGE_NONE then we need to move the data to the right language.

        $translations = $handler->getTranslations();

        if ($copy_all_languages && !empty($translations->data)) {
          foreach ($translations->data as $translation) {
            $entity->{$field_name}[$translation['language']] = $entity->{$field_name}[LANGUAGE_NONE];
          }
        }
        else {
          $entity->{$field_name}[$langcode] = $entity->{$field_name}[LANGUAGE_NONE];
        }

        // Store the original value.
        _entity_translation_update_field($entity_type, $entity, $field_name);
        $entity->{$field_name}[LANGUAGE_NONE] = array();
        // Remove the language neutral value.
        _entity_translation_update_field($entity_type, $entity, $field_name);
      }
      elseif (!$translatable && isset($entity->{$field_name}[$langcode])) {
        // The field has been marked untranslatable and has data in the entity
        // language: we need to move it to LANGUAGE_NONE and drop the other
        // translations.
        $entity->{$field_name}[LANGUAGE_NONE] = $entity->{$field_name}[$langcode];
        // Store the original value.
        _entity_translation_update_field($entity_type, $entity, $field_name);
        // Remove translations.
        foreach ($entity->{$field_name} as $langcode => $items) {
          if ($langcode != LANGUAGE_NONE) {
            $entity->{$field_name}[$langcode] = array();
          }
        }
        _entity_translation_update_field($entity_type, $entity, $field_name);
      }
      else {
        // No need to save unchanged entities.
        continue;
      }
    }
  }

  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
}

/**
 * Stores the given field translations.
 */
function _entity_translation_update_field($entity_type, $entity, $field_name) {
  $empty = 0;
  $field = field_info_field($field_name);

  // Ensure that we are trying to store only valid data.
  foreach ($entity->{$field_name} as $langcode => $items) {
    $entity->{$field_name}[$langcode] = _field_filter_items($field, $entity->{$field_name}[$langcode]);
    $empty += empty($entity->{$field_name}[$langcode]);
  }

  // Save the field value only if there is at least one item available,
  // otherwise any stored empty field value would be deleted. If this happens
  // the range queries would be messed up.
  if ($empty < count($entity->{$field_name})) {
    entity_translation_entity_save($entity_type, $entity);
  }
}

/**
 * Check the exit status of the batch operation.
 */
function entity_translation_translatable_batch_done($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t("Data successfully processed."));
  }
  else {
    // @todo: Do something about this case.
    drupal_set_message(t("Something went wrong while processing data. Some nodes may appear to have lost fields."));
  }
}

/**
 * Returns language fallback candidates for a certain language.
 *
 * @param object $language
 *   Drupal language object.
 *
 * @return array
 *   An array of language codes in the fallback order.
 */
function _entity_translation_language_fallback_get_candidates($language) {
  // Save original fallback candidates.
  $language_fallback_original = drupal_static('language_fallback_get_candidates');
  // Replace all global languages with the given one. We need this because the
  // language_fallback_get_candidates() does not take the $language parameter,
  // however other modules (like language_fallback) may use current language(s)
  // when they alter the language fallback candidates.
  $languages_original = array();
  foreach (language_types() as $language_type) {
    $languages_original[$language_type] = $GLOBALS[$language_type];
    $GLOBALS[$language_type] = $language;
  }
  // Clear static cache, so that fallback candidates are recalculated.
  drupal_static_reset('language_fallback_get_candidates');
  $language_fallback_candidates = language_fallback_get_candidates();
  // Restore original data.
  $language_fallback =& drupal_static('language_fallback_get_candidates');
  $language_fallback = $language_fallback_original;
  foreach ($languages_original as $language_type => $_language) {
    $GLOBALS[$language_type] = $_language;
  }
  return $language_fallback_candidates;
}
