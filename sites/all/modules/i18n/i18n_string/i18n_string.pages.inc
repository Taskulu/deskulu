<?php

/**
 * @file
 * Internationalization (i18n) package - translatable strings reusable admin UI.
 *
 * @author Jose A. Reyero, 2007
 */

// Load locale libraries
include_once DRUPAL_ROOT . '/includes/locale.inc';
include_once drupal_get_path('module', 'locale') . '/locale.admin.inc';

/**
 * Generate translate page from object.
 *
 * @param string $object_type
 *   Obejct type as declared in hook_i18n_object_info().
 * @param object $object_value
 *   Drupal object to translate.
 * @param object $language
 *   Optional language object.
 */
function i18n_string_translate_page_object($object_type, $object_value, $language = NULL) {
  // For backwards compatibility, ensure parameter is a language object
  $language = i18n_language_object($language);
  $langcode = $language ? $language->language : NULL;
  // Get base keys for all these strings. Object key may be multiple like for blocks (module, delta)
  $object = i18n_object($object_type, $object_value);
  $strings = $object->get_strings(array('empty' => TRUE));

  // If no localizable strings, print message and fail gracefully.
  // Possibly this object comes from some other contrib module.
  // See http://drupal.org/node/1889878
  if (!$strings) {
    return t('This object has no strings available for translation.');
  }

  if (empty($langcode)) {
    drupal_set_title(t('Translate !name', array('!name' => i18n_object_info($object_type, 'title'))));
    return i18n_string_translate_page_overview($object, $strings);
  }
  else {
    drupal_set_title(t('Translate to !language', array('!language' => i18n_language_name($langcode))));
    return drupal_get_form('i18n_string_translate_page_form', $strings, $langcode);
  }
}

/**
 * Provide a core translation module like overview page for this object.
 */
function i18n_string_translate_page_overview($object, $strings) {
  $build['i18n_overview'] = drupal_get_form('i18n_string_translate_page_overview_form', $object, $strings);
  return $build;
}

/**
 * Provide a core translation module like overview page for this object.
 */
function i18n_string_translate_page_overview_form($form, &$form_state, $object, $strings) {
  // Set the default item key, assume it's the first.
  $item_title = reset($strings);
  $header = array(
    'language' => t('Language'),
    'title' => t('Title'),
    'status' => t('Status'),
    'operations' => t('Operations')
  );
  $source_language = variable_get_value('i18n_string_source_language');
  $rows = array();

  foreach (language_list() as $langcode => $language) {
    if ($langcode == $source_language) {
      $items = array(
        'language' => check_plain($language->name) . ' ' . t('(source)'),
        'title' => check_plain($item_title->get_string()),
        'status' => t('original'),
        'operations' => l(t('edit'), $object->get_edit_path()),
      );
    }
    else {
      // Try to figure out if this item has any of its properties translated.
      $translated = FALSE;
      foreach ($strings as $i18nstring) {
        if ($i18nstring->get_translation($langcode)) {
          $translated = TRUE;
          break;
        }
      }
      // Translate the item that was requested to be displayed as title.
      $items = array(
        'language' => check_plain($language->name),
        'title' => $item_title->format_translation($langcode, array('sanitize default' => TRUE)),
        'status' => $translated ? t('translated') : t('not translated'),
        'operations' => l(t('translate'), $object->get_translate_path($langcode), array('query' => drupal_get_destination())),
      );
    }
    foreach ($items as $key => $markup) {
      $rows[$langcode][$key] = $markup;
      //$form['#rows'][$langcode][$key]['#markup'] = $markup;
    }
  }
  // Build a form so it can be altered later, with all this information.
  $form['object'] = array('#type' => 'value', '#value' => $object);
  $form['source_language'] = array('#type' => 'value', '#value' => $source_language);
  $form['languages'] = array(
    '#header' => $header,
    '#rows' => $rows,
    '#theme' => 'table',
  );
  return $form;
}

