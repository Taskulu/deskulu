<?php

/**
 * @file
 * Helper functions for taxonomy administration.
 */

/**
 * This is the callback for taxonomy translations.
 *
 * Gets the urls:
 *  admin/content/taxonomy/%taxonomy_vocabulary/translation
 *  admin/content/taxonomy/i18n/term/xx
 *  admin/content/taxonomy/i18n/term/new/xx
 *  admin/content/taxonomy/vid/translation/op/trid
 */
function i18n_taxonomy_page_vocabulary($vocabulary, $op = NULL, $tid = NULL) {
  switch ($op) {
    case 'edit':
      drupal_set_title(t('Edit term translations'));
      $output = drupal_get_form('i18n_taxonomy_translation_term_form', $vocabulary, $tid);
      break;

    default:
      $output = i18n_taxonomy_translation_overview($vocabulary);
  }
  return $output;
}

/**
 * Produces a vocabulary translation form.
 */
function i18n_taxonomy_translation_term_form($form, $form_state, $vocabulary, $translation_set = NULL, $source = NULL) {
  $form['translation_set'] = array('#type' => 'value', '#value' => $translation_set);
  $translations = $translation_set ? $translation_set->get_translations() : array();
  $form['vocabulary'] = array('#type' => 'value', '#value' => $vocabulary);
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select translations'),
    '#description' => t('Select existing terms or type new ones that will be created for each language.'),
    '#tree' => TRUE
  );
  // List of terms for languages.
  foreach (i18n_language_list() as $lang => $langname) {
    if ($source && $source->language == $lang) {
      // This is the source term, we don't change it
      $form['source_term'] = array('#type' => 'value', '#value' => $source);
      $form['translations'][$lang] = array(
        '#title' => $langname,
        '#type' => 'item',
        '#markup' => $source->name,
        '#langcode' => $lang,
      );
    }
    else {
      $form['translations'][$lang] = array(
        '#title' => $langname,
        '#type' => 'textfield',
        '#default_value' => isset($translations[$lang]) ? $translations[$lang]->name : '',
        '#autocomplete_path' => 'i18n/taxonomy/autocomplete/vocabulary/' . $vocabulary->machine_name . '/' . $lang,
        '#langcode' => $lang,
        '#maxlength' => 1024,
        '#element_validate' => array('i18n_taxonomy_autocomplete_validate'),
      );
    }

  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  if ($translation_set) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete')
    );
  }
  return $form;
}

/**
 * Form element validate handler for taxonomy term autocomplete element.
 */
function i18n_taxonomy_autocomplete_validate($element, &$form_state) {
  // Autocomplete widgets do not send their tids in the form, so we must detect
  // them here and process them independently.
  $value = array();
  if ($tags = $element['#value']) {
    // Collect candidate vocabularies.
    $vocabulary = $form_state['values']['vocabulary'];
    $vocabularies[$vocabulary->vid] = $vocabulary;

    // Translate term names into actual terms.
    $typed_terms = drupal_explode_tags($tags);
    foreach ($typed_terms as $typed_term) {
      // See if the term exists in the chosen vocabulary and return the tid;
      // otherwise, create a new 'autocreate' term for insert/update.
      if ($possibilities = taxonomy_term_load_multiple(array(), array('name' => trim($typed_term), 'vid' => $vocabulary->vid, 'language' => $element['#langcode']))) {
        $term = array_pop($possibilities);
      }
      else {
        $vocabulary = reset($vocabularies);
        $term = array(
          'tid' => 'autocreate',
          'vid' => $vocabulary->vid,
          'name' => $typed_term,
          'vocabulary_machine_name' => $vocabulary->machine_name,
          'language' => $element['#langcode'],
        );
      }
      $value[] = (array)$term;
    }
  }

  form_set_value($element, $value, $form_state);
}

/**
 * Form callback: Process vocabulary translation form.
 */
