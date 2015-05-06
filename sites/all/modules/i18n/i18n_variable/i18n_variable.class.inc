<?php
/**
 * @file
 * Variable Realm controller.
 */

/**
 * Controller for Language realms.
 */
class I18nVariableLanguageRealm extends VariableRealmDefaultController {
  /**
   * Implementation of VariableRealmControllerInterface::getAvailableVariables().
   */
  public function getAvailableVariables() {
    $translatable = array();
    $conf = variable_get('i18n_variables', array());
    foreach (variable_get_info() as $name => $variable) {
      if (!empty($variable['localize']) || in_array($name, $conf)) {
        $translatable[] = $name;
      }
    }
    return $translatable;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getDefaultKey().
   */
  public function getDefaultKey() {
    // The default key will match the default language.
    return language_default('language');
  }
  /**
   * Implementation of VariableRealmControllerInterface::getRequestKey().
   */
  public function getRequestKey() {
    return i18n_variable_language()->language;
  }
  /**
   * Implementation of VariableRealmControllerInterface::getAllKeys().
   */
  public function getAllKeys() {
    return locale_language_list('name', TRUE);
  }
}