/**
 * Form builder callback for in-place string translation.
 *
 * @param $strings
 *   Array of strings indexed by string name (may be indexed by group key too if $groups is present)
 * @param $langcode
 *   Language code to translate to.
 * @param $groups
 *   Optional groups to provide some string grouping. Array with group key and title pairs.
 */
function i18n_string_translate_page_form($form, &$form_state, $strings, $langcode, $groups = NULL) {
  $form = i18n_string_translate_page_form_base($form, $langcode);
  if ($groups) {
    // I we've got groups, string list is grouped by group key.
    $form['string_groups'] = array('#type' => 'value', '#value' => $strings);
    foreach ($groups as $key => $title) {
      $form['display'] = array(
        '#type' => 'vertical_tabs',
      );
      $form['strings'][$key] = array(
        '#group' => 'display',
        '#title' => $title,
        '#type' => 'fieldset',
      ) + i18n_string_translate_page_form_strings($strings[$key], $langcode);
    }
  }
  else {
    // We add a single group with key 'all', but no tabs.
    $form['string_groups'] = array('#type' => 'value', '#value' => array('all' => $strings));
    $form['strings']['all'] = i18n_string_translate_page_form_strings($strings, $langcode);
  }
  return $form;
}

/**
 * Create base form for string translation
 */
function i18n_string_translate_page_form_base($form, $langcode, $redirect = NULL) {
  $form['langcode'] = array(
    '#type' => 'value',
    '#value' => $langcode,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save translation'),
    '#weight' => 10,
  );
  if ($redirect) {
    $form['#redirect'] = array(
      $redirect,
    );
  }
  // Add explicit validate and submit hooks so this can be used from inside any form.
  $form['#submit'] = array('i18n_string_translate_page_form_submit');
  return $form;
}

/**
 * Create field elements for strings
 */
function i18n_string_translate_page_form_strings($strings, $langcode) {
  $formats = filter_formats();
  foreach ($strings as $item) {
    // We may have a source or not. Load it, our string may get the format from it.
    $source = $item->get_source();
    $format_id = $source ? $source->format : $item->format;
    $description = '';
    // Check permissions to translate this string, depends on format, etc..
    if ($message = $item->check_translate_access()) {
      // We'll display a disabled element with the reason it cannot be translated.
      $disabled = TRUE;
      $description = $message;
    }
    else {
      $disabled = FALSE;
      $description = '';
      // If we don't have a source and it can be translated, we create it.
      if (!$source) {
        // Enable messages just as a reminder these strings are not being updated properly.
        $status = $item->update(array('messages' => TRUE));
        if ($status === FALSE || $status === SAVED_DELETED) {
          // We don't have a source string so nothing to translate here
          $disabled = TRUE;
        }
        else {
          $source = $item->get_source();
        }
      }
    }

    $default_value = $item->format_translation($langcode, array('langcode' => $langcode, 'sanitize' => FALSE, 'debug' => FALSE));
    $form[$item->get_name()] = array(
      '#title' => $item->get_title(),
      '#type' => 'textarea',
      '#default_value' => $default_value,
      '#disabled' => $disabled,
      '#description' => $description . _i18n_string_translate_format_help($format_id),
      //'#i18n_string_format' => $source ? $source->format : 0,
      // If disabled, provide smaller textarea (that can be expanded anyway).
      '#rows' => $disabled ? 1 : min(ceil(str_word_count($default_value) / 12), 10),
      // Change the parent for disabled strings so we don't get empty values later
      '#parents' => array($disabled ? 'disabled_strings': 'strings', $item->get_name()),
    );
  }
  return $form;
}

/**
 * Form submission callback for in-place string translation.
 */
