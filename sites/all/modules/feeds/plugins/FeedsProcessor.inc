<?php

/**
 * @file
 * Contains FeedsProcessor and related classes.
 */

// Update mode for existing items.
define('FEEDS_SKIP_EXISTING', 0);
define('FEEDS_REPLACE_EXISTING', 1);
define('FEEDS_UPDATE_EXISTING', 2);
// Options for handling content in Drupal but not in source data.
define('FEEDS_SKIP_NON_EXISTENT', 'skip');
define('FEEDS_DELETE_NON_EXISTENT', 'delete');

// Default limit for creating items on a page load, not respected by all
// processors.
define('FEEDS_PROCESS_LIMIT', 50);

/**
 * Thrown if a validation fails.
 */
class FeedsValidationException extends Exception {}

/**
 * Thrown if a an access check fails.
 */
class FeedsAccessException extends Exception {}

/**
 * Abstract class, defines interface for processors.
 */
abstract class FeedsProcessor extends FeedsPlugin {

  /**
   * Implements FeedsPlugin::pluginType().
   */
  public function pluginType() {
    return 'processor';
  }

  /**
   * @defgroup entity_api_wrapper Entity API wrapper.
   */

  /**
   * Entity type this processor operates on.
   */
  public abstract function entityType();

  /**
   * Bundle type this processor operates on.
   *
   * Defaults to the entity type for entities that do not define bundles.
   *
   * @return string|NULL
   *   The bundle type this processor operates on, or NULL if it is undefined.
   */
  public function bundle() {
    return $this->config['bundle'];
  }

  /**
   * Provides a list of bundle options for use in select lists.
   *
   * @return array
   *   A keyed array of bundle => label.
   */
  public function bundleOptions() {
    $options = array();
    foreach (field_info_bundles($this->entityType()) as $bundle => $info) {
      if (!empty($info['label'])) {
        $options[$bundle] = $info['label'];
      }
      else {
        $options[$bundle] = $bundle;
      }
    }
    return $options;
  }

  /**
   * Create a new entity.
   *
   * @param $source
   *   The feeds source that spawns this entity.
   *
   * @return
   *   A new entity object.
   */
  protected abstract function newEntity(FeedsSource $source);

  /**
   * Load an existing entity.
   *
   * @param $source
   *   The feeds source that spawns this entity.
   * @param $entity_id
   *   The unique id of the entity that should be loaded.
   *
   * @return
   *   A new entity object.
   *
   * @todo We should be able to batch load these, if we found all of the
   *   existing ids first.
   */
  protected function entityLoad(FeedsSource $source, $entity_id) {
    if ($this->config['update_existing'] == FEEDS_UPDATE_EXISTING) {
      $entities = entity_load($this->entityType(), array($entity_id));
      return reset($entities);
    }

    $info = $this->entityInfo();

    $args = array(':entity_id' => $entity_id);

    $table = db_escape_table($info['base table']);
    $key = db_escape_field($info['entity keys']['id']);

    return db_query("SELECT * FROM {" . $table . "} WHERE $key = :entity_id", $args)->fetchObject();
  }

  /**
   * Validate an entity.
   *
   * @throws FeedsValidationException $e
   *   If validation fails.
   */
  protected function entityValidate($entity) {}

  /**
   * Access check for saving an enity.
   *
   * @param $entity
   *   Entity to be saved.
   *
   * @throws FeedsAccessException $e
   *   If the access check fails.
   */
  protected function entitySaveAccess($entity) {}

  /**
   * Save an entity.
   *
   * @param $entity
   *   Entity to be saved.
   */
  protected abstract function entitySave($entity);

  /**
   * Delete a series of entities.
   *
   * @param $entity_ids
   *   Array of unique identity ids to be deleted.
   */
  protected abstract function entityDeleteMultiple($entity_ids);

  /**
   * Wrap entity_get_info() into a method so that extending classes can override
   * it and more entity information. Allowed additional keys:
   *
   * 'label plural' ... the plural label of an entity type.
   */
  protected function entityInfo() {
    return entity_get_info($this->entityType());
  }

  /**
   * @}
   */

