<?php
/**
 * @file
 * Class defining MandrillActivityUIController.
 */

/**
 * Override EntityDefaultUIController to customize our menu items.
 */
class MandrillActivityUIController extends EntityDefaultUIController {

  /**
   * Override hook_menu() to add a proper description.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage Mandrill Activity entity settings.';
    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    return $items;
  }

}
