<?php

/**
 * @file
 * Definition of FeedsSourceInterface and FeedsSource class.
 */

/**
 * Distinguish exceptions occurring when handling locks.
 */
class FeedsLockException extends Exception {}

/**
 * Denote a import or clearing stage. Used for multi page processing.
 */
define('FEEDS_START', 'start_time');
define('FEEDS_FETCH', 'fetch');
define('FEEDS_PARSE', 'parse');
define('FEEDS_PROCESS', 'process');
define('FEEDS_PROCESS_CLEAR', 'process_clear');
define('FEEDS_PROCESS_EXPIRE', 'process_expire');

/**
 * Declares an interface for a class that defines default values and form
 * descriptions for a FeedSource.
 */
interface FeedsSourceInterface {

  /**
   * Crutch: for ease of use, we implement FeedsSourceInterface for every
   * plugin, but then we need to have a handle which plugin actually implements
   * a source.
   *
   * @see FeedsPlugin class.
   *
   * @return
   *   TRUE if a plugin handles source specific configuration, FALSE otherwise.
   */
  public function hasSourceConfig();

  /**
   * Return an associative array of default values.
   */
  public function sourceDefaults();

  /**
   * Return a Form API form array that defines a form configuring values. Keys
   * correspond to the keys of the return value of sourceDefaults().
   */
  public function sourceForm($source_config);

  /**
   * Validate user entered values submitted by sourceForm().
   */
  public function sourceFormValidate(&$source_config);

  /**
   * A source is being saved.
   */
  public function sourceSave(FeedsSource $source);

  /**
   * A source is being deleted.
   */
  public function sourceDelete(FeedsSource $source);
}

/**
 * Status of an import or clearing operation on a source.
 */
class FeedsState {
  /**
   * Floating point number denoting the progress made. 0.0 meaning no progress
   * 1.0 = FEEDS_BATCH_COMPLETE meaning finished.
   */
  public $progress;

  /**
   * Used as a pointer to store where left off. Must be serializable.
   */
  public $pointer;

  /**
   * Natural numbers denoting more details about the progress being made.
   */
  public $total;
  public $created;
  public $updated;
  public $deleted;
  public $unpublished;
  public $blocked;
  public $skipped;
  public $failed;

  /**
   * IDs of entities to be removed.
   */
  public $removeList;

  /**
   * Constructor, initialize variables.
   */
  public function __construct() {
    $this->progress = FEEDS_BATCH_COMPLETE;
    $this->total =
    $this->created =
    $this->updated =
    $this->deleted =
    $this->unpublished =
    $this->blocked =
    $this->skipped =
    $this->failed = 0;
  }

  /**
   * Safely report progress.
   *
   * When $total == $progress, the state of the task tracked by this state is
   * regarded to be complete.
   *
   * Handles the following cases gracefully:
   *
   * - $total is 0
   * - $progress is larger than $total
   * - $progress approximates $total so that $finished rounds to 1.0
   *
   * @param $total
   *   A natural number that is the total to be worked off.
   * @param $progress
   *   A natural number that is the progress made on $total.
   */
  public function progress($total, $progress) {
    if ($progress > $total) {
      $this->progress = FEEDS_BATCH_COMPLETE;
    }
    elseif ($total) {
      $this->progress = (float) $progress / $total;
      if ($this->progress == FEEDS_BATCH_COMPLETE && $total != $progress) {
        $this->progress = 0.99;
      }
    }
    else {
      $this->progress = FEEDS_BATCH_COMPLETE;
    }
  }
}

/**
 * This class encapsulates a source of a feed. It stores where the feed can be
 * found and how to import it.
 *
 * Information on how to import a feed is encapsulated in a FeedsImporter object
 * which is identified by the common id of the FeedsSource and the
 * FeedsImporter. More than one FeedsSource can use the same FeedsImporter
 * therefore a FeedsImporter never holds a pointer to a FeedsSource object, nor
 * does it hold any other information for a particular FeedsSource object.
 *
 * Classes extending FeedsPlugin can implement a sourceForm to expose
 * configuration for a FeedsSource object. This is for instance how FeedsFetcher
 * exposes a text field for a feed URL or how FeedsCSVParser exposes a select
 * field for choosing between colon or semicolon delimiters.
 *
 * It is important that a FeedsPlugin does not directly hold information about
 * a source but leave all storage up to FeedsSource. An instance of a
 * FeedsPlugin class only exists once per FeedsImporter configuration, while an
 * instance of a FeedsSource class exists once per feed_nid to be imported.
 *
 * As with FeedsImporter, the idea with FeedsSource is that it can be used
 * without actually saving the object to the database.
 */
