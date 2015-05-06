<?php

/**
 * @file
 * Contains event handler interface and base classes.
 */

/**
 * Interface for handling rules events.
 *
 * Configurable events (i.e. events making use of settings) have a custom
 * event suffix, which gets appended to the base event name. The configured
 * event name of, e.g. the event for viewing an article node, would be
 * node_view--article, whereas "node_view" is the base event name and "article"
 * the event suffix as returned from
 * RulesEventHandlerInterface::getEventNameSuffix(). The event suffix is
 * generated based upon the event settings and must map to this settings, i.e.
 * each set of event settings must always generate the same suffix.
 * For a configurable event to be invoked, rules_invoke_event() has to be called
 * with the configured event name, e.g.
 * @code
 * rules_invoke_event('node_view--' . $node->type, $node, $view_mode);
 * @endcode
 * If the event settings are optional, both events have to be invoked whereas
 * usually the more general event is invoked last. E.g.:
 * @code
 * rules_invoke_event('node_view--' . $node->type, $node, $view_mode);
 * rules_invoke_event('node_view', $node, $view_mode);
 * @endcode
 *
 * Rules event handlers have to be declared using the 'class' key in
 * hook_rules_event_info(), or may be discovered automatically, see
 * rules_discover_plugins() for details.
 *
 * @see RulesEventHandlerBase
 * @see RulesEventDefaultHandler
 */
interface RulesEventHandlerInterface {

  /**
   * Constructs the event handler.
   *
   * @param string $event_name
   *   The base event string.
   * @param array $info
   *   The event info of the given event.
   */
  public function __construct($event_name, $info);

  /**
   * Sets the event settings.
   *
   * @param array $settings
   *   An array of settings to set.
   *
   * @return RulesEventHandlerInterface
   *   The handler itself for chaining.
   */
  public function setSettings(array $settings);

  /**
   * Gets the event settings.
   *
   * @return array
   *   The array of settings.
   */
  public function getSettings();

  /**
   * Returns an array of default settings.
   *
   * @return array
   */
  public function getDefaults();

  /**
   * Returns a user-facing summary of the settings.
   *
   * @return string
   *   The summary in HTML, i.e. properly escaped or filtered.
   */
  public function summary();

  /**
   * Builds the event settings form.
   *
   * @param array $form_state
   *   An associative array containing the current state of the form.
   *
   * @return array
   *   The form structure.
   */
  public function buildForm(array &$form_state);

  /**
   * Validate the event settings independent from a form submission.
   *
   * @throws RulesIntegrityException
   *   In case of validation errors, RulesIntegrityExceptions are thrown.
   */
  public function validate();

  /**
   * Extract the form values and update the event settings.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   */
  public function extractFormValues(array &$form, array &$form_state);

  /**
   * Returns the suffix to be added to the base event named based upon settings.
   *
   * If event settings are used, the event name Rules uses for the configured
   * event is {EVENT_NAME}--{SUFFIX}.
   *
   * @return string
   *   The suffix string. Return an empty string for not appending a suffix.
   */
  public function getEventNameSuffix();

  /**
   * Returns info about the variables provided by this event.
   *
   * @return array
   *   An array of provided variables, keyed by variable names and with the
   *   variable info array as value.
   */
  public function availableVariables();

  /**
   * Returns the base name of the event the event handler belongs to.
   *
   * @return string
   *   The name of the event the event handler belongs to.
   */
  public function getEventName();

  /**
   * Returns the info array of the event the event handler belongs to.
   *
   * @return string
   *   The info array of the event the event handler belongs to.
   */
  public function getEventInfo();
}

/**
 * Interface for event dispatchers.
 */
interface RulesEventDispatcherInterface extends RulesEventHandlerInterface {

  /**
   * Starts the event watcher.
   */
  public function startWatching();

  /**
   * Stops the event watcher.
   */
  public function stopWatching();

  /**
   * Returns whether the event dispatcher is currently active.
   *
   * @return bool
   *   TRUE if the event dispatcher is currently active, FALSE otherwise.
   */
  public function isWatching();
}

/**
 * Base class for event handler.
 */
abstract class RulesEventHandlerBase implements RulesEventHandlerInterface {

  /**
   * The event name.
   *
   * @var string
   */
  protected $eventName;

  /**
   * The event info.
   *
   * @var array
   */
  protected $eventInfo;

  /**
   * The event settings.
   *
   * @var array
   */
  protected $settings = array();

  /**
   * Implements RulesEventHandlerInterface::__construct()
   */
  public function __construct($event_name, $info) {
    $this->eventName = $event_name;
    $this->eventInfo = $info;
    $this->settings = $this->getDefaults();
  }

  /**
   * Implements RulesEventHandlerInterface::getSettings()
   */
  public function getSettings() {
    return $this->settings;
  }

  /**
   * Implements RulesEventHandlerInterface::setSettings()
   */
  public function setSettings(array $settings) {
    $this->settings = $settings + $this->getDefaults();
    return $this;
  }

  /**
   * Implements RulesEventHandlerInterface::validate()
   */
  public function validate() {
    // Nothing to check by default.
  }