  /**
   * Process the result of the parsing stage.
   *
   * @param FeedsSource $source
   *   Source information about this import.
   * @param FeedsParserResult $parser_result
   *   The result of the parsing stage.
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
    $state = $source->state(FEEDS_PROCESS);
    if (!isset($state->removeList) && $parser_result->items) {
      $this->initEntitiesToBeRemoved($source, $state);
    }

    while ($item = $parser_result->shiftItem()) {

      // Check if this item already exists.
      $entity_id = $this->existingEntityId($source, $parser_result);
      // If it's included in the feed, it must not be removed on clean.
      if ($entity_id) {
        unset($state->removeList[$entity_id]);
      }
      $skip_existing = $this->config['update_existing'] == FEEDS_SKIP_EXISTING;

      module_invoke_all('feeds_before_update', $source, $item, $entity_id);

      // If it exists, and we are not updating, pass onto the next item.
      if ($entity_id && $skip_existing) {
        continue;
      }

      try {

        $hash = $this->hash($item);
        $changed = ($hash !== $this->getHash($entity_id));
        $force_update = $this->config['skip_hash_check'];

        // Do not proceed if the item exists, has not changed, and we're not
        // forcing the update.
        if ($entity_id && !$changed && !$force_update) {
          continue;
        }

        // Load an existing entity.
        if ($entity_id) {
          $entity = $this->entityLoad($source, $entity_id);

          // The feeds_item table is always updated with the info for the most
          // recently processed entity. The only carryover is the entity_id.
          $this->newItemInfo($entity, $source->feed_nid, $hash);
          $entity->feeds_item->entity_id = $entity_id;
          $entity->feeds_item->is_new = FALSE;
        }

        // Build a new entity.
        else {
          $entity = $this->newEntity($source);
          $this->newItemInfo($entity, $source->feed_nid, $hash);
        }

        // Set property and field values.
        $this->map($source, $parser_result, $entity);
        $this->entityValidate($entity);

        // Allow modules to alter the entity before saving.
        module_invoke_all('feeds_presave', $source, $entity, $item, $entity_id);
        if (module_exists('rules')) {
          rules_invoke_event('feeds_import_'. $source->importer()->id, $entity);
        }

        // Enable modules to skip saving at all.
        if (!empty($entity->feeds_item->skip)) {
          continue;
        }

        // This will throw an exception on failure.
        $this->entitySaveAccess($entity);
        $this->entitySave($entity);

        // Allow modules to perform operations using the saved entity data.
        // $entity contains the updated entity after saving.
        module_invoke_all('feeds_after_save', $source, $entity, $item, $entity_id);

        // Track progress.
        if (empty($entity_id)) {
          $state->created++;
        }
        else {
          $state->updated++;
        }
      }

      // Something bad happened, log it.
      catch (Exception $e) {
        $state->failed++;
        drupal_set_message($e->getMessage(), 'warning');
        list($message, $arguments) = $this->createLogEntry($e, $entity, $item);
        $source->log('import', $message, $arguments, WATCHDOG_ERROR);
      }
    }

    // Set messages if we're done.
    if ($source->progressImporting() != FEEDS_BATCH_COMPLETE) {
      return;
    }
    // Remove not included items if needed.
    // It depends on the implementation of the clean() method what will happen
    // to items that were no longer in the source.
    $this->clean($state);
    $info = $this->entityInfo();
    $tokens = array(
      '@entity' => strtolower($info['label']),
      '@entities' => strtolower($info['label plural']),
    );
    $messages = array();
    if ($state->created) {
      $messages[] = array(
       'message' => format_plural(
          $state->created,
          'Created @number @entity.',
          'Created @number @entities.',
          array('@number' => $state->created) + $tokens
        ),
      );
    }
    if ($state->updated) {
      $messages[] = array(
       'message' => format_plural(
          $state->updated,
          'Updated @number @entity.',
          'Updated @number @entities.',
          array('@number' => $state->updated) + $tokens
        ),
      );
    }
    if ($state->unpublished) {
      $messages[] = array(
        'message' => format_plural(
            $state->unpublished,
            'Unpublished @number @entity.',
            'Unpublished @number @entities.',
            array('@number' => $state->unpublished) + $tokens
        ),
      );
    }
    if ($state->blocked) {
      $messages[] = array(
        'message' => format_plural(
          $state->blocked,
          'Blocked @number @entity.',
          'Blocked @number @entities.',
          array('@number' => $state->blocked) + $tokens
        ),
      );
    }
    if ($state->deleted) {
      $messages[] = array(
       'message' => format_plural(
          $state->deleted,
          'Removed @number @entity.',
          'Removed @number @entities.',
          array('@number' => $state->deleted) + $tokens
        ),
      );
    }
    if ($state->failed) {
      $messages[] = array(
       'message' => format_plural(
          $state->failed,
          'Failed importing @number @entity.',
          'Failed importing @number @entities.',
          array('@number' => $state->failed) + $tokens
        ),
        'level' => WATCHDOG_ERROR,
      );
    }
    if (empty($messages)) {
      $messages[] = array(
        'message' => t('There are no new @entities.', array('@entities' => strtolower($info['label plural']))),
      );
    }
    foreach ($messages as $message) {
      drupal_set_message($message['message']);
      $source->log('import', $message['message'], array(), isset($message['level']) ? $message['level'] : WATCHDOG_INFO);
    }
  }

  /**
   * Initialize the array of entities to remove with all existing entities
   * previously imported from the source.
   *
   * @param FeedsSource $source
   *   Source information about this import.
   * @param FeedsState $state
   *   The FeedsState object for the given stage.
   */
  protected function initEntitiesToBeRemoved(FeedsSource $source, FeedsState $state) {
    $state->removeList = array();
    // We fill it only if needed.
    if (!isset($this->config['update_non_existent']) || $this->config['update_non_existent'] == FEEDS_SKIP_NON_EXISTENT) {
      return;
    }
    // Build base select statement.
    $info = $this->entityInfo();
    $id_key = db_escape_field($info['entity keys']['id']);
    $select = db_select($info['base table'], 'e');
    $select->addField('e', $info['entity keys']['id'], 'entity_id');
    $select->join(
      'feeds_item',
      'fi',
      'e.' . $id_key . ' = fi.entity_id AND fi.entity_type = :entity_type', array(
        ':entity_type' => $this->entityType(),
    ));

    $select->condition('fi.id', $this->id);
    $select->condition('fi.feed_nid', $source->feed_nid);
    // No need to remove item again if same method of removal was already used.
    $select->condition('fi.hash', $this->config['update_non_existent'], '<>');
    $entities = $select->execute();
    // If not found on process, existing entities will be deleted.
    foreach ($entities as $entity) {
      // Obviously, items which are still included in the source feed will be
      // removed from this array when processed.
      $state->removeList[$entity->entity_id] = $entity->entity_id;
    }
  }

