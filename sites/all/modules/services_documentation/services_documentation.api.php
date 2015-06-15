<?php

/**
 * @file
 * Example implementation of Services Documentation API.
 */

/**
 * Implements hook_services_resources().
 *
 * This is similar to a standard Services resource definition, with two
 * additional array keys: 'documentation callback' and 'documentation versions'.
 *
 * @see services/services.services.api.php
 */
function api_resource_users_services_resources() {
  $resources = array();

  // Standard Services resource definition.
  $resources['users'] = array(
    'index' => array(
      'callback' => '_api_resource_users_resource_index',
      'args' => array(
        array(
          'name' => 'page',
          'optional' => TRUE,
          'type' => 'int',
          'description' => 'The zero-based index of the page to get, defaults to 0.',
          'default value' => 0,
          'source' => array('param' => 'page'),
        ),
      ),
      'access arguments' => array('access user profiles'),
      'access arguments append' => FALSE,
      // New documentation array keys made available by services_documentation.
      'documentation callback' => '_api_resource_users_index_doc',
    ),
  );

  return $resources;
}

/**
 * Documentation callback for index operation of users resource.
 */
function _api_resource_users_index_doc() {
  $element = array(
    '#name' => t('name'),
    '#description' => t('Returns a list of users.'),
    // Example request. E.g., a request URL, headers, and a JSON array.
    '#request' => t('request'),
    // Example response. E.g., a JSON array.
    '#response' => t('response'),
    // Does the method require authentication?
    '#auth' => TRUE,
    // The endpoint path will be prepended to this. You should include the
    // resource and method suffixes.
    '#path' => 'users/index',
    // '#example_implementations_bundles' => '',
  );

  return $element;
}
