<?php

/**
 * @file
 * Contains SearchApiServiceInterface and SearchApiAbstractService.
 */

/**
 * Interface defining the methods search services have to implement.
 *
 * Before a service object is used, the corresponding server's data will be read
 * from the database (see SearchApiAbstractService for a list of fields).
 *
 * Most methods in this interface (where any change in data occurs) can throw a
 * SearchApiException. The server entity class SearchApiServer catches these
 * exceptions and uses the server tasks system to assure that the action is
 * later retried.
 */
interface SearchApiServiceInterface {

  /**
   * Constructs a service object.
   *
   * This will set the server configuration used with this service.
   *
   * @param SearchApiServer $server
   *   The server object for this service.
   */
  public function __construct(SearchApiServer $server);

  /**
   * Form constructor for the server configuration form.
   *
   * Might be called with an incomplete server (no ID). In this case, the form
   * is displayed for the initial creation of the server.
   *
   * @param array $form
   *   The server options part of the form.
   * @param array $form_state
   *   The current form state.
   *
   * @return array
   *   A form array for setting service-specific options.
   */
  public function configurationForm(array $form, array &$form_state);

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * $form_state['server'] will contain the server that is created or edited.
   * Use form_error() to flag errors on form elements.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state);

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * This method should set the options of this service' server according to
   * $values.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state);

  /**
   * Determines whether this service class supports a given feature.
   *
   * Features are optional extensions to Search API functionality and usually
   * defined and used by third-party modules.
   *
   * There are currently three features defined directly in the Search API
   * project:
   * - "search_api_facets", by the search_api_facetapi module.
   * - "search_api_facets_operator_or", also by the search_api_facetapi module.
   * - "search_api_mlt", by the search_api_views module.
   * Other contrib modules might define additional features. These should always
   * be properly documented in the module by which they are defined.
   *
   * @param string $feature
   *   The name of the optional feature.
   *
   * @return bool
   *   TRUE if this service knows and supports the specified feature. FALSE
   *   otherwise.
   */
  public function supportsFeature($feature);

  /**
   * Displays this server's settings.
   *
   * Output can be HTML or a render array, a <dl> listing all relevant settings
   * is preferred.
   */
  public function viewSettings();

  /**
   * Reacts to the server's creation.
   *
   * Called once, when the server is first created. Allows it to set up its
   * necessary infrastructure.
   */
  public function postCreate();

  /**
   * Notifies this server that its fields are about to be updated.
   *
   * The server's $original property can be used to inspect the old property
   * values.
   *
   * @return bool
   *   TRUE, if the update requires reindexing of all content on the server.
   */
  public function postUpdate();

  /**
   * Notifies this server that it is about to be deleted from the database.
   *
   * This should execute any necessary cleanup operations.
   *
   * Note that you shouldn't call the server's save() method, or any
   * methods that might do that, from inside of this method as the server isn't
   * present in the database anymore at this point.
   */
  public function preDelete();

  /**
   * Adds a new index to this server.
   *
   * If the index was already added to the server, the object should treat this
   * as if removeIndex() and then addIndex() were called.
   *
   * @param SearchApiIndex $index
   *   The index to add.
   *
   * @throws SearchApiException
   *   If an error occurred while adding the index.
   */
  public function addIndex(SearchApiIndex $index);

  /**
   * Notifies the server that the field settings for the index have changed.
   *
   * If any user action is necessary as a result of this, the method should
   * use drupal_set_message() to notify the user.
   *
   * @param SearchApiIndex $index
   *   The updated index.
   *
   * @return bool
   *   TRUE, if this change affected the server in any way that forces it to
   *   re-index the content. FALSE otherwise.
   *
   * @throws SearchApiException
   *   If an error occurred while reacting to the change of fields.
   */
  public function fieldsUpdated(SearchApiIndex $index);

  /**
   * Removes an index from this server.
   *
   * This might mean that the index has been deleted, or reassigned to a
   * different server. If you need to distinguish between these cases, inspect
   * $index->server.
   *
   * If the index wasn't added to the server, the method call should be ignored.
   *
   * Implementations of this method should also check whether $index->read_only
   * is set, and don't delete any indexed data if it is.
   *
   * @param $index
   *   Either an object representing the index to remove, or its machine name
   *   (if the index was completely deleted).
   *
   * @throws SearchApiException
   *   If an error occurred while removing the index.
   */
  public function removeIndex($index);

  /**
   * Indexes the specified items.
   *
   * @param SearchApiIndex $index
   *   The search index for which items should be indexed.
   * @param array $items
   *   An array of items to be indexed, keyed by their id. The values are
   *   associative arrays of the fields to be stored, where each field is an
   *   array with the following keys:
   *   - type: One of the data types recognized by the Search API, or the
   *     special type "tokens" for fulltext fields.
   *   - original_type: The original type of the property, as defined by the
   *     datasource controller for the index's item type.
   *   - value: The value to index.
   *
   *   The special field "search_api_language" contains the item's language and
   *   should always be indexed.
   *
   *   The value of fields with the "tokens" type is an array of tokens. Each
   *   token is an array containing the following keys:
   *   - value: The word that the token represents.
   *   - score: A score for the importance of that word.
   *
   * @return array
   *   An array of the ids of all items that were successfully indexed.
   *
   * @throws SearchApiException
   *   If indexing was prevented by a fundamental configuration error.
   */
  public function indexItems(SearchApiIndex $index, array $items);

  /**
   * Deletes indexed items from this server.
   *
   * Might be either used to delete some items (given by their ids) from a
   * specified index, or all items from that index, or all items from all
   * indexes on this server.
   *
   * @param $ids
   *   Either an array containing the ids of the items that should be deleted,
   *   or 'all' if all items should be deleted. Other formats might be
   *   recognized by implementing classes, but these are not standardized.
   * @param SearchApiIndex $index
   *   The index from which items should be deleted, or NULL if all indexes on
   *   this server should be cleared (then, $ids has to be 'all').
   *
   * @throws SearchApiException
   *   If an error occurred while trying to delete the items.
   */
  public function deleteItems($ids = 'all', SearchApiIndex $index = NULL);

  /**
   * Creates a query object for searching on an index lying on this server.
   *
   * @param SearchApiIndex $index
   *   The index to search on.
   * @param $options
   *   Associative array of options configuring this query. See
   *   SearchApiQueryInterface::__construct().
   *
   * @return SearchApiQueryInterface
   *   An object for searching the given index.
   *
   * @throws SearchApiException
   *   If the server is currently disabled.
   */
  public function query(SearchApiIndex $index, $options = array());

  /**
   * Executes a search on the server represented by this object.
   *
   * @param $query
   *   The SearchApiQueryInterface object to execute.
   *
   * @return array
   *   An associative array containing the search results, as required by
   *   SearchApiQueryInterface::execute().
   *
   * @throws SearchApiException
   *   If an error prevented the search from completing.
   */
  public function search(SearchApiQueryInterface $query);

}

