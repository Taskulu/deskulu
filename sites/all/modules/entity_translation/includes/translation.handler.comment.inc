<?php

/**
 * @file
 * Comment translation handler for the entity translation module.
 */


/**
 * Comment translation handler.
 */
class EntityTranslationCommentHandler extends EntityTranslationDefaultHandler {

  public function __construct($entity_type, $entity_info, $entity) {
    parent::__construct('comment', $entity_info, $entity);
  }

  /**
   * @see EntityTranslationDefaultHandler::entityForm()
   */
  public function entityForm(&$form, &$form_state) {
    parent::entityForm($form, $form_state);
    // Adjust the translation fieldset weight to move it below the admin one.
    $form['translation']['#weight'] = 1;
  }

  /**
   * @see EntityTranslationDefaultHandler::entityFormLanguageWidgetSubmit()
   */
  public function entityFormLanguageWidgetSubmit($form, &$form_state) {
    $this->updateFormLanguage($form_state);
  }

  /**
   * @see EntityTranslationDefaultHandler::entityFormTitle()
   */
  protected function entityFormTitle() {
    return t('Edit comment @subject', array('@subject' => $this->getLabel()));
  }

  /**
   * @see EntityTranslationDefaultHandler::getStatus()
   */
  protected function getStatus() {
    return (boolean) $this->entity->status;
  }
}
