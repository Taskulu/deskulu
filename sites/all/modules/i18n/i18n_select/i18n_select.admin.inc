<?php

/**
 * @file
 * Helper functions for select administration.
 */

function i18n_select_admin_settings() {
  $form['types'] = array(
    '#type' => 'variable_fieldset',
    '#title' => t('Content to filter by language'),
    '#variable_list' => array('i18n_select_nodes', 'i18n_select_taxonomy'),
  );
  $form['mode'] = array(
    '#type' => 'variable_fieldset',
    '#title' => t('Content selection mode'),
    '#variable_list' => array('i18n_select_missing_translation', 'i18n_select_skip_tags'),
  );
  // Enable for specific pages. This works pretty much like block visibility
  // Note this page requires 'administer site configuration' so we don't need to check permissions
  $form['pages'] = array(
    '#type' => 'variable_fieldset',
    '#title' => t('Enable for specific pages'),
    '#variable_list' => array('i18n_select_page_mode', 'i18n_select_page_list', 'i18n_select_page_block'),
  );
  return system_settings_form($form);
}
