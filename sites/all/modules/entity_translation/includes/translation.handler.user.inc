<?php

/**
 * @file
 * User translation handler for the entity translation module.
 */


/**
 * User translation handler.
 */
class EntityTranslationUserHandler extends EntityTranslationDefaultHandler {

  public function __construct($entity_type, $entity_info, $entity) {
    parent::__construct('user', $entity_info, $entity);
  }

  /**
   * @see EntityTranslationDefaultHandler::entityForm()
   */
  public function entityForm(&$form, &$form_state) {
    parent::entityForm($form, $form_state);
    $form['picture']['#multilingual'] = FALSE;
    $form['locale']['#multilingual'] = FALSE;
    $form['locale']['#title'] = t('Preferred language settings');
  }

  /**
   * @see EntityTranslationDefaultHandler::getLanguageKey()
   */
  public function getLanguageKey() {
    return 'entity_language';
  }
}
