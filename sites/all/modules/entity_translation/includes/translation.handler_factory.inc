<?php

/**
 * @file
 * Translation handler factory for the Entity Translation module.
 */

/**
 * Class implementing the entity translation handler factory.
 */
class EntityTranslationHandlerFactory {

  /**
   * A singleton instance of the factory.
   *
   * @var EntityTranslationHandlerFactory
   */
  protected static $instance;

  /**
   * Counter used to generate handler ids for new entities.
   *
   * @var int
   */
  protected static $newId = 1;

  /**
   * Handlers cache.
   *
   * @var array
   */
  protected $handlers = array();

  /**
   * The last translation handler retrieved.
   *
   * @var EntityTranslationHandlerInterface
   */
  protected $last;

  /**
   * An array of translation handler retrieved by type.
   *
   * @var array
   */
  protected $lastByType = array();

  /**
   * Returns the singleton instance of the translation handler factory.
   *
   * @return EntityTranslationHandlerFactory
   */
  public static function getInstance() {
    if (!isset(self::$instance)) {
      self::$instance = new EntityTranslationHandlerFactory();
    }
    return self::$instance;
  }

  /**
   * Prevents the factory from being publicly instantiated.
   */
  protected function __construct() {}

  /**
   * Translation handler factory.
   *
   * @param $entity_type
   *   The type of $entity; e.g. 'node' or 'user'.
   * @param $entity
   *   The entity to be translated. A bundle name may be passed to instantiate
   *   an empty entity.
   *
   * @return EntityTranslationHandlerInterface
   *   A class implementing EntityTranslationHandlerInterface.
   */
  public function getHandler($entity_type, $entity) {
    if (is_numeric($entity)) {
      $entities = entity_load($entity_type, array($entity));
      $entity = reset($entities);
    }
    elseif (is_string($entity)) {
      $entity = entity_create_stub_entity($entity_type, array(NULL, NULL, $entity));
    }

    $id = $this->getHandlerId($entity_type, $entity);
    if (!isset($this->handlers[$entity_type][$id])) {
      $entity_info = entity_get_info($entity_type);
      $class = $entity_info['translation']['entity_translation']['class'];
      // @todo Remove the fourth parameter once 3rd-party translation handlers
      //   have been fixed and no longer require the deprecated entity_id
      //   parameter.
      $handler = new $class($entity_type, $entity_info, $entity, NULL);
      $handler->setFactory($this);
      $this->handlers[$entity_type][$id] = $handler;
    }

    $this->last = $this->handlers[$entity_type][$id];
    $this->lastByType[$entity_type] = $this->last;
    $this->last->setEntity($entity);

    return $this->last;
  }

  /**
   * Retrieves the translation handler identifier for the given entity.
   *
   * @param $entity_type
   *   The type of the entity the translation handler will wrap.
   * @param $entity
   *   The entity the translation handler will wrap.
   */
  public function getHandlerId($entity_type, $entity) {
    if (!isset($entity->entity_translation_handler_id)) {
      list($id, $revision_id, $bundle) = entity_extract_ids($entity_type, $entity);
      $revision_id = isset($revision_id) ? $revision_id : 0;
      $bundle = isset($bundle) ? $bundle : $entity_type;
      $entity->entity_translation_handler_id = $entity_type . '-' . $bundle . '-' . (!empty($id) ? 'eid-' . $id . '-' . $revision_id : 'new-' . self::$newId++);
    }
    return $entity->entity_translation_handler_id;
  }

  /**
   * Returns the last translation handler retrieved.
   *
   * @param $entity_type
   *   (optional) The entity type of the translation handler. Defaults to the
   *   last one.
   *
   * @return EntityTranslationHandlerInterface
   *   A class implementing EntityTranslationHandlerInterface.
   */
  public function getLastHandler($entity_type = NULL) {
    if (isset($entity_type)) {
      return isset($this->lastByType[$entity_type]) ? $this->lastByType[$entity_type] : NULL;
    }
    return $this->last;
  }

}