class FeedsSource extends FeedsConfigurable {

  // Contains the node id of the feed this source info object is attached to.
  // Equals 0 if not attached to any node - i. e. if used on a
  // standalone import form within Feeds or by other API users.
  protected $feed_nid;

  // The FeedsImporter object that this source is expected to be used with.
  protected $importer;

  // A FeedsSourceState object holding the current import/clearing state of this
  // source.
  protected $state;

  // Fetcher result, used to cache fetcher result when batching.
  protected $fetcher_result;

  // Timestamp when this source was imported the last time.
  protected $imported;

  // Holds an exception object in case an exception occurs during importing.
  protected $exception;

  /**
   * Instantiate a unique object per class/id/feed_nid. Don't use
   * directly, use feeds_source() instead.
   */
  public static function instance($importer_id, $feed_nid) {
    $class = variable_get('feeds_source_class', 'FeedsSource');

    $instances = &drupal_static(__METHOD__, array());

    if (!isset($instances[$class][$importer_id][$feed_nid])) {
      $instances[$class][$importer_id][$feed_nid] = new $class($importer_id, $feed_nid);
    }
    return $instances[$class][$importer_id][$feed_nid];
  }

  /**
   * Constructor.
   */
  protected function __construct($importer_id, $feed_nid) {
    $this->feed_nid = $feed_nid;
    $this->importer = feeds_importer($importer_id);
    parent::__construct($importer_id);
    $this->load();
  }

  /**
   * Returns the FeedsImporter object that this source is expected to be used with.
   */
  public function importer() {
    return $this->importer;
  }

  /**
   * Preview = fetch and parse a feed.
   *
   * @return
   *   FeedsParserResult object.
   *
   * @throws
   *   Throws Exception if an error occurs when fetching or parsing.
   */
  public function preview() {
    $result = $this->importer->fetcher->fetch($this);
    $result = $this->importer->parser->parse($this, $result);
    module_invoke_all('feeds_after_parse', $this, $result);
    return $result;
  }

  /**
   * Start importing a source.
   *
   * This method starts an import job. Depending on the configuration of the
   * importer of this source, a Batch API job or a background job with Job
   * Scheduler will be created.
   *
   * @throws Exception
   *   If processing in background is enabled, the first batch chunk of the
   *   import will be executed on the current page request. This means that this
   *   method may throw the same exceptions as FeedsSource::import().
   */
  public function startImport() {
    $config = $this->importer->getConfig();
    if ($config['process_in_background']) {
      $this->startBackgroundJob('import');
    }
    else {
      $this->startBatchAPIJob(t('Importing'), 'import');
    }
  }

  /**
   * Start deleting all imported items of a source.
   *
   * This method starts a clear job. Depending on the configuration of the
   * importer of this source, a Batch API job or a background job with Job
   * Scheduler will be created.
   *
   * @throws Exception
   *   If processing in background is enabled, the first batch chunk of the
   *   clear task will be executed on the current page request. This means that
   *   this method may throw the same exceptions as FeedsSource::clear().
   */
  public function startClear() {
    $config = $this->importer->getConfig();
    if ($config['process_in_background']) {
      $this->startBackgroundJob('clear');
    }
    else {
      $this->startBatchAPIJob(t('Deleting'), 'clear');
    }
  }

  /**
   * Schedule all periodic tasks for this source.
   */
  public function schedule() {
    $this->scheduleImport();
    $this->scheduleExpire();
  }

