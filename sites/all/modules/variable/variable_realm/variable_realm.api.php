<?php
/**
 * @file
 * Documents hooks provided by Variable Realm API.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Provides information about controller classes and weights needed by the realm system
 * and some more meta-information for realm administration.
 *
 * This information is mainly used to build exportable realms as Features.
 *
 * @see i18n_variable_variable_realm_info()
 *
 * @return array
 *   Array keyed by realm name which contains the following elements:
 *   - 'title', Humam readable name for the realm.
 *   - 'controller class', Class name for realm controller, implementing VariableRealmControllerInterface.
 *   - 'store class', Class name for realm store, implementing VariableRealmStoreInterface.
 *   - 'weight', Default weight for this realm.
 *   - 'keys', Associative array with human readable names for keys.
 *   - 'keys callback', Callback function to provide the keys.
 *   - 'default key', The default key.
 *   - 'options', Array of variable names that may be set for this realm. If not set
 *     any variable will be allowed for this realm.
 *   - 'list callback', Callback function to provide variable list for this realm.
 *   - 'select', Boolean flag whether variables for this realm can be selected from a list.
 *   - 'select path', Path to variable selection form (optional).
 *   - 'variable name', Name for variables that belong to this realm: e.g. 'multilingual' variable/s
 *   - 'variable class', CSS class name for annotated variables in system settings forms.
 *   - 'form settings', Boolean flag, whether realm variables should be handled automatically
 *     in system settings forms.
 *   - 'form switcher', Boolean flag, whether a realm switcher should be auto-generated
 *     for settings forms which contain variables that belong to this realm.
 */
function hook_variable_realm_info() {
  $realm['language'] = array(
    'title' => t('Language'),
    'controller class' => 'I18nVariableLanguageRealm',
    'store class' => 'VariableStoreRealmStore',
    'keys' => locale_language_list('name', TRUE),
    'default key' => language_default('language'),
    'options' => _i18n_variable_variable_realm_list(),
    'select' => TRUE,
    'select path' => 'admin/config/regional/i18n/variable',
    'variable name' => t('multilingual'),
    'variable class' => 'i18n-variable',
    'form settings' => TRUE,
  );
  return $realm;
}

/**
 * Allow other modules to act when enabling a realm.
 *
 * This hook is invoked right after the realm controller is enabled
 * and it may have a valid key already set or it may be FALSE.
 *
 * @param $realm_name
 *   Realm that is switched.
 * @param $realm_key
 *   New realm key.
 */
function hook_variable_realm_enable($realm_name, $realm_key) {
}

/**
 * Allow other modules to act on realm switching.
 *
 * This hook is invoked right after the realm key is switched but before
 * the global variables are rebuilt.
 *
 * @param $realm_name
 *   Realm that is switched.
 * @param $realm_key
 *   New realm key.
 */
function hook_variable_realm_switch($realm_name, $realm_key) {
}

/**
 * Allow other modules to act when rebuilding the configuration.
 *
 * This hook is invoked before the global variables are rebuilt
 * using the active realms.
 */
function hook_variable_realm_rebuild() {
}
/**
 * @} End of "addtogroup hooks".
 */
