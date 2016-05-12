<?php

/**
 * @file
 * Taxonomy term translation handler for the entity translation module.
 */


/**
 * Taxonomy term translation handler.
 */
class EntityTranslationTaxonomyTermHandler extends EntityTranslationDefaultHandler {

  public function __construct($entity_type, $entity_info, $entity) {
    parent::__construct('taxonomy_term', $entity_info, $entity);
  }

  /**
   * @see EntityTranslationDefaultHandler::getLanguage()
   */
  public function getLanguage() {
    if (isset($this->entity->vid) && module_exists('i18n_taxonomy')) {
      $mode = i18n_taxonomy_vocabulary_mode($this->entity->vid);
      if ($mode == I18N_MODE_NONE) {
        $translations = $this->getTranslations();
        if (!empty($translations->original)) {
          return $translations->original;
        }
      }
    }
    return parent::getLanguage();
  }

  /**
   * @see EntityTranslationDefaultHandler::entityForm()
   */
  public function entityForm(&$form, &$form_state) {
    parent::entityForm($form, $form_state);

    // Remove the translation fieldset when the deletion confirm form is being
    // displayed.
    if (isset($form_state['confirm_delete'])) {
      unset(
        $form[$this->getLanguageKey()],
        $form['source_language'],
        $form['translation'],
        $form['actions']['delete_translation']
      );
    }
  }
}
