<?php

/**
 * @file
 * Contains SearchApiServer.
 */

/**
 * Class representing a search server.
 *
 * This can handle the same calls as defined in the SearchApiServiceInterface
 * and pass it on to the service implementation appropriate for this server.
 */
class SearchApiServer extends Entity {

  /* Database values that will be set when object is loaded: */

  /**
   * The primary identifier for a server.
   *
   * @var integer
   */
  public $id = 0;

  /**
   * The displayed name for a server.
   *
   * @var string
   */
  public $name = '';

  /**
   * The machine name for a server.
   *
   * @var string
   */
  public $machine_name = '';

  /**
   * The displayed description for a server.
   *
   * @var string
   */
  public $description = '';

  /**
   * The id of the service class to use for this server.
   *
   * @var string
   */
  public $class = '';

  /**
   * The options used to configure the service object.
   *
   * @var array
   */
  public $options = array();

  /**
   * A flag indicating whether the server is enabled.
   *
   * @var integer
   */
  public $enabled = 1;

  /**
   * Proxy object for invoking service methods.
   *
   * @var SearchApiServiceInterface
   */
  protected $proxy;

  /**
   * Constructor as a helper to the parent constructor.
   */
  public function __construct(array $values = array(), $entity_type = 'search_api_server') {
    parent::__construct($values, $entity_type);
  }

  /**
   * Helper method for updating entity properties.
   *
   * NOTE: You shouldn't change any properties of this object before calling
   * this method, as this might lead to the fields not being saved correctly.
   *
   * @param array $fields
   *   The new field values.
   *
   * @return int|false
   *   SAVE_UPDATED on success, FALSE on failure, 0 if the fields already had
   *   the specified values.
   */
  public function update(array $fields) {
    $changeable = array('name' => 1, 'enabled' => 1, 'description' => 1, 'options' => 1);
    $changed = FALSE;
    foreach ($fields as $field => $value) {
      if (isset($changeable[$field]) && $value !== $this->$field) {
        $this->$field = $value;
        $changed = TRUE;
      }
    }
    // If there are no new values, just return 0.
    if (!$changed) {
      return 0;
    }
    return $this->save();
  }

  /**
   * Magic method for determining which fields should be serialized.
   *
   * Serialize all properties except the proxy object.
   *
   * @return array
   *   An array of properties to be serialized.
   */
  public function __sleep() {
    $ret = get_object_vars($this);
    unset($ret['proxy'], $ret['status'], $ret['module'], $ret['is_new']);
    return array_keys($ret);
  }

  /**
   * Helper method for ensuring the proxy object is set up.
   */
  protected function ensureProxy() {
    if (!isset($this->proxy)) {
      $class = search_api_get_service_info($this->class);
      if ($class && class_exists($class['class'])) {
        if (empty($this->options)) {
          // We always have to provide the options.
          $this->options = array();
        }
        $this->proxy = new $class['class']($this);
      }
      if (!($this->proxy instanceof SearchApiServiceInterface)) {
        throw new SearchApiException(t('Search server with machine name @name specifies illegal service class @class.', array('@name' => $this->machine_name, '@class' => $this->class)));
      }
    }
  }

  /**
   * Reacts to calls of undefined methods on this object.
   *
   * If the service class defines additional methods, not specified in the
   * SearchApiServiceInterface interface, then they are called via this magic
   * method.
   */
  public function __call($name, $arguments = array()) {
    $this->ensureProxy();
    return call_user_func_array(array($this->proxy, $name), $arguments);
  }

  // Proxy methods

  // For increased clarity, and since some parameters are passed by reference,
  // we don't use the __call() magic method for those. This also gives us the
  // opportunity to do additional error handling.

