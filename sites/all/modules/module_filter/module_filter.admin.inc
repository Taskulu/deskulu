<?php

/**
 * @file
 *
 * @author greenSkin
 */

/*******************************************************************************
 * Callback Functions, Forms, and Tables
 ******************************************************************************/

/**
 * Settings form for module filter.
 */
function module_filter_settings() {
  $form['module_filter_set_focus'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set focus to filter field on page load'),
    '#description' => t('Currently has no effect when using Overlay module.'),
    '#default_value' => variable_get('module_filter_set_focus', 1),
  );

  $form['module_filter_tabs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enhance the modules page with tabs'),
    '#description' => t('Alternate tabbed theme that restructures packages into tabs.'),
    '#default_value' => variable_get('module_filter_tabs', 1)
  );
  $form['tabs'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tabs'),
    '#description' => t('Settings used with the tabs view of the modules page.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['tabs']['module_filter_count_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Number of enabled modules'),
    '#description' => t('Display the number of enabled modules in the active tab along with the total number of modules.'),
    '#default_value' => variable_get('module_filter_count_enabled', 1)
  );
  $form['tabs']['module_filter_visual_aid'] = array(
    '#type' => 'checkbox',
    '#title' => t('Visual aids'),
    '#description' => t('When enabling/disabling modules, the module name will display in the tab summary.<br />When filtering, a count of results for each tab will be presented.'),
    '#default_value' => variable_get('module_filter_visual_aid', 1)
  );
  $form['tabs']['module_filter_hide_empty_tabs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide tabs with no results'),
    '#description' => t('When a filter returns no results for a tab, the tab is hidden. This is dependent on visual aids being enabled.'),
    '#default_value' => variable_get('module_filter_hide_empty_tabs', 0)
  );
  $form['tabs']['module_filter_dynamic_save_position'] = array(
    '#type' => 'checkbox',
    '#title' => t('Dynamically position Save button'),
    '#description' => t("For sites with lots of tabs, enable to help keep the 'Save configuration' button more accessible."),
    '#default_value' => variable_get('module_filter_dynamic_save_position', 1)
  );
  $form['tabs']['module_filter_use_url_fragment'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use URL fragment'),
    '#description' => t('Use URL fragment when navigating between tabs. This lets you use the browsers back/forward buttons to navigate through the tabs you selected.') . '<br />' . t('When the Overlay module is enabled this functionality will not be used since overlay relies on the URL fragment.'),
    '#default_value' => variable_get('module_filter_use_url_fragment', 1)
  );
  $form['tabs']['module_filter_use_switch'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use switch instead of checkbox'),
    '#description' => t('This is purely cosmetic (at least for now). Displays a ON/OFF switch rather than a checkbox to enable/disable modules.<br /><strong>Modules will not actually be enabled/disabled until the form is saved.</strong>'),
    '#default_value' => variable_get('module_filter_use_switch', 1),
  );
  $form['tabs']['module_filter_track_recent_modules'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track recently enabled/disabled modules'),
    '#description' => t('Adds a "Recent" tab that displays modules that have been enabled or disabled with the last week.'),
    '#default_value' => variable_get('module_filter_track_recent_modules', 1),
  );
  $form['tabs']['module_filter_remember_active_tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember active tab.'),
    '#description' => t('When enabled, the active tab will be remembered.'),
    '#default_value' => variable_get('module_filter_remember_active_tab', 1),
  );

  $form['update'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update status'),
    '#collapsible' => TRUE,
    '#collapsed' => (module_exists('update')) ? FALSE : TRUE,
  );
  $form['update']['module_filter_remember_update_state'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember the last selected filter.'),
    '#description' => t('When enabled, the last state (All, Update available, Security update, Unknown) will be remembered.'),
    '#default_value' => variable_get('module_filter_remember_update_state', 0),
  );

  if (module_exists('page_actions')) {
    $form['tabs']['module_filter_dynamic_save_position']['#description'] .= '<br />' . t('The module %name is enabled and thus this setting will have no affect.', array('%name' => t('Page actions')));
  }

  return system_settings_form($form);
}