  /**
   * Schedule periodic or background import tasks.
   */
  public function scheduleImport() {
    // Check whether any fetcher is overriding the import period.
    $period = $this->importer->config['import_period'];
    $fetcher_period = $this->importer->fetcher->importPeriod($this);
    if (is_numeric($fetcher_period)) {
      $period = $fetcher_period;
    }
    $job = array(
      'type' => $this->id,
      'id' => $this->feed_nid,
      'period' => $period,
      'periodic' => TRUE,
    );
    if ($period == FEEDS_SCHEDULE_NEVER && $this->progressImporting() === FEEDS_BATCH_COMPLETE) {
      JobScheduler::get('feeds_source_import')->remove($job);
    }
    elseif ($this->progressImporting() === FEEDS_BATCH_COMPLETE) {
      JobScheduler::get('feeds_source_import')->set($job);
    }
    else {
      // Feed is not fully imported yet, so we put this job back in the queue
      // immediately for further processing.
      $queue = DrupalQueue::get('feeds_source_import');
      $queue->createItem($job);
    }
  }

  /**
   * Schedule background expire tasks.
   */
  public function scheduleExpire() {
    // Schedule as soon as possible if a batch is active.
    $period = $this->progressExpiring() === FEEDS_BATCH_COMPLETE ? 3600 : 0;

    $job = array(
      'type' => $this->id,
      'id' => $this->feed_nid,
      'period' => $period,
      'periodic' => TRUE,
    );
    if ($this->importer->processor->expiryTime() == FEEDS_EXPIRE_NEVER) {
      JobScheduler::get('feeds_source_expire')->remove($job);
    }
    else {
      JobScheduler::get('feeds_source_expire')->set($job);
    }
  }

  /**
   * Schedule background clearing tasks.
   */
  public function scheduleClear() {
    $job = array(
      'type' => $this->id,
      'id' => $this->feed_nid,
      'period' => 0,
      'periodic' => TRUE,
    );
    // Remove job if batch is complete.
    if ($this->progressClearing() === FEEDS_BATCH_COMPLETE) {
      JobScheduler::get('feeds_source_clear')->remove($job);
    }
    // Schedule as soon as possible if batch is not complete.
    else {
      JobScheduler::get('feeds_source_clear')->set($job);
    }
  }

  /**
   * Import a source: execute fetching, parsing and processing stage.
   *
   * This method only executes the current batch chunk, then returns. If you are
   * looking to import an entire source, use FeedsSource::startImport() instead.
   *
   * @return
   *   FEEDS_BATCH_COMPLETE if the import process finished. A decimal between
   *   0.0 and 0.9 periodic if import is still in progress.
   *
   * @throws
   *   Throws Exception if an error occurs when importing.
   */
  public function import() {
    $this->acquireLock();
    try {
      // If fetcher result is empty, we are starting a new import, log.
      if (empty($this->fetcher_result)) {
        module_invoke_all('feeds_before_import', $this);
        $this->state[FEEDS_START] = time();
      }

      // Fetch.
      if (empty($this->fetcher_result) || FEEDS_BATCH_COMPLETE == $this->progressParsing()) {
        $this->fetcher_result = $this->importer->fetcher->fetch($this);
        // Clean the parser's state, we are parsing an entirely new file.
        unset($this->state[FEEDS_PARSE]);
      }

      // Parse.
      $parser_result = $this->importer->parser->parse($this, $this->fetcher_result);
      module_invoke_all('feeds_after_parse', $this, $parser_result);

      // Process.
      $this->importer->processor->process($this, $parser_result);

      // Import finished without exceptions, so unset any potentially previously
      // recorded exceptions.
      unset($this->exception);
    }
    catch (Exception $e) {
      // $e is stored and re-thrown once we've had a chance to log our progress.
      // Set the exception so that other modules can check if an exception
      // occurred in hook_feeds_after_import().
      $this->exception = $e;
    }

    // Clean up.
    $result = $this->progressImporting();
    if ($result == FEEDS_BATCH_COMPLETE || isset($e)) {
      $this->imported = time();
      $this->log('import', 'Imported in @s seconds.', array('@s' => $this->imported - $this->state[FEEDS_START]), WATCHDOG_INFO);
      module_invoke_all('feeds_after_import', $this);
      unset($this->fetcher_result, $this->state);
    }
    $this->save();

    $this->releaseLock();

    if (isset($e)) {
      throw $e;
    }

    return $result;
  }

