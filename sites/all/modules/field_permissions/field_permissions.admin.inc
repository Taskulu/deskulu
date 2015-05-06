<?php

/**
 * @file
 * Administrative interface for the Field Permissions module.
 */

/**
 * Obtain the list of field permissions.
 *
 * @param $field_label
 *   The human readable name of the field to use when constructing permission
 *   names. Usually this will be derived from one or more of the field instance
 *   labels.
 */
function field_permissions_list($field_label = '') {
  return array(
    'create' => array(
      'label' => t('Create field'),
      'title' => t('Create own value for field %field', array('%field' => $field_label)),
    ),
    'edit own' => array(
      'label' => t('Edit own field'),
      'title' => t('Edit own value for field %field', array('%field' => $field_label)),
    ),
    'edit' => array(
      'label' => t('Edit field'),
      'title' => t("Edit anyone's value for field %field", array('%field' => $field_label)),
    ),
    'view own' => array(
      'label' => t('View own field'),
      'title' => t('View own value for field %field', array('%field' => $field_label)),
    ),
    'view' => array(
      'label' => t('View field'),
      'title' => t("View anyone's value for field %field", array('%field' => $field_label)),
    ),
  );
}

/**
 * Returns field permissions in a format suitable for use in hook_permission().
 *
 * @param $field
 *   The field to return permissions for.
 * @param $label
 *   (optional) The human readable name of the field to use when constructing
 *   permission names; for example, this might be the label of one of the
 *   corresponding field instances. If not provided, an appropriate label will
 *   be automatically derived from all the field's instances.
 *
 * @return
 *   An array of permission information, suitable for use in hook_permission().
 */
function field_permissions_list_field_permissions($field, $label = NULL) {
  $description = '';

  // If there is no preferred label, construct one from all the instance
  // labels.
  if (!isset($label)) {
    $labels = array();
    foreach ($field['bundles'] as $entity_type => $bundles) {
      foreach ($bundles as $bundle_name) {
        $instance = field_info_instance($entity_type, $field['field_name'], $bundle_name);
        $labels[] = $instance['label'];
      }
    }
    // If all the instances have the same label, just use that. Otherwise, use
    // the field name (with the full list of instance labels as the permission
    // description).
    $labels = array_unique($labels);
    if (count($labels) == 1) {
      $label = array_shift($labels);
    }
    else {
      $label = $field['field_name'];
      $description = t('This field appears as: %instances', array('%instances' => implode(', ', $labels)));
    }
  }

  $permissions = array();
  foreach (field_permissions_list($label) as $permission_type => $permission_info) {
    $permission = $permission_type . ' ' . $field['field_name'];
    $permissions[$permission] = array(
      'title' => $permission_info['title'],
      'description' => $description,
    );
  }

  return $permissions;
}

/**
 * Implementation of hook_permission().
 */
function _field_permissions_permission() {
  $perms = array(
    'administer field permissions' => array(
      'title' => t('Administer field permissions'),
      'description' => t('Manage field permissions and field permissions settings.'),
      'restrict access' => TRUE,
    ),
    'access private fields' => array(
      'title' => t("Access other users' private fields"),
      'description' => t('View and edit the stored values of all private fields.'),
      'restrict access' => TRUE,
    ),
  );

  foreach (field_info_fields() as $field) {
    if (isset($field['field_permissions']['type']) && $field['field_permissions']['type'] == FIELD_PERMISSIONS_CUSTOM) {
      $perms += field_permissions_list_field_permissions($field);
    }
  }

  return $perms;
}

/**
 * Alter the field settings form.
 */
