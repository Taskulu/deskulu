<?php
/**
 * @file
 * Provides page callbacks for services_documentation module.
 */

/**
 * Page callback for documentation homepage.
 */
function services_documentation_versions_overview_page($endpoints) {

  $base_path = variable_get('services_documentation_base_path', 'developers');

  $items = array();
  foreach ($endpoints as $endpoint_id => $endpoint) {
    $items[$endpoint_id] = l(t('@endpoint', array('@endpoint' => $endpoint->name)), $base_path . '/' . $endpoint_id);
  }

  $output = array(
    '#items' => $items,
    '#theme' => 'item_list',
  );

  return theme('services_documentation_versions_overview', array('versions' => $output));
}

/**
 * Page callback for a given documentation version.
 */
function services_documentation_version_page($endpoint) {
  module_load_include('inc', 'services_documentation');

  // Conditionally add CSS styling.
  if (variable_get('services_documentation_css', TRUE)) {
    $theme_path = drupal_get_path('module', 'services_documentation') . '/theme';
    drupal_add_css($theme_path . '/services_documentation.css');  
  }
  
  $resources = services_documentation_build_documentation($endpoint);
  if (empty($resources)) {
    return t('There are no resources to document');
  }

  $table_of_contents = array();
  $output = array();

  // Build render array for methods belonging to this API version.
  // Cycle through resources. E.g., users, taxonomy_term, etc.
  foreach ($resources as $resource_name => $resource) {
    // Cycle through operation bundles. E.g., targeted_actions, actions, etc.
    foreach ($resource as $method_bundle_type => $method_bundle) {
      // Cycle through each operation in bundle. E.g., create, retrieve, etc.
      foreach ($method_bundle as $method_name => $method) {
        $permission_name = 'view ' . $endpoint->name . ' ' . $resource_name . ' ' . $method_name . ' documentation';
        if (user_access($permission_name)) {
          // Attach helpful properties to $method.
          $method['name'] = $method_name;
          $method['endpoint'] = $endpoint;
          $method['resource'] = $resource + array('name' => $resource_name);
          $method['bundle'] = $method_bundle_type;

          $element = services_documentation_method_element($method);

          // Add properties for this method.
          $output[$resource_name]['#method_bundles'][$method_bundle_type]['#methods'][$method_name] = $element;

          // Add this resource to the table of contents.
          $link_title = '<span class="verb">' . $element['#verb'] . '</span> <span class="path">' . $element['#path'] . '</span>';
          $table_of_contents['#items'][$resource_name . '_' . $method_name] = l($link_title, current_path(), array('fragment' => $element['#anchor'], 'html' => TRUE));
        }
      }

      // Add properties for this method bundle.
      if (!empty($output[$resource_name]['#method_bundles'][$method_bundle_type])) {
        $output[$resource_name]['#method_bundles'][$method_bundle_type]['#name'] = $method_bundle_type;
        $output[$resource_name]['#method_bundles'][$method_bundle_type]['#theme'] = 'services_documentation_method_bundle';
      }
    }

    // Add properties to for this resource.
    if (!empty($output[$resource_name])) {
      $output[$resource_name]['#theme'] = 'services_documentation_resource';
      $output[$resource_name]['#name'] = $resource_name;
    }
  }

  // Restructure for template processing.
  $output = array(
    '#description' => variable_get('services_documentation_intro'),
    '#resources' => $output,
    '#theme' => 'services_documentation_version',
  );

  // Add table of contents to output.
  if (!empty($table_of_contents['#items']) && variable_get('services_documentation_toc', TRUE)) {
    asort($table_of_contents['#items']);
    $table_of_contents['#theme'] = 'item_list';
    $output = array_merge(array('#table_of_contents' => $table_of_contents), $output);
  }

  return $output;
}