  /**
   * Implements RulesEventHandlerInterface::extractFormValues()
   */
  public function extractFormValues(array &$form, array &$form_state) {
    foreach ($this->getDefaults() as $key => $setting) {
      $this->settings[$key] = isset($form_state['values'][$key]) ? $form_state['values'][$key] : $setting;
    }
  }

  /**
   * Implements RulesEventHandlerInterface::availableVariables()
   */
  public function availableVariables() {
    return isset($this->eventInfo['variables']) ? $this->eventInfo['variables'] : array();
  }

  /**
   * Implements RulesEventHandlerInterface::getEventName()
   */
  public function getEventName() {
    return $this->eventName;
  }

  /**
   * Implements RulesEventHandlerInterface::getEventInfo()
   */
  public function getEventInfo() {
    return $this->eventInfo;
  }
}

/**
 * A handler for events having no settings. This is the default handler.
 */
class RulesEventDefaultHandler extends RulesEventHandlerBase  {

  /**
   * Implements RulesEventHandlerInterface::buildForm()
   */
  public function buildForm(array &$form_state) {
    return array();
  }

  /**
   * Implements RulesEventHandlerInterface::getConfiguredEventName()
   */
  public function getEventNameSuffix() {
    return '';
  }

  /**
   * Implements RulesEventHandlerInterface::summary()
   */
  public function summary() {
    return check_plain($this->eventInfo['label']);
  }

  /**
   * Implements RulesEventHandlerInterface::getDefaults()
   */
  public function getDefaults() {
    return array();
  }

  /**
   * Implements RulesEventHandlerInterface::getSettings()
   */
  public function getSettings() {
    return NULL;
  }
}

/**
 * Exposes the bundle of an entity as event setting.
 */
class RulesEventHandlerEntityBundle extends RulesEventHandlerBase {

  protected $entityType, $entityInfo, $bundleKey;

  /**
   * Implements RulesEventHandlerInterface::__construct()
   */
  public function __construct($event_name, $info) {
    parent::__construct($event_name, $info);
    // Cut off the suffix, e.g. remove 'view' from node_view.
    $this->entityType = implode('_', explode('_', $event_name, -1));
    $this->entityInfo = entity_get_info($this->entityType);
    if (!$this->entityInfo) {
      throw new InvalidArgumentException('Unsupported event name passed.');
    }
  }

  /**
   * Implements RulesEventHandlerInterface::summary()
   */
  public function summary() {
    $bundle = &$this->settings['bundle'];
    $bundle_label = isset($this->entityInfo['bundles'][$bundle]['label']) ? $this->entityInfo['bundles'][$bundle]['label'] : $bundle;
    $suffix = isset($bundle) ? ' ' . t('of @bundle-key %name', array('@bundle-key' => $this->getBundlePropertyLabel(), '%name' => $bundle_label)) : '';
    return check_plain($this->eventInfo['label']) . $suffix;
  }

  /**
   * Implements RulesEventHandlerInterface::buildForm()
   */
  public function buildForm(array &$form_state) {
    $form['bundle'] = array(
      '#type' => 'select',
      '#title' => t('Restrict by @bundle', array('@bundle' => $this->getBundlePropertyLabel())),
      '#description' => t('If you need to filter for multiple values, either add multiple events or use the "Entity is of bundle" condition instead.'),
      '#default_value' => $this->settings['bundle'],
      '#empty_value' => '',
    );
    foreach ($this->entityInfo['bundles'] as $name => $bundle_info) {
      $form['bundle']['#options'][$name] = $bundle_info['label'];
    }
    return $form;
  }

  /**
   * Returns the label to use for the bundle property.
   *
   * @return string
   */
  protected function getBundlePropertyLabel() {
    return $this->entityInfo['entity keys']['bundle'];
  }

  /**
   * Implements RulesEventHandlerInterface::extractFormValues()
   */
  public function extractFormValues(array &$form, array &$form_state) {
    $this->settings['bundle'] = !empty($form_state['values']['bundle']) ? $form_state['values']['bundle'] : NULL;
  }

  /**
   * Implements RulesEventHandlerInterface::validate()
   */
  public function validate() {
    if ($this->settings['bundle'] && empty($this->entityInfo['bundles'][$this->settings['bundle']])) {
      throw new RulesIntegrityException(t('The @bundle %bundle of %entity_type is not known.',
        array(
          '%bundle' => $this->settings['bundle'],
          '%entity_type' => $this->entityInfo['label'],
          '@bundle' => $this->getBundlePropertyLabel(),
      )), array(NULL, 'bundle'));
    }
  }

  /**
   * Implements RulesEventHandlerInterface::getConfiguredEventName()
   */
  public function getEventNameSuffix() {
    return $this->settings['bundle'];
  }

  /**
   * Implements RulesEventHandlerInterface::getDefaults()
   */
  public function getDefaults() {
    return array(
      'bundle' => NULL,
    );
  }

  /**
   * Implements RulesEventHandlerInterface::availableVariables()
   */
  public function availableVariables() {
    $variables = $this->eventInfo['variables'];
    if ($this->settings['bundle']) {
      // Add the bundle to all variables of the entity type.
      foreach ($variables as $name => $variable_info) {
        if ($variable_info['type'] == $this->entityType) {
          $variables[$name]['bundle'] = $this->settings['bundle'];
        }
      }
    }
    return $variables;
  }
}
