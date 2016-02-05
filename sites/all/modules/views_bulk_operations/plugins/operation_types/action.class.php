<?php

/**
 * @file
 * Defines the class for core actions.
 * Belongs to the "action" operation type plugin.
 */

class ViewsBulkOperationsAction extends ViewsBulkOperationsBaseOperation {

  /**
   * Contains the options provided by the user in the configuration form.
   *
   * @var array
   */
  public $formOptions = array();

  /**
   * Returns the access bitmask for the operation, used for entity access checks.
   */
  public function getAccessMask() {
    // Assume edit by default.
    if (empty($this->operationInfo['behavior'])) {
      $this->operationInfo['behavior'] = array('changes_property');
    }

    $mask = 0;
    if (in_array('views_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_VIEW;
    }
    if (in_array('changes_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_UPDATE;
    }
    if (in_array('creates_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_CREATE;
    }
    if (in_array('deletes_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_DELETE;
    }
    return $mask;
  }

  /**
   * Returns whether the provided account has access to execute the operation.
   *
   * @param $account
   */
  public function access($account) {
    // Use actions_permissions if enabled.
    if (module_exists('actions_permissions')) {
      $perm = actions_permissions_get_perm($this->operationInfo['label'], $this->operationInfo['key']);
      if (!user_access($perm, $account)) {
        return FALSE;
      }
    }
    // Check against additional permissions.
    if (!empty($this->operationInfo['permissions'])) {
      foreach ($this->operationInfo['permissions'] as $perm) {
        if (!user_access($perm, $account)) {
          return FALSE;
        }
      }
    }
    // Access granted.
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
  public function form($form, &$form_state, array $context) {
    // Some modules (including this one) place their action callbacks
    // into separate files. At this point those files might no longer be
    // included due to an #ajax rebuild, so we call actions_list() to trigger
    // inclusion. The same thing is done by actions_do() on execute.
    actions_list();

    $context['settings'] = $this->getAdminOption('settings', array());
    $form_callback = $this->operationInfo['callback'] . '_form';
    return $form_callback($context, $form_state);
  }

  /**
   * Validates the configuration form.
   * Only called if the operation is declared as configurable.
   *
   * @param $form
   *   The views form.
   * @param $form_state
   *   An array containing the current state of the form.
   */
  public function formValidate($form, &$form_state) {
    // Some modules (including this one) place their action callbacks
    // into separate files. At this point those files might no longer be
    // included due to a page reload, so we call actions_list() to trigger
    // inclusion. The same thing is done by actions_do() on execute.
    actions_list();

    $validation_callback = $this->operationInfo['callback'] . '_validate';
    if (function_exists($validation_callback)) {
      $validation_callback($form, $form_state);
    }
  }

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
  public function formSubmit($form, &$form_state) {
    // Some modules (including this one) place their action callbacks
    // into separate files. At this point those files might no longer be
    // included due to a page reload, so we call actions_list() to trigger
    // inclusion. The same thing is done by actions_do() on execute.
    actions_list();

    $submit_callback = $this->operationInfo['callback'] . '_submit';
    $this->formOptions = $submit_callback($form, $form_state);
  }

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
    $form = parent::adminOptionsForm($dom_id, $field_handler);

    $settings_form_callback = $this->operationInfo['callback'] . '_views_bulk_operations_form';
    if (function_exists($settings_form_callback)) {
      $settings = $this->getAdminOption('settings', array());

      $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Operation settings'),
        '#collapsible' => TRUE,
        '#dependency' => array(
          $dom_id . '-selected' => array(1),
        ),
      );
      $settings_dom_id = $dom_id . '-settings';
      $form['settings'] += $settings_form_callback($settings, $this->entityType, $settings_dom_id);
    }

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
   *   Needed because the admin settings form is embedded into a bigger form.
   */
  public function adminOptionsFormValidate($form, &$form_state, $error_element_base) {
    parent::adminOptionsFormValidate($form, $form_state, $error_element_base);

    if (!empty($form['settings'])) {
      $settings_validation_callback = $this->operationInfo['callback'] . '_views_bulk_operations_form_validate';
      if (function_exists($settings_validation_callback)) {
        $fake_form = $form['settings'];
        $fake_form_state = array('values' => &$form_state['values']['settings']);
        $error_element_base .= 'settings][';

        $settings_validation_callback($fake_form, $fake_form_state, $error_element_base);
      }
    }
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
    parent::adminOptionsFormSubmit($form, $form_state);

    if (!empty($form['settings'])) {
      $settings_submit_callback = $this->operationInfo['callback'] . '_views_bulk_operations_form_submit';
      if (function_exists($settings_submit_callback)) {
        $fake_form = $form['settings'];
        $fake_form_state = array('values' => &$form_state['values']['settings']);

        $settings_submit_callback($form, $form_state);
      }
    }
  }

  /**
   * Returns whether the operation needs the full selected views rows to be
   * passed to execute() as a part of $context.
   */
  public function needsRows() {
    return !empty($this->operationInfo['pass rows']);
  }

  /**
   * Executes the selected operation on the provided data.
   *
   * @param $data
   *   The data to to operate on. An entity or an array of entities.
   * @param $context
   *   An array of related data (selected views rows, etc).
   */
  public function execute($data, array $context) {
    $context['entity_type'] = $this->entityType;
    $context['settings'] = $this->getAdminOption('settings', array());
    $context += $this->formOptions;
    $context += $this->operationInfo['parameters'];
    // Actions provided by the Drupal system module require the entity to be
    // present in $context, keyed by entity type.
    if (is_object($data)) {
      $context[$this->entityType] = $data;
    }

    actions_do($this->operationInfo['callback'], $data, $context);

    // The action might need to have its entities saved after execution.
    if (in_array('changes_property', $this->operationInfo['behavior'])) {
      $data = is_array($data) ? $data : array($data);
      foreach ($data as $entity) {
        entity_save($this->entityType, $entity);
      }
    }
  }
}
