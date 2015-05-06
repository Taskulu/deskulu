<?php

/**
 * @file
 * Base current search item plugin class.
 */

/**
 * Abstract class extended by current search item plugins.
 *
 * Current search items are plugins that  contextual information about the
 * search being executed, for example a list of active items or a message
 * containing information about the number of results returned by the query.
 */
abstract class CurrentSearchItem {

  /**
   * The machine readable name of current search block configuration.
   *
   * @var string
   */
  protected $configName;

  /**
   * The machine readable name of the current search item plugin.
   *
   * @var string
   */
  protected $name;

  /**
   * An array of the current search item's settings.
   *
   * This is the item's settings under the "items" key of the settings property
   * in the object returned by current_search_item_load().
   *
   * @var array
   */
  protected $settings;

  /**
   * An array keyed by item position to its value and all its children.
   *
   * @var array
   */
  protected $itemValues = array();

  /**
   * Constructs a CurrentSearchItem object.
   *
   * @param string $name
   *   The machine readable name of the current search item plugin as defined in
   *   hook_current_search_items() implementations.
   * @param stdClass $config
   *   The current search block configurations as returned by
   *   current_search_item_load().
   */
  public function __construct($name, $config = NULL) {
    $this->name = $name;
    if (isset($config->settings['items'][$name])) {
      $this->configName = $config->name;
      $this->settings = $config->settings['items'][$name] + $this->getDefaultSettings();
    }
    else {
      $this->settings = $this->getDefaultSettings();
    }
  }

  /**
   * Executes the plugin that returns the current search item's render array.
   *
   * See the documentation at http://drupal.org/node/930760 for more information
   * on how to work with render arrays.
   *
   * @param FacetapiAdapter $adapter
   *   The adapter object of the current search.
   *
   * @return array
   *   The current search item's render array.
   *
   * @see http://drupal.org/node/930760
   */
  abstract public function execute(FacetapiAdapter $adapter);

  /**
   * Allows the plugin to add settings to the current search item form.
   *
   * @see current_search_settings_form()
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do...
  }

  /**
   * Provides default values for the plugin settings.
   *
   * All settings added via CurrentSearchItem::settingsForm() should have
   * corresponding defaults in this method.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }

  /**
   * Helper function for translating strings.
   *
   * @param string $key
   *   The array key of the item being translated as defined in the plugin's
   *   CurrentSearchItem::settingsForm() function.
   * @param string $string
   *   The string being translated.
   *
   * @return
   *   The translated string.
   *
   * @see facetapi_translate_string()
   */
  public function translate($key, $string) {
    $name = 'current_search:' . $this->configName . ':' . $this->name . ':' . $key;
    return facetapi_translate_string($name, $string);
  }

