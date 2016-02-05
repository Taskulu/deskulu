<?php

/**
 * @file
 * Defines the class for rules components (rule, ruleset, action).
 * Belongs to the "rules_component" operation type plugin.
 */

class ViewsBulkOperationsRulesComponent extends ViewsBulkOperationsBaseOperation {

  /**
   * Returns the access bitmask for the operation, used for entity access checks.
   *
   * Rules has its own permission system, so the lowest bitmask is enough.
   */
  public function getAccessMask() {
    return VBO_ACCESS_OP_VIEW;
  }

  /**
   * Returns whether the provided account has access to execute the operation.
   *
   * @param $account
   */
  public function access($account) {
    return rules_action('component_' . $this->operationInfo['key'])->access();
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
    $entity_key = $this->operationInfo['parameters']['entity_key'];
    // List types need to match the original, so passing list<node> instead of
    // list<entity> won't work. However, passing 'node' instead of 'entity'
    // will work, and is needed in order to get the right tokens.
    $list_type = 'list<' . $this->operationInfo['type'] . '>';
    $entity_type = $this->aggregate() ? $list_type : $this->entityType;
    $info = entity_get_info($this->entityType);

    // The component action is wrapped in an action set using the entity, so
    // that the action configuration form can make use of the entity e.g. for
    // tokens.
    $set = rules_action_set(array($entity_key => array('type' => $entity_type, 'label' => $info['label'])));
    $action = rules_action('component_' . $this->operationInfo['key'], array($entity_key . ':select' => $entity_key));
    $set->action($action);

    // Embed the form of the component action, but default to "input" mode for
    // all parameters if available.
    foreach ($action->parameterInfo() as $name => $info) {
      $form_state['parameter_mode'][$name] = 'input';
    }
    $action->form($form, $form_state);

    // Remove the configuration form element for the "entity" param, as it
    // should just use the passed in entity.
    unset($form['parameter'][$entity_key]);

    // Tweak direct input forms to be more end-user friendly.
    foreach ($action->parameterInfo() as $name => $info) {
      // Remove the fieldset and move its title to the form element.
      if (isset($form['parameter'][$name]['settings'][$name]['#title'])) {
        $form['parameter'][$name]['#type'] = 'container';
        $form['parameter'][$name]['settings'][$name]['#title'] = $form['parameter'][$name]['#title'];
      }
      // Hide the switch button if it's there.
      if (isset($form['parameter'][$name]['switch_button'])) {
        $form['parameter'][$name]['switch_button']['#access'] = FALSE;
      }
    }

    return $form;
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
    rules_ui_form_rules_config_validate($form, $form_state);
  }

  /**
   * Stores the rules element added to the form state in form(), so that it
   * can be used in execute().
   * Only called if the operation is declared as configurable.
   *
   * @param $form
   *   The views form.
   * @param $form_state
   *   An array containing the current state of the form.
   */
  public function formSubmit($form, &$form_state) {
    $this->rulesElement = $form_state['rules_element']->root();
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
    // If there was a config form, there's a rules_element.
    // If not, fallback to the component key.
    if ($this->configurable()) {
      $element = $this->rulesElement;
    }
    else {
     $element = rules_action('component_' . $this->operationInfo['parameters']['component_key']);
    }
    $wrapper_type = is_array($data) ? "list<{$this->entityType}>" : $this->entityType;
    $wrapper = entity_metadata_wrapper($wrapper_type, $data);
    $element->execute($wrapper);
  }
}