  /**
   * Imports a fetcher result all at once in memory.
   *
   * @param FeedsFetcherResult $fetcher_result
   *   The fetcher result to process.
   *
   * @throws Exception
   *   Thrown if an error occurs when importing.
   */
  public function pushImport(FeedsFetcherResult $fetcher_result) {
    // Since locks only work during a request, check if an import is active.
    if (!empty($this->fetcher_result) || !empty($this->state)) {
      throw new RuntimeException('The feed is currently importing.');
    }

    $this->acquireLock();
    $start = time();

    try {
      module_invoke_all('feeds_before_import', $this);

      // Parse.
      do {
        $parser_result = $this->importer->parser->parse($this, $fetcher_result);
        module_invoke_all('feeds_after_parse', $this, $parser_result);

        // Process.
        $this->importer->processor->process($this, $parser_result);

      } while ($this->progressParsing() !== FEEDS_BATCH_COMPLETE);
    }
    catch (Exception $e) {
      // $e is stored and re-thrown once we've had a chance to log our progress.
      // Set the exception so that other modules can check if an exception
      // occurred in hook_feeds_after_import().
      $this->exception = $e;
    }

    module_invoke_all('feeds_after_import', $this);

    $this->imported = time();
    $this->log('import', 'Imported in @s seconds.', array('@s' => $this->imported - $start), WATCHDOG_INFO);

    unset($this->fetcher_result, $this->state);

    $this->save();

    $this->releaseLock();

    if (isset($e)) {
      throw $e;
    }
  }

  /**
   * Remove all items from a feed.
   *
   * This method only executes the current batch chunk, then returns. If you are
   * looking to delete all items of a source, use FeedsSource::startClear()
   * instead.
   *
   * @return
   *   FEEDS_BATCH_COMPLETE if the clearing process finished. A decimal between
   *   0.0 and 0.9 periodic if clearing is still in progress.
   *
   * @throws
   *   Throws Exception if an error occurs when clearing.
   */
  public function clear() {
    $this->acquireLock();
    try {
      $this->importer->fetcher->clear($this);
      $this->importer->parser->clear($this);
      $this->importer->processor->clear($this);
    }
    catch (Exception $e) {
      // $e is stored and re-thrown once we've had a chance to log our progress.
    }
    $this->releaseLock();

    // Clean up.
    $result = $this->progressClearing();
    if ($result == FEEDS_BATCH_COMPLETE || isset($e)) {
      module_invoke_all('feeds_after_clear', $this);
      unset($this->state);
    }
    $this->save();
    if (isset($e)) {
      throw $e;
    }
    return $result;
  }

  /**
   * Removes all expired items from a feed.
   */
  public function expire() {
    $this->acquireLock();
    try {
      $result = $this->importer->processor->expire($this);
    }
    catch (Exception $e) {
      // Will throw after the lock is released.
    }
    $this->releaseLock();

    if (isset($e)) {
      throw $e;
    }

    return $result;
  }

  /**
   * Report progress as float between 0 and 1. 1 = FEEDS_BATCH_COMPLETE.
   */
  public function progressParsing() {
    return $this->state(FEEDS_PARSE)->progress;
  }

  /**
   * Report progress as float between 0 and 1. 1 = FEEDS_BATCH_COMPLETE.
   */
  public function progressImporting() {
    $fetcher = $this->state(FEEDS_FETCH);
    $parser = $this->state(FEEDS_PARSE);
    if ($fetcher->progress == FEEDS_BATCH_COMPLETE && $parser->progress == FEEDS_BATCH_COMPLETE) {
      return FEEDS_BATCH_COMPLETE;
    }
    // Fetching envelops parsing.
    // @todo: this assumes all fetchers neatly use total. May not be the case.
    $fetcher_fraction = $fetcher->total ? 1.0 / $fetcher->total : 1.0;
    $parser_progress = $parser->progress * $fetcher_fraction;
    $result = $fetcher->progress - $fetcher_fraction + $parser_progress;
    if ($result == FEEDS_BATCH_COMPLETE) {
      return 0.99;
    }
    return $result;
  }

  /**
   * Report progress on clearing.
   */
  public function progressClearing() {
    return $this->state(FEEDS_PROCESS_CLEAR)->progress;
  }

  /**
   * Report progress on expiry.
   */
  public function progressExpiring() {
    return $this->state(FEEDS_PROCESS_EXPIRE)->progress;
  }