  /**
   * Deletes entities which were not found during processing.
   *
   * @todo batch delete?
   *
   * @param FeedsState $state
   *   The FeedsState object for the given stage.
   */
  protected function clean(FeedsState $state) {
    // We clean only if needed.
    if (!isset($this->config['update_non_existent']) || $this->config['update_non_existent'] == FEEDS_SKIP_NON_EXISTENT) {
      return;
    }

    $total = count($state->removeList);
    if ($total) {
      $this->entityDeleteMultiple($state->removeList);
      $state->deleted += $total;
    }
  }

  /**
   * Remove all stored results or stored results up to a certain time for a
   * source.
   *
   * @param FeedsSource $source
   *   Source information for this expiry. Implementers should only delete items
   *   pertaining to this source. The preferred way of determining whether an
   *   item pertains to a certain souce is by using $source->feed_nid. It is the
   *   processor's responsibility to store the feed_nid of an imported item in
   *   the processing stage.
   */
  public function clear(FeedsSource $source) {
    $state = $source->state(FEEDS_PROCESS_CLEAR);

    // Build base select statement.
    $info = $this->entityInfo();
    $select = db_select($info['base table'], 'e');
    $select->addField('e', $info['entity keys']['id'], 'entity_id');
    $select->join(
      'feeds_item',
      'fi',
      "e.{$info['entity keys']['id']} = fi.entity_id AND fi.entity_type = '{$this->entityType()}'");
    $select->condition('fi.id', $this->id);
    $select->condition('fi.feed_nid', $source->feed_nid);

    // If there is no total, query it.
    if (!$state->total) {
      $state->total = $select->countQuery()
        ->execute()
        ->fetchField();
    }

    // Delete a batch of entities.
    $entities = $select->range(0, $this->getLimit())->execute();
    $entity_ids = array();
    foreach ($entities as $entity) {
      $entity_ids[$entity->entity_id] = $entity->entity_id;
    }
    $this->entityDeleteMultiple($entity_ids);

    // Report progress, take into account that we may not have deleted as
    // many items as we have counted at first.
    if (count($entity_ids)) {
      $state->deleted += count($entity_ids);
      $state->progress($state->total, $state->deleted);
    }
    else {
      $state->progress($state->total, $state->total);
    }

    // Report results when done.
    if ($source->progressClearing() == FEEDS_BATCH_COMPLETE) {
      if ($state->deleted) {
        $message = format_plural(
          $state->deleted,
          'Deleted @number @entity',
          'Deleted @number @entities',
          array(
            '@number' => $state->deleted,
            '@entity' => strtolower($info['label']),
            '@entities' => strtolower($info['label plural']),
          )
        );
        $source->log('clear', $message, array(), WATCHDOG_INFO);
        drupal_set_message($message);
      }
      else {
        drupal_set_message(t('There are no @entities to be deleted.', array('@entities' => $info['label plural'])));
      }
    }
  }

