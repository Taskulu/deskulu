<?php

/**
 * @file
 * Default translation handler for the translation module.
 */


/**
 * Interface for enabling entity translation.
 *
 * Defines a set of methods to allow any entity to be processed by the entity
 * translation UI.
 */
interface EntityTranslationHandlerInterface {

  /**
   * Injects the translation handler factory.
   */
  public function setFactory(EntityTranslationHandlerFactory $factory);

  /**
   * Registers a child translation handler for the given entity.
   */
  public function addChild($entity_type, $entity);

  /**
   * Removes a previously registered child translation handler.
   */
  public function removeChild($entity_type, $entity);

  /**
   * Loads the translation data into the wrapped entity.
   */
  public function loadTranslations();

  /**
   * Writes the translation status to the storage.
   */
  public function saveTranslations();

  /**
   * Returns the translation data for the current (wrapped) entity.
   */
  public function getTranslations();

  /**
   * Adds/updates an entity translation.
   *
   * @param $translation
   *   A translation array as defined by the translation table's schema.
   * @param $values
   *   (optional) the values that should be assigned to the field translations.
   */
  public function setTranslation($translation, $values = NULL);

  /**
   * Removes a translation from the translation set.
   *
   * @param $langcode
   *   The language code of the translation to be removed.
   */
  public function removeTranslation($langcode);

  /**
   * Initializes the translation set by creating the original translation.
   */
  public function initTranslations();

  /**
   * Updates the translation set from the current entity status.
   */
  public function updateTranslations();

  /**
   * Removes all translations from the translation set.
   */
  public function removeTranslations();

  /**
   * Removes all translations from the current revision.
   */
  public function removeRevisionTranslations();

  /**
   * Initialize the language of the original field values.
   *
   * Ensure that the original translation language matches the language assigned
   * to the original field values.
   *
   * @return
   *   TRUE if any initialization was necessary, FALSE otherwise.
   */
  public function initOriginalTranslation();

  /**
   * Returns the entity language.
   */
  public function getLanguage();

  /**
   * Returns the translation object key for the wrapped entity type.
   */
  public function getLanguageKey();

  /**
   * Returns the default language for the wrapped entity type and bundle.
   */
  public function getDefaultLanguage();

  /**
   * Sets the language of the orginal translation.
   *
   * @param $langcode
   *   The language code of the original content values.
   */
  public function setOriginalLanguage($langcode);

  /**
   * Returns TRUE if the entity is currently being translated.
   *
   * @deprecated This is no longer used and will be removed before the first
   *   stable release.
   */
  public function isTranslating();

  /**
   * Notifies the translation handler that its entity is being translated.
   *
   * @param $translating
   *   A boolean value.
   *
   * @deprecated This is no longer used and will be removed before the first
   *   stable release.
   */
  public function setTranslating($translating);

  /**
   * Return TRUE if a new revision of the entity has just been created.
   */
  public function isRevision();

  /**
   * Return TRUE if the entity type supports revisions.
   */
  public function isRevisionable();

  /**
   * Replaces the wrapped entity.
   *
   * @param $entity
   *   The entity to be translated.
   */
  public function setEntity($entity);

  /**
   * Returns the wrapped entity.
   *
   * @param return
   *   The wrapped entity.
   */
  public function getEntity();

  /**
   * Returns the wrapped entity type.
   *
   * @param return
   *   The wrapped entity type.
   */
  public function getEntityType();

  /**
   * Checks that the wrapped entity matches the give entity
   *
   * @param $entity_type
   *   The type of the entity to be matched.
   * @param $entity
   *   The entity to be matched.
   */
  public function isWrappedEntity($entity_type, $entity);

  /**
   * Sets the translation update status.
   *
   * @param $outdated
   *   A boolean value.
   */
  public function setOutdated($outdated);

  /**
   * Returns the base path for the current entity.
   *
   * This path will be prepended to the URL of any administration page.
   *
   * @return
   *   A string to be used as a URL path prefix.
   */
  public function getBasePath();

  /**
   * Returns the path of the entity edit form.
   *
   * @param $langcode
   *   (optional) The language the edit form should be presented in.
   */
  public function getEditPath($langcode = NULL);

  /**
   * Returns the path of the translation overview page.
   */
  public function getTranslatePath();

  /**
   * Returns the path of the entity view page.
   */
  public function getViewPath();

  /**
   * Returns the active path scheme.
   */
  public function getPathScheme();

  /**
   * Changes the active path scheme.
   *
   * @param $scheme
   *   The new path scheme.
   */
  public function setPathScheme($scheme);

  /**
   * Initializes the most suited path scheme based on the given path.
   *
   * @param $path
   *   (optional) The path to match the defined path schemes against. Defaults
   *   to the current path.
   *
   * @return
   *   The matched path scheme key.
   */
  public function initPathScheme($path = NULL);

  /**
   * A string allowing the user to identify the entity.
   */
  public function getLabel();

  /**
   * Checks if the user can perform the given operation on the wrapped entity.
   *
   * @param $op
   *   The operation to be performed.
   *
   * @return
   *   TRUE if the user is allowed to perform the given operation, FALSE
   *   otherwise.
   */
  public function getAccess($op);

  /**
   * Checks if a user is allowed to edit the given translation.
   */
  public function getTranslationAccess($langcode);

  /**
   * Checks if a user is allowed to edit shared fields on the active form.
   */
  public function getSharedFieldsAccess();

  /**
   * Return TRUE if the entity supports URL aliasing.
   */
  public function isAliasEnabled();

  /**
   * Sets the active form language.
   */
  public function setFormLanguage($langcode);

  /**
   * Retrieves the active form language.
   */
  public function getFormLanguage();

  /**
   * Sets the source language for the translation being created.
   */
  public function setSourceLanguage($langcode);

  /**
   * Retrieves the source language for the translation being created.
   */
  public function getSourceLanguage();

  /**
   * Returns TRUE if a new entity is currently wrapped.
   */
  public function isNewEntity();

  /**
   * Returns TRUE whether we are displying an entity form.
   */
  public function isEntityForm();

  /**
   * Performs the needed alterations to the entity form.
   */
  public function entityForm(&$form, &$form_state);

  /**
   * Adds an language selection widget to the entity form.
   */
  public function entityFormLanguageWidget(&$form, &$form_state);

