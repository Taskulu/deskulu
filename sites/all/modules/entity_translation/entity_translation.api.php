<?php

/**
 * @file
 * API documentation for the Entity translation module.
 */

/**
 * Allows modules to define their own translation info.
 *
 * Entity Translation relies on the core entity information to provide its
 * translation features. See the documentation of hook_entity_info() in the core
 * API documentation (system.api.php) for more details on all the entity info
 * keys that may be defined.
 *
 * To make Entity Translation automatically support an entity type some keys
 * may need to be defined, but none of them is required except the 'base path'
 * key if the entity path is different from ENTITY_TYPE/%ENTITY_TYPE (e.g.
 * taxonomy/term/1). The 'base path' key is used to determine the view, edit and
 * translate path if they follow the default path patterns  and to reliably
 * alter menu information to provide the translation UI. If the entity path
 * matches the default pattern above, and there is no need for a dedicated
 * translation handler class, Entity Translation will provide built-in support
 * for the entity.
 *
 * The entity translation info is an associative array that has to match the
 * following structure. Three nested sub-arrays keyed respectively by entity
 * type, the 'translation' key and the 'entity_translation' key: the second one
 * is the key defined by the core entity system while the third one registers
 * Entity translation as a field translation handler. Elements:
 * - class: The name of the translation handler class, which is used to handle
 *   the translation process. Defaults to 'EntityTranslationDefaultHandler'.
 * - base path: The base menu router path to which attach the administration
 *   user interface. Defaults to ENTITY_TYPE/%ENTITY_TYPE.
 * - access callback: The access callback for the translation pages. Defaults to
 *   'entity_translation_tab_access'.
 * - access arguments: The access arguments for the translation pages. Defaults
 *   to "array($entity_type, $entity_position)".
 * - view path: The menu router path to be used to view the entity. Defaults to
 *   the base path.
 * - edit path: The menu router path to be used to edit the entity. Defaults to
 *   "$base_path/edit".
 * - translate path: The menu router path to be used for attaching the
 *   translation UI. Defaults to "$base_path/translate".
 * - path wildcard: The menu router path wildcard identifying the entity.
 *   Defaults to %ENTITY_TYPE.
 * - admin theme: Whether the translation UI should use the administration
 *   theme. Defaults to TRUE.
 * - path schemes: An array of menu router path schemes used for attaching the
 *   entity translation UI. This element can be used to declare additional path
 *   schemes, if an entity type uses multiple schemes for managing entities
 *   (e.g. different schemes for different bundles). Each path scheme can define
 *   the following elements (descriptions see above): 'base path', 'view path',
 *   'edit path', 'translate path', 'path wildcard' and 'admin theme'. All path
 *   elements that are defined directly on the entity translation info array are
 *   automatically added as a 'default' path scheme.
 * - theme callback: The callback to be used to determine the translation
 *   theme. Defaults to 'variable_get'.
 * - theme arguments: The arguments to be used to determine the translation
 *   theme. Defaults to "array('admin_theme')".
 * - edit form: The key to be used to retrieve the entity object from the form
 *   state array. An empty value prevents Entity translation from performing
 *   alterations to the entity form. Defaults to ENTITY_TYPE.
 * - skip original values access: A flag specifying whether skipping access
 *   control when editing original values for this entity. Defaults to FALSE.
 * - bundle callback: A callback to check whether the passed bundle has entity
 *   translation enabled. Defaults to TRUE for all bundles.
 * - default settings: The defaults to be applied to settings when an explicit
 *   choice is missing.
 */
function hook_entity_info() {
  $info['custom_entity'] = array(
    'translation' => array(
      'entity_translation' => array(
        'class' => 'EntityTranslationCustomEntityHandler',
        'base path' => 'custom_entity/%custom_entity',
        'access callback' => 'custom_entity_tab_access',
        'access arguments' => array(1),
        'edit form' => 'custom_entity_form_state_key',
        'bundle callback' => 'custom_entity_translation_enabled_bundle',
        'default settings' => array(
          'default_language' => LANGUAGE_NONE,
          'hide_language_selector' => FALSE,
        ),
      ),
    ),
  );

  // Entity type which has multiple (e.g. bundle-specific) paths.
  $info['custom_entity_2'] = array(
    'translation' => array(
      'entity_translation' => array(
        'class' => 'EntityTranslationCustomEntityHandler',
        'path sets' => array(
          'default' => array(
            'base path' => 'custom_entity_2/%custom_entity',
            'path wildcard' => '%custom_entity',
          ),
          'fancy' => array(
            // Base path is not required.
            'edit path' => 'fancy/%entity/edit',
            'path wildcard' => '%entity',
          ),
        ),
      )
    )
  );

  return $info;
}

/**
 * Allows modules to act when a new translation is added.
 *
 * @param $entity_type
 *   The entity type.
 * @param $entity
 *   The entity.
 * @param $translation
 *   The inserted translation array.
 * @param $values
 *   The translated set of values, if any.
 */
function hook_entity_translation_insert($entity_type, $entity, $translation, $values = array()) {
}

/**
 * Allows modules to act when a translation is updated.
 *
 * @param $entity_type
 *   The entity type.
 * @param $entity
 *   The entity.
 * @param $translation
 *   The updated translation array.
 * @param $values
 *   The translated set of values, if any.
 */
function hook_entity_translation_update($entity_type, $entity, $translation, $values = array()) {
}

/**
 * Allows modules to act when a translation is deleted.
 *
 * @param $entity_type
 *   The entity type.
 * @param $entity
 *   The entity.
 * @param $langcode
 *   The langcode of the translation which was deleted.
 */
function hook_entity_translation_delete($entity_type, $entity, $langcode) {
}

/**
 * Allows modules to act when a revision translation is deleted.
 *
 * @param $entity_type
 *   The entity type.
 * @param $entity
 *   The entity.
 * @param $langcode
 *   The langcode of the revision translation which was deleted.
 */
function hook_entity_translation_delete_revision($entity_type, $entity, $langcode) {
}