  /**
   * Return a state object for a given stage. Lazy instantiates new states.
   *
   * @todo Rename getConfigFor() accordingly to config().
   *
   * @param $stage
   *   One of FEEDS_FETCH, FEEDS_PARSE, FEEDS_PROCESS or FEEDS_PROCESS_CLEAR.
   *
   * @return
   *   The FeedsState object for the given stage.
   */
  public function state($stage) {
    if (!is_array($this->state)) {
      $this->state = array();
    }
    if (!isset($this->state[$stage])) {
      $this->state[$stage] = new FeedsState();
    }
    return $this->state[$stage];
  }

  /**
   * Count items imported by this source.
   */
  public function itemCount() {
    return $this->importer->processor->itemCount($this);
  }

  /**
   * Save configuration.
   */
  public function save() {
    // Alert implementers of FeedsSourceInterface to the fact that we're saving.
    foreach ($this->importer->plugin_types as $type) {
      $this->importer->$type->sourceSave($this);
    }
    $config = $this->getConfig();

    // Store the source property of the fetcher in a separate column so that we
    // can do fast lookups on it.
    $source = '';
    if (isset($config[get_class($this->importer->fetcher)]['source'])) {
      $source = $config[get_class($this->importer->fetcher)]['source'];
    }
    $object = array(
      'id' => $this->id,
      'feed_nid' => $this->feed_nid,
      'imported' => $this->imported,
      'config' => $config,
      'source' => $source,
      'state' => isset($this->state) ? $this->state : FALSE,
      'fetcher_result' => isset($this->fetcher_result) ? $this->fetcher_result : FALSE,
    );
    if (db_query_range("SELECT 1 FROM {feeds_source} WHERE id = :id AND feed_nid = :nid", 0, 1, array(':id' => $this->id, ':nid' => $this->feed_nid))->fetchField()) {
      drupal_write_record('feeds_source', $object, array('id', 'feed_nid'));
    }
    else {
      drupal_write_record('feeds_source', $object);
    }
  }

  /**
   * Load configuration and unpack.
   *
   * @todo Patch CTools to move constants from export.inc to ctools.module.
   */
  public function load() {
    if ($record = db_query("SELECT imported, config, state, fetcher_result FROM {feeds_source} WHERE id = :id AND feed_nid = :nid", array(':id' => $this->id, ':nid' => $this->feed_nid))->fetchObject()) {
      // While FeedsSource cannot be exported, we still use CTool's export.inc
      // export definitions.
      ctools_include('export');
      $this->export_type = EXPORT_IN_DATABASE;
      $this->imported = $record->imported;
      $this->config = unserialize($record->config);
      if (!empty($record->state)) {
        $this->state = unserialize($record->state);
      }
      if (!is_array($this->state)) {
        $this->state = array();
      }
      if (!empty($record->fetcher_result)) {
        $this->fetcher_result = unserialize($record->fetcher_result);
      }
    }
  }

  /**
   * Delete configuration. Removes configuration information
   * from database, does not delete configuration itself.
   */
  public function delete() {
    // Alert implementers of FeedsSourceInterface to the fact that we're
    // deleting.
    foreach ($this->importer->plugin_types as $type) {
      $this->importer->$type->sourceDelete($this);
    }
    db_delete('feeds_source')
      ->condition('id', $this->id)
      ->condition('feed_nid', $this->feed_nid)
      ->execute();
    // Remove from schedule.
    $job = array(
      'type' => $this->id,
      'id' => $this->feed_nid,
    );
    JobScheduler::get('feeds_source_import')->remove($job);
    JobScheduler::get('feeds_source_expire')->remove($job);
  }

  /**
   * Only return source if configuration is persistent and valid.
   *
   * @see FeedsConfigurable::existing().
   */
  public function existing() {
    // If there is no feed nid given, there must be no content type specified.
    // If there is a feed nid given, there must be a content type specified.
    // Ensure that importer is persistent (= defined in code or DB).
    // Ensure that source is persistent (= defined in DB).
    if ((empty($this->feed_nid) && empty($this->importer->config['content_type'])) ||
        (!empty($this->feed_nid) && !empty($this->importer->config['content_type']))) {
      $this->importer->existing();
      return parent::existing();
    }
    throw new FeedsNotExistingException(t('Source configuration not valid.'));
  }

  /**
   * Returns the configuration for a specific client class.
   *
   * @param FeedsSourceInterface $client
   *   An object that is an implementer of FeedsSourceInterface.
   *
   * @return
   *   An array stored for $client.
   */
  public function getConfigFor(FeedsSourceInterface $client) {
    $class = get_class($client);
    return isset($this->config[$class]) ? $this->config[$class] : $client->sourceDefaults();
  }

