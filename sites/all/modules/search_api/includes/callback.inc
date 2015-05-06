<?php

/**
 * @file
 * Contains base definitions for data alterations.
 *
 * Contains the SearchApiAlterCallbackInterface interface and the
 * SearchApiAbstractAlterCallback class.
 */

/**
 * Interface representing a Search API data-alter callback.
 */
interface SearchApiAlterCallbackInterface {

  /**
   * Construct a data-alter callback.
   *
   * @param SearchApiIndex $index
   *   The index whose items will be altered.
   * @param array $options
   *   The callback options set for this index.
   */
  public function __construct(SearchApiIndex $index, array $options = array());

  /**
   * Check whether this data-alter callback is applicable for a certain index.
   *
   * This can be used for hiding the callback on the index's "Filters" tab. To
   * avoid confusion, you should only use criteria that are immutable, such as
   * the index's entity type. Also, since this is only used for UI purposes, you
   * should not completely rely on this to ensure certain index configurations
   * and at least throw an exception with a descriptive error message if this is
   * violated on runtime.
   *
   * @param SearchApiIndex $index
   *   The index to check for.
   *
   * @return boolean
   *   TRUE if the callback can run on the given index; FALSE otherwise.
   */
  public function supportsIndex(SearchApiIndex $index);

  /**
   * Display a form for configuring this callback.
   *
   * @return array
   *   A form array for configuring this callback, or FALSE if no configuration
   *   is possible.
   */
  public function configurationForm();

  /**
   * Validation callback for the form returned by configurationForm().
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
   * This method should both return the new options and set them internally.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   *
   * @return array
   *   The new options array for this callback.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state);

  /**
   * Alter items before indexing.
   *
   * Items which are removed from the array won't be indexed, but will be marked
   * as clean for future indexing. This could for instance be used to implement
   * some sort of access filter for security purposes (e.g., don't index
   * unpublished nodes or comments).
   *
   * @param array $items
   *   An array of items to be altered, keyed by item IDs.
   */
  public function alterItems(array &$items);

  /**
   * Declare the properties that are added to items by this callback.
   *
   * If one of the specified properties already exists for an entity it will be
   * overridden, so keep a clear namespace by prefixing the properties with the
   * module name if this is not desired.
   *
   * CAUTION: Since this method is used when calling
   * SearchApiIndex::getFields(), calling that method from inside propertyInfo()
   * will lead to a recursion and should therefore be avoided.
   *
   * @see hook_entity_property_info()
   *
   * @return array
   *   Information about all additional properties, as specified by
   *   hook_entity_property_info() (only the inner "properties" array).
   */
  public function propertyInfo();

}

/**
 * Abstract base class for data-alter callbacks.
 *
 * This class implements most methods with sensible defaults.
 *
 * Extending classes will at least have to implement the alterItems() method to
 * make this work. If that method adds additional fields to the items,
 * propertyInfo() has to be overridden, too.
 */
abstract class SearchApiAbstractAlterCallback implements SearchApiAlterCallbackInterface {

  /**
   * The index whose items will be altered.
   *
   * @var SearchApiIndex
   */
  protected $index;

  /**
   * The configuration options for this callback, if it has any.
   *
   * @var array
   */
  protected $options;

  /**
   * Implements SearchApiAlterCallbackInterface::__construct().
   */
  public function __construct(SearchApiIndex $index, array $options = array()) {
    $this->index = $index;
    $this->options = $options;
  }

  /**
   * Implements SearchApiAlterCallbackInterface::supportsIndex().
   *
   * The default implementation always returns TRUE.
   */
  public function supportsIndex(SearchApiIndex $index) {
    return TRUE;
  }

  /**
   * Implements SearchApiAlterCallbackInterface::configurationForm().
   */
  public function configurationForm() {
    return array();
  }

  /**
   * Implements SearchApiAlterCallbackInterface::configurationFormValidate().
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) { }

  /**
   * Implements SearchApiAlterCallbackInterface::configurationFormSubmit().
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $this->options = $values;
    return $values;
  }

  /**
   * Implements SearchApiAlterCallbackInterface::propertyInfo().
   */
  public function propertyInfo() {
    return array();
  }

}
