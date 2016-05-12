<?php

/**
 * @file
 * Contains SearchApiAlterLanguageControl.
 */

/**
 * Search API data alteration callback that filters out items based on their
 * bundle.
 */
class SearchApiAlterLanguageControl extends SearchApiAbstractAlterCallback {

  /**
   * {@inheritdoc}
   */
  public function __construct(SearchApiIndex $index, array $options = array()) {
    $options += array(
      'lang_field' => '',
      'languages' => array(),
    );
    parent::__construct($index, $options);
  }

  /**
   * Overrides SearchApiAbstractAlterCallback::supportsIndex().
   *
   * Only returns TRUE if the system is multilingual.
   *
   * @see drupal_multilingual()
   */
  public function supportsIndex(SearchApiIndex $index) {
    return drupal_multilingual();
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm() {
    $form = array();

    $wrapper = $this->index->entityWrapper();
    $fields[''] = t('- Use default -');
    foreach ($wrapper as $key => $property) {
      if ($key == 'search_api_language') {
        continue;
      }
      $type = $property->type();
      // Only single-valued string properties make sense here. Also, nested
      // properties probably don't make sense.
      if ($type == 'text' || $type == 'token') {
        $info = $property->info();
        $fields[$key] = $info['label'];
      }
    }

    if (count($fields) > 1) {
      $form['lang_field'] = array(
        '#type' => 'select',
        '#title' => t('Language field'),
        '#description' => t("Select the field which should be used to determine an item's language."),
        '#options' => $fields,
        '#default_value' => $this->options['lang_field'],
      );
    }

    $languages[LANGUAGE_NONE] = t('Language neutral');
    $list = language_list('enabled') + array(array(), array());
    foreach (array($list[1], $list[0]) as $list) {
      foreach ($list as $lang) {
        $name = t($lang->name);
        $native = $lang->native;
        $languages[$lang->language] = check_plain(($name == $native) ? $name : "$name ($native)");
        if (!$lang->enabled) {
          $languages[$lang->language] .= ' [' . t('disabled') . ']';
        }
      }
    }
    $form['languages'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Indexed languages'),
      '#description' => t('Index only items in the selected languages. ' .
          'When no language is selected, there will be no language-related restrictions.'),
      '#options' => $languages,
      '#default_value' => $this->options['languages'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $values['languages'] = array_filter($values['languages']);
    return parent::configurationFormSubmit($form, $values, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function alterItems(array &$items) {
    foreach ($items as $i => &$item) {
      // Set item language, if a custom field was selected.
      if ($field = $this->options['lang_field']) {
        $wrapper = $this->index->entityWrapper($item);
        if (isset($wrapper->$field)) {
          try {
            $item->search_api_language = $wrapper->$field->value();
          }
          catch (EntityMetadataWrapperException $e) {
            // Something went wrong while accessing the language field. Probably
            // doesn't really matter.
          }
        }
      }
      // Filter out items according to language, if any were selected.
      if ($languages = $this->options['languages']) {
        if (empty($languages[$item->search_api_language])) {
          unset($items[$i]);
        }
      }
    }
  }

}
