<?php

/**
 * @file
 * The role dependency class.
 */

/**
 * Dependency plugin adding the user role conditions.
 *
 * The role dependency plugin adds conditions based on the current user's role.
 * Although the backend should handle access control, this plugin can help
 * personalize the search interface based on the user's role by exposing facets
 * to certain users while hiding them from others.
 */
class FacetapiDependencyRole extends FacetapiDependency {

  /**
   * Implements FacetapiDependency::execute().
   */
  public function execute() {
    global $user;
    if (1 != $user->uid) {
      $roles = array_filter($this->settings['roles']);
      if ($roles && !array_intersect_key($user->roles, $roles)) {
        return FALSE;
      }
    }
  }

  /**
   * Overrides FacetapiDependency::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {
    $form[$this->id]['roles'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Show facet for specific roles'),
      '#default_value' => $this->settings['roles'],
      '#options' => array_map('check_plain', user_roles()),
      '#description' => t('Show this facet only for the selected role(s). If you select no roles, the facet will be visible to all users.'),
    );
  }

  /**
   * Overrides FacetapiDependency::getDefaultSettings().
   */
  public function getDefaultSettings() {
    return array(
      'roles' => array(),
    );
  }
}