  /**
   * Returns form elements that allow users to add wrapper HTML around items.
   *
   * Inspired by Semantic Views which was integrated into Views 3, these form
   * elements allow the user to add wrapper HTML and CSS classes around the
   * current search items for easier theming and better semantic markup. This
   * method is usually called in CurrentSearchItem::settingsForm() overrides.
   *
   * @return array
   *   The wrapper HTML form elements.
   *
   * @see CurrentSearchItem::settingsForm()
   */
  public function wrapperForm(&$form, &$form_state) {

    $form['wrapper'] = array(
      '#type' => 'checkbox',
      '#title' => t('Customize wrapper HTML'),
      '#default_value' => $this->settings['wrapper'],
    );

    $form['element'] = array(
      '#type' => 'select',
      '#title' => t('HTML element'),
      '#default_value' => $this->settings['element'],
      '#description' => t('Choose the HTML element to wrap around this item, e.g. H1, H2, etc.'),
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][wrapper]"]' => array('checked' => TRUE),
        ),
      ),
      '#options' => array(
        '0' => t('<None>'),
        'div' => 'DIV',
        'span' => 'SPAN',
        'h1' => 'H1',
        'h2' => 'H2',
        'h3' => 'H3',
        'h4' => 'H4',
        'h5' => 'H5',
        'h6' => 'H6',
        'p' => 'P',
        'strong' => 'STRONG',
        'em' => 'EM',
      ),
    );

    $form['css'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add CSS classes to wrapper element'),
      '#default_value' => $this->settings['css'],
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][wrapper]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['classes'] = array(
      '#type' => 'textfield',
      '#title' => t('CSS classes'),
      '#default_value' => $this->settings['classes'],
      '#description' => t('A comma separated list of CSS classes. Token replacement patterns are allowed.'),
      '#maxlength' => 128,
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][wrapper]"]' => array('checked' => TRUE),
          ':input[name="plugin_settings[' . $this->name . '][css]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  /**
   * Returns defaults for the wrapper HTML elements.
   *
   * This method is usually called in CurrentSearchItem::getDefaultSettings()
   * overrides.
   *
   * @return array
   *   An array of default values.
   *
   * @see CurrentSearchItem::wrapperForm()
   * @see CurrentSearchItem::getDefaultSettings()
   */
  public function wrapperDefaults() {
    return array(
      'wrapper' => FALSE,
      'element' => '0',
      'css' => FALSE,
      'classes' => '',
    );
  }

  /**
   * Returns the token tree element.
   *
   * if the Token module is not installed, alerts the administrator that using
   * Token module will allow them to view the replacement patterns.
   *
   * @return array
   *   The token FAPI element.
   */
  public function getTokenTree(array $types = array()) {
    if (module_exists('token')) {
      return array(
        '#theme' => 'token_tree',
        '#token_types' => $types,
      );
    }
    else {
      return array(
        '#markup' => '<p>' . t('Install the <a href="http://drupal.org/project/token" target="_blank" title="Token module project page">Token</a> module to view available replacement patterns.') . '</p>'
      );
    }
  }

  /**
   * Helper function that returns the item's value and its children.
   *
   * @param array $item
   *   The item as returned by FacetapiAdapter::getAllActiveItems().
   * @param FacetapiAdapter $adapter
   *   The adapter object associated with the searcher executing the search.
   *
   * @return array
   *   An array of the item's value and all its children.
   */
  public function getItemValues(array $item, FacetapiAdapter $adapter) {
    if (!isset($this->itemValues[$item['pos']])) {

      // Initializes items, sets $values as a reference for code readability.
      $this->itemValues[$item['pos']] = array();
      $values = &$this->itemValues[$item['pos']];

      // Gets all children so they are deactivated as well.
      foreach ($item['facets'] as $facet_name) {
        $active_children = $adapter->getProcessor($facet_name)->getActiveChildren($item['value']);
        $values = array_merge($values, $active_children);
      }

      // Handle the case of a URL value that matches no actual facet values.
      // Otherwise, it can't be unclicked.
      if (!in_array($item['value'], $values)) {
        $values[] = $item['value'];
      }
    }

    return $this->itemValues[$item['pos']];
  }

  /**
   * Helper function that returns a facet's path.
   *
   * Ensures that all active child values are deactivated along with the parent.
   *
   * @param array $item
   *   The item as returned by FacetapiAdapter::getAllActiveItems().
   * @param FacetapiAdapter $adapter
   *   The adapter object of the current search.
   *
   * @return string
   *   The facet path.
   */
  public function getFacetPath(array $item, FacetapiAdapter $adapter) {
    return $adapter
      ->getProcessor($item['facets'][0])
      ->getFacetPath($this->getItemValues($item, $adapter), 1);
  }

  /**
   * Helper function that returns a facet's query string.
   *
   * Ensures that all active child values are deactivated along with the parent.
   *
   * @param array $item
   *   The item as returned by FacetapiAdapter::getAllActiveItems().
   * @param FacetapiAdapter $adapter
   *   The adapter object associated with the searcher executing the search.
   *
   * @return array
   *   An array containing the query string variables.
   *
   * @see UrlProcessor::getQueryString()
   */
  public function getQueryString(array $item, FacetapiAdapter $adapter) {
    return $adapter
      ->getProcessor($item['facets'][0])
      ->getQueryString($this->getItemValues($item, $adapter), 1);
  }
}

/**
 * Parses the classes setting into an array of sanitized classes.
 *
 * @param string $setting
 *   The classes setting passed by the user.
 * @param array $data
 *   An optional array of data to pass to token_replace().
 *
 * @return array
 *   An array of sanitized classes.
 */
function current_search_get_classes($setting, array $data = array()) {
  $classes = array();
  foreach (drupal_explode_tags($setting) as $class) {
    if ($data) {
      $class = token_replace($class, $data);
    }
    $classes[] = drupal_html_class($class);
  }
  return $classes;
}