  /**
   * Sets the configuration for a specific client class.
   *
   * @param FeedsSourceInterface $client
   *   An object that is an implementer of FeedsSourceInterface.
   * @param $config
   *   The configuration for $client.
   *
   * @return
   *   An array stored for $client.
   */
  public function setConfigFor(FeedsSourceInterface $client, $config) {
    $this->config[get_class($client)] = $config;
  }

  /**
   * Return defaults for feed configuration.
   */
  public function configDefaults() {
    // Collect information from plugins.
    $defaults = array();
    foreach ($this->importer->plugin_types as $type) {
      if ($this->importer->$type->hasSourceConfig()) {
        $defaults[get_class($this->importer->$type)] = $this->importer->$type->sourceDefaults();
      }
    }
    return $defaults;
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    // Collect information from plugins.
    $form = array();
    foreach ($this->importer->plugin_types as $type) {
      if ($this->importer->$type->hasSourceConfig()) {
        $class = get_class($this->importer->$type);
        $config = isset($this->config[$class]) ? $this->config[$class] : array();
        $form[$class] = $this->importer->$type->sourceForm($config);
        $form[$class]['#tree'] = TRUE;
      }
    }
    return $form;
  }

  /**
   * Override parent::configFormValidate().
   */
  public function configFormValidate(&$values) {
    foreach ($this->importer->plugin_types as $type) {
      $class = get_class($this->importer->$type);
      if (isset($values[$class]) && $this->importer->$type->hasSourceConfig()) {
        $this->importer->$type->sourceFormValidate($values[$class]);
      }
    }
  }

  /**
   * Writes to feeds log.
   */
  public function log($type, $message, $variables = array(), $severity = WATCHDOG_NOTICE) {
    feeds_log($this->id, $this->feed_nid, $type, $message, $variables, $severity);
  }

  /**
   * Background job helper. Starts a background job using Job Scheduler.
   *
   * Execute the first batch chunk of a background job on the current page load,
   * moves the rest of the job processing to a cron powered background job.
   *
   * Executing the first batch chunk is important, otherwise, when a user
   * submits a source for import or clearing, we will leave her without any
   * visual indicators of an ongoing job.
   *
   * @see FeedsSource::startImport().
   * @see FeedsSource::startClear().
   *
   * @param $method
   *   Method to execute on importer; one of 'import' or 'clear'.
   *
   * @throws Exception $e
   */
  protected function startBackgroundJob($method) {
    if (FEEDS_BATCH_COMPLETE != $this->$method()) {
      $job = array(
        'type' => $this->id,
        'id' => $this->feed_nid,
        'period' => 0,
        'periodic' => FALSE,
      );
      JobScheduler::get("feeds_source_{$method}")->set($job);
    }
  }

  /**
   * Batch API helper. Starts a Batch API job.
   *
   * @see FeedsSource::startImport().
   * @see FeedsSource::startClear().
   * @see feeds_batch()
   *
   * @param $title
   *   Title to show to user when executing batch.
   * @param $method
   *   Method to execute on importer; one of 'import' or 'clear'.
   */
  protected function startBatchAPIJob($title, $method) {
    $batch = array(
      'title' => $title,
      'operations' => array(
        array('feeds_batch', array($method, $this->id, $this->feed_nid)),
      ),
      'progress_message' => '',
    );
    batch_set($batch);
  }

  /**
   * Acquires a lock for this source.
   *
   * @throws FeedsLockException
   *   If a lock for the requested job could not be acquired.
   */
  protected function acquireLock() {
    if (!lock_acquire("feeds_source_{$this->id}_{$this->feed_nid}", 60.0)) {
      throw new FeedsLockException(t('Cannot acquire lock for source @id / @feed_nid.', array('@id' => $this->id, '@feed_nid' => $this->feed_nid)));
    }
  }

  /**
   * Releases a lock for this source.
   */
  protected function releaseLock() {
    lock_release("feeds_source_{$this->id}_{$this->feed_nid}");
  }

  /**
   * Implements FeedsConfigurable::dependencies().
   */
  public function dependencies() {
    $dependencies = parent::dependencies();
    return array_merge($dependencies, $this->importer()->dependencies());
  }

}