  /**
   * Form constructor for the server configuration form.
   *
   * @see SearchApiServiceInterface::configurationForm()
   */
  public function configurationForm(array $form, array &$form_state) {
    $this->ensureProxy();
    return $this->proxy->configurationForm($form, $form_state);
  }

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * @see SearchApiServiceInterface::configurationFormValidate()
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    $this->ensureProxy();
    return $this->proxy->configurationFormValidate($form, $values, $form_state);
  }

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * @see SearchApiServiceInterface::configurationFormSubmit()
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $this->ensureProxy();
    return $this->proxy->configurationFormSubmit($form, $values, $form_state);
  }

  /**
   * Determines whether this service class supports a given feature.
   *
   * @see SearchApiServiceInterface::supportsFeature()
   */
  public function supportsFeature($feature) {
    $this->ensureProxy();
    return $this->proxy->supportsFeature($feature);
  }

  /**
   * Displays this server's settings.
   *
   * @see SearchApiServiceInterface::viewSettings()
   */
  public function viewSettings() {
    $this->ensureProxy();
    return $this->proxy->viewSettings();
  }

  /**
   * Reacts to the server's creation.
   *
   * @see SearchApiServiceInterface::postCreate()
   */
  public function postCreate() {
    $this->ensureProxy();
    return $this->proxy->postCreate();
  }

  /**
   * Notifies this server that its fields are about to be updated.
   *
   * @see SearchApiServiceInterface::postUpdate()
   */
  public function postUpdate() {
    $this->ensureProxy();
    return $this->proxy->postUpdate();
  }

  /**
   * Notifies this server that it is about to be deleted from the database.
   *
   * @see SearchApiServiceInterface::preDelete()
   */
  public function preDelete() {
    $this->ensureProxy();
    return $this->proxy->preDelete();
  }

  /**
   * Adds a new index to this server.
   *
   * If an exception in the service class implementation of this method occcurs,
   * it will be caught and the operation saved as an pending server task.
   *
   * @see SearchApiServiceInterface::addIndex()
   * @see search_api_server_tasks_add()
   */
  public function addIndex(SearchApiIndex $index) {
    $this->ensureProxy();
    try {
      $this->proxy->addIndex($index);
    }
    catch (SearchApiException $e) {
      $vars = array(
        '%server' => $this->name,
        '%index' => $index->name,
      );
      watchdog_exception('search_api', $e, '%type while adding index %index to server %server: !message in %function (line %line of %file).', $vars);
      search_api_server_tasks_add($this, __FUNCTION__, $index);
    }
  }

  /**
   * Notifies the server that the field settings for the index have changed.
   *
   * If the service class implementation of the method returns TRUE, this will
   * automatically take care of marking the items on the index for re-indexing.
   *
   * If an exception in the service class implementation of this method occcurs,
   * it will be caught and the operation saved as an pending server task.
   *
   * @see SearchApiServiceInterface::fieldsUpdated()
   * @see search_api_server_tasks_add()
   */
  public function fieldsUpdated(SearchApiIndex $index) {
    $this->ensureProxy();
    try {
      if ($this->proxy->fieldsUpdated($index)) {
        _search_api_index_reindex($index);
        return TRUE;
      }
    }
    catch (SearchApiException $e) {
      $vars = array(
        '%server' => $this->name,
        '%index' => $index->name,
      );
      watchdog_exception('search_api', $e, '%type while updating the fields of index %index on server %server: !message in %function (line %line of %file).', $vars);
      search_api_server_tasks_add($this, __FUNCTION__, $index, isset($index->original) ? $index->original : NULL);
    }
    return FALSE;
  }

  /**
   * Removes an index from this server.
   *
   * If an exception in the service class implementation of this method occcurs,
   * it will be caught and the operation saved as an pending server task.
   *
   * @see SearchApiServiceInterface::removeIndex()
   * @see search_api_server_tasks_add()
   */
  public function removeIndex($index) {
    // When removing an index from a server, it doesn't make any sense anymore to
    // delete items from it, or react to other changes.
    search_api_server_tasks_delete(NULL, $this, $index);

    $this->ensureProxy();
    try {
      $this->proxy->removeIndex($index);
    }
    catch (SearchApiException $e) {
      $vars = array(
        '%server' => $this->name,
        '%index' => is_object($index) ? $index->name : $index,
      );
      watchdog_exception('search_api', $e, '%type while removing index %index from server %server: !message in %function (line %line of %file).', $vars);
      search_api_server_tasks_add($this, __FUNCTION__, $index);
    }
  }

  /**
   * Indexes the specified items.
   *
   * @see SearchApiServiceInterface::indexItems()
   */
  public function indexItems(SearchApiIndex $index, array $items) {
    $this->ensureProxy();
    return $this->proxy->indexItems($index, $items);
  }

  /**
   * Deletes indexed items from this server.
   *
   * If an exception in the service class implementation of this method occcurs,
   * it will be caught and the operation saved as an pending server task.
   *
   * @see SearchApiServiceInterface::deleteItems()
   * @see search_api_server_tasks_add()
   */
  public function deleteItems($ids = 'all', SearchApiIndex $index = NULL) {
    $this->ensureProxy();
    try {
      $this->proxy->deleteItems($ids, $index);
    }
    catch (SearchApiException $e) {
      $vars = array(
        '%server' => $this->name,
      );
      watchdog_exception('search_api', $e, '%type while deleting items from server %server: !message in %function (line %line of %file).', $vars);
      search_api_server_tasks_add($this, __FUNCTION__, $index, $ids);
    }
  }

  /**
   * Creates a query object for searching on an index lying on this server.
   *
   * @see SearchApiServiceInterface::query()
   */
  public function query(SearchApiIndex $index, $options = array()) {
    $this->ensureProxy();
    return $this->proxy->query($index, $options);
  }

  /**
   * Executes a search on the server represented by this object.
   *
   * @see SearchApiServiceInterface::search()
   */
  public function search(SearchApiQueryInterface $query) {
    $this->ensureProxy();
    return $this->proxy->search($query);
  }

  /**
   * Retrieves additional information for the server, if available.
   *
   * Retrieving such information is only supported if the service class supports
   * the "search_api_service_extra" feature.
   *
   * @return array
   *   An array containing additional, service class-specific information about
   *   the server.
   *
   * @see SearchApiAbstractService::getExtraInformation()
   */
  public function getExtraInformation() {
    if ($this->proxy->supportsFeature('search_api_service_extra')) {
      return $this->proxy->getExtraInformation();
    }
    return array();
  }

}
