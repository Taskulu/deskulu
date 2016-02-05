<?php

function views_bulk_operations_user_roles_action_info() {
  return array('views_bulk_operations_user_roles_action' => array(
    'type' => 'user',
    'label' => t('Modify user roles'),
    'configurable' => TRUE,
    'triggers' => array('any'),
  ));
}

function views_bulk_operations_user_roles_action_form($context) {
  $roles = user_roles(TRUE);
  unset($roles[DRUPAL_AUTHENTICATED_RID]);  // Can't edit authenticated role.

  $form['add_roles'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#title' => t('Add roles'),
    '#description' => t('Choose one or more roles you would like to assign to the selected users.'),
    '#options' => $roles,
    '#size' => 5
  );
  $form['remove_roles'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#title' => t('Remove roles'),
    '#description' => t('Choose one or more roles you would like to remove from the selected users.'),
    '#options' => $roles,
    '#size' => 5
  );
  return $form;
}

function views_bulk_operations_user_roles_action_validate($form, $form_state) {
  if (!$form_state['values']['add_roles'] && !$form_state['values']['remove_roles']) {
    form_set_error('add_roles', t('You have not chosen any role to add or remove. Please select something to do.'));
  }
}

function views_bulk_operations_user_roles_action_submit($form, $form_state) {
  return array(
    'add_roles' => array_filter($form_state['values']['add_roles']),
    'remove_roles' => array_filter($form_state['values']['remove_roles']),
  );
}

function views_bulk_operations_user_roles_action($user, $context) {
  $wrapper = entity_metadata_wrapper('user', $user);
  if (!$wrapper->roles->access("update")) {
    // No access.
    return;
  }
  $roles = $wrapper->roles->value();
  if (is_array($context['add_roles'])) {
    $roles = array_merge($roles, $context['add_roles']);
  }
  if (is_array($context['remove_roles'])) {
    $roles = array_diff($roles, $context['remove_roles']);
  }
  $wrapper->roles->set($roles);
  $wrapper->save();
}
