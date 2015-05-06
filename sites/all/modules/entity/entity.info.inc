<?php

/**
 * @file
 * Provides basic entity property info for entities provided via the CRUD API,
 * as well as property info for all entity types defined by core. For that
 * the respective modules/MODULE.info.inc files are included.
 */

/**
 * Implements hook_entity_property_info().
 */
function entity_entity_property_info() {
  $items = array();
  // Add in info about entities provided by the CRUD API.
  foreach (entity_crud_get_info() as $type => $info) {
    // Automatically enable the controller only if the module does not implement
    // the hook itself.
    if (!isset($info['metadata controller class']) && !empty($info['base table']) && (!isset($info['module']) || !module_hook($info['module'], 'entity_property_info'))) {
      $info['metadata controller class'] = 'EntityDefaultMetadataController';
    }
    if (!empty($info['metadata controller class'])) {
      $controller = new $info['metadata controller class']($type);
      $items += $controller->entityPropertyInfo();
    }
  }
  // Add in info for all core entities.
  foreach (_entity_metadata_core_modules() as $module) {
    module_load_include('inc', 'entity', "modules/$module.info");
    if (function_exists($function = "entity_metadata_{$module}_entity_property_info")) {
      if ($return = $function()) {
        $items = array_merge_recursive($items, $return);
      }
    }
  }
  return $items;
}

/**
 * Implements hook_entity_property_info_alter().
 */
function entity_entity_property_info_alter(&$entity_info) {
  // Add in info for all core entities.
  foreach (_entity_metadata_core_modules() as $module) {
    module_load_include('inc', 'entity', "modules/$module.info");
    if (function_exists($function = "entity_metadata_{$module}_entity_property_info_alter")) {
      $function($entity_info);
    }
  }
}

function _entity_metadata_core_modules() {
  return array_filter(array('book', 'comment', 'field', 'locale', 'node', 'taxonomy', 'user', 'system', 'statistics'), 'module_exists');
}

/**
 * Default controller for generating some basic metadata for CRUD entity types.
 */
class EntityDefaultMetadataController {

  protected $type, $info;

  public function __construct($type) {
    $this->type = $type;
    $this->info = entity_get_info($type);
  }

  public function entityPropertyInfo() {
    $entity_label = drupal_strtolower($this->info['label']);

    // Provide defaults based on the schema.
    $info['properties'] = $this->convertSchema();
    foreach ($info['properties'] as $name => &$property) {
      // Add a description.
      $property['description'] = t('@entity "@property" property.', array('@entity' => drupal_ucfirst($entity_label), '@property' => $name));
    }

    // Set better metadata for known entity keys.
    $id_key = $this->info['entity keys']['id'];

    if (!empty($this->info['entity keys']['name']) && $key = $this->info['entity keys']['name']) {
      $info['properties'][$key]['type'] = 'token';
      $info['properties'][$key]['label'] = t('Machine-readable name');
      $info['properties'][$key]['description'] = t('The machine-readable name identifying this @entity.', array('@entity' => $entity_label));
      $info['properties'][$id_key]['label'] = t('Internal, numeric @entity ID', array('@entity' => $entity_label));
      $info['properties'][$id_key]['description'] = t('The ID used to identify this @entity internally.', array('@entity' => $entity_label));
    }
    else {
      $info['properties'][$id_key]['label'] = t('@entity ID', array('@entity' => drupal_ucfirst($entity_label)));
      $info['properties'][$id_key]['description'] = t('The unique ID of the @entity.', array('@entity' => $entity_label));
    }
    // Care for the bundle.
    if (!empty($this->info['entity keys']['bundle']) && $key = $this->info['entity keys']['bundle']) {
      $info['properties'][$key]['type'] = 'token';
      $info['properties'][$key]['options list'] = array(get_class($this), 'bundleOptionsList');
    }
    // Care for the label.
    if (!empty($this->info['entity keys']['label']) && $key = $this->info['entity keys']['label']) {
      $info['properties'][$key]['label'] = t('Label');
      $info['properties'][$key]['description'] = t('The human readable label.');
    }

    // Add a computed property for the entity URL and expose it to views.
    if (empty($info['properties']['url']) && !empty($this->info['uri callback'])) {
      $info['properties']['url'] = array(
        'label' => t('URL'),
        'description' => t('The URL of the entity.'),
        'getter callback' => 'entity_metadata_entity_get_properties',
        'type' => 'uri',
        'computed' => TRUE,
        'entity views field' => TRUE,
      );
    }

    return array($this->type => $info);
  }