/**
 * Abstract class with generic implementation of most service methods.
 *
 * For creating your own service class extending this class, you only need to
 * implement indexItems(), deleteItems() and search() from the
 * SearchApiServiceInterface interface.
 */
abstract class SearchApiAbstractService implements SearchApiServiceInterface {

  /**
   * @var SearchApiServer
   */
  protected $server;

  /**
   * Direct reference to the server's $options property.
   *
   * @var array
   */
  protected $options = array();

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation sets $this->server and $this->options.
   */
  public function __construct(SearchApiServer $server) {
    $this->server = $server;
    $this->options = &$server->options;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * Returns an empty form by default.
   */
  public function configurationForm(array $form, array &$form_state) {
    return array();
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * Does nothing by default.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) {
    return;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation just ensures that additional elements in
   * $options, not present in the form, don't get lost at the update.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    if (!empty($this->options)) {
      $values += $this->options;
    }
    $this->options = $values;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation always returns FALSE.
   */
  public function supportsFeature($feature) {
    return FALSE;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation does a crude output as a definition list, with
   * option names taken from the configuration form.
   */
  public function viewSettings() {
    $output = '';
    $form = $form_state = array();
    $option_form = $this->configurationForm($form, $form_state);
    $option_names = array();
    foreach ($option_form as $key => $element) {
      if (isset($element['#title']) && isset($this->options[$key])) {
        $option_names[$key] = $element['#title'];
      }
    }

    foreach ($option_names as $key => $name) {
      $value = $this->options[$key];
      $output .= '<dt>' . check_plain($name) . '</dt>' . "\n";
      $output .= '<dd>' . nl2br(check_plain(print_r($value, TRUE))) . '</dd>' . "\n";
    }

    return $output ? "<dl>\n$output</dl>" : '';
  }

  /**
   * Returns additional, service-specific information about this server.
   *
   * If a service class implements this method and supports the
   * "search_api_service_extra" option, this method will be used to add extra
   * information to the server's "View" tab.
   *
   * In the default theme implementation this data will be output in a table
   * with two columns along with other, generic information about the server.
   *
   * @return array
   *   An array of additional server information, with each piece of information
   *   being an associative array with the following keys:
   *   - label: The human-readable label for this data.
   *   - info: The information, as HTML.
   *   - status: (optional) The status associated with this information. One of
   *     "info", "ok", "warning" or "error". Defaults to "info".
   *
   * @see supportsFeature()
   */
  public function getExtraInformation() {
    return array();
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * Does nothing, by default.
   */
  public function postCreate() {
    return;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation always returns FALSE.
   */
  public function postUpdate() {
    return FALSE;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * By default, deletes all indexes from this server.
   */
  public function preDelete() {
    $indexes = search_api_index_load_multiple(FALSE, array('server' => $this->server->machine_name));
    foreach ($indexes as $index) {
      $this->removeIndex($index);
    }
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * Does nothing, by default.
   */
  public function addIndex(SearchApiIndex $index) {
    return;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation always returns FALSE.
   */
  public function fieldsUpdated(SearchApiIndex $index) {
    return FALSE;
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * By default, removes all items from that index.
   */
  public function removeIndex($index) {
    if (is_object($index) && empty($index->read_only)) {
      $this->deleteItems('all', $index);
    }
  }

  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The default implementation returns a SearchApiQuery object.
   */
  public function query(SearchApiIndex $index, $options = array()) {
    return new SearchApiQuery($index, $options);
  }

}