  /*
   * Report number of items that can be processed per call.
   *
   * 0 means 'unlimited'.
   *
   * If a number other than 0 is given, Feeds parsers that support batching
   * will only deliver this limit to the processor.
   *
   * @see FeedsSource::getLimit()
   * @see FeedsCSVParser::parse()
   */
  public function getLimit() {
    return variable_get('feeds_process_limit', FEEDS_PROCESS_LIMIT);
  }

  /**
   * Deletes feed items older than REQUEST_TIME - $time.
   *
   * Do not invoke expire on a processor directly, but use
   * FeedsSource::expire() instead.
   *
   * @param FeedsSource $source
   *   The source to expire entities for.
   *
   * @param $time
   *   (optional) All items produced by this configuration that are older than
   *   REQUEST_TIME - $time should be deleted. If NULL, processor should use
   *   internal configuration. Defaults to NULL.
   *
   * @return float
   *   FEEDS_BATCH_COMPLETE if all items have been processed, a float between 0
   *   and 0.99* indicating progress otherwise.
   *
   * @see FeedsSource::expire()
   */
  public function expire(FeedsSource $source, $time = NULL) {
    $state = $source->state(FEEDS_PROCESS_EXPIRE);

    if ($time === NULL) {
      $time = $this->expiryTime();
    }
    if ($time == FEEDS_EXPIRE_NEVER) {
      return;
    }

    $select = $this->expiryQuery($source, $time);

    // If there is no total, query it.
    if (!$state->total) {
      $state->total = $select->countQuery()->execute()->fetchField();
    }

    // Delete a batch of entities.
    $entity_ids = $select->range(0, $this->getLimit())->execute()->fetchCol();
    if ($entity_ids) {
      $this->entityDeleteMultiple($entity_ids);
      $state->deleted += count($entity_ids);
      $state->progress($state->total, $state->deleted);
    }
    else {
      $state->progress($state->total, $state->total);
    }
  }

  /**
   * Returns a database query used to select entities to expire.
   *
   * Processor classes should override this method to set the age portion of the
   * query.
   *
   * @param FeedsSource $source
   *   The feed source.
   * @param int $time
   *   Delete entities older than this.
   *
   * @return SelectQuery
   *   A select query to execute.
   *
   * @see FeedsNodeProcessor::expiryQuery()
   */
  protected function expiryQuery(FeedsSource $source, $time) {
    // Build base select statement.
    $info = $this->entityInfo();
    $id_key = db_escape_field($info['entity keys']['id']);

    $select = db_select($info['base table'], 'e');
    $select->addField('e', $info['entity keys']['id'], 'entity_id');
    $select->join(
      'feeds_item',
      'fi',
      "e.$id_key = fi.entity_id AND fi.entity_type = :entity_type", array(
        ':entity_type' => $this->entityType(),
    ));
    $select->condition('fi.id', $this->id);
    $select->condition('fi.feed_nid', $source->feed_nid);

    return $select;
  }

  /**
   * Counts the number of items imported by this processor.
   */
  public function itemCount(FeedsSource $source) {
    return db_query("SELECT count(*) FROM {feeds_item} WHERE id = :id AND entity_type = :entity_type AND feed_nid = :feed_nid", array(':id' => $this->id, ':entity_type' => $this->entityType(), ':feed_nid' => $source->feed_nid))->fetchField();
  }

  /**
   * Returns a statically cached version of the target mappings.
   *
   * @return array
   *   The targets for this importer.
   */
  protected function getCachedTargets() {
    $targets = &drupal_static('FeedsProcessor::getCachedTargets', array());

    if (!isset($targets[$this->id])) {
      $targets[$this->id] = $this->getMappingTargets();
    }

    return $targets[$this->id];
  }

