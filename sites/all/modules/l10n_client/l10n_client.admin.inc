<?php

/**
 * @file
 *   Administrative page callbacks for the Localization client module.
 */

/**
 * Settings form for l10n_client.
 *
 * Enable users to set up a central server to share translations with.
 */
function l10n_client_settings_form() {
  $form = array();
  $form['l10n_client_disabled_paths'] = array(
    '#title'         => t('Disable on-page translation on the following system paths'),
    '#type'          => 'textarea',
    '#description'   => t('One per line. Wildcard-enabled. Examples: system/ajax, admin*'),
    '#default_value' => variable_get('l10n_client_disabled_paths', ''),
  );
  $form['l10n_client_use_server'] = array(
    '#title'         => t('Enable sharing translations with server'),
    '#type'          => 'checkbox',
    '#default_value' => variable_get('l10n_client_use_server', FALSE),
  );
  $form['l10n_client_server'] = array(
    '#title'         => t('Address of localization server to use'),
    '#type'          => 'textfield',
    '#description'   => t('Each translation submission will also be submitted to this server. We suggest you enter <a href="@localize">http://localize.drupal.org</a> to share with the greater Drupal community. Make sure you set up an API-key in the user account settings for each user that will participate in the translations.', array('@localize' => 'http://localize.drupal.org')),
    '#default_value' => variable_get('l10n_client_server', 'http://localize.drupal.org'),
  );
  return system_settings_form($form);
}

/**
 * Validation to make sure the provided server can handle our submissions.
 *
 * Make sure it supports the exact version of the API we will try to use.
 */
function l10n_client_settings_form_validate($form, &$form_state) {
  if ($form_state['values']['l10n_client_use_server']) {

    if (!empty($form_state['values']['l10n_client_server'])) {
      // Try to invoke the remote string submission with a test request.
      $response = xmlrpc($form_state['values']['l10n_client_server'] . '/xmlrpc.php', array('l10n.server.test' => array('2.0')));
      if ($response && !empty($response['name']) && !empty($response['version'])) {
        if (empty($response['supported']) || !$response['supported']) {
          form_set_error('l10n_client_server', t('The given server could not handle the v2.0 remote submission API.'));
        }
        else {
          drupal_set_message(t('Verified that the specified server can handle remote string submissions. Supported languages: %languages.', array('%languages' => $response['languages'])));
        }
      }
      else {
        form_set_error('l10n_client_server', t('Invalid localization server address specified. Make sure you specified the right server address.'));
      }
    }
    else {
      form_set_error('l10n_client_server', t('You should provide a server address, such as http://localize.drupal.org'));
    }
  }
}
