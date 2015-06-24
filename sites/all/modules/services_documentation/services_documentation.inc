<?php
/**
 * @file
 * Provides various helper functions for services_documentation.
 */


/**
 * Generates an array of Services resources keyed by endpoint.
 */
function services_documentation_build_documentation($endpoint) {
  $documentation_resources = array();

  // Build menu items for Services resource documentation.
  if ($resources = services_get_resources($endpoint->name)) {
    // Cycle through resources. E.g., users, taxonomy_term, etc.
    foreach ($resources as $resource_name => $resource) {
      // Skip to next iteration if this resource is not enabled for endpoint.
      if (empty($resource['endpoint'])) {
        continue;
      }
      // Cycle through operation bundles. E.g., targeted_actions, actions, etc.
      foreach ($resource as $method_bundle_type => $method_bundle) {
        // Cycle through each operation in bundle. E.g., create, retrieve, etc.
        if (is_array($method_bundle)) {
          foreach ($method_bundle as $method_name => $method) {
            if (empty($resource['endpoint'][$method_bundle_type][$method_name]['enabled'])) {
              continue;
            }
            if (is_array($resource[$method_bundle_type][$method_name])) {
              // If there is a documentation callback or saved values in the db.
              if (!empty($method['documentation callback']) ||
                  !empty($endpoint->server_settings['docs'][$resource_name][$method_bundle_type][$method_name]['enable'])) {
                $documentation_resources[$resource_name][$method_bundle_type][$method_name] = $method;
              }
            }
          }
        }
      }
    }
  }

  return $documentation_resources;
}

/**
 * Generate a renderable element for a services documentation method.
 */
function services_documentation_method_element($method) {

  $resource_name = $method['resource']['name'];
  $method_bundle = $method['bundle'];
  $method_name = $method['name'];

  // Call the documentation callback to generate the render element.
  if (!empty($method['documentation callback']) && function_exists(($method['documentation callback']))) {
    $documentation_callback = $method['documentation callback'];
    $element = call_user_func_array($documentation_callback, array());
  }
  // Load docs settings from db.
  elseif (!empty($method['endpoint']->server_settings['docs'][$resource_name][$method_bundle][$method_name])) {
    $docs = $method['endpoint']->server_settings['docs'][$resource_name][$method_bundle][$method_name];
    $element = array(
      '#name' => $method_name,
      '#weight' => 1,
      '#description' => filter_xss($docs['description']),
      '#request_url' => filter_xss($docs['request_url']),
      '#response' => filter_xss($docs['response']),
      '#auth' => (int) $docs['auth'],
    );
  }
  else {
    $element = array(
      '#name' => $method_name,
      '#weight' => 1,
      '#description' => '',
      '#request_url' => '',
      '#response' => '',
      '#auth' => !empty($method['endpoint']->authentication) ? TRUE : FALSE,
    );
  }

  // Specifying #type attaches #pre_render and #theme callbacks
  // via hook_element_info() implementation.
  $element['#type'] = 'services_documentation_method';
  $element['#method'] = isset($element['#method']) ? array_merge_recursive($method, (array) $element['#method']) : $method;
  $element['#anchor'] = drupal_clean_css_identifier($method['resource']['name'] . '-' . $method['name']);

  // Set full path to method, if necessary.
  if (empty($element['#path'])) {
    $element['#path'] = '/' . trim($method['endpoint']->path, '/') . '/' . $method['resource']['name'];

    // Index methods to not include the method suffix in their paths.
    if ($method['name'] == 'retrieve') {
      $element['#path'] .= '/[id]';
    }
    elseif ($method['name'] != 'index') {
      $element['#path'] .= '/' . $method['name'];
    }
  }
  else {
    $element['#path'] = '/' . trim($method['endpoint']->path, '/') . '/' . trim($element['#path'], '/');
  }

  // Set HTTP verb for method, if necessary.
  if (empty($element['#verb'])) {
    $verbs = services_documentation_verbs(variable_get('services_documentation_verbs', 'services'));
    if (!empty($verbs[$method['name']])) {
      $element['#verb'] = $verbs[$method['name']];
    }
    elseif (!empty($verbs[$method['bundle']])) {
      // If there is no verb for the method, see if there's one for the bundle.
      // This allows customized verbs for "actions", "relationships", etc.
      $element['#verb'] = $verbs[$method['bundle']];
    }
  }

  $element['#title'] = $element['#verb'] . ' ' . $element['#path'];

  // Attach example implementations.
  services_documentation_method_element_examples($element);

  foreach ($element['#method']['args'] as $key => $value) {
    if (isset($value['hidden']) && $value['hidden']) {
      unset($element['#method']['args'][$key]);
    }
  }

  // Load and attach errors by looping through the error arrays to apply
  // appropriate #theme functions and#anchor values.
  if (empty($element['#errors'])) return $element;
  foreach ($element['#errors'] as $key => $value) {
    $element['#errors'][$key]['#anchor'] = drupal_clean_css_identifier($method['resource']['name'] . '-' . $method['name'] . '-error-' . $key);
    $element['#errors'][$key]['#theme'] = 'services_documentation_method_error';
  }

  return $element;
}