  /**
   * Returns a statically cached version of the source mappings.
   *
   * @return array
   *   The sources for this importer.
   */
  protected function getCachedSources() {
    $sources = &drupal_static('FeedsProcessor::getCachedSources', array());

    if (!isset($sources[$this->id])) {

      $sources[$this->id] = feeds_importer($this->id)->parser->getMappingSources();

      if (is_array($sources[$this->id])) {
        foreach ($sources[$this->id] as $source_key => $source) {
          if (empty($source['callback']) || !is_callable($source['callback'])) {
            unset($sources[$this->id][$source_key]['callback']);
          }
        }
      }
    }

    return $sources[$this->id];
  }

  /**
   * Execute mapping on an item.
   *
   * This method encapsulates the central mapping functionality. When an item is
   * processed, it is passed through map() where the properties of $source_item
   * are mapped onto $target_item following the processor's mapping
   * configuration.
   *
   * For each mapping FeedsParser::getSourceElement() is executed to retrieve
   * the source element, then FeedsProcessor::setTargetElement() is invoked
   * to populate the target item properly. Alternatively a
   * hook_x_targets_alter() may have specified a callback for a mapping target
   * in which case the callback is asked to populate the target item instead of
   * FeedsProcessor::setTargetElement().
   *
   * @ingroup mappingapi
   *
   * @see hook_feeds_parser_sources_alter()
   * @see hook_feeds_processor_targets()
   * @see hook_feeds_processor_targets_alter()
   */
  protected function map(FeedsSource $source, FeedsParserResult $result, $target_item = NULL) {
    $targets = $this->getCachedTargets();

    if (empty($target_item)) {
      $target_item = array();
    }

    // Many mappers add to existing fields rather than replacing them. Hence we
    // need to clear target elements of each item before mapping in case we are
    // mapping on a prepopulated item such as an existing node.
    foreach ($this->config['mappings'] as $mapping) {
      if (isset($targets[$mapping['target']]['real_target'])) {
        $target_item->{$targets[$mapping['target']]['real_target']} = NULL;
      }
      else {
        $target_item->{$mapping['target']} = NULL;
      }
    }

    // This is where the actual mapping happens: For every mapping we invoke
    // the parser's getSourceElement() method to retrieve the value of the
    // source element and pass it to the processor's setTargetElement() to stick
    // it on the right place of the target item.
    foreach ($this->config['mappings'] as $mapping) {
      $value = $this->getSourceValue($source, $result, $mapping['source']);

      $this->mapToTarget($source, $mapping['target'], $target_item, $value, $mapping);
    }

    return $target_item;
  }

  /**
   * Returns the values from the parser, or callback.
   *
   * @param FeedsSource $source
   *   The feed source.
   * @param FeedsParserResult $result
   *   The parser result.
   * @param string $source_key
   *   The current key being processed.
   *
   * @return mixed
   *   A value, or a list of values.
   */
  protected function getSourceValue(FeedsSource $source, FeedsParserResult $result, $source_key) {
    $sources = $this->getCachedSources();

    if (isset($sources[$source_key]['callback'])) {
      return call_user_func($sources[$source_key]['callback'], $source, $result, $source_key);
    }

    return feeds_importer($this->id)->parser->getSourceElement($source, $result, $source_key);
  }

  /**
   * Maps values onto the target item.
   *
   * @param FeedsSource $source
   *   The feed source.
   * @param mixed &$target_item
   *   The target item to apply values into.
   * @param mixed $value
   *   A value, or a list of values.
   * @param array $mapping
   *   The mapping configuration.
   */
  protected function mapToTarget(FeedsSource $source, $target, &$target_item, $value, array $mapping) {
    $targets = $this->getCachedTargets();

    if (isset($targets[$target]['preprocess_callbacks'])) {
      foreach ($targets[$target]['preprocess_callbacks'] as $callback) {
        call_user_func_array($callback, array($source, $target_item, $target, &$mapping));
      }
    }

    // Map the source element's value to the target.
    // If the mapping specifies a callback method, use the callback instead of
    // setTargetElement().
    if (isset($targets[$target]['callback'])) {

      // All target callbacks expect an array.
      if (!is_array($value)) {
        $value = array($value);
      }

      call_user_func($targets[$target]['callback'], $source, $target_item, $target, $value, $mapping);
    }

    else {
      $this->setTargetElement($source, $target_item, $target, $value, $mapping);
    }
  }