function _field_permissions_field_settings_form_alter(&$form, $form_state, $form_id) {
  // Put the field permissions extensions at the top of the field settings
  // fieldset.
  $form['field']['field_permissions'] = array(
    '#weight' => -10,
    '#access' => user_access('administer field permissions'),
  );

  $form['field']['field_permissions']['type'] = array(
    '#title' => t('Field visibility and permissions'),
    '#type' => 'radios',
    '#options' => array(
      FIELD_PERMISSIONS_PUBLIC => t('Public (author and administrators can edit, everyone can view)'),
      FIELD_PERMISSIONS_PRIVATE => t('Private (only author and administrators can edit and view)'),
      FIELD_PERMISSIONS_CUSTOM => t('Custom permissions'),
    ),
    '#default_value' => isset($form['#field']['field_permissions']['type']) ? $form['#field']['field_permissions']['type'] : FIELD_PERMISSIONS_PUBLIC,
  );

  // Add the container in which the field permissions matrix will be displayed.
  // (and make it so that it is only visible when custom permissions are being
  // used).
  $form['field']['field_permissions']['permissions'] = array(
    '#type' => 'container',
    '#states' => array(
      'visible' => array(
        // We must cast this to a string until http://drupal.org/node/879580 is
        // fixed.
        ':input[name="field[field_permissions][type]"]' => array('value' => (string) FIELD_PERMISSIONS_CUSTOM),
      ),
    ),
    // Custom styling for the permissions matrix on the field settings page.
    '#attached' => array(
      'css' => array(drupal_get_path('module', 'field_permissions') . '/field_permissions.admin.css'),
    ),
  );

  // Add the field permissions matrix itself. Wait until the #pre_render stage
  // to move it to the above container, to avoid having the permissions data
  // saved as part of the field record.
  $form['field_permissions']['#tree'] = TRUE;
  $form['field_permissions']['#access'] = user_access('administer field permissions');
  $form['field_permissions']['permissions'] = field_permissions_permissions_matrix($form['#field'], $form['#instance']);
  $form['#pre_render'][] = '_field_permissions_field_settings_form_pre_render';

  // Add a submit handler to process the field permissions settings. Note that
  // it is important for this to run *after* the main field UI submit handler
  // (which saves the field itself), since when a new field is being created,
  // our submit handler will try to assign any new custom permissions
  // immediately, and our hook_permission() implementation relies on the field
  // info being up-to-date in order for that to work correctly.
  $form['#submit'][] = '_field_permissions_field_settings_form_submit';
}

/**
 * Returns a field permissions matrix that can be inserted into a form.
 *
 * The matrix's display is based on that of Drupal's default permissions page.
 *
 * Note that this matrix must be accompanied by an appropriate submit handler
 * (attached to the top level of the form) in order for the permissions in it
 * to actually be saved. For an example submit handler, see
 * _field_permissions_field_settings_form_submit().
 *
 * @param $field
 *   The field whose permissions will be displayed in the matrix.
 * @param $instance
 *   The field instance for which the permissions will be displayed. Although
 *   the permissions are per-field rather than per-instance, the instance label
 *   will be used to display an appropriate human-readable name for each
 *   permission.
 *
 * @return
 *   A form array defining the permissions matrix.
 *
 * @see user_admin_permissions()
 * @see _field_permissions_field_settings_form_submit()
 */
