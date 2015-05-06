<?php

/**
 * @file
 * Contains the SearchApiAlterRoleFilter class.
 */

/**
 * Data alteration that filters out users based on their role.
 */
class SearchApiAlterRoleFilter extends SearchApiAbstractAlterCallback {

  /**
   * Overrides SearchApiAbstractAlterCallback::supportsIndex().
   *
   * This plugin only supports indexes containing users.
   */
  public function supportsIndex(SearchApiIndex $index) {
    return $index->getEntityType() == 'user';
  }

  /**
   * Implements SearchApiAlterCallbackInterface::alterItems().
   */
  public function alterItems(array &$items) {
    $roles = $this->options['roles'];
    $default = (bool) $this->options['default'];
    foreach ($items as $id => $account) {
      $role_match = (count(array_diff_key($account->roles, $roles)) !== count($account->roles));
      if ($role_match === $default) {
        unset($items[$id]);
      }
    }
  }

  /**
   * Overrides SearchApiAbstractAlterCallback::configurationForm().
   *
   * Add option for the roles to include/exclude.
   */
  public function configurationForm() {
    $options = array_map('check_plain', user_roles());
    $form = array(
      'default' => array(
        '#type' => 'radios',
        '#title' => t('Which users should be indexed?'),
        '#default_value' => isset($this->options['default']) ? $this->options['default'] : 1,
        '#options' => array(
          1 => t('All but those from one of the selected roles'),
          0 => t('Only those from the selected roles'),
        ),
      ),
      'roles' => array(
        '#type' => 'select',
        '#title' => t('Roles'),
        '#default_value' => isset($this->options['roles']) ? $this->options['roles'] : array(),
        '#options' => $options,
        '#size' => min(4, count($options)),
        '#multiple' => TRUE,
      ),
    );
    return $form;
  }

}