  /**
   * Per default, don't support expiry. If processor supports expiry of imported
   * items, return the time after which items should be removed.
   */
  public function expiryTime() {
    return FEEDS_EXPIRE_NEVER;
  }

  /**
   * Declare default configuration.
   */
  public function configDefaults() {
    $info = $this->entityInfo();
    $bundle = NULL;
    if (empty($info['entity keys']['bundle'])) {
      $bundle = $this->entityType();
    }
    return array(
      'mappings' => array(),
      'update_existing' => FEEDS_SKIP_EXISTING,
      'update_non_existent' => FEEDS_SKIP_NON_EXISTENT,
      'input_format' => NULL,
      'skip_hash_check' => FALSE,
      'bundle' => $bundle,
    );
  }

  /**
   * Overrides parent::configForm().
   */
  public function configForm(&$form_state) {
    $info = $this->entityInfo();
    $form = array();

    if (!empty($info['entity keys']['bundle'])) {
      $form['bundle'] = array(
        '#type' => 'select',
        '#options' => $this->bundleOptions(),
        '#title' => !empty($info['bundle name']) ? $info['bundle name'] : t('Bundle'),
        '#required' => TRUE,
        '#default_value' => $this->bundle(),
      );
    }
    else {
      $form['bundle'] = array(
        '#type' => 'value',
        '#value' => $this->entityType(),
      );
    }

    $tokens = array('@entities' => strtolower($info['label plural']));

    $form['update_existing'] = array(
      '#type' => 'radios',
      '#title' => t('Update existing @entities', $tokens),
      '#description' =>
        t('Existing @entities will be determined using mappings that are a "unique target".', $tokens),
      '#options' => array(
        FEEDS_SKIP_EXISTING => t('Do not update existing @entities', $tokens),
        FEEDS_REPLACE_EXISTING => t('Replace existing @entities', $tokens),
        FEEDS_UPDATE_EXISTING => t('Update existing @entities', $tokens),
      ),
      '#default_value' => $this->config['update_existing'],
    );
    global $user;
    $formats = filter_formats($user);
    foreach ($formats as $format) {
      $format_options[$format->format] = $format->name;
    }
    $form['skip_hash_check'] = array(
      '#type' => 'checkbox',
      '#title' => t('Skip hash check'),
      '#description' => t('Force update of items even if item source data did not change.'),
      '#default_value' => $this->config['skip_hash_check'],
    );
    $form['input_format'] = array(
      '#type' => 'select',
      '#title' => t('Text format'),
      '#description' => t('Select the default input format for the text fields of the nodes to be created.'),
      '#options' => $format_options,
      '#default_value' => isset($this->config['input_format']) ? $this->config['input_format'] : 'plain_text',
      '#required' => TRUE,
    );

    $form['update_non_existent'] = array(
      '#type' => 'radios',
      '#title' => t('Action to take when previously imported @entities are missing in the feed', $tokens),
      '#description' => t('Select how @entities previously imported and now missing in the feed should be updated.', $tokens),
      '#options' => array(
        FEEDS_SKIP_NON_EXISTENT => t('Skip non-existent @entities', $tokens),
        FEEDS_DELETE_NON_EXISTENT => t('Delete non-existent @entities', $tokens),
      ),
      '#default_value' => $this->config['update_non_existent'],
    );

    return $form;
  }

  /**
   * Get mappings.
   */
  public function getMappings() {
    return isset($this->config['mappings']) ? $this->config['mappings'] : array();
  }

  /**
   * Declare possible mapping targets that this processor exposes.
   *
   * @ingroup mappingapi
   *
   * @return
   *   An array of mapping targets. Keys are paths to targets
   *   separated by ->, values are TRUE if target can be unique,
   *   FALSE otherwise.
   */
  public function getMappingTargets() {

    // The bundle has not been selected.
    if (!$this->bundle()) {
      $info = $this->entityInfo();
      $bundle_name = !empty($info['bundle name']) ? drupal_strtolower($info['bundle name']) : t('bundle');
      $plugin_key = feeds_importer($this->id)->config['processor']['plugin_key'];
      $url = url('admin/structure/feeds/' . $this->id . '/settings/' . $plugin_key);
      drupal_set_message(t('Please <a href="@url">select a @bundle_name</a>.', array('@url' => $url, '@bundle_name' => $bundle_name)), 'warning', FALSE);
    }

    return array(
      'url' => array(
        'name' => t('URL'),
        'description' => t('The external URL of the item. E. g. the feed item URL in the case of a syndication feed. May be unique.'),
        'optional_unique' => TRUE,
      ),
      'guid' => array(
        'name' => t('GUID'),
        'description' => t('The globally unique identifier of the item. E. g. the feed item GUID in the case of a syndication feed. May be unique.'),
        'optional_unique' => TRUE,
      ),
    );
  }

