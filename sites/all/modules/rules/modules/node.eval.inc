<?php

/**
 * @file
 * Contains rules integration for the node module needed during evaluation.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Base class providing node condition defaults.
 */
abstract class RulesNodeConditionBase extends RulesConditionHandlerBase {

  public static function defaults() {
    return array(
      'parameter' => array(
        'node' => array('type' => 'node', 'label' => t('Content')),
      ),
      'category' => 'node',
      'access callback' => 'rules_node_integration_access',
    );
  }
}

/**
 * Condition: Check for selected content types
 */
class RulesNodeConditionType extends RulesNodeConditionBase {

  /**
   * Defines the condition.
   */
  public static function getInfo() {
    $info = self::defaults() + array(
      'name' => 'node_is_of_type',
      'label' => t('Content is of type'),
      'help' => t('Evaluates to TRUE if the given content is of one of the selected content types.'),
    );
    $info['parameter']['type'] = array(
      'type' => 'list<text>',
      'label' => t('Content types'),
      'options list' => 'node_type_get_names',
      'description' => t('The content type(s) to check for.'),
      'restriction' => 'input',
    );
    return $info;
  }

  /**
   * Executes the condition.
   */
  public function execute($node, $types) {
    return in_array($node->type, $types);
  }

  /**
   * Provides the content type of a node as asserted metadata.
   */
  function assertions() {
    return array('node' => array('bundle' => $this->element->settings['type']));
  }
}

/**
 * Condition: Check if the node is published.
 */
class RulesNodeConditionPublished extends RulesNodeConditionBase {

  /**
   * Defines the condition.
   */
  public static function getInfo() {
    return self::defaults() + array(
      'name' => 'node_is_published',
      'label' => t('Content is published'),
    );
  }

  /**
   * Executes the condition.
   */
  public function execute($node) {
    return $node->status == 1;
  }
}

/**
 * Condition: Check if the node is sticky.
 */
class RulesNodeConditionSticky extends RulesNodeConditionBase {

  /**
   * Defines the condition.
   */
  public static function getInfo() {
    return self::defaults() + array(
      'name' => 'node_is_sticky',
      'label' => t('Content is sticky'),
    );
  }

  /**
   * Executes the condition.
   */
  public function execute($node) {
    return $node->sticky == 1;
  }
}

/**
 * Condition: Check if the node is promoted to the frontpage
 */
class RulesNodeConditionPromoted extends RulesNodeConditionBase {

  /**
   * Defines the condition.
   */
  public static function getInfo() {
    return self::defaults() + array(
      'name' => 'node_is_promoted',
      'label' => t('Content is promoted to frontpage'),
    );
  }

  /**
   * Executes the condition.
   */
  public function execute($node) {
    return $node->promote == 1;
  }
}

/**
 * @}
 */