/**
 * Attaches example implementations to a method element.
 */
function services_documentation_method_element_examples(&$element) {
  // Loop through the example implementation arrays and apply appropriate
  // #theme functions.
  if (!empty($element['#example_implementations_bundles'])) {
    $example_bundles = $element['#example_implementations_bundles'];

    // For this method, loop through each example bundle, keyed by
    // language. E.g., Javascript, PHP, etc.
    foreach ($example_bundles as $language => $example_bundle) {
      $element['#example_implementations_bundles'][$language]['#theme'] = 'services_documentation_implementation_bundle';

      // For this example bundle, loop through the implementations.
      // E.g., myExampleJavascriptApp.
      foreach ($example_bundle['#examples'] as $example_name => $example) {
        $element['#example_implementations_bundles'][$language]['#examples'][$example_name]['#theme'] = 'services_documentation_implementation';

        // For this implementation, scan the files and directories.
        if ($element['#example_implementations_bundles'][$language]['#examples'][$example_name]['#location']) {

          // Recursively scan directories.
          $directory = $element['#example_implementations_bundles'][$language]['#examples'][$example_name]['#location'];
          $directory_contents = services_documentation_example_implementation_scan_recursively($directory);
          $element['#example_implementations_bundles'][$language]['#examples'][$example_name]['#files'] = $directory_contents;
        }
      }
    }
  }
}

/**
 * Recursively scan the directory of an example implementation.
 */
function services_documentation_example_implementation_scan_recursively($directory) {
  $results = array();
  $current_directory = scandir($directory);

  // Remove "." and ".." from the array.
  if (in_array(".", $current_directory) && in_array("..", $current_directory)) {
    array_splice($current_directory, 0, 2);
  }

  // Construct the array that will be returned for output.
  foreach ($current_directory as $key => $value) {
    if (!in_array($value, array(".", ".."))) {
      $path = $directory . '/' . $value;
      $results[$value] = array();
      $results[$value]['#name'] = $value;
      $results[$value]['#path'] = $path;
      // Recursively scan directories.
      if (is_dir($path)) {
        $results[$value]['#type'] = 'directory';
        $results[$value]['#children'] = services_documentation_example_implementation_scan_recursively($path);
      }
      // Otherwise output code.
      else {
        $results[$value]['#type'] = 'file';
        $results[$value]['#contents'] = file_get_contents($path);
      }
      $results[$value]['#theme'] = 'services_documentation_implementation_file';
    }
  }

  return $results;
}

/**
 * Define method verbs.
 *
 * @param string $style
 *   The style of method verbs to use.
 *
 * @return array
 *   An array of method verbs values
 */
function services_documentation_verbs($style) {
  $verbs = array();

  switch ($style) {
    case 'services':
      $verbs = array(
        'retrieve' => 'retrieve',
        'create' => 'create',
        'update' => 'update',
        'delete' => 'delete',
        'index' => 'index',
        'actions' => 'action',
        'targeted_actions' => 'targeted action',
        'relationships' => 'relationship',
      );
      break;

    case 'http':
      $verbs = array(
        'retrieve' => 'GET',
        'create' => 'POST',
        'update' => 'PUT',
        'delete' => 'DELETE',
        'index' => 'GET',
        'actions' => 'POST',
        'targeted_actions' => 'POST',
        'relationships' => 'GET',
      );
      break;

    case 'crud':
      $verbs = array(
        'retrieve' => 'READ',
        'create' => 'CREATE',
        'update' => 'UPDATE',
        'delete' => 'DELETE',
        'index' => 'READ',
        'actions' => 'ACTION',
        'targeted_actions' => 'ACTION',
        'relationships' => 'READ',
      );
      break;

    case 'custom':
      $verbs = array(
        'retrieve' => variable_get('services_documentation_verbs_custom_retrieve', 'retrieve'),
        'create' => variable_get('services_documentation_verbs_custom_create', 'create'),
        'update' => variable_get('services_documentation_verbs_custom_update', 'update'),
        'delete' => variable_get('services_documentation_verbs_custom_delete', 'delete'),
        'index' => variable_get('services_documentation_verbs_custom_index', 'index'),
      );
      break;
  }

  return $verbs;
}