  /**
   * Allows other modules to expose targets.
   *
   * @param array &$targets
   *   The existing target array.
   */
  protected function getHookTargets(array &$targets) {
    self::loadMappers();

    $entity_type = $this->entityType();
    $bundle = $this->bundle();
    $targets += module_invoke_all('feeds_processor_targets', $entity_type, $bundle);

    drupal_alter('feeds_processor_targets', $targets, $entity_type, $bundle);
  }

  /**
   * Set a concrete target element. Invoked from FeedsProcessor::map().
   *
   * @ingroup mappingapi
   */
  public function setTargetElement(FeedsSource $source, $target_item, $target_element, $value) {
    switch ($target_element) {
      case 'url':
      case 'guid':
        $target_item->feeds_item->$target_element = $value;
        break;

      default:
        $target_item->$target_element = $value;
        break;
    }
  }

  /**
   * Retrieve the target entity's existing id if available. Otherwise return 0.
   *
   * @ingroup mappingapi
   *
   * @param FeedsSource $source
   *   The source information about this import.
   * @param FeedsParserResult $result
   *   A FeedsParserResult object.
   *
   * @return int
   *   The serial id of an entity if found, 0 otherwise.
   */
  protected function existingEntityId(FeedsSource $source, FeedsParserResult $result) {
    $targets = $this->getCachedTargets();

    $entity_id = 0;

    // Iterate through all unique targets and test whether they already exist in
    // the database.
    foreach ($this->uniqueTargets($source, $result) as $target => $value) {
      if ($target === 'guid' || $target === 'url') {
        $entity_id = db_select('feeds_item')
          ->fields('feeds_item', array('entity_id'))
          ->condition('feed_nid', $source->feed_nid)
          ->condition('entity_type', $this->entityType())
          ->condition('id', $source->id)
          ->condition($target, $value)
          ->execute()
          ->fetchField();
      }

      if (!$entity_id && !empty($targets[$target]['unique_callbacks'])) {
        if (!is_array($value)) {
          $value = array($value);
        }

        foreach ($targets[$target]['unique_callbacks'] as $callback) {
          if ($entity_id = call_user_func($callback, $source, $this->entityType(), $this->bundle(), $target, $value)) {
            // Stop at the first unique ID returned by a callback.
            break;
          }
        }
      }

      // Return with the content id found.
      if ($entity_id) {
        return $entity_id;
      }
    }

    return $entity_id;
  }


  /**
   * Utility function that iterates over a target array and retrieves all
   * sources that are unique.
   *
   * @param $batch
   *   A FeedsImportBatch.
   *
   * @return
   *   An array where the keys are target field names and the values are the
   *   elements from the source item mapped to these targets.
   */
  protected function uniqueTargets(FeedsSource $source, FeedsParserResult $result) {
    $parser = feeds_importer($this->id)->parser;
    $targets = array();
    foreach ($this->config['mappings'] as $mapping) {
      if (!empty($mapping['unique'])) {
        // Invoke the parser's getSourceElement to retrieve the value for this
        // mapping's source.
        $targets[$mapping['target']] = $parser->getSourceElement($source, $result, $mapping['source']);
      }
    }
    return $targets;
  }

  /**
   * Adds Feeds specific information on $entity->feeds_item.
   *
   * @param $entity
   *   The entity object to be populated with new item info.
   * @param $feed_nid
   *   The feed nid of the source that produces this entity.
   * @param $hash
   *   The fingerprint of the source item.
   */
  protected function newItemInfo($entity, $feed_nid, $hash = '') {
    $entity->feeds_item = new stdClass();
    $entity->feeds_item->is_new = TRUE;
    $entity->feeds_item->entity_id = 0;
    $entity->feeds_item->entity_type = $this->entityType();
    $entity->feeds_item->id = $this->id;
    $entity->feeds_item->feed_nid = $feed_nid;
    $entity->feeds_item->imported = REQUEST_TIME;
    $entity->feeds_item->hash = $hash;
    $entity->feeds_item->url = '';
    $entity->feeds_item->guid = '';
  }

