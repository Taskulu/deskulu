<?php

/**
 * @file
 * Mandrill variable hooks.
 */

/**
 * Implements hook_variable_group_info().
 */
function mandrill_variable_group_info() {
  $groups['mandrill'] = array(
    'title' => t('Mandrill'),
    'description' => t('Settings related to Mandrill.'),
    'access' => 'administer mandrill',
    'path' => array('admin/config/services/mandrill'),
  );
  return $groups;
}

/**
 * Implements hook_variable_info().
 */
function mandrill_variable_info($options) {
  $variables = array();

  $variables['mandrill_api_key'] = array(
    'title' => t('Mandrill API Key', array(), $options),
    'description' => t('Create or grab your API key from the !link.',
      array('!link' => l(t('Mandrill settings'), 'https://mandrillapp.com/settings/index')),
      $options),
    'type' => 'string',
    'group' => 'mandrill',
    'default' => variable_get('site_mail'),
  );
  $variables['mandrill_from'] = array(
    'title' => t('From address', array(), $options),
    'description' => t('The sender email address. If this address has not been verified, messages will be queued and not sent until it is.', array(), $options),
    'type' => 'string',
    'group' => 'mandrill',
    'default' => '',
  );
  $variables['mandrill_from_name'] = array(
    'title' => t('From name', array(), $options),
    'description' => t('Optionally enter a from name to be used.', array(), $options),
    'type' => 'string',
    'group' => 'mandrill',
    'default' => '',
  );
  $formats = filter_formats();
  $mandrill_filter_format_options = array();
  foreach ($formats as $v => $format) {
    $mandrill_filter_format_options[$v] = $format->name;
  }
  $variables['mandrill_filter_format'] = array(
    'title' => t('Input format', array(), $options),
    'description' => t('If selected, the input format to apply to the message body before sending to the Mandrill API.', array(), $options),
    'type' => 'select',
    'options' => $mandrill_filter_format_options,
    'group' => 'mandrill',
    'default' => 'full_html',
  );

  $variables['mandrill_track_opens'] = array(
    'title' => t('Track opens', array(), $options),
    'description' => t('Whether or not to turn on open tracking for messages.', array(), $options),
    'type' => 'boolean',
    'group' => 'mandrill',
    'default' => TRUE,
  );
  $variables['mandrill_track_clicks'] = array(
    'title' => t('mandrill_track_clicks', array(), $options),
    'description' => t('Whether or not to turn on click tracking for messages.', array(), $options),
    'type' => 'boolean',
    'group' => 'mandrill',
    'default' => TRUE,
  );
  $variables['mandrill_url_strip_qs'] = array(
    'title' => t('Strip query string', array(), $options),
    'description' => t('Whether or not to strip the query string from URLs when aggregating tracked URL data.', array(), $options),
    'type' => 'boolean',
    'group' => 'mandrill',
    'default' => FALSE,
  );
  $variables['mandrill_mail_key_blacklist'] = array(
    'title' => t('Content logging blacklist', array(), $options),
    'description' => t('Comma delimited list of Drupal mail keys to exclude content logging for. CAUTION: Removing the default password reset key may expose a security risk.', array(), $options),
    'type' => 'text',
    'group' => 'mandrill',
    'default' => mandrill_mail_key_blacklist(),
  );

  $variables['mandrill_analytics_domains'] = array(
    'title' => t('Google analytics domains', array(), $options),
    'description' => t('One or more domains for which any matching URLs will automatically have Google Analytics parameters appended to their query string. Separate each domain with a comma.', array(), $options),
    'type' => 'string',
    'group' => 'mandrill',
    'default' => '',
  );
  $variables['mandrill_analytics_campaign'] = array(
    'title' => t('Google analytics campaign', array(), $options),
    'description' => t("The value to set for the utm_campaign tracking parameter. If this isn't provided the messages from address will be used instead.", array(), $options),
    'type' => 'string',
    'group' => 'mandrill',
    'default' => '',
  );

  return $variables;
}
