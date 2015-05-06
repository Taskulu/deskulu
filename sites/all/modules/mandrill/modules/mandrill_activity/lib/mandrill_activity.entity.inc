<?php
/**
 * @file
 * Class defining a MandrillActivityEntity.
 */

/**
 * Class MandrillActivityEntity
 *
 * Extend Entity to define a MandrillActivityEntity.
 */
class MandrillActivityEntity extends Entity {
  public
    $mandrill_activity_entity_id,
    $name,
    $label,
    $entity_type,
    $bundle,
    $email_property,
    $entity_path,
    $enabled;

  /**
   * Override constructor to set entity type.
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'mandrill_activity_entity');
  }

  /**
   * Return a label for this entity.
   *
   * @return string
   *   Label.
   */
  public function label() {
    return $this->label;
  }

}