  /**
   * Loads existing entity information and places it on $entity->feeds_item.
   *
   * @param $entity
   *   The entity object to load item info for. Id key must be present.
   *
   * @return
   *   TRUE if item info could be loaded, false if not.
   */
  protected function loadItemInfo($entity) {
    $entity_info = entity_get_info($this->entityType());
    $key = $entity_info['entity keys']['id'];
    if ($item_info = feeds_item_info_load($this->entityType(), $entity->$key)) {
      $entity->feeds_item = $item_info;
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Create MD5 hash of item and mappings array.
   *
   * Include mappings as a change in mappings may have an affect on the item
   * produced.
   *
   * @return Always returns a hash, even with empty, NULL, FALSE:
   *  Empty arrays return 40cd750bba9870f18aada2478b24840a
   *  Empty/NULL/FALSE strings return d41d8cd98f00b204e9800998ecf8427e
   */
  protected function hash($item) {
    return hash('md5', serialize($item) . serialize($this->config['mappings']));
  }

  /**
   * Retrieves the MD5 hash of $entity_id from the database.
   *
   * @return string
   *   Empty string if no item is found, hash otherwise.
   */
  protected function getHash($entity_id) {

    if ($hash = db_query("SELECT hash FROM {feeds_item} WHERE entity_type = :type AND entity_id = :id", array(':type' => $this->entityType(), ':id' => $entity_id))->fetchField()) {
      // Return with the hash.
      return $hash;
    }
    return '';
  }

  /**
   * DEPRECATED: Creates a log message for exceptions during import.
   *
   * Don't use this method as it concatenates user variables into the log
   * message, which will pollute the locales_source table when the log message
   * is translated. Use ::createLogEntry instead.
   *
   * @param Exception $e
   *   The exception that was throwned during processing the item.
   * @param $entity
   *   The entity object.
   * @param $item
   *   The parser result for this entity.
   *
   * @return string
   *   The message to log.
   *
   * @deprecated
   *   Use ::createLogEntry instead.
   */
  protected function createLogMessage(Exception $e, $entity, $item) {
    $message = $e->getMessage();
    $message .= '<h3>Original item</h3>';
    // $this->exportObjectVars() already runs check_plain() for us, so we can
    // concatenate here as is.
    $message .= '<pre>' . $this->exportObjectVars($item) . '</pre>';
    $message .= '<h3>Entity</h3>';
    $message .= '<pre>' . $this->exportObjectVars($entity) . '</pre>';

    return $message;
  }

  /**
   * Creates a log entry for when an exception occured during import.
   *
   * @param Exception $e
   *   The exception that was throwned during processing the item.
   * @param object $entity
   *   The entity object.
   * @param array $item
   *   The parser result for this entity.
   *
   * @return array
   *   The message and arguments to log.
   */
  protected function createLogEntry(Exception $e, $entity, $item) {
    $message = '@exception';
    $message .= '<h3>Original item</h3>';
    $message .= '<pre>!item</pre>';
    $message .= '<h3>Entity</h3>';
    $message .= '<pre>!entity</pre>';
    $arguments = array(
      '@exception' => $e->getMessage(),
      // $this->exportObjectVars() already runs check_plain() for us, so we can
      // use the "!" placeholder.
      '!item' => $this->exportObjectVars($item),
      '!entity' => $this->exportObjectVars($entity),
    );

    return array($message, $arguments);
  }

  /**
   * Returns a string representation of an object or array for log messages.
   *
   * @param object|array $object
   *   The object to convert.
   *
   * @return string
   *   The sanitized string representation of the object.
   */
  protected function exportObjectVars($object) {
    include_once DRUPAL_ROOT . '/includes/utility.inc';

    $out = is_array($object) ? $object : get_object_vars($object);
    $out = array_filter($out, 'is_scalar');

    foreach ($out as $key => $value) {
      if (is_string($value)) {
        $out[$key] = truncate_utf8($value, 100, FALSE, TRUE);
      }
    }

    if (is_array($object)) {
      return check_plain(drupal_var_export($out));
    }

    return check_plain(drupal_var_export((object) $out));
  }

  /**
   * Overrides FeedsPlugin::dependencies().
   */
  public function dependencies() {
    $dependencies = parent::dependencies();

    // Find out which module defined the entity type.
    $info = $this->entityInfo();
    if (isset($info['module'])) {
      $dependencies[$info['module']] = $info['module'];
    }

    return $dependencies;
  }

}

class FeedsProcessorBundleNotDefined extends Exception {}