function i18n_taxonomy_translation_term_form_submit($form, &$form_state) {
  $translation_set = $form_state['values']['translation_set'];
  $vocabulary = $form_state['values']['vocabulary'];
  switch ($form_state['values']['op']) {
    case t('Save'):
      $term_translations = array_filter($form_state['values']['translations']);
      foreach ($term_translations as $lang => $lang_terms) {
        $item = reset($lang_terms);
        if ($item['tid'] == 'autocreate') {
          $term = (object) $item;
          unset($term->tid);
          taxonomy_term_save($term);
        }
        else {
          $term = (object) $item;
        }
        $translations[$lang] = $term;
      }
      if (!empty($form_state['values']['source_term'])) {
        $term = $form_state['values']['source_term'];
        $translations[$term->language] = $term;
      }
      if (!empty($translations)) {
        $translation_set = $translation_set ? $translation_set : i18n_translation_set_create('taxonomy_term', $vocabulary->machine_name);
        $translation_set
          ->reset_translations($translations)
          ->save(TRUE);
        drupal_set_message(t('Term translations have been updated.'));
      }
      else {
        drupal_set_message(t('There are no translations to be saved.'), 'error');
      }
      break;

    case t('Delete'):
      $translation_set->delete(TRUE);
      drupal_set_message(t('The term translation has been deleted.'));
      $form_state['redirect'] = 'admin/structure/taxonomy/' . $form_state['values']['vocabulary']->machine_name . '/translation';
      break;
  }
}

/**
 * Generate a tabular listing of translations for vocabularies.
 */
function i18n_taxonomy_translation_sets_overview($vocabulary) {
  module_load_include('admin.inc', 'i18n_translation');
  drupal_set_title($vocabulary->name);
  $query = db_select('i18n_translation_set', 't')
    ->condition('t.bundle', $vocabulary->machine_name);
  return i18n_translation_admin_overview('taxonomy_term', $query);
}

/**
 * Callback for term translation tab.
 */
function i18n_taxonomy_translation_term_overview($term) {
  if ($term->i18n_tsid) {
    // Already part of a set, grab that set.
    $i18n_tsid = $term->i18n_tsid;
    $translation_set = i18n_translation_set_load($term->i18n_tsid);
    $translation_set->get_translations();
    $translations = $translation_set->get_translations();
  }
  else {
    // We have no translation source nid, this could be a new set, emulate that.
    $i18n_tsid = $term->tid;
    $translations = array($term->language => $term);
  }
  $type = variable_get('translation_language_type', LANGUAGE_TYPE_INTERFACE);
  $header = array(t('Language'), t('Title'), t('Operations'));

  foreach (i18n_language_list() as $langcode => $language_name) {
    $options = array();
    if (isset($translations[$langcode])) {
      // Existing translation in the translation set: display status.
      // We load the full node to check whether the user can edit it.
      $translation_term = taxonomy_term_load($translations[$langcode]->tid);
      $path = 'taxonomy/term/' . $translation_term->tid;
      $title = l($translation_term->name, $path);

      $options['edit'] = array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => $path . '/edit',
        '#options' => array(
          'query' => drupal_get_destination(),
        ),
      );

      if ($translation_term->tid == $i18n_tsid) {
        $language_name = t('<strong>@language_name</strong> (source)', array('@language_name' => $language_name));
      }
    }
    else {
      // No such translation in the set yet: help user to create it.
      $title = t('n/a');
      $options['add'] = array(
        '#type' => 'link',
        '#title' => t('add translation'),
        '#href' => 'admin/structure/taxonomy/' . $term->vocabulary_machine_name . '/add',
        '#options' => array(
          'query' => array('translation' => $term->tid, 'target' => $langcode) + drupal_get_destination()
        ),
      );
    }
    $rows[$langcode] = array(
      'language' => $language_name,
      'title' => $title,
      'operations' => array('data' => $options),
    );
  }

  drupal_set_title(t('Translations of term %title', array('%title' => $term->name)), PASS_THROUGH);

  $build['translation_overview'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}