function field_permissions_permissions_matrix($field, $instance) {
  // This function primarily contains a simplified version of the code from
  // user_admin_permissions().
  $form['#theme'] = 'user_admin_permissions';
  $options = array();
  $status = array();

  // Retrieve all role names for use in the submit handler.
  $role_names = user_roles();
  $form['role_names'] = array(
    '#type' => 'value',
    '#value' => $role_names,
  );

  // Retrieve the permissions for each role, and the field permissions we will
  // be assigning here.
  $role_permissions = user_role_permissions($role_names);
  $field_permissions = field_permissions_list_field_permissions($field, $instance['label']);

  // Determine if it is safe to reset the default values for this field's
  // permissions. If this is a new field (never saved with field permission
  // data before), or if it's an existing field that is not currently using
  // custom permissions and doesn't have any previously-saved ones already in
  // the database, then it will be safe to reset them.
  $reset_permissions_defaults = FALSE;
  if (!isset($field['field_permissions']['type'])) {
    $reset_permissions_defaults = TRUE;
  }
  elseif ($field['field_permissions']['type'] != FIELD_PERMISSIONS_CUSTOM) {
    $all_assigned_permissions = call_user_func_array('array_merge_recursive', $role_permissions);
    $assigned_field_permissions = array_intersect_key($all_assigned_permissions, $field_permissions);
    $reset_permissions_defaults = empty($assigned_field_permissions);
  }
  // Store this information on the form so that other modules can use it (for
  // example, if they want to set default permissions for other roles besides
  // the admin role which we use it for below).
  $form['#field_permissions_are_new'] = $reset_permissions_defaults;

  // Go through each field permission we will display.
  foreach ($field_permissions as $permission => $info) {
    // Display the name of the permission as a form item.
    $form['permission'][$permission] = array(
      '#type' => 'item',
      '#markup' => $info['title'],
    );
    // Save it to be displayed as one of the role checkboxes.
    $options[$permission] = '';
    // If we are in a situation where we can reset the field permissions
    // defaults, we do so by pre-checking the admin role's checkbox for this
    // permission.
    if ($reset_permissions_defaults) {
      if (($admin_rid = variable_get('user_admin_role', 0)) && isset($role_names[$admin_rid])) {
        $status[$admin_rid][] = $permission;
      }
      // For fields attached to users, we also pre-check the anonymous user's
      // checkbox for the permission to create the field, since that is the
      // most common way in which new user entities are created.
      if ($instance['entity_type'] == 'user' && $permission == 'create ' . $field['field_name']) {
        $status[DRUPAL_ANONYMOUS_RID][] = $permission;
      }
    }
    // Otherwise (e.g., for fields with custom permissions already saved),
    // determine whether the permission is already assigned and check each
    // checkbox accordingly.
    else {
      foreach ($role_names as $rid => $name) {
        if (isset($role_permissions[$rid][$permission])) {
          $status[$rid][] = $permission;
        }
      }
    }
  }

  // Build the checkboxes for each role.
  foreach ($role_names as $rid => $name) {
    $form['checkboxes'][$rid] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => isset($status[$rid]) ? $status[$rid] : array(),
      '#attributes' => array('class' => array('rid-' . $rid)),
    );
    $form['role_names'][$rid] = array('#markup' => check_plain($name), '#tree' => TRUE);
  }

  // Attach the default permissions page JavaScript.
  $form['#attached']['js'][] = drupal_get_path('module', 'user') . '/user.permissions.js';

  // Attach our custom JavaScript for the permission matrix.
  $form['#attached']['js'][] = drupal_get_path('module', 'field_permissions') . '/field_permissions.admin.js';

  return $form;
}

/**
 * Pre-render function for the permissions matrix on the field settings form.
 */
function _field_permissions_field_settings_form_pre_render($form) {
  // Move the permissions matrix to its final location.
  $form['field']['field_permissions']['permissions']['matrix'] = $form['field_permissions']['permissions'];
  unset($form['field_permissions']);
  return $form;
}

/**
 * Form callback; Submit handler for the Field Settings form.
 */
function _field_permissions_field_settings_form_submit($form, &$form_state) {
  // Save the field permissions when appropriate to do so.
  $new_field_permissions_type = $form_state['values']['field']['field_permissions']['type'];
  if ($new_field_permissions_type == FIELD_PERMISSIONS_CUSTOM && isset($form_state['values']['field_permissions']['permissions'])) {
    $field_permissions = $form_state['values']['field_permissions']['permissions'];
    foreach ($field_permissions['role_names'] as $rid => $name) {
      user_role_change_permissions($rid, $field_permissions['checkboxes'][$rid]);
    }
  }

  // We must clear the page and block caches whenever the field permission type
  // setting has changed (because users may now be allowed to see a different
  // set of fields). For similar reasons, we must clear these caches whenever
  // custom field permissions are being used, since those may have changed too;
  // see user_admin_permissions_submit().
  if (!isset($form['#field']['field_permissions']['type']) || $new_field_permissions_type != $form['#field']['field_permissions']['type'] || $new_field_permissions_type == FIELD_PERMISSIONS_CUSTOM) {
    cache_clear_all();
  }
}

/**
 * Menu callback; Field permissions overview.
 */
