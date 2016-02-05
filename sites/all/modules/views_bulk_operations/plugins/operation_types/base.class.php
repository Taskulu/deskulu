<?php

/**
 * @file
 * Defines the base class for operations.
 */

abstract class ViewsBulkOperationsBaseOperation {

  /**
   * The id of the operation.
   *
   * Composed of the operation_type plugin name and the operation key,
   * divided by '::'. For example: action::node_publish_action.
   */
  public $operationId;

  /**
   * The entity type that the operation is operating on.
   *
   * Not the same as $operationInfo['type'] since that value can be just
   * "entity", which means "available to every entity type".
   */
  public $entityType;

  /**
   * Contains information about the current operation, as generated
   * by the "list callback" function in the plugin file.
   *
   * @var array
   */
  protected $operationInfo;

  /**
   * Contains the options set by the admin for the current operation.
   *
   * @var array
   */
  protected $adminOptions;

  /**
   * Constructs an operation object.
   *
   * @param $operationId
   *   The id of the operation.
   * @param $entityType
   *   The entity type that the operation is operating on.
   * @param $operationInfo
   *   An array of information about the operation.
   * @param $adminOptions
   *   An array of options set by the admin.
   */
  public function __construct($operationId, $entityType, array $operationInfo, array $adminOptions) {
    $this->operationId = $operationId;
    $this->entityType = $entityType;
    $this->operationInfo = $operationInfo;
    $this->adminOptions = $adminOptions;
  }

  /**
   * Returns the value of an admin option.
   */
  public function getAdminOption($key, $default = NULL) {
    return isset($this->adminOptions[$key]) ? $this->adminOptions[$key] : $default;
  }

  /**
   * Returns the access bitmask for the operation, used for entity access checks.
   */
  public function getAccessMask() {
    // Assume edit by default.
    return VBO_ACCESS_OP_UPDATE;
  }

  /**
   * Returns the id of the operation.
   */
  public function id() {
    return $this->operationId;
  }

  /**
   * Returns the name of the operation_type plugin that provides the operation.
   */
  public function type() {
    return $this->operationInfo['operation_type'];
  }

  /**
   * Returns the human-readable name of the operation, meant to be shown
   * to the user.
   */
  public function label() {
    $admin_label = $this->getAdminOption('label');
    if (!empty($admin_label)) {
      $label = t($admin_label);
    }
    else {
      // If the admin didn't specify any label, fallback to the default one.
      $label = $this->operationInfo['label'];
    }
    return $label;
  }

  /**
   * Returns the human-readable name of the operation, meant to be shown
   * to the admin.
   */
  public function adminLabel() {
    return $this->operationInfo['label'];
  }

  /**
   * Returns whether the operation is configurable. Used to determine
   * whether the operation's form methods should be invoked.
   */
  public function configurable() {
    return !empty($this->operationInfo['configurable']);
  }

  /**
   * Returns whether the provided account has access to execute the operation.
   *
   * @param $account
   */
  public function access($account) {
    return TRUE;
  }

  /**
   * Returns the configuration form for the operation.
   * Only called if the operation is declared as configurable.
   *
   * @param $form
   *   The views form.
   * @param $form_state
   *   An array containing the current state of the form.
   * @param $context
   *   An array of related data provided by the caller.
   */
  abstract function form($form, &$form_state, array $context);

  /**
   * Validates the configuration form.
   * Only called if the operation is declared as configurable.
   *
   * @param $form
   *   The views form.
   * @param $form_state
   *   An array containing the current state of the form.
   */
  abstract function formValidate($form, &$form_state);

  /**
   * Handles the submitted configuration form.
   * This is where the operation can transform and store the submitted data.
   * Only called if the operation is declared as configurable.
   *
   * @param $form
   *   The views form.
   * @param $form_state
   *   An array containing the current state of the form.
   */
  abstract function formSubmit($form, &$form_state);

  /**
   * Returns the admin options form for the operation.
   *
   * The admin options form is embedded into the VBO field settings and used
   * to configure operation behavior. The options can later be fetched
   * through the getAdminOption() method.
   *
   * @param $dom_id
   *   The dom path to the level where the admin options form is embedded.
   *   Needed for #dependency.
   * @param $field_handler
   *   The Views field handler object for the VBO field.
   */
  public function adminOptionsForm($dom_id, $field_handler) {
    $label = $this->getAdminOption('label', '');

    $form = array();
    $form['override_label'] = array(
      '#type' => 'checkbox',
      '#title' => t('Override label'),
      '#default_value' => $label !== '',
      '#dependency' => array(
        $dom_id . '-selected' => array(1),
      ),
    );
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Provide label'),
      '#title_display' => 'invisible',
      '#default_value' => $label,
      '#dependency' => array(
        $dom_id . '-selected' => array(1),
        $dom_id . '-override-label' => array(1),
      ),
      '#dependency_count' => 2,
    );

    return $form;
  }

  /**
   * Validates the admin options form.
   *
   * @param $form
   *   The admin options form.
   * @param $form_state
   *   An array containing the current state of the form. Note that this array
   *   is constructed by the VBO views field handler, so it's not a real form
   *   state, it contains only the 'values' key.
   * @param $error_element_base
   *   The base to prepend to field names when using form_set_error().
   *   Needed because the admin options form is embedded into a bigger form.
   */
  public function adminOptionsFormValidate($form, &$form_state, $error_element_base) {
    // No need to do anything, but make the function have a body anyway
    // so that it's callable by overriding methods.
  }

  /**
   * Handles the submitted admin options form.
   * Note that there is no need to handle saving the options, that is done
   * by the VBO views field handler, which also injects the options into the
   * operation object upon instantiation.
   *
   * @param $form
   *   The admin options form.
   * @param $form_state
   *   An array containing the current state of the form. Note that this array
   *   is constructed by the VBO views field handler, so it's not a real form
   *   state, it contains only the 'values' key.
   */
  public function adminOptionsFormSubmit($form, &$form_state) {
    // If the "Override label" checkbox was deselected, clear the entered value.
    if (empty($form_state['values']['override_label'])) {
      $form_state['values']['label'] = '';
    }
  }

  /**
   * Returns whether the selected entities should be aggregated
   * (loaded in bulk and passed in together).
   * To be avoided if possible, since aggregation makes it impossible to use
   * Batch API or the Drupal Queue for execution.
   */
  public function aggregate() {
    return !empty($this->operationInfo['aggregate']);
  }

  /**
   * Returns whether the operation needs the full selected views rows to be
   * passed to execute() as a part of $context.
   */
  public function needsRows() {
    return FALSE;
  }

  /**
   * Executes the selected operation on the provided data.
   *
   * @param $data
   *   The data to to operate on. An entity or an array of entities.
   * @param $context
   *   An array of related data (selected views rows, etc).
   */
  abstract function execute($data, array $context);
}