function i18n_string_translate_page_form_submit($form, &$form_state) {
  $count = $success = 0;
  foreach ($form_state['values']['strings'] as $name => $value) {
    $count++;
    list($textgroup, $context) = i18n_string_context(explode(':', $name));
    $result = i18n_string_textgroup($textgroup)->update_translation($context, $form_state['values']['langcode'], $value);
    $success += ($result ? 1 : 0);
  }
  if ($success) {
    drupal_set_message(format_plural($success, 'A translation was saved successfully.', '@count translations were saved successfully.'));
  }
  if ($error = $count - $success) {
    drupal_set_message(format_plural($error, 'A translation could not be saved.', '@count translations could not be saved.'), 'warning');
  }
  if (isset($form['#redirect'])) {
    $form_state['redirect'] = $form['#redirect'];
  }
}

/**
 * Menu callback. Saves a string translation coming as POST data.
 */
function i18n_string_l10n_client_save_string() {
  global $user, $language;

  if (user_access('use on-page translation')) {
    $textgroup = !empty($_POST['textgroup']) ? $_POST['textgroup'] : 'default';
    // Other textgroups will be handled by l10n_client module
    if (!i18n_string_group_info($textgroup)) {
      return l10n_client_save_string();
    }
    elseif (isset($_POST['source']) && isset($_POST['target']) && !empty($_POST['context']) && !empty($_POST['form_token']) && drupal_valid_token($_POST['form_token'], 'l10n_client_form')) {
      $name = $textgroup . ':' . $_POST['context'];
      if ($i18nstring = i18n_string_get_source($name)) {
        // Since this is not a real form, we double check access permissions here too.
        if ($error = $i18nstring->check_translate_access()) {
          $message = theme('l10n_client_message', array('message' => t('Not saved due to: !reason', array('!reason' => $error)), 'level' => WATCHDOG_WARNING));
        }
        else {
          $result = i18n_string_translation_update($name, $_POST['target'], $language->language, $_POST['source']);
          if ($result) {
            $message = theme('l10n_client_message', array('message' => t('Translation saved locally for user defined string.'), 'level' => WATCHDOG_INFO));
          }
          elseif ($result === FALSE) {
            $message = theme('l10n_client_message', array('message' => t('Not saved due to insufficient permissions.')));
          }
          else {
            $message = theme('l10n_client_message', array('message' => t('Not saved due to unknown reason.')));
          }
        }
      }
      else {
        $message = theme('l10n_client_message', array('message' => t('Not saved due to source string missing.')));
      }
    }
    else {
      $message = theme('l10n_client_message', array('message' => t('Not saved due to missing form values.')));
    }
    drupal_json_output($message);
    exit;
  }
}


/**
 * User interface for string editing.
 */