function field_permissions_overview() {
  drupal_add_css(drupal_get_path('module', 'field_permissions') .'/field_permissions.admin.css');

  $headers = array(t('Field name'), t('Field type'), t('Entity type'), t('Used in'));
  foreach (field_permissions_list() as $permission_type => $permission_info) {
    $headers[] = array('data' => $permission_info['label'], 'class' => 'field-permissions-header');
  }
  $destination = drupal_get_destination();

  // Load list of field instances, types and bundles in the system.
  $instances = field_info_instances();
  $field_types = field_info_field_types();
  $bundles = field_info_bundles();

  // Retrieve the permissions for each role.
  $role_permissions = user_role_permissions(user_roles());

  // Based on field_ui_fields_list() in field_ui.admin.inc.
  $rows = array();
  foreach ($instances as $obj_type => $type_bundles) {
    foreach ($type_bundles as $bundle => $bundle_instances) {
      foreach ($bundle_instances as $field_name => $instance) {
        // Each field will have a row in the table.
        $field = field_info_field($field_name);
        $admin_path = _field_ui_bundle_admin_path($obj_type, $bundle);
        $rows[$field_name]['data'][0] = $field['locked'] ? t('@field_name (Locked)', array('@field_name' => $field_name)) : $field_name;
        $rows[$field_name]['data'][1] = t($field_types[$field['type']]['label']);
        $rows[$field_name]['data'][2] = $obj_type;
        $rows[$field_name]['data'][3][] = l($bundles[$obj_type][$bundle]['label'], $admin_path . '/fields/'. $field_name, array(
          'query' => $destination,
          'fragment' => 'edit-field-field-permissions-type',
        ));
        $rows[$field_name]['class'] = $field['locked'] ? array('menu-disabled') : array('');

        // Append field permissions information to the report.
        $type = isset($field['field_permissions']['type']) ? $field['field_permissions']['type'] : FIELD_PERMISSIONS_PUBLIC;
        foreach (array_keys(field_permissions_list_field_permissions($field)) as $index => $permission) {
          // Put together the data value for the cell.
          $data = '';
          $full_colspan = FALSE;
          if ($type == FIELD_PERMISSIONS_PUBLIC) {
            $data = t('Public field (author and administrators can edit, everyone can view)');
            $full_colspan = TRUE;
          }
          elseif ($type == FIELD_PERMISSIONS_PRIVATE) {
            $data = t('Private field (only author and administrators can edit and view)');
            $full_colspan = TRUE;
          }
          else {
            // This is a field with custom permissions. Link the field to the
            // appropriate row of the permissions page, and theme it based on
            // whether all users have access.
            $all_users_have_access = isset($role_permissions[DRUPAL_ANONYMOUS_RID][$permission]) && isset($role_permissions[DRUPAL_AUTHENTICATED_RID][$permission]);
            $status_class = $all_users_have_access ? 'field-permissions-status-on' : 'field-permissions-status-off';
            $title = $all_users_have_access ? t('All users have this permission') : t('Not all users have this permission');
            $data = l('', 'admin/people/permissions', array(
              'attributes' => array(
                'class' => array('field-permissions-status', $status_class),
                'title' => $title,
              ),
              'query' => $destination,
              'fragment' => drupal_html_class("edit $permission"),
            ));
          }

          // Construct the cell.
          $rows[$field_name]['data'][4 + $index] = array(
            'data' => $data,
            'class' => array('field-permissions-cell'),
          );
          if ($full_colspan) {
            $rows[$field_name]['data'][4 + $index]['colspan'] = 5;
            break;
          }
        }
      }
    }
  }
  foreach ($rows as $field_name => $cell) {
    $rows[$field_name]['data'][3] = implode(', ', $cell['data'][3]);
  }
  if (empty($rows)) {
    $output = t('No fields have been defined for any content type yet.');
  }
  else {
    // Sort rows by field name.
    ksort($rows);

    // Allow external modules alter the table headers and rows.
    foreach (module_implements('field_permissions_overview_alter') as $module) {
      $function = $module .'_field_permissions_overview_alter';
      $function($headers, $rows);
    }

    $output = theme('table', array('header' => $headers, 'rows' => $rows));
  }
  return $output;
}