  /**
   * A options list callback returning all bundles for an entity type.
   */
  public static function bundleOptionsList($name, $info) {
    if (!empty($info['parent']) && $type = $info['parent']) {
      $entity_info = $info['parent']->entityInfo();
      $options = array();
      foreach ($entity_info['bundles'] as $name => $bundle_info) {
        $options[$name] = $bundle_info['label'];
      }
      return $options;
    }
  }

  /**
   * Return a set of properties for an entity based on the schema definition
   */
  protected function convertSchema() {
    return entity_metadata_convert_schema($this->info['base table']);
  }
}

/**
 * Converts the schema information available for the given table to property info.
 *
 * @param $table
 *   The name of the table as used in hook_schema().
 * @return
 *   An array of property info as suiting for hook_entity_property_info().
 */
function entity_metadata_convert_schema($table) {
  $schema = drupal_get_schema($table);
  $properties = array();
  foreach ($schema['fields'] as $name => $info) {
    if ($type = _entity_metadata_convert_schema_type($info['type'])) {
      $properties[$name] = array(
        'type' => $type,
        'label' => drupal_ucfirst($name),
        'schema field' => $name,
        // As we cannot know about any setter access, leave out the setter
        // callback. For getting usually no further access callback is needed.
      );
      if ($info['type'] == 'serial') {
        $properties[$name]['validation callback'] = 'entity_metadata_validate_integer_positive';
      }
    }
  }
  return $properties;
}

function _entity_metadata_convert_schema_type($type) {
  switch ($type) {
    case 'int':
    case 'serial':
    case 'date':
      return 'integer';
    case 'float':
    case 'numeric':
      return 'decimal';
    case 'char':
    case 'varchar':
    case 'text':
      return 'text';
  }
}

/**
 * Interface for extra fields controller.
 *
 * Note: Displays extra fields exposed by this controller are rendered by
 * default by the EntityAPIController.
 */
interface EntityExtraFieldsControllerInterface {

  /**
   * Returns extra fields for this entity type.
   *
   * @see hook_field_extra_fields().
   */
  public function fieldExtraFields();
}

/**
 * Default controller for generating extra fields based on property metadata.
 *
 * By default a display extra field for each property not being a field, ID or
 * bundle is generated.
 */
class EntityDefaultExtraFieldsController implements EntityExtraFieldsControllerInterface {

  /**
   * @var string
   */
  protected $entityType;

  /**
   * @var array
   */
  protected $entityInfo;

  /**
   * Constructor.
   */
  public function __construct($type) {
    $this->entityType = $type;
    $this->entityInfo = entity_get_info($type);
    $this->propertyInfo = entity_get_property_info($type);
  }

  /**
   * Implements EntityExtraFieldsControllerInterface::fieldExtraFields().
   */
  public function fieldExtraFields() {
    $extra = array();
    foreach ($this->propertyInfo['properties'] as $name => $property_info) {
      // Skip adding the ID or bundle.
      if ($this->entityInfo['entity keys']['id'] == $name || $this->entityInfo['entity keys']['bundle'] == $name) {
        continue;
      }
      $extra[$this->entityType][$this->entityType]['display'][$name] = $this->generateExtraFieldInfo($name, $property_info);
    }

    // Handle bundle properties.
    $this->propertyInfo += array('bundles' => array());
    foreach ($this->propertyInfo['bundles'] as $bundle_name => $info) {
      foreach ($info['properties'] as $name => $property_info) {
        if (empty($property_info['field'])) {
          $extra[$this->entityType][$bundle_name]['display'][$name] = $this->generateExtraFieldInfo($name, $property_info);
        }
      }
    }
    return $extra;
  }

  /**
   * Generates the display field info for a given property.
   */
  protected function generateExtraFieldInfo($name, $property_info) {
    $info = array(
      'label' => $property_info['label'],
      'weight' => 0,
    );
    if (!empty($property_info['description'])) {
      $info['description'] = $property_info['description'];
    }
    return $info;
  }
}
