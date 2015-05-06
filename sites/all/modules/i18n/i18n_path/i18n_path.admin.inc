<?php
/**
 * @file
 * Administration pages for path translation.
 */

/**
 * Path overview page
 */
function i18n_path_admin_overview($type = NULL) {
  module_load_include('admin.inc', 'i18n_translation');
  return i18n_translation_admin_overview('path');
}

/**
 * Path add/edit form
 */
function i18n_path_admin_form($form, $form_state, $translation_set = NULL) {
  $form['translation_set'] = array('#type' => 'value', '#value' => $translation_set);
  if ($translation_set) {
    $paths = $translation_set->get_translations();
  }
  else {
    $paths = array();
  }
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $translation_set ? $translation_set->title : '',
    '#description' => t('Optional descriptive name for this set.'),
  );
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translations'),
    '#tree' => TRUE,
    '#description' => t('The path for this menu link. This can be an internal Drupal path such as %add-node or an external URL such as %drupal. Enter %front to link to the front page.', array('%front' => '<front>', '%add-node' => 'node/add', '%drupal' => 'http://drupal.org')),
  );
  foreach (i18n_language_list() as $langcode => $name) {
    $form['translations'][$langcode] = array(
      '#type' => 'textfield',
      '#title' => check_plain($name),
      '#default_value' => !empty($paths[$langcode]) ? $paths[$langcode]->path : '',
    );
  }

  $form['controls']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#name' => 'save',
  );

  if ($translation_set) {
    $form['controls']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#name' => 'delete',
    );
  }
  return $form;
}

/**
 * Process form validation
 */
function i18n_path_admin_form_validate($form, &$form_state)  {
  if ($form_state['triggering_element']['#name'] == 'save') {
    $paths = &$form_state['values']['translations'];
    if ($paths = array_filter($paths)) {
      module_load_include('inc', 'menu', 'menu.admin');
      foreach ($paths as $language => &$link_path) {
        $link_path = i18n_prepare_normal_path($link_path, $language);
        $validation_form_state = array(
          'values' => array(
            'link_path' => $link_path,
          ),
        );
        menu_edit_item_validate(array(), $validation_form_state);
      }
    }
    else {
      form_set_error('paths', t('There are no path translations to save.'));
    }
  }
}

/**
 * Process form submission
 */
function i18n_path_admin_form_submit($form, &$form_state) {
  $translation_set = $form_state['values']['translation_set'];
  switch ($form_state['triggering_element']['#name']) {
    case 'save':
      $paths = array_filter($form_state['values']['translations']);
      $translation_set = $translation_set ? $translation_set : i18n_translation_set_create('path');
      $translation_set->title = '';
      $translations = $translation_set->get_translations();
      foreach ($paths as $lang => $path) {
        if (isset($translations[$lang])) {
          $translations[$lang]->path = $path;
        }
        else {
          $translation_set->add_item($path, $lang);
        }
      }

      foreach (array_diff(array_keys($translation_set->get_translations()), array_keys($paths)) as $language) {
        $translation_set->remove_language($language);
        unset($translations[$language]);
      }

      if (!empty($form_state['values']['title'])) {
        $translation_set->title = $form_state['values']['title'];
      }

      $translation_set->save(TRUE);
      drupal_set_message(t('The path translation has been saved.'));
      break;
    case 'delete':
      $destination = array();
      if (isset($_GET['destination'])) {
        $destination = drupal_get_destination();
        unset($_GET['destination']);
      }
      $form_state['redirect'] = array($translation_set->get_delete_path(), array('query' => $destination));
      return;
  }
  $form_state['redirect'] = 'admin/config/regional/i18n_translation/path';
}

/**
 * Save path translation set.
 */
function i18n_path_save_translations($paths, $tpid = NULL) {
  $paths = array_filter($paths);
  if (lock_acquire('i18n_path')) {
    if ($tpid) {
      db_delete('i18n_path')->condition('tpid', $tpid)->execute();
    }
    else {
      $tpid = 1 + (int)db_query('SELECT MAX(tpid) FROM {i18n_path}')->fetchField();
    }
    foreach ($paths as $langcode => $path) {
      db_insert('i18n_path')
        ->fields(array('tpid' => $tpid, 'language' => $langcode, 'path' => $path))
        ->execute();
    }
    lock_release('i18n_path');
    return $tpid;
  }
}
