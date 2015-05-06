<?php


/**
 * @file
 * Hooks provided by the Entity reference prepopulate module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Register a new prepopulate provider.
 *
 * @return
 *   Array of providers keyed by the provider name, and the following as value:
 *   - title: The title of the provider.
 *   - description: The description of the provider.
 *   - callback: The function that will be called to get the values.
 *   - disabled: (optional), determines if the provider should be disabled.
 */
function hook_entityreference_prepopulate_providers_info() {
  return array(
    'url' => array(
      'title' => t('URL'),
      'description' => t('Prepopulate from URL'),
      'callback' => 'entityreference_prepopulate_get_values_from_url',
    ),
  );
}

/**
 * Alter providers list.
 *
 * @param $providers
 *   Array keyed by the provider name, and and array of values.
 */
function entityreference_prepopulate_providers_info_alter(&$providers) {
  $providers['url']['disabled'] = TRUE;
}

/**
 * @} End of "addtogroup hooks".
 */