function i18n_string_locale_translate_edit_form($form, &$form_state, $lid) {
  // Fetch source string, if possible.
  $source = db_query('SELECT source, context, textgroup, location FROM {locales_source} WHERE lid = :lid', array(':lid' => $lid))->fetchObject();
  if (!$source) {
    drupal_set_message(t('String not found.'), 'error');
    drupal_goto('admin/config/regional/translate/translate');
  }

  // Add original text to the top and some values for form altering.
  $form['original'] = array(
    '#type'  => 'item',
    '#title' => t('Original text'),
    '#markup' => check_plain(wordwrap($source->source, 0)),
  );
  if (!empty($source->context)) {
    $form['context'] = array(
      '#type' => 'item',
      '#title' => t('Context'),
      '#markup' => check_plain($source->context),
    );
  }
  $form['lid'] = array(
    '#type'  => 'value',
    '#value' => $lid
  );
  $form['textgroup'] = array(
    '#type'  => 'value',
    '#value' => $source->textgroup,
  );
  $form['location'] = array(
    '#type'  => 'value',
    '#value' => $source->location
  );

  // Include default form controls with empty values for all languages.
  // This ensures that the languages are always in the same order in forms.
  $languages = language_list();

  // We don't need the default language value, that value is in $source.
  $omit = $source->textgroup == 'default' ? 'en' : i18n_string_source_language();
  unset($languages[($omit)]);
  $form['translations'] = array('#tree' => TRUE);
  // Approximate the number of rows to use in the default textarea.
  $rows = min(ceil(str_word_count($source->source) / 12), 10);
  foreach ($languages as $langcode => $language) {
    $form['translations'][$langcode] = array(
      '#type' => 'textarea',
      '#title' => t($language->name),
      '#rows' => $rows,
      '#default_value' => '',
    );
  }

  // Fetch translations and fill in default values in the form.
  $result = db_query("SELECT DISTINCT translation, language FROM {locales_target} WHERE lid = :lid AND language <> :omit", array(':lid' => $lid, ':omit' => $omit));
  foreach ($result as $translation) {
    $form['translations'][$translation->language]['#default_value'] = $translation->translation;
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save translations'));

  // Restrict filter permissions and handle validation and submission for i18n strings.
  if (i18n_string_group_info($source->textgroup)) {
    if ($i18nstring = i18n_string_get_by_lid($form['lid']['#value'])) {
      $form['i18n_string'] = array('#type' => 'value', '#value' => $i18nstring);
      if ($message = $i18nstring->check_translate_access()) {
        drupal_set_message($message);
        $disabled = TRUE;
      }
      // Add format help anyway, though the form may be disabled.
      $form['translations']['format_help']['#markup'] = _i18n_string_translate_format_help($i18nstring->format);
    }
    else {
      drupal_set_message(t('Source string not found.'), 'warning');
      $disabled = TRUE;
    }
    if (!empty($disabled)) {
      // Disable all form elements
      $form['submit']['#disabled'] = TRUE;
      foreach (element_children($form['translations']) as $langcode) {
        $form['translations'][$langcode]['#disabled'] = TRUE;
      }
    }
  }
  return $form;
}

/**
 * Process string editing form validations.
 *
 * If it is an allowed format, skip default validation, the text will be filtered later
 */
function i18n_string_locale_translate_edit_form_validate($form, &$form_state) {
  if (empty($form_state['values']['i18n_string'])) {
    // If not i18n string use regular locale validation.
    $copy_state = $form_state;
    locale_translate_edit_form_validate($form, $copy_state);
  }
}

/**
 * Process string editing form submissions.
 *
 * Mark translations as current.
 */
function i18n_string_locale_translate_edit_form_submit($form, &$form_state) {
  // Invoke locale submission.
  locale_translate_edit_form_submit($form, $form_state);
  $lid = $form_state['values']['lid'];
  if ($i18n_string_object = i18n_string_get_by_lid($lid)) {
    $i18n_string_object->cache_reset();
  }
  foreach ($form_state['values']['translations'] as $key => $value) {
    if (!empty($value)) {
      // An update has been made, so we assume the translation is now current.
      db_update('locales_target')
        ->fields(array('i18n_status' => I18N_STRING_STATUS_CURRENT))
        ->condition('lid', $lid)
        ->condition('language', $key)
        ->execute();
    }
  }
}

/**
 * Help for text format.
 */
function _i18n_string_translate_format_help($format_id) {
  $output = '';
  if ($format = filter_format_load($format_id)) {
    $title = t('Text format: @name', array('@name' => $format->name));
    $tips =  theme('filter_tips', array('tips' => _filter_tips($format_id, FALSE)));
  }
  elseif ($format_id == I18N_STRING_FILTER_XSS) {
    $title = t('Standard XSS filter.');
    $allowed_html = '<a> <em> <strong> <cite> <blockquote> <code> <ul> <ol> <li> <dl> <dt> <dd>';
    $tips[] = t('Allowed HTML tags: @tags', array('@tags' => $allowed_html));
  }
  elseif ($format_id == I18N_STRING_FILTER_XSS_ADMIN) {
    $title = t('Administration XSS filter.');
    $tips[] = t('It will allow most HTML tags but not scripts nor styles.');
  }
  elseif ($format_id) {
    $title = t('Unknown filter: @name', array('@name' => $format_id));
  }

  if (!empty($title)) {
    $output .= '<h5>' . $title . '</h5>';
  }
  if (!empty($tips)) {
    $output .= is_array($tips) ? theme('item_list', array('items' => $tips)) : $tips;
  }
  return $output;
}
