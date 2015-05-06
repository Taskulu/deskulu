<?php

/**
 * @file
 * Provides a base class for entities.
 */

/**
 * A common class for entities.
 *
 * It's suggested, but not required, to extend this class and to override
 * __construct() in order to specify a fixed entity type.
 *
 * For providing an entity label and URI it is suggested to override the
 * defaultLabel() and defaultUri() methods, and to specify the
 * entity_class_label() and entity_class_uri() as respective callbacks in
 * hook_entity_info(). That way modules are able to override your defaults
 * by altering the hook_entity_info() callbacks, while $entity->label() and
 * $entity->uri() reflect this changes as well.
 *
 * Defaults for entity properties can be easily defined by adding class
 * properties, e.g.:
 * @code
 *   public $name = '';
 *   public $count = 0;
 * @endcode
 */
class Entity {

  protected $entityType;
  protected $entityInfo;
  protected $idKey, $nameKey, $statusKey;
  protected $defaultLabel = FALSE;
  protected $wrapper;

  /**
   * Creates a new entity.
   *
   * @see entity_create()
   */
  public function __construct(array $values = array(), $entityType = NULL) {
    if (empty($entityType)) {
      throw new Exception('Cannot create an instance of Entity without a specified entity type.');
    }
    $this->entityType = $entityType;
    $this->setUp();
    // Set initial values.
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }
  }

  /**
   * Set up the object instance on construction or unserializiation.
   */
  protected function setUp() {
    $this->entityInfo = entity_get_info($this->entityType);
    $this->idKey = $this->entityInfo['entity keys']['id'];
    $this->nameKey = isset($this->entityInfo['entity keys']['name']) ? $this->entityInfo['entity keys']['name'] : $this->idKey;
    $this->statusKey = empty($this->entityInfo['entity keys']['status']) ? 'status' : $this->entityInfo['entity keys']['status'];
  }

  /**
   * Returns the internal, numeric identifier.
   *
   * Returns the numeric identifier, even if the entity type has specified a
   * name key. In the latter case, the numeric identifier is supposed to be used
   * when dealing generically with entities or internally to refer to an entity,
   * i.e. in a relational database. If unsure, use Entity:identifier().
   */
  public function internalIdentifier() {
    return isset($this->{$this->idKey}) ? $this->{$this->idKey} : NULL;
  }

  /**
   * Returns the entity identifier, i.e. the entities name or numeric id.
   *
   * @return
   *   The identifier of the entity. If the entity type makes use of a name key,
   *   the name is returned, else the numeric id.
   *
   * @see entity_id()
   */
  public function identifier() {
    return isset($this->{$this->nameKey}) ? $this->{$this->nameKey} : NULL;
  }

  /**
   * Returns the info of the type of the entity.
   *
   * @see entity_get_info()
   */
  public function entityInfo() {
    return $this->entityInfo;
  }

  /**
   * Returns the type of the entity.
   */
  public function entityType() {
    return $this->entityType;
  }

  /**
   * Returns the bundle of the entity.
   *
   * @return
   *   The bundle of the entity. Defaults to the entity type if the entity type
   *   does not make use of different bundles.
   */
  public function bundle() {
    return !empty($this->entityInfo['entity keys']['bundle']) ? $this->{$this->entityInfo['entity keys']['bundle']} : $this->entityType;
  }

  /**
   * Returns the EntityMetadataWrapper of the entity.
   *
   * @return EntityDrupalWrapper
   *   An EntityMetadataWrapper containing the entity.
   */
  public function wrapper() {
    if (empty($this->wrapper)) {
      $this->wrapper = entity_metadata_wrapper($this->entityType, $this);
    }
    elseif ($this->wrapper->value() !== $this) {
      // Wrapper has been modified outside, so let's better create a new one.
      $this->wrapper = entity_metadata_wrapper($this->entityType, $this);
    }
    return $this->wrapper;
  }

  /**
   * Returns the label of the entity.
   *
   * Modules may alter the label by specifying another 'label callback' using
   * hook_entity_info_alter().
   *
   * @see entity_label()
   */
  public function label() {
    // If the default label flag is enabled, this is being invoked recursively.
    // In this case we need to use our default label callback directly. This may
    // happen if a module provides a label callback implementation different
    // from ours, but then invokes Entity::label() or entity_class_label() from
    // there.
    if ($this->defaultLabel || (isset($this->entityInfo['label callback']) && $this->entityInfo['label callback'] == 'entity_class_label')) {
      return $this->defaultLabel();
    }
    $this->defaultLabel = TRUE;
    $label = entity_label($this->entityType, $this);
    $this->defaultLabel = FALSE;
    return $label;
  }

  /**
   * Defines the entity label if the 'entity_class_label' callback is used.
   *
   * Specify 'entity_class_label' as 'label callback' in hook_entity_info() to
   * let the entity label point to this method. Override this in order to
   * implement a custom default label.
   */
  protected function defaultLabel() {
    // Add in the translated specified label property.
    return $this->getTranslation($this->entityInfo['entity keys']['label']);
  }

  /**
   * Returns the uri of the entity just as entity_uri().
   *
   * Modules may alter the uri by specifying another 'uri callback' using
   * hook_entity_info_alter().
   *
   * @see entity_uri()
   */
  public function uri() {
    if (isset($this->entityInfo['uri callback']) && $this->entityInfo['uri callback'] == 'entity_class_uri') {
      return $this->defaultUri();
    }
    return entity_uri($this->entityType, $this);
  }

  /**
   * Override this in order to implement a custom default URI and specify
   * 'entity_class_uri' as 'uri callback' hook_entity_info().
   */
  protected function defaultUri() {
    return array('path' => 'default/' . $this->identifier());
  }

  /**
   * Checks if the entity has a certain exportable status.
   *
   * @param $status
   *   A status constant, i.e. one of ENTITY_CUSTOM, ENTITY_IN_CODE,
   *   ENTITY_OVERRIDDEN or ENTITY_FIXED.
   *
   * @return
   *   For exportable entities TRUE if the entity has the status, else FALSE.
   *   In case the entity is not exportable, NULL is returned.
   *
   * @see entity_has_status()
   */
  public function hasStatus($status) {
    if (!empty($this->entityInfo['exportable'])) {
      return isset($this->{$this->statusKey}) && ($this->{$this->statusKey} & $status) == $status;
    }
  }

  /**
   * Permanently saves the entity.
   *
   * @see entity_save()
   */
  public function save() {
    return entity_get_controller($this->entityType)->save($this);
  }

  /**
   * Permanently deletes the entity.
   *
   * @see entity_delete()
   */
  public function delete() {
    $id = $this->identifier();
    if (isset($id)) {
      entity_get_controller($this->entityType)->delete(array($id));
    }
  }

  /**
   * Exports the entity.
   *
   * @see entity_export()
   */
  public function export($prefix = '') {
    return entity_get_controller($this->entityType)->export($this, $prefix);
  }

  /**
   * Generate an array for rendering the entity.
   *
   * @see entity_view()
   */
  public function view($view_mode = 'full', $langcode = NULL, $page = NULL) {
    return entity_get_controller($this->entityType)->view(array($this), $view_mode, $langcode, $page);
  }

  /**
   * Builds a structured array representing the entity's content.
   *
   * @see entity_build_content()
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    return entity_get_controller($this->entityType)->buildContent($this, $view_mode, $langcode);
  }

  /**
   * Gets the raw, translated value of a property or field.
   *
   * Supports retrieving field translations as well as i18n string translations.
   *
   * Note that this returns raw data values, which might not reflect what
   * has been declared for hook_entity_property_info() as no 'getter callbacks'
   * are invoked or no referenced entities are loaded. For retrieving values
   * reflecting the property info make use of entity metadata wrappers, see
   * entity_metadata_wrapper().
   *
   * @param $property_name
   *   The name of the property to return; e.g., 'title'.
   * @param $langcode
   *   (optional) The language code of the language to which the value should
   *   be translated. If set to NULL, the default display language is being
   *   used.
   *
   * @return
   *   The raw, translated property value; or the raw, un-translated value if no
   *   translation is available.
   *
   * @todo Implement an analogous setTranslation() method for updating.
   */
  public function getTranslation($property, $langcode = NULL) {
    $all_info = entity_get_all_property_info($this->entityType);
    // Assign by reference to avoid triggering notices if metadata is missing.
    $property_info = &$all_info[$property];

    if (!empty($property_info['translatable'])) {
      if (!empty($property_info['field'])) {
        return field_get_items($this->entityType, $this, $property, $langcode);
      }
      elseif (!empty($property_info['i18n string'])) {
        $name = $this->entityInfo['module'] . ':' . $this->entityType . ':' . $this->identifier() . ':' . $property;
        return entity_i18n_string($name, $this->$property, $langcode);
      }
    }
    return $this->$property;
  }

  /**
   * Checks whether the entity is the default revision.
   *
   * @return Boolean
   *
   * @see entity_revision_is_default()
   */
  public function isDefaultRevision() {
    if (!empty($this->entityInfo['entity keys']['revision'])) {
      $key = !empty($this->entityInfo['entity keys']['default revision']) ? $this->entityInfo['entity keys']['default revision'] : 'default_revision';
      return !empty($this->$key);
    }
    return TRUE;
  }

  /**
   * Magic method to only serialize what's necessary.
   */
  public function __sleep() {
    $vars = get_object_vars($this);
    unset($vars['entityInfo'], $vars['idKey'], $vars['nameKey'], $vars['statusKey']);
    // Also key the returned array with the variable names so the method may
    // be easily overridden and customized.
    return drupal_map_assoc(array_keys($vars));
  }

  /**
   * Magic method to invoke setUp() on unserialization.
   */
  public function __wakeup() {
    $this->setUp();
  }
}

/**
 * These classes are deprecated by "Entity" and are only here for backward
 * compatibility reasons.
 */
class EntityDB extends Entity {}
class EntityExtendable extends Entity {}
class EntityDBExtendable extends Entity {}
