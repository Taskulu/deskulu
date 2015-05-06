<?php
/**
 * @file
 * Contains the php code argument default plugin.
 */

/**
 * Default argument plugin to provide a variable value.
 */
class views_plugin_argument_default_variable extends views_plugin_argument_default {
  function option_definition() {
    $options = parent::option_definition();
    $options['variable'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['variable'] = array(
      '#type' => 'select',
      '#title' => t('Variable value'),
      '#options' => _variable_views_variable_list(),
      '#default_value' => $this->options['variable'],
      '#description' => t('Select a variable whose value will be retrieved at run time.'),
    );
  }

  function convert_options(&$options) {
    if (!isset($options['variable']) && isset($this->argument->options['default_argument_variable'])) {
      $options['variable'] = $this->argument->options['default_argument_variable'];
    }
  }

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * default plugin.
   */
  function access() {
    return user_access('administer site configuration');
  }

  function get_argument() {
    return variable_get_value($this->options['variable']);
  }
}