  /**
   * Performs submission tasks on the submitted entity language.
   */
  public function entityFormLanguageWidgetSubmit($form, &$form_state);

  /**
   * Handle shared form elements.
   */
  public function entityFormSharedElements(&$element);

  /**
   * Performs validation tasks on the submitted entity forms.
   */
  public function entityFormValidate($form, &$form_state);

  /**
   * Performs submission tasks on the submitted entity forms.
   */
  public function entityFormSubmit($form, &$form_state);

  /**
   * Alters the local tasks render array to populate the language tabs.
   */
  public function localTasksAlter(&$data, $router_item, $root_path);
}

/**
 * Class implementing the default entity translation behaviours.
 */
class EntityTranslationDefaultHandler implements EntityTranslationHandlerInterface {

  protected $entityType;
  protected $entity;
  protected $entityInfo;
  protected $entityId;
  protected $bundle;
  protected $revisionable;

  /**
   * The translation handler factory.
   *
   * @var EntityTranslationHandlerFactory
   */
  protected $factory;

  /**
   * The translation handler hierarchy storage.
   *
   * @var array
   */
  protected $children = array();

  private $entityForm;
  private $translating;
  private $outdated;
  private $formLanguage;
  private $sourceLanguage;

  private $pathScheme;
  private $pathWildcard;
  private $basePath;
  private $editPath;
  private $translatePath;
  private $viewPath;
  private $routerMap;

  /**
   * Initializes an instance of the translation handler.
   *
   * @param $entity_type
   *   The type of the entity being wrapped.
   * @param $entity_info
   *   The entity information for the entity being wrapped.
   * @param $entity
   *   The entity being wrapped.
   */
  public function __construct($entity_type, $entity_info, $entity) {
    $this->entityType = $entity_type;
    $this->entityInfo = $entity_info;
    $this->setEntity($entity);

    $this->entityForm = FALSE;
    $this->translating = FALSE;
    $this->outdated = FALSE;
    $this->formLanguage = FALSE;
    $this->sourceLanguage = FALSE;
    $this->pathScheme = 'default';
    $this->routerMap = array();

    if (entity_translation_enabled($entity_type)) {
      $this->initPathVariables();
    }
  }

  /**
   * Read the translation data from the storage.
   */
  public static function loadMultiple($entity_type, $entities) {
    $entity_info = entity_get_info($entity_type);
    if (isset($entity_info['entity keys']['translations'])){
      $translations_key = $entity_info['entity keys']['translations'];
    }
    else {
      // If no translations key is defined we cannot proceed.
      return;
    }

    $revisionable = self::isEntityTypeRevisionable($entity_type);
    $revisions_ids = array();
    foreach ($entities as $id => $entity) {
      $entities[$id]->{$translations_key} = self::emptyTranslations();
      if ($revisionable) {
        list(, $revisions_id,) = entity_extract_ids($entity_type, $entity);
        $revisions_ids[$id] = $revisions_id;
      }
    }

    $table = $revisionable ? 'entity_translation_revision' : 'entity_translation';
    $query = db_select($table, 'et')
      ->fields('et')
      ->condition('entity_type', $entity_type);

    if (!$revisionable) {
      $query->condition('entity_id', array_keys($entities), 'IN');
    }
    else {
      $query->condition('revision_id', $revisions_ids, 'IN');
    }

    $results = $query
      ->orderBy('entity_id')
      ->orderBy('created')
      ->execute();

    foreach ($results as $row) {
      $id = $row->entity_id;
      $entities[$id]->{$translations_key}->data[$row->language] = (array) $row;

      // Only the original translation has an empty source.
      if (empty($row->source)) {
        $entities[$id]->{$translations_key}->original = $row->language;
      }
    }
  }

  /**
   * Returns the localized links for the given path.
   */
  public static function languageSwitchLinks($path) {
    $links = language_negotiation_get_switch_links(LANGUAGE_TYPE_CONTENT, $path);
    if (empty($links)) {
      // If content language is set up to fall back to the interface language,
      // then there will be no switch links for LANGUAGE_TYPE_CONTENT, ergo we
      // also need to use interface switch links.
      $links = language_negotiation_get_switch_links(LANGUAGE_TYPE_INTERFACE, $path);
    }
    return $links;
  }

  /**
   * @see EntityTranslationHandlerInterface::setFactory()
   */
  public function setFactory(EntityTranslationHandlerFactory $factory) {
    $this->factory = $factory;
  }

