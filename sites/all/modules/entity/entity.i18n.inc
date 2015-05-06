<?php
/**
 * @file
 * Internationalization (i18n) integration.
 */

/**
 * Gets the i18n controller for a given entity type.
 *
 * @return EntityDefaultI18nStringController|array|false
 *   If a type is given, the controller for the given entity type. Else an array
 *   of all enabled controllers keyed by entity type is returned.
 */
function entity_i18n_controller($type = NULL) {
  $static = &drupal_static(__FUNCTION__);

  if (!isset($type)) {
    // Invoke the function for each type to ensure we have fully populated the
    // static variable.
    foreach (entity_get_info() as $entity_type => $info) {
      entity_i18n_controller($entity_type);
    }
    return array_filter($static);
  }

  if (!isset($static[$type])) {
    $info = entity_get_info($type);
    // Do not activate it by default. Modules have to explicitly enable it by
    // specifying EntityDefaultI18nStringController or their customization.
    $class = isset($info['i18n controller class']) ? $info['i18n controller class'] : FALSE;
    $static[$type] = $class ? new $class($type, $info) : FALSE;
  }

  return $static[$type];
}

/**
 * Implements hook_i18n_string_info().
 */
function entity_i18n_string_info() {
  $groups = array();
  foreach (entity_i18n_controller() as $entity_type => $controller) {
    $groups += $controller->hook_string_info();
  }
  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function entity_i18n_object_info() {
  $info = array();
  foreach (entity_i18n_controller() as $entity_type => $controller) {
    $info += $controller->hook_object_info();
  }
  return $info;
}

/**
 * Implements hook_i18n_string_objects().
 */
function entity_i18n_string_objects($type) {
  if ($controller = entity_i18n_controller($type)) {
    return $controller->hook_string_objects();
  }
}

/**
 * Default controller handling i18n integration.
 *
 * Implements i18n string translation for all non-field properties marked as
 * 'translatable' and having the flag 'i18n string' set. This translation
 * approach fits in particular for translating configuration, i.e. exportable
 * entities.
 *
 * Requirements for the default controller:
 *  - The entity type providing module must be specified using the 'module' key
 *    in hook_entity_info().
 *  - An 'entity class' derived from the provided class 'Entity' must be used.
 *  - Properties must be declared as 'translatable' and the 'i18n string' flag
 *    must be set to TRUE using hook_entity_property_info().
 *  - i18n must be notified about changes manually by calling
 *    i18n_string_object_update(), i18n_string_object_remove() and
 *    i18n_string_update_context(). Ideally, this is done in a small integration
 *    module depending on the entity API and i18n_string. Look at the provided
 *    testing module "entity_test_i18n" for an example.
 *  - If the entity API admin UI is used, the "translate" tab will be
 *    automatically enabled and linked from the UI.
 *  - There are helpers for getting translated values which work regardless
 *    whether the i18n_string module is enabled, i.e. entity_i18n_string()
 *    and Entity::getTranslation().
 *
 *  Current limitations:
 *   - Translatable property values cannot be updated via the metadata wrapper,
 *     however reading works fine. See Entity::getTranslation().
 */
class EntityDefaultI18nStringController {

  protected $entityType, $entityInfo;

  /**
   * The i18n textgroup we are using.
   */
  protected $textgroup;

  public function __construct($type) {
    $this->entityType = $type;
    $this->entityInfo = entity_get_info($type);
    // By default we go with the module name as textgroup.
    $this->textgroup = $this->entityInfo['module'];
  }

  /**
   * Implements hook_i18n_string_info() via entity_i18n_string_info().
   */
  public function hook_string_info() {
    $list = system_list('module_enabled');
    $info = $list[$this->textgroup]->info;

    $groups[$this->textgroup] = array(
      'title' => $info['name'],
      'description' => !empty($info['description']) ? $info['description'] : NULL,
      'format' => FALSE,
      'list' => TRUE,
    );
    return $groups;
  }

  /**
   * Implements hook_i18n_object_info() via entity_i18n_object_info().
   *
   * Go with the same default values as the admin UI as far as possible.
   */
  public function hook_object_info() {
    $wildcard = $this->menuWildcard();
    $id_key = !empty($this->entityInfo['entity keys']['name']) ? $this->entityInfo['entity keys']['name'] : $this->entityInfo['entity keys']['id'];

    $info[$this->entityType] = array(
      // Generic object title.
      'title' => $this->entityInfo['label'],
      // The object key field.
      'key' => $id_key,
      // Placeholders for automatic paths.
      'placeholders' => array(
        $wildcard => $id_key,
      ),

      // Properties for string translation.
      'string translation' => array(
        // Text group that will handle this object's strings.
        'textgroup' => $this->textgroup,
        // Object type property for string translation.
        'type' => $this->entityType,
        // Translatable properties of these objects.
        'properties' => $this->translatableProperties(),
      ),
    );

    // Integrate the translate tab into the admin-UI if enabled.
    if ($base_path = $this->menuBasePath()) {
      $info[$this->entityType] += array(
        // To produce edit links automatically.
        'edit path' => $base_path . '/manage/' . $wildcard,
        // Auto-generate translate tab.
        'translate tab' => $base_path . '/manage/' . $wildcard . '/translate',
      );
      $info[$this->entityType]['string translation'] += array(
        // Path to translate strings to every language.
        'translate path' => $base_path . '/manage/' . $wildcard . '/translate/%i18n_language',
      );
    }
    return $info;
  }

  /**
   * Defines the menu base path used by self::hook_object_info().
   */
  protected function menuBasePath() {
    return !empty($this->entityInfo['admin ui']['path']) ? $this->entityInfo['admin ui']['path'] : FALSE;
  }

  /**
   * Defines the menu wildcard used by self::hook_object_info().
   */
  protected function menuWildcard() {
    return isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%entity_object';
  }

  /**
   * Defines translatable properties used by self::hook_object_info().
   */
  protected function translatableProperties() {
    $list = array();
    foreach (entity_get_all_property_info($this->entityType) as $name => $info) {
      if (!empty($info['translatable']) && !empty($info['i18n string'])) {
        $list[$name] = array(
          'title' => $info['label'],
        );
      }
    }
    return $list;
  }

  /**
   * Implements hook_i18n_string_objects() via entity_i18n_string_objects().
   */
  public function hook_string_objects() {
    return entity_load_multiple_by_name($this->entityType, FALSE);
  }
}
