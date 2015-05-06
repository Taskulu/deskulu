<?php
/**
 * @file
 * The UI controller for the template map entity.
 */

/**
 * Override EntityDefaultUIController to customize our menu items.
 */
class MandrillTemplateMapUIController extends EntityDefaultUIController {

  /**
   * Overrides EntityDefaultUIController::hook_menu().
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage Mandrill Template Mapping entity settings.';
    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    return $items;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableHeaders().
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header[] = t('Mandrill Template');
    $additional_header[] = t('Primary Content Zone');
    $additional_header[] = t('In Use By');
    $header = parent::overviewTableHeaders($conditions, $rows, $additional_header);
    return $header;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableRow().
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $additional_cols[] = $entity->template_id;
    $additional_cols[] = $entity->main_section;
    $additional_cols[] = $entity->mailsystem_key;
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }
}