  /**
   * @see EntityTranslationHandlerInterface::addChild()
   */
  public function addChild($entity_type, $entity) {
    if (!empty($this->factory)) {
      $handler = $this->factory->getHandler($entity_type, $entity);
      $handler->setFormLanguage($this->getFormLanguage());
      $handler->setSourceLanguage($this->getSourceLanguage());
      // Avoid registering more than one child handler for each entity.
      $hid = $this->factory->getHandlerId($entity_type, $entity);
      $this->children[$hid] = $handler;
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::removeChild()
   */
  public function removeChild($entity_type, $entity) {
    if (!empty($this->factory)) {
      $hid = $this->factory->getHandlerId($entity_type, $entity);
      unset($this->children[$hid]);
    }
  }

  /**
   * Proxies the specified method invocation to a child translation handler.
   */
  protected function notifyChildren($method, $args) {
    foreach ($this->children as $handler) {
      call_user_func_array(array($handler, $method), $args);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::loadTranslations()
   */
  public function loadTranslations() {
    if (isset($this->entityId)) {
      $this->loadMultiple($this->entityType, array($this->entityId => $this->entity));
    }
    elseif ($translations_key = $this->getTranslationsKey()) {
      $this->entity->{$translations_key} = $this->emptyTranslations();
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::saveTranslations()
   */
  public function saveTranslations() {
    $translations = $this->getTranslations();

    // Save current values.
    $this->doSaveTranslations($translations, 'entity_translation');

    // Save revision values.
    if ($this->isRevisionable()) {
      $this->doSaveTranslations($translations, 'entity_translation_revision', TRUE);
    }

    // The translation handler interface decouples operations on translations at
    // data structure level from CRUD operations. Hence hooks must be fired
    // after changes are actually persisted.
    if (!empty($translations->hook)) {

      // Hook info is keyed by language code so that subsequent operations at
      // data structure level do not cause multiple hooks for the same data to
      // be fired. For instance if a translation is first updated and then
      // deleted, only the 'delete' hook should be fired, because it is the only
      // change that has actually been persisted.
      foreach ($translations->hook as $langcode => $info) {
        $translation = isset($translations->data[$langcode]) ? $translations->data[$langcode] : $langcode;
        $data = isset($info['data']) ? $info['data'] : NULL;
        module_invoke_all('entity_translation_' . $info['hook'], $this->entityType, $this->entity, $translation, $data);
        // Provide Rules events integration if available.
        if (module_exists('rules')) {
          // Pass the entity as a wrapped one since rules can't do it for us
          // when using the variable type 'entity'.
          rules_invoke_event('entity_translation_' . $info['hook'], $this->entityType, entity_metadata_wrapper($this->entityType, $this->entity), $translation, $data);
        }
      }

      // Avoid firing hooks more than once for the same changes.
      $translations->hook = array();
    }
  }

  /**
   * Saves entity translation records to the storage.
   */
  protected function doSaveTranslations($translations, $table, $revision = FALSE) {
    // Delete and insert, rather than update, in case a value was added.
    $query = db_delete($table)
      ->condition('entity_type', $this->entityType)
      ->condition('entity_id', $this->entityId);
    // If we are storing translations for the current revision or we are
    // deleting the entity we should remove all translation data.
    $langcode = $translations->original;
    $hook = isset($translations->hook) ? $translations->hook : array();
    if ($revision && $this->isRevisionable() && (empty($hook[$langcode]['hook']) || $hook[$langcode]['hook'] != 'delete')) {
      $query->condition('revision_id', $this->revisionId);
    }
    $query->execute();

    if (count($translations->data)) {
      $columns = array('entity_type', 'entity_id', 'revision_id', 'language', 'source', 'uid', 'status', 'translate', 'created', 'changed');
      $query = db_insert($table)->fields($columns);

      // These values should override the translation ones as they are not
      // supposed to change.
      $overrides = array(
        'entity_type' => $this->entityType,
        'entity_id' => $this->entityId,
        'revision_id' => $this->isRevisionable() ? $this->revisionId : $this->entityId,
      );

      // These instead are just defaults.
      $defaults = array(
        'source' => '',
        'uid' => $GLOBALS['user']->uid,
        'translate' => 0,
        'status' => 0,
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
      );

      foreach ($translations->data as $translation) {
        $translation = $overrides + $translation + $defaults;
        $query->values($translation);
      }

      $query->execute();
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::getTranslations()
   */
  public function getTranslations() {
    if ($translations_key = $this->getTranslationsKey()) {
      // Lazy load translations if for some reason the wrapped entity did not go
      // through hook_entity_load().
      if (!isset($this->entity->{$translations_key})) {
        $this->loadTranslations();
      }
      return $this->entity->{$translations_key};
    }
    return self::emptyTranslations();
  }

  /**
   * @see EntityTranslationHandlerInterface::setTranslation()
   */
  public function setTranslation($translation, $values = NULL) {
    if (isset($translation['source']) && $translation['language'] == $translation['source']) {
      throw new Exception('Invalid translation language');
    }

    $translations = $this->getTranslations();
    $langcode = $translation['language'];

    $this->setTranslating(TRUE);

    if (isset($translations->data[$langcode])) {
      $translation = array_merge($translations->data[$langcode], $translation);
      $translation['changed'] = REQUEST_TIME;
      // If a newly inserted translation has not been stored yet do not fire an
      // update hook.
      $hook = empty($translations->hook[$langcode]['hook']) ? 'update' : $translations->hook[$langcode]['hook'];
    }
    else {
      $hook = 'insert';
    }

    // Store the translation data.
    $translations->data[$langcode] = $translation;
    // Keep track that the translation has been inserted or updated.
    $translations->hook[$langcode] = array('hook' => $hook, 'data' => $values);

    if (is_array($values)) {
      // Update field translations.
      foreach (field_info_instances($this->entityType, $this->bundle) as $instance) {
        $field_name = $instance['field_name'];
        $field = field_info_field($field_name);
        if ($field['translatable'] && isset($values[$field_name])) {
          $this->entity->{$field_name}[$langcode] = $values[$field_name][$langcode];
        }
      }
    }

    $args = func_get_args();
    $this->notifyChildren(__FUNCTION__, $args);
  }

  /**
   * @see EntityTranslationHandlerInterface::removeTranslation()
   */
  public function removeTranslation($langcode) {
    $translations_key = $this->getTranslationsKey();
    if (empty($translations_key)) {
      return;
    }

    $hook_info = array('hook' => 'delete');
    if (!empty($langcode)) {
      unset($this->entity->{$translations_key}->data[$langcode]);
      // Keep track that the current translation has been removed.
      $this->entity->{$translations_key}->hook[$langcode] = $hook_info;
    }
    elseif (!empty($this->entity->{$translations_key}->data)) {
      $keys = array_keys($this->entity->{$translations_key}->data);
      $values = array_fill(0, count($keys), $hook_info);
      // Keep track that the all translations have been removed.
      $this->entity->{$translations_key}->hook = array_combine($keys, $values);
      // Actually remove translations.
      $this->entity->{$translations_key}->data = array();
    }

    // Remove field translations.
    foreach (field_info_instances($this->entityType, $this->bundle) as $instance) {
      $field_name = $instance['field_name'];
      $field = field_info_field($field_name);

      if ($field['translatable']) {
        if (!empty($langcode)) {
          $this->entity->{$field_name}[$langcode] = array();
        }
        else {
          $this->entity->{$field_name} = array();
        }
      }
    }

    // Clear the cache for this entity.
    entity_get_controller($this->entityType)->resetCache(array($this->getEntityId()));
  }

  /**
   * @see EntityTranslationHandlerInterface::initTranslations()
   */
  public function initTranslations() {
    $langcode = $this->getLanguage();

    if (!empty($langcode)) {
      $translation = array('language' => $langcode, 'status' => 1);
      $this->setTranslation($translation);
      $this->setOriginalLanguage($langcode);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::updateTranslations()
   */
  public function updateTranslations() {
    $langcode = $this->getLanguage();

    // Only create a translation on edit if the translation set is empty:
    // the entity might have been created with language set to "language
    // neutral".
    if (empty($this->getTranslations()->data)) {
      $this->initTranslations();
    }
    elseif (!empty($langcode)) {
      $this->setOriginalLanguage($langcode);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::removeTranslations()
   */
  public function removeTranslations() {
    $this->removeTranslation(NULL);
  }

  /**
   * @see EntityTranslationHandlerInterface::removeRevisionTranslations()
   */
  public function removeRevisionTranslations() {
    $translations_key = $this->getTranslationsKey();
    $keys = array_keys($this->entity->{$translations_key}->data);
    $values = array_fill(0, count($keys), array('hook' => 'delete_revision'));
    $this->removeTranslation(NULL);
    $this->entity->{$translations_key}->hook = array_combine($keys, $values);
  }

  /**
   * @see EntityTranslationHandlerInterface::initOriginalTranslation()
   */
  public function initOriginalTranslation() {
    $fixed = FALSE;
    $translations = $this->getTranslations();

    foreach (field_info_instances($this->entityType, $this->bundle) as $instance) {
      $field_name = $instance['field_name'];
      $field = field_info_field($field_name);
      $langcode = count($this->entity->{$field_name}) == 1 ? key($this->entity->{$field_name}) : $translations->original;

      if ($langcode == LANGUAGE_NONE && $field['translatable']) {
        $this->entity->{$field_name}[$translations->original] = $this->entity->{$field_name}[$langcode];
        $this->entity->{$field_name}[$langcode] = array();
        $fixed = TRUE;
      }
    }

    return $fixed;
  }

  /**
   * @see EntityTranslationHandlerInterface::getLanguage()
   */
  public function getLanguage() {
    if (!empty($this->entityInfo['entity keys']['language'])) {
      $language_key = $this->entityInfo['entity keys']['language'];
      if (!empty($this->entity->{$language_key})) {
        return $this->entity->{$language_key};
      }
    }

    $translations = $this->getTranslations();
    if (!empty($translations->original)) {
      return $translations->original;
    }
    else {
      // When we are creating an entity and no language is specified fall back
      // to the default language for the current entity and bundle.
      return $this->getDefaultLanguage();
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::getLanguageKey()
   */
  public function getLanguageKey() {
    return !empty($this->entityInfo['entity keys']['language']) ? $this->entityInfo['entity keys']['language'] : 'language';
  }

  /**
   * @see EntityTranslationHandlerInterface::getDefaultLanguage()
   */
  public function getDefaultLanguage() {
    $settings = variable_get('entity_translation_settings_' . $this->entityType . '__' . $this->bundle, array());

    if (!empty($settings['default_language'])) {
      switch ($settings['default_language']) {
        case ENTITY_TRANSLATION_LANGUAGE_DEFAULT:
          $langcode = language_default()->language;
          break;

        case ENTITY_TRANSLATION_LANGUAGE_CURRENT:
          $langcode = $GLOBALS[LANGUAGE_TYPE_CONTENT]->language;
          break;

        case ENTITY_TRANSLATION_LANGUAGE_AUTHOR:
          $langcode = $GLOBALS['user']->language;
          break;

        default:
          // An actual language code has been explicitly configured.
          $langcode = $settings['default_language'];
      }
    }
    else {
      // Fall back to the default language to keep backward compatibility.
      $langcode = language_default()->language;
    }

    return $langcode;
  }

  /**
   * @see EntityTranslationHandlerInterface::setOriginalLanguage()
   */
  public function setOriginalLanguage($langcode) {
    $translations = $this->getTranslations();

    if (!isset($translations->original) || $translations->original != $langcode) {
      if (isset($translations->original)) {
        $translations->data[$langcode] = $translations->data[$translations->original];
        $translations->data[$langcode]['language'] = $langcode;
        unset($translations->data[$translations->original]);
      }

      $translations->original = $langcode;
      $args = func_get_args();
      $this->notifyChildren(__FUNCTION__, $args);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::isTranslating()
   */
  public function isTranslating() {
    return $this->translating;
  }

  /**
   * @see EntityTranslationHandlerInterface::setTranslating()
   */
  public function setTranslating($translating) {
    $this->translating = $translating;
  }

  /**
   * @see EntityTranslationHandlerInterface::isRevision()
   */
  public function isRevision() {
    return FALSE;
  }

  /**
   * @see EntityTranslationHandlerInterface::isRevisionable()
   */
  public function isRevisionable() {
    $result = FALSE;
    if (!isset($this->revisionable)) {
      $result = self::isEntityTypeRevisionable($this->entityType);
    }
    return $result;
  }

  /**
   * Returns whether the entity type is revisionable.
   */
  public static function isEntityTypeRevisionable($entity_type) {
    $entity_info = entity_get_info($entity_type);
    return variable_get('entity_translation_revision_enabled', FALSE) && !empty($entity_info['entity keys']['revision']);
  }

  /**
   * @see EntityTranslationHandlerInterface::setEntity()
   */
  public function setEntity($entity) {
    $this->entity = $entity;

    // Ensure translations data is populated.
    $translations_key = $this->getTranslationsKey();
    if ($translations_key && !isset($this->entity->{$translations_key})) {
      $this->entity->{$translations_key} = self::emptyTranslations();
    }

    // Update entity properties.
    list($this->entityId, $this->revisionId, $this->bundle) = entity_extract_ids($this->entityType, $this->entity);

    // Initialize the handler id if needed.
    if (!empty($this->factory)) {
      $this->factory->getHandlerId($this->entityType, $entity);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::getEntity()
   */
  public function getEntity() {
    return $this->entity;
  }

  /**
   * @see EntityTranslationHandlerInterface::getEntityType()
   */
  public function getEntityType() {
    return $this->entityType;
  }

  /**
   * @see EntityTranslationHandlerInterface::isWrappedEntity()
   */
  public function isWrappedEntity($entity_type, $entity) {
    list($id,,) = entity_extract_ids($entity_type, $entity);
    return $entity_type == $this->entityType && $id == $this->entityId;
  }

  /**
   * @see EntityTranslationHandlerInterface::setOutdated()
   */
  public function setOutdated($outdated) {
    if ($outdated) {
      $translations = $this->getTranslations();
      foreach ($translations->data as $langcode => &$translation) {
        if ($langcode != $this->getFormLanguage()) {
          $translation['translate'] = 1;
        }
      }
      $args = func_get_args();
      $this->notifyChildren(__FUNCTION__, $args);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::getBasePath()
   */
  public function getBasePath() {
    return $this->basePath;
  }

  /**
   * @see EntityTranslationHandlerInterface::getEditPath()
   */
  public function getEditPath($langcode = NULL) {
    return empty($this->editPath) ? FALSE : (empty($langcode) ? $this->editPath : $this->editPath . '/' . $langcode);
  }

  /**
   * @see EntityTranslationHandlerInterface::getTranslatePath()
   */
  public function getTranslatePath() {
    return $this->translatePath;
  }

  /**
   * @see EntityTranslationHandlerInterface::getViewPath()
   */
  public function getViewPath() {
    return $this->viewPath;
  }

  /**
   * @see EntityTranslationHandlerInterface::getPathScheme()
   */
  public function getPathScheme() {
    return $this->pathScheme;
  }

  /**
   * @see EntityTranslationHandlerInterface::setPathScheme()
   */
  public function setPathScheme($scheme) {
    if ($scheme != $this->pathScheme) {
      $this->pathScheme = $scheme;
      $this->initPathVariables();
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::initPathScheme()
   */
  public function initPathScheme($path = NULL) {
    $scheme = 'default';

    // If only one path scheme is defined no need to find one.
    if (count($this->entityInfo['translation']['entity_translation']['path schemes']) > 1) {
      $item = menu_get_item($path);
      if (!empty($item['path'])) {
        $current_path_scheme = $this->findMatchingPathScheme($item['path']);
        if ($current_path_scheme) {
          $scheme = $current_path_scheme;
          $this->routerMap = $item['original_map'];
        }
      }
    }

    $this->setPathScheme($scheme);
    return $scheme;
  }

  /**
   * Find a path scheme matching the given path.
   *
   * @param $router_path
   *   The path to match against.
   *
   * @return
   *   The key of the path scheme if found, FALSE otherwise.
   */
  protected function findMatchingPathScheme($router_path) {
    $path_keys = array_flip(array('base path', 'view path', 'edit path', 'translate path'));

    foreach ($this->entityInfo['translation']['entity_translation']['path schemes'] as $delta => $scheme) {
      // Construct regular expression pattern for determining whether any path
      // in the current scheme matches the current request path.
      $path_elements = array_intersect_key($scheme, $path_keys);

      // Add additional path elements which were added during
      // entity_translation_menu_alter().
      if (isset($path_elements['edit path'])) {
        $path_elements[] = $path_elements['edit path'] . '/%entity_translation_language';
        $path_elements[] = $path_elements['edit path'] . '/add/%entity_translation_language/%entity_translation_language';
      }
      if (isset($path_elements['translate path'])) {
        $path_elements[] = $path_elements['translate path'] . '/delete/%entity_translation_language';
      }

      // Replace wildcards with % for matching parameters.
      $path_elements = array_flip(preg_replace('|%[^/]+|', '%', $path_elements));

      if (isset($path_elements[$router_path])) {
        return $delta;
      }
    }

    return FALSE;
  }

  /**
   * @see EntityTranslationHandlerInterface::getLabel()
   */
  public function getLabel() {
    if (($label = entity_label($this->entityType, $this->entity)) !== FALSE) {
      return $label;
    }
    else {
      return "{$this->entityType}:{$this->getEntityId()}" ;
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::getAccess()
   */
  public function getAccess($op) {
    return TRUE;
  }

  /**
   * @see EntityTranslationHandlerInterface::getTranslationAccess()
   */
  public function getTranslationAccess($langcode) {
    return !empty($this->entityInfo['translation']['entity_translation']['skip original values access']) || !entity_translation_workflow_enabled() || $langcode != $this->getLanguage() || user_access('edit original values') || user_access("edit {$this->entityType} original values");
  }

  /**
   * @see EntityTranslationHandlerInterface::getSharedFieldsAccess()
   */
  public function getSharedFieldsAccess() {
    $settings = entity_translation_settings($this->entityType, $this->bundle);
    return ($settings['shared_fields_original_only'] == FALSE || $this->getLanguage() == $this->getFormLanguage()) &&
      (!entity_translation_workflow_enabled() || user_access('edit translation shared fields') || user_access("edit {$this->entityType} translation shared fields"));
  }

  /**
   * @see EntityTranslationHandlerInterface::isAliasEnabled()
   */
  public function isAliasEnabled() {
    return !empty($this->entityInfo['translation']['entity_translation']['alias']);
  }

  /**
   * @see EntityTranslationHandlerInterface::setFormLanguage()
   */
  public function setFormLanguage($langcode) {
    $this->formLanguage = $langcode;
    $args = func_get_args();
    $this->notifyChildren(__FUNCTION__, $args);
  }

  /**
   * @see EntityTranslationHandlerInterface::getFormLanguage()
   */
  public function getFormLanguage() {
    return !empty($this->formLanguage) ? $this->formLanguage : $this->getLanguage();
  }

  /**
   * @see EntityTranslationHandlerInterface::setSourceLanguage()
   */
  public function setSourceLanguage($langcode) {
    $this->sourceLanguage = $langcode;
    $args = func_get_args();
    $this->notifyChildren(__FUNCTION__, $args);
  }

  /**
   * @see EntityTranslationHandlerInterface::getSourceLanguage()
   */
  public function getSourceLanguage() {
    return $this->sourceLanguage;
  }

  /**
   * @see EntityTranslationHandlerInterface::isNewEntity()
   */
  public function isNewEntity() {
    $id = $this->getEntityId();
    return empty($id);
  }

  /**
   * @see EntityTranslationHandlerInterface::isEntityForm()
   */
  public function isEntityForm() {
    return $this->entityForm;
  }

  /**
   * @see EntityTranslationHandlerInterface::entityForm()
   */
  public function entityForm(&$form, &$form_state) {
    $this->entityForm = TRUE;
    $translations = $this->getTranslations();
    $form_langcode = $this->getFormLanguage();
    $langcode = $this->getLanguage();
    $is_translation = $this->isTranslationForm();
    $new_translation = !isset($translations->data[$form_langcode]);
    $no_translations = count($translations->data) < 2;
    $languages = language_list();
    $access = user_access('translate any entity') || user_access("translate $this->entityType entities");

    // Store contextual information in the form state.
    $form_state['entity_translation']['form_langcode'] = $form_langcode;
    $form_state['entity_translation']['source_langcode'] = $this->getSourceLanguage();
    // The only way to determine whether we are editing the original values is
    // comparing form language and entity language. Since a language change
    // might render impossible to make this check after form submission, we
    // store the related information here.
    $form_state['entity_translation']['is_translation'] = $is_translation;


    // Adjust page title to specify the current language being edited, if we
    // have at least one translation.
    if ($form_langcode != LANGUAGE_NONE && (!$no_translations || $new_translation)) {
      drupal_set_title($this->entityFormTitle() . ' [' . t($languages[$form_langcode]->name) . ']', PASS_THROUGH);
    }

    // Display source language selector only if we are creating a new
    // translation and there are at least two translations available.
    if (!$no_translations && $new_translation) {
      $form['source_language'] = array(
        '#type' => 'fieldset',
        '#title' => t('Source language'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
        '#weight' => -100,
        '#access' => $access,
        '#multilingual' => TRUE,
        'language' => array(
          '#type' => 'select',
          '#default_value' => $this->getSourceLanguage(),
          '#options' => array(),
        ),
        'submit' => array(
          '#type' => 'submit',
          '#value' => t('Change'),
          '#submit' => array('entity_translation_entity_form_source_language_submit'),
        ),
      );
      foreach (language_list() as $language) {
        if (isset($translations->data[$language->language])) {
          $form['source_language']['language']['#options'][$language->language] = t($language->name);
        }
      }
    }

    // Add the entity language switcher.
    $this->entityFormLanguageWidget($form, $form_state);

    if ($is_translation && isset($form['actions']['delete'])) {
      // Replace the delete button with the delete translation one.
      if (!$new_translation) {
        $weight = 100;
        foreach (array('delete', 'submit') as $key) {
          if (isset($form['actions'][$key]['weight'])) {
            $weight = $form['actions'][$key]['weight'];
            break;
          }
        }
        $form['actions']['delete_translation'] = array(
          '#type' => 'submit',
          '#value' => t('Delete translation'),
          '#weight' => $weight,
          '#submit' => array('entity_translation_entity_form_delete_translation_submit'),
        );
      }

      // Always remove the delete button on translation forms.
      unset($form['actions']['delete']);
    }

    // We need to display the translation tab only when there is at least one
    // translation available or a new one is about to be created.
    if ($new_translation || count($translations->data) > 1) {
      $form['translation'] = array(
        '#type' => 'fieldset',
        '#title' => t('Translation'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
        '#weight' => 10,
        '#access' => $access,
        '#multilingual' => TRUE,
      );

      // A new translation is enabled by default.
      $status = $new_translation || $translations->data[$form_langcode]['status'];
      // If there is only one published translation we cannot unpublish it,
      // since there would be no content left to display. The whole entity
      // should be unpublished instead, where possible.
      $enabled = !$status;
      if (!empty($status)) {
        // A new translation is not available in the translation data hence it
        // should count as one more.
        $published = $new_translation;
        foreach ($translations->data as $langcode => $translation) {
          $published += $translation['status'];
        }
        $enabled = $published > 1;
      }
      $description = $enabled ?
        t('An unpublished translation will not be visible for non-administrators.') :
        t('Only this translation is published. You must publish at least one more translation to unpublish this one.');

      $form['translation']['status'] = array(
        '#type' => 'checkbox',
        '#title' => t('This translation is published'),
        '#default_value' => $status,
        '#description' => $description,
        '#disabled' => !$enabled,
      );

      $translate = !$new_translation && $translations->data[$form_langcode]['translate'];
      if (!$translate) {
        $form['translation']['retranslate'] = array(
          '#type' => 'checkbox',
          '#title' => t('Flag translations as outdated'),
          '#default_value' => 0,
          '#description' => t('If you made a significant change, which means translations should be updated, you can flag all translations of this post as outdated. This will not change any other property of those posts, like whether they are published or not.'),
        );
      }
      else {
        $form['translation']['translate'] = array(
          '#type' => 'checkbox',
          '#title' => t('This translation needs to be updated'),
          '#default_value' => $translate,
          '#description' => t('When this option is checked, this translation needs to be updated because the source post has changed. Uncheck when the translation is up to date again.'),
          '#disabled' => !$translate,
        );
      }

      $translation_author = $new_translation ? $GLOBALS['user'] : user_load($translations->data[$form_langcode]['uid']);
      $name = isset($translation_author->name) ? $translation_author->name : '';
      $form['translation']['name'] = array(
        '#type' => 'textfield',
        '#title' => t('Authored by'),
        '#maxlength' => 60,
        '#autocomplete_path' => 'user/autocomplete',
        '#default_value' => $name,
        '#description' => t('Leave blank for %anonymous.', array('%anonymous' => variable_get('anonymous', t('Anonymous')))),
      );

      $date = $new_translation ? REQUEST_TIME : $translations->data[$form_langcode]['created'];
      $form['translation']['created'] = array(
        '#type' => 'textfield',
        '#title' => t('Authored on'),
        '#maxlength' => 25,
        '#description' => t('Format: %time. The date format is YYYY-MM-DD and %timezone is the time zone offset from UTC. Leave blank to use the time of form submission.', array('%time' => format_date($date, 'custom', 'Y-m-d H:i:s O'), '%timezone' => format_date($date, 'custom', 'O'))),
        '#default_value' => $new_translation ? '' : format_date($date, 'custom', 'Y-m-d H:i:s O'),
      );
    }

    // If Menu translation is available translate the menu strings.
    if (module_exists('entity_translation_i18n_menu')) {
      $this->menuForm($form, $form_state);
    }

    // Process entity form submission.
    $form['#submit'][] = 'entity_translation_entity_form_submit';

    // This allows to intercept deletions. The check is needed because
    // action-specific submit handlers prevent global ones from being called.
    if (!empty($form['actions']['delete']['#submit'])) {
      $form['actions']['delete']['#submit'][] = 'entity_translation_entity_form_submit';
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::entityFormSharedElements()
   *
   * Either remove access or add a translatability clue depending on the current
   * user's "edit translation shared fields" permissions.
   */
  public function entityFormSharedElements(&$element, $access = NULL) {
    static $ignored_types, $shared_labels;

    if (!isset($ignored_types)) {
      $ignored_types = array_flip(array('actions', 'value', 'hidden', 'vertical_tabs', 'token'));
    }
    if (!isset($shared_labels)) {
      $shared_labels = variable_get('entity_translation_shared_labels', TRUE);
    }
    if (!isset($access)) {
      $access = $this->getSharedFieldsAccess();
    }

    foreach (element_children($element) as $key) {
      if (!isset($element[$key]['#type'])) {
        $this->entityFormSharedElements($element[$key], $access);
      }
      else {
        // Ignore non-widget form elements.
        if (isset($ignored_types[$element[$key]['#type']])) {
          continue;
        }
        // Elements are considered to be non multilingual by default.
        // Update #access only if it has not been set already or if we have
        // explicit mlutilingual support.
        if (!isset($element[$key]['#access']) || isset($element[$key]['#multilingual'])) {
          $element[$key]['#access'] = (!isset($element[$key]['#access']) || $element[$key]['#access']) && ($access || !empty($element[$key]['#multilingual']));
        }
        // Add translatability clue for visible elements.
        if ($access && $shared_labels) {
          _entity_translation_element_add_callback($element[$key], '#process', 'entity_translation_element_translatability_clue');
        }
      }
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::entityFormLanguageWidget()
   */
  public function entityFormLanguageWidget(&$form, &$form_state) {
    if (entity_translation_enabled($this->entityType, $this->bundle)) {
      $is_new = $this->isNewEntity();
      $is_translation = !empty($form_state['entity_translation']['is_translation']);
      $translations = $this->getTranslations();
      $settings = entity_translation_settings($this->entityType, $this->bundle);
      $languages = entity_translation_languages($this->entityType, $this->entity);
      $options = count($translations->data) > 1 || !empty($settings['exclude_language_none']) ? array() : array(LANGUAGE_NONE => t('Language neutral'));

      foreach ($languages as $langcode => $language) {
        // Disable languages for existing translations, so it is not possible to
        // switch this entity to some language which is already in the
        // translation set.
        if (!isset($translations->data[$langcode]) || empty($translations->data[$langcode]['source'])) {
          $options[$langcode] = t($language->name);
        }
      }

      $langcode = $is_new ? $this->getDefaultLanguage() : $this->getLanguage();
      $language_key = $this->getLanguageKey();

      $form[$language_key] = array(
        '#type' => 'select',
        '#title' => t('Language'),
        '#default_value' => $langcode,
        '#options' => $options,
        '#access' => empty($settings['hide_language_selector']),
        '#disabled' => $is_translation || (!$is_new && !empty($settings['lock_language'])),
        '#multilingual' => TRUE,
      );

      if ($is_translation) {
        // @todo Consider supporting the ability to change translation language.
        $form[$language_key]['#title'] = t('Original language');
      }
    }

    // Prepend an empty form element to the form array so that we can update the
    // form language before any other form handler has been called.
    $form = array(
      'entity_translation_entity_form_language_update' => array(
        '#element_validate' => array('entity_translation_entity_form_language_update'),
        '#entity_type' => $this->entityType,
       ),
    ) + $form;
  }

  /**
   * Performs alterations to the menu widget if available.
   */
  protected function menuForm(&$form, &$form_state) {
    // By default do nothing: only nodes are currently supported.
  }

  /**
   * @see EntityTranslationHandlerInterface::entityFormValidate()
   */
  public function entityFormValidate($form, &$form_state) {
    if (!empty($form_state['values']['translation'])) {
      $values = $form_state['values']['translation'];
      // Validate the "authored by" field.
      if (!empty($values['name']) && !($account = user_load_by_name($values['name']))) {
        form_set_error('translation][name', t('The translation authoring username %name does not exist.', array('%name' => $values['name'])));
      }
      // Validate the "authored on" field.
      if (!empty($values['created']) && strtotime($values['created']) === FALSE) {
        form_set_error('translation][created', t('You have to specify a valid translation authoring date.'));
      }
    }
  }

  /**
   * Update the current form language based on the submitted value.
   */
  protected function updateFormLanguage($form_state) {
    // Update the form language as it might have changed. We exploit the
    // validation phase to be sure to act as early as possible.
    $language_key = $this->getLanguageKey();
    if (isset($form_state['values'][$language_key]) && !$this->isTranslationForm()) {
      $langcode = $form_state['values'][$language_key];
      $this->setFormLanguage($langcode);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::entityFormLanguageWidgetSubmit()
   */
  public function entityFormLanguageWidgetSubmit($form, &$form_state) {
    if (!entity_translation_enabled($this->entityType, $this->bundle)) {
      return;
    }

    $this->updateFormLanguage($form_state);
    $form_langcode = $this->getFormLanguage();

    foreach (field_info_instances($this->entityType, $this->bundle) as $instance) {
      $field_name = $instance['field_name'];
      if (isset($form[$field_name]['#language'])) {
        $field = field_info_field($field_name);
        $previous_langcode = $form[$field_name]['#language'];

        // Handle a possible language change: new language values are inserted,
        // previous ones are deleted.
        if ($field['translatable'] && $previous_langcode != $form_langcode && isset($form_state['values'][$field_name][$previous_langcode])) {
          $form_state['values'][$field_name][$form_langcode] = $form_state['values'][$field_name][$previous_langcode];
          $form_state['values'][$field_name][$previous_langcode] = array();
        }
      }
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::entityFormSubmit()
   */
  public function entityFormSubmit($form, &$form_state) {
    $form_langcode = $this->getFormLanguage();
    $translations = $this->getTranslations();
    $is_translation = !empty($form_state['entity_translation']['is_translation']);
    $new_translation = !isset($translations->data[$form_langcode]);
    $values = isset($form_state['values']['translation']) ? $form_state['values']['translation'] : array();

    // Ensure every key has at least a default value. Subclasses may provide use
    // entity-specific values to alter them.
    $values += array(
      'status' => TRUE,
      'retranslate' => 0,
      'name' => isset($GLOBALS['user']->name) ? $GLOBALS['user']->name : '',
    );

    if (!isset($translations->data[$form_langcode])) {
      // If we have a new translation the language is the original entity
      // language.
      $translation = $is_translation ? array('language' => $form_langcode, 'source' => $this->getSourceLanguage()) : array('language' => $form_langcode, 'source' => '');
    }
    else {
      $translation = $translations->data[$form_langcode];
    }

    if (isset($values['translate'])) {
      $translation['translate'] = intval($values['translate']);
    }
    else {
      $this->setOutdated($values['retranslate']);
    }

    // Handle possible language changes for the original values.
    if (!$is_translation) {
      $this->setOriginalLanguage($form_langcode);
    }

    $translation['status'] = intval($values['status']);
    $translation['uid'] = $values['name'] ? user_load_by_name($values['name'])->uid : 0;
    $translation['created'] = empty($values['created']) ? REQUEST_TIME : strtotime($values['created']);
    $this->setTranslation($translation);

    // If no redirect has been explicitly set, go to the edit form for the
    // current form language.
    if ($new_translation && empty($form_state['redirect']) && !$this->isNewEntity()) {
      $form_state['redirect'] = $this->getEditPath($form_langcode);
    }
  }

  /**
   * @see EntityTranslationHandlerInterface::localTasksAlter()
   */
  public function localTasksAlter(&$data, $router_item, $root_path) {
    $translations = $this->getTranslations();

    if (count($translations->data) > 0) {
      $languages = language_list();
      $form_langcode = $this->getFormLanguage();
      $language_tabs = array();

      if ($this->getSourceLanguage()) {
        foreach ($data['tabs'][1]['output'] as $index => &$add_tab) {
          if ($add_tab['#link']['path'] == $root_path) {
            $add_tab['#link']['title'] = $languages[$form_langcode]->name;
            $add_tab['#link']['weight'] = $languages[$form_langcode]->weight;
            $add_tab['#active'] = TRUE;
            $add_tab['#language_tab'] = TRUE;
            $language_tabs[] = $add_tab;
            unset($data['tabs'][1]['output'][$index]);
            break;
          }
        }
      }

      foreach ($translations->data as $langcode => $translation) {
        if ($this->getTranslationAccess($langcode)) {
          $links = $this->languageSwitchLinks($this->getEditPath($langcode));
          $link = !empty($links->links[$langcode]) ? $links->links[$langcode] : array();

          if (isset($link['href'])) {
            $tab = array();
            $tab['#theme'] = 'menu_local_task';
            $tab['#active'] = $langcode == $form_langcode;
            $tab['#language_tab'] = TRUE;
            $tab['#link'] = array(
              'href' => $link['href'],
              'title' => t($languages[$langcode]->name),
              'weight' => $languages[$langcode]->weight,
              'localized_options' => $link,
            ) + $router_item;
            $language_tabs[] = $tab;
          }
        }
      }

      // Reorder tabs to make the add tab respect language weights.
      usort($language_tabs, array($this, 'translationTabSort'));

      // Merge the reordered language tabs into the second level tabs.
      if (count($language_tabs) > 1) {
        if (empty($data['tabs'][1])) {
          $data['tabs'][1] = array('output' => array());
        }
        $data['tabs'][1]['output'] = array_merge($data['tabs'][1]['output'], $language_tabs);
        $data['tabs'][1]['count'] = count($data['tabs'][1]['output']);
      }
    }
  }

  /**
   * Helper callback. Sorts language tabs by weight.
   */
  protected function translationTabSort($a, $b) {
    return $a['#link']['weight'] > $b['#link']['weight'];
  }

  /**
   * Returns the title to be used for the entity form page.
   */
  protected function entityFormTitle() {
    return $this->getLabel();
  }

  /**
   * Returns TRUE if an entity translation is being edited.
   */
  protected function isTranslationForm() {
    return !$this->isNewEntity() && $this->getFormLanguage() != $this->getLanguage();
  }

  /**
   * Returns the translation object key for the wrapped entity type.
   */
  protected function getTranslationsKey() {
    return isset($this->entityInfo['entity keys']['translations']) ? $this->entityInfo['entity keys']['translations'] : FALSE;
  }

  /**
   * Returns the entity accessibility.
   */
  protected function getStatus() {
    return TRUE;
  }

  /**
   * Returns the entity identifier.
   */
  protected function getEntityId() {
    return $this->entityId;
  }

  /**
   * Initializes handler path variables based on the active path scheme.
   *
   * @throws Exception
   */
  private function initPathVariables() {
    if (empty($this->pathScheme) || !isset($this->entityInfo['translation']['entity_translation']['path schemes'][$this->pathScheme])) {
      throw new Exception("Cannot initialize entity translation path variables (invalid path scheme).");
    }

    $path_scheme = $this->entityInfo['translation']['entity_translation']['path schemes'][$this->pathScheme];
    $this->pathWildcard = $path_scheme['path wildcard'];
    $this->basePath = isset($path_scheme['base path']) ? $this->getPathInstance($path_scheme['base path']) : FALSE;
    $this->editPath = isset($path_scheme['edit path']) ? $this->getPathInstance($path_scheme['edit path']) : FALSE;
    $this->translatePath = isset($path_scheme['translate path']) ? $this->getPathInstance($path_scheme['translate path']) : FALSE;
    $this->viewPath = isset($path_scheme['view path']) ? $this->getPathInstance($path_scheme['view path']) : FALSE;
  }

  /**
   * Returns an instance of the given path.
   *
   * @param $path
   *   An internal path containing the entity id wildcard.
   *
   * @return
   *   The instantiated path.
   */
  protected function getPathInstance($path) {
    $path_segments = explode('/', $path);

    foreach ($path_segments as $index => $segment) {
      if ($segment == $this->pathWildcard) {
        $path_segments[$index] = $this->getEntityId();
      }
      elseif ($segment{0} == '%' && isset($this->routerMap[$index])) {
        $path_segments[$index] = $this->routerMap[$index];
      }
    }

    return implode('/', $path_segments);
  }

  /**
   * Returns an empty translations data structure.
   */
  protected static function emptyTranslations() {
    return (object) array('original' => NULL, 'data' => array());
  }
}
