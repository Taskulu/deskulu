<?php

/**
 * @file
 * Contains SearchApiDbService.
 */

/**
 * Indexes and searches items using the database.
 *
 * Database SELECT queries issued by this service class will be marked with tags
 * according to their context. The following are used:
 * - search_api_db_search: For all queries that are based on a search query.
 * - search_api_db_facets_base: For the query which creates a temporary results
 *   table to be used for facetting. (Is always used in conjunction with
 *   "search_api_db_search".)
 * - search_api_db_facet: For queries on the temporary results table for
 *   determining the items of a specific facet.
 * - search_api_db_facet_all: For queries to return all indexed values for a
 *   specific field. Is used when a facet has a "min_count" of 0.
 * - search_api_db_autocomplete: For queries which create a temporary results
 *   table to be used for computing autocomplete suggestions. (Is always used in
 *   conjunction with "search_api_db_search".)
 *
 * The following metadata will be present for those SELECT queries:
 * - search_api_query: The Search API query object. (Always present.)
 * - search_api_db_fields: Internal storage information for the indexed fields,
 *   as used by this service class. (Always present.)
 * - search_api_db_facet: The settings array of the facet currently being
 *   computed. (Present for "search_api_db_facet" and "search_api_db_facet_all"
 *   queries.)
 * - search_api_db_autocomplete: An array containing the parameters of the
 *   getAutocompleteSuggestions() call, except "query". (Present for
 *   "search_api_db_autocomplete" queries.)
 */
class SearchApiDbService extends SearchApiAbstractService {

  /**
   * Multiplier for scores to have precision when converted from float to int.
   */
  const SCORE_MULTIPLIER = 1000;

  /**
   * The database connection to use for this server.
   *
   * @var DatabaseConnection
   */
  protected $connection;

  /**
   * The keywords ignored during the current search query.
   *
   * @var array
   */
  protected $ignored = array();

  /**
   * All warnings for the current search query.
   *
   * @var array
   */
  protected $warnings = array();

  /**
   * {@inheritdoc}
   */
  public function __construct(SearchApiServer $server) {
    parent::__construct($server);
    if (isset($this->options['database'])) {
      list($key, $target) = explode(':', $this->options['database'], 2);
      $this->connection = Database::getConnection($target, $key);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function configurationForm(array $form, array &$form_state) {
    // Discern between creation and editing of a server, since we don't allow
    // the database to be changed later on.
    if (empty($this->options)) {
      global $databases;
      foreach ($databases as $key => $targets) {
        foreach ($targets as $target => $info) {
          $options[$key]["$key:$target"] = "$key > $target";
        }
      }
      if (count($options) > 1 || count(reset($options)) > 1) {
        $form['database'] = array(
          '#type' => 'select',
          '#title' => t('Database'),
          '#description' => t('Select the database key and target to use for storing indexing information in. ' .
              'Cannot be changed after creation.'),
          '#options' => $options,
          '#default_value' => 'default:default',
          '#required' => TRUE,
        );
      }
      else {
        $form['database'] = array(
          '#type' => 'value',
          '#value' => "$key:$target",
        );
      }
    }
    else {
      $form = array(
        'database' => array(
          '#type' => 'value',
          '#value' => $this->options['database'],
        ),
        'database_text' => array(
          '#type' => 'item',
          '#title' => t('Database'),
          '#markup' => check_plain(str_replace(':', ' > ', $this->options['database'])),
        ),
      );
    }

    // Set default settings.
    $options = $this->options + array(
      'min_chars' => 1,
      'autocomplete' => array(),
      'partial_matches' => FALSE,
    );
    $options['autocomplete'] += array(
      'suggest_suffix' => TRUE,
      'suggest_words' => TRUE,
    );

    $form['min_chars'] = array(
      '#type' => 'select',
      '#title' => t('Minimum word length'),
      '#description' => t('The minimum number of characters a word must consist of to be indexed.'),
      '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6)),
      '#default_value' => $options['min_chars'],
    );

    $form['partial_matches'] = array(
      '#type' => 'checkbox',
      '#title' => t('Search on parts of a word'),
      '#description' => t('Find keywords in parts of a word, too. (E.g., find results with "database" when searching for "base"). <strong>Caution:</strong> This can make searches much slower on large sites!'),
      '#default_value' => $options['partial_matches'],
    );

    if (module_exists('search_api_autocomplete')) {
      $form['autocomplete'] = array(
        '#type' => 'fieldset',
        '#title' => t('Autocomplete settings'),
        '#description' => t('These settings allow you to configure how suggestions are computed when autocompletion is used. If you are seeing many inappropriate suggestions you might want to deactivate the corresponding suggestion type. You can also deactivate one method to speed up the generation of suggestions.'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['autocomplete']['suggest_suffix'] = array(
        '#type' => 'checkbox',
        '#title' => t('Suggest word endings'),
        '#description' => t('Suggest endings for the currently entered word.'),
        '#default_value' => $options['autocomplete']['suggest_suffix'],
      );
      $form['autocomplete']['suggest_words'] = array(
        '#type' => 'checkbox',
        '#title' => t('Suggest additional words'),
        '#description' => t('Suggest additional words the user might want to search for.'),
        '#default_value' => $options['autocomplete']['suggest_words'],
      );
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function supportsFeature($feature) {
    $supported = array(
      'search_api_autocomplete' => TRUE,
      'search_api_facets' => TRUE,
      'search_api_facets_operator_or' => TRUE,
      'search_api_service_extra' => TRUE,
    );
    return isset($supported[$feature]);
  }

  /**
   * Overrides SearchApiAbstractService::viewSettings().
   *
   * Returns an empty string since information is instead added via
   * getExtraInformation().
   */
  public function viewSettings() {
    return '';
  }

  /**
   * {@inheritdoc}
   */
  public function getExtraInformation() {
    // Set default settings.
    $options = $this->options + array(
      'min_chars' => 1,
      'partial_matches' => FALSE,
    );

    $info = array();

    $info[] = array(
      'label' => t('Database'),
      'info' => check_plain(str_replace(':', ' > ', $options['database'])),
    );
    if ($options['min_chars'] > 1) {
      $info[] = array(
        'label' => t('Minimum word length'),
        'info' => $options['min_chars'],
      );
    }
    $info[] = array(
      'label' => t('Search on parts of a word'),
      'info' => $options['partial_matches'] ? t('enabled') : t('disabled'),
    );
    if (!empty($options['autocomplete'])) {
      $options['autocomplete'] += array(
        'suggest_suffix' => TRUE,
        'suggest_words' => TRUE,
      );
      $autocomplete_modes = array();
      if ($options['autocomplete']['suggest_suffix']) {
        $autocomplete_modes[] = t('Suggest word endings');
      }
      if ($options['autocomplete']['suggest_words']) {
        $autocomplete_modes[] = t('Suggest additional words');
      }
      $autocomplete_modes = $autocomplete_modes ? implode('; ', $autocomplete_modes) : t('none');
      $info[] = array(
        'label' => t('Autocomplete suggestions'),
        'info' => $autocomplete_modes,
      );
    }

    return $info;
  }

  /**
   * {@inheritdoc}
   */
  public function postUpdate() {
    return !empty($this->server->original) && $this->server->options != $this->server->original->options;
  }

  /**
   * {@inheritdoc}
   */
  public function preDelete() {
    // Only react on real deletes, not on reverts.
    if ($this->server->hasStatus(ENTITY_IN_CODE)) {
      return;
    }
    if (empty($this->options['indexes'])) {
      return;
    }
    foreach ($this->options['indexes'] as $index) {
      foreach ($index as $field) {
        // Some fields share a de-normalized table, brute force since
        // everything is going.
        if ($this->connection->schema()->tableExists($field['table'])) {
          $this->connection->schema()->dropTable($field['table']);
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function addIndex(SearchApiIndex $index) {
    try {
      // If there are no fields, we can take a shortcut.
      if (!isset($index->options['fields'])) {
        if (!isset($this->options['indexes'][$index->machine_name])) {
          $this->options['indexes'][$index->machine_name] = array();
          $this->server->save();
        }
        elseif ($this->options['indexes'][$index->machine_name]) {
          $this->removeIndex($index);
          $this->options['indexes'][$index->machine_name] = array();
          $this->server->save();
        }
        return;
      }
      $this->options += array('indexes' => array());
      $this->options['indexes'] += array($index->machine_name => array());
    }
    // The database operations might throw PDO or other exceptions, so we catch
    // them all and re-wrap them appropriately.
    catch (Exception $e) {
      throw new SearchApiException($e->getMessage());
    }

    // If dealing with features or stale data or whatever, we might already have
    // settings stored for this index. If we have, we should take care to only
    // change what is needed, so we don't save the server (potentially setting
    // it to "Overridden") unnecessarily.
    // The easiest way to do this is by just pretending the index was already
    // present, but its fields were updated.
    $this->fieldsUpdated($index);
  }

  /**
   * Finds a free table name using a certain prefix and name base.
   *
   * Used as a helper method in fieldsUpdated().
   *
   * MySQL 5.0 imposes a 64 characters length limit for table names, PostgreSQL
   * 8.3 only allows 62 bytes. Therefore, always return a name at most 62
   * bytes long.
   *
   * @param string $prefix
   *   Prefix to start the table name.
   * @param string $name
   *   Name to base the table name on.
   *
   * @return string
   *   A database table name that isn't in use yet.
   */
  protected function findFreeTable($prefix, $name) {
    // A DB prefix might further reduce the maximum length of the table name.
    $maxbytes = 62;
    if ($db_prefix = $this->connection->tablePrefix()) {
      // Use strlen instead of drupal_strlen since we want to measure bytes
      // instead of characters.
      $maxbytes -= strlen($db_prefix);
    }

    $base = $table = self::mbStrcut($prefix . drupal_strtolower(preg_replace('/[^a-z0-9]/i', '_', $name)), 0, $maxbytes);
    $i = 0;
    while ($this->connection->schema()->tableExists($table)) {
      $suffix = '_' . ++$i;
      $table = self::mbStrcut($base, 0, $maxbytes - strlen($suffix)) . $suffix;
    }
    return $table;
  }

  /**
   * Finds a free column name within a database table.
   *
   * Used as a helper method in fieldsUpdated().
   *
   * MySQL 5.0 imposes a 64 characters length limit for identifier names,
   * PostgreSQL 8.3 only allows 62 bytes. Therefore, always return a name at
   * most 62 bytes long.
   *
   * @param string $table
   *   Name of the table.
   * @param string $column
   *   If adding a column to $name, the name to base the column name on.
   *
   * @return string
   *   A column name that isn't in use in the specified table yet.
   */
  protected function findFreeColumn($table, $column) {
    $maxbytes = 62;

    $base = $name = self::mbStrcut(drupal_strtolower(preg_replace('/[^a-z0-9]/i', '_', $column)), 0, $maxbytes);
    // If the table does not exist yet, the initial name is not taken.
    if ($this->connection->schema()->tableExists($table)) {
      $i = 0;
      while ($this->connection->schema()->fieldExists($table, $name)) {
        $suffix = '_' . ++$i;
        $name = self::mbStrcut($base, 0, $maxbytes - strlen($suffix)) . $suffix;
      }
    }
    return $name;
  }

  /**
   * Creates or modifies a table to add an indexed field.
   *
   * Used as a helper method in fieldsUpdated().
   *
   * @param SearchApiIndex $index
   *   Search API index for this field.
   * @param array $field
   *   Single field definition from SearchApiIndex::getFields().
   * @param array $db
   *   Associative array containing the following:
   *   - table: The table to use for the field.
   *   - column: (optional) The column to use in that table. Defaults to
   *     "value".
   */
  protected function createFieldTable(SearchApiIndex $index, $field, &$db) {
    $type = search_api_extract_inner_type($field['type']);
    $new_table = !$this->connection->schema()->tableExists($db['table']);
    if ($new_table) {
      $table = array(
        'name' => $db['table'],
        'module' => 'search_api_db',
        'fields' => array(
          'item_id' => array(
            'description' => 'The primary identifier of the item.',
            'not null' => TRUE,
          ),
        ),
      );
      // The type of the item_id field depends on the ID field's type.
      $id_field = $index->datasource()->getIdFieldInfo();
      $table['fields']['item_id'] += $this->sqlType($id_field['type'] == 'text' ? 'string' : $id_field['type']);
      if (isset($table['fields']['item_id']['length'])) {
        // A length of 255 is overkill for IDs. 50 should be more than enough.
        $table['fields']['item_id']['length'] = 50;
      }

      $this->connection->schema()->createTable($db['table'], $table);

      // Some DBMSs will need a character encoding and collation set.
      switch ($this->connection->databaseType()) {
        case 'mysql':
          $this->connection->query("ALTER TABLE {{$db['table']}} CONVERT TO CHARACTER SET 'utf8' COLLATE 'utf8_bin'");
          break;

          // @todo Add fixes for other DBMSs.
        case 'oracle':
        case 'pgsql':
        case 'sqlite':
        case 'sqlsrv':
          break;
      }
    }

    if (!isset($db['column'])) {
      $db['column'] = 'value';
    }
    $db_field = $this->sqlType($type);
    $db_field += array(
      'description' => "The field's value for this item.",
    );
    if ($new_table && search_api_is_list_type($field['type'])) {
      $db_field['not null'] = TRUE;
    }
    $this->connection->schema()->addField($db['table'], $db['column'], $db_field);
    if ($db_field['type'] === 'varchar') {
      $this->connection->schema()->addIndex($db['table'], $db['column'], array(array($db['column'], 10)));
    }
    else {
      $this->connection->schema()->addIndex($db['table'], $db['column'], array($db['column']));
    }
    if ($new_table) {
      if (search_api_is_list_type($field['type'])) {
        // Add a covering index for lists.
        $this->connection->schema()->addPrimaryKey($db['table'], array('item_id', $db['column']));
      }
      else {
        // Otherwise, a denormalized table with many columns, where we can't
        // predict the best covering index.
        $this->connection->schema()->addPrimaryKey($db['table'], array('item_id'));
      }
    }
  }

  /**
   * Returns the schema definition for a database column for a search data type.
   *
   * @param string $type
   *   An indexed field's search type. One of the keys from
   *   search_api_default_field_types().
   *
   * @return array
   *   Column configurations to use for the field's database column.
   *
   * @throws SearchApiException
   *   If $type is unknown.
   */
  protected function sqlType($type) {
    $type = search_api_extract_inner_type($type);
    switch ($type) {
      case 'string':
      case 'uri':
        return array('type' => 'varchar', 'length' => 255);
      case 'integer':
      case 'duration':
      case 'date':
        // 'datetime' sucks. Therefore, we just store the timestamp.
        return array('type' => 'int', 'size' => 'big');
      case 'decimal':
        return array('type' => 'float');
      case 'boolean':
        return array('type' => 'int', 'size' => 'tiny');

      default:
        throw new SearchApiException(t('Unknown field type @type. Database search module might be out of sync with Search API.', array('@type' => $type)));
    }
  }

  /**
   * Overrides SearchApiAbstractService::fieldsUpdated().
   *
   * Internally, this is also used by addIndex().
   */
  public function fieldsUpdated(SearchApiIndex $index) {
    try {
      $fields = &$this->options['indexes'][$index->machine_name];
      $new_fields = $index->getFields();

      $reindex = FALSE;
      $cleared = FALSE;
      $change = FALSE;
      $text_table = NULL;

      foreach ($fields as $name => $field) {
        if (!isset($text_table) && search_api_is_text_type($field['type'])) {
          // Stash the shared text table name for the index.
          $text_table = $field['table'];
        }

        if (!isset($new_fields[$name])) {
          // The field is no longer in the index, drop the data.
          $this->removeFieldStorage($name, $field);
          unset($fields[$name]);
          $change = TRUE;
          continue;
        }
        $old_type = $field['type'];
        $new_type = $new_fields[$name]['type'];
        $fields[$name]['type'] = $new_type;
        $fields[$name]['boost'] = $new_fields[$name]['boost'];
        $old_inner_type = search_api_extract_inner_type($old_type);
        $new_inner_type = search_api_extract_inner_type($new_type);
        if ($old_type != $new_type) {
          $change = TRUE;
          $list_old = (bool) search_api_list_nesting_level($old_type);
          $list_new = (bool) search_api_list_nesting_level($new_type);
          if ($old_inner_type == 'text' || $new_inner_type == 'text' || $list_old != $list_new) {
            // A change in fulltext or list status necessitates completely
            // clearing the index.
            $reindex = TRUE;
            if (!$cleared) {
              $cleared = TRUE;
              $this->deleteItems('all', $index);
            }
            $this->removeFieldStorage($name, $field);
            // Keep the table in $new_fields to create the new storage.
            continue;
          }
          elseif ($this->sqlType($old_inner_type) != $this->sqlType($new_inner_type)) {
            // There is a change in SQL type. We don't have to clear the index,
            // since types can be converted.
            $column = isset($field['column']) ? $field['column'] : 'value';
            $this->connection->schema()->changeField($field['table'], $column, $column, $this->sqlType($new_type) + array('description' => "The field's value for this item."));
            $reindex = TRUE;
          }
          elseif ($old_inner_type == 'date' || $new_inner_type == 'date') {
            // Even though the SQL type stays the same, we have to reindex since
            // conversion rules change.
            $reindex = TRUE;
          }
        }
        elseif ($new_inner_type == 'text' && $field['boost'] != $new_fields[$name]['boost']) {
          $change = TRUE;
          if (!$reindex) {
            $multiplier = $new_fields[$name]['boost'] / $field['boost'];
            $this->connection->update($text_table)
              ->expression('score', 'score * :mult', array(':mult' => $multiplier))
              ->condition('field_name', self::getTextFieldName($name))
              ->execute();
          }
        }
        // Make sure the table and column now exist. (Especially important when
        // we actually add the index for the first time.)
        if (!search_api_is_text_type($field['type'])) {
          $storageExists = $this->connection->schema()->tableExists($field['table'])
              && (!isset($field['column'])
                  || $this->connection->schema()->fieldExists($field['table'], $field['column']));
          if (!$storageExists) {
            $this->createFieldTable($index, $new_fields[$name], $field);
          }
        }
        unset($new_fields[$name]);
      }

      $prefix = 'search_api_db_' . $index->machine_name;
      // These are new fields that were previously not indexed.
      foreach ($new_fields as $name => $field) {
        $reindex = TRUE;
        if (search_api_is_text_type($field['type'])) {
          if (!isset($text_table)) {
            // If we have not encountered a text table, assign a name for it.
            $text_table = $this->findFreeTable($prefix . '_', 'text');
          }
          $fields[$name]['table'] = $text_table;
        }
        else {
          if ($this->canDenormalize($field)) {
            $fields[$name]['table'] = $prefix;
            $fields[$name]['column'] = $this->findFreeColumn($fields[$name]['table'], $name);
          }
          else {
            $fields[$name]['table'] = $this->findFreeTable($prefix . '_', $name);
          }
          $this->createFieldTable($index, $field, $fields[$name]);
        }
        $fields[$name]['type'] = $field['type'];
        $fields[$name]['boost'] = $field['boost'];
        $change = TRUE;
      }

      // If needed, make sure the text table exists.
      if (isset($text_table) && !$this->connection->schema()->tableExists($text_table)) {
        $table = array(
          'name' => $text_table,
          'module' => 'search_api_db',
          'fields' => array(
            'item_id' => array(
              'description' => 'The primary identifier of the item.',
              'not null' => TRUE,
            ),
            'field_name' => array(
              'description' => "The name of the field in which the token appears, or an MD5 hash of the field.",
              'not null' => TRUE,
              'type' => 'varchar',
              'length' => 255,
            ),
            'word' => array(
              'description' => 'The text of the indexed token.',
              'type' => 'varchar',
              'length' => 50,
              'not null' => TRUE,
            ),
            'score' => array(
              'description' => 'The score associated with this token.',
              'type' => 'int',
              'unsigned' => TRUE,
              'not null' => TRUE,
              'default' => 0,
            ),
          ),
          'indexes' => array(
            'word_field' => array(array('word', 20), 'field_name'),
          ),
          // Add a covering index since word is not repeated for each item.
          'primary key' => array('item_id', 'field_name', 'word'),
        );
        // The type of the item_id field depends on the ID field's type.
        $id_field = $index->datasource()->getIdFieldInfo();
        $table['fields']['item_id'] += $this->sqlType($id_field['type'] == 'text' ? 'string' : $id_field['type']);
        if (isset($table['fields']['item_id']['length'])) {
          // A length of 255 is overkill for IDs. 50 should be more than enough.
          $table['fields']['item_id']['length'] = 50;
        }
        $this->connection->schema()->createTable($text_table, $table);

        // Some DBMSs will need a character encoding and collation set. Since
        // this largely circumvents Drupal's database layer (but isn't integral
        // enough to failing completely when it doesn't work, we wrap it in a
        // try/catch, to be on the safe side.
        try {
          switch ($this->connection->databaseType()) {
            case 'mysql':
              $this->connection->query("ALTER TABLE {{$text_table}} CONVERT TO CHARACTER SET 'utf8' COLLATE 'utf8_bin'");
              break;

            case 'pgsql':
              $this->connection->query("ALTER TABLE {{$text_table}} ALTER COLUMN word SET DATA TYPE character varying(50) COLLATE \"C\"");
              break;

            // @todo Add fixes for other DBMSs.
            case 'oracle':
            case 'sqlite':
            case 'sqlsrv':
              break;
          }
        }
        catch (PDOException $e) {
          $vars['%index'] = $index->name;
          watchdog_exception('search_api_db', $e, '%type while trying to change collation for the fulltext table of index %index: !message in %function (line %line of %file).', $vars);
        }
      }

      if ($change) {
        $this->server->save();
      }
      return $reindex;
    }
    // The database operations might throw PDO or other exceptions, so we catch
    // them all and re-wrap them appropriately.
    catch (Exception $e) {
      throw new SearchApiException($e->getMessage());
    }
  }

  /**
   * Checks if a field can be denormalized.
   *
   * List fields have multiple values, so cannot be denormalized. Text fields
   * are tokenized into words, so cannot be denormalized either.
   *
   * @param array $field
   *   Single field definition from SearchApiIndex::getFields().
   *
   * @return bool
   *   TRUE if the field can be stored in a table with other fields (i.e., will
   *   only need a single row), FALSE otherwise.
   */
  protected function canDenormalize($field) {
    return !search_api_is_list_type($field['type']) && !search_api_is_text_type($field['type']);
  }

  /**
   * Drops a field's table or column for storage.
   *
   * @param string $name
   *   The field name.
   * @param array $field
   *   Server-internal information about the field.
   */
  protected function removeFieldStorage($name, $field) {
    if (search_api_is_text_type($field['type'])) {
      $this->connection->delete($field['table'])
        ->condition('field_name', self::getTextFieldName($name))
        ->execute();
    }
    // Legacy non-denormalized fields will not have a column.
    elseif ($this->canDenormalize($field) && isset($field['column'])) {
      $this->connection->schema()->dropField($field['table'], $field['column']);
    }
    elseif ($this->connection->schema()->tableExists($field['table'])) {
      $this->connection->schema()->dropTable($field['table']);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function removeIndex($index) {
    try {
      $id = is_object($index) ? $index->machine_name : $index;
      if (!isset($this->options['indexes'][$id])) {
        return;
      }
      // Don't delete the index data of read-only indexes!
      if (!is_object($index) || empty($index->read_only)) {
        foreach ($this->options['indexes'][$id] as $field) {
          // Some fields share a de-normalized table, brute force since
          // everything is going.
          if ($this->connection->schema()->tableExists($field['table'])) {
            $this->connection->schema()->dropTable($field['table']);
          }
        }
      }
      unset($this->options['indexes'][$id]);
      $this->server->save();
    }
    // The database operations might throw PDO or other exceptions, so we catch
    // them all and re-wrap them appropriately.
    catch (Exception $e) {
      throw new SearchApiException($e->getMessage());
    }
  }

  /**
   * {@inheritdoc}
   */
  public function indexItems(SearchApiIndex $index, array $items) {
    if (empty($this->options['indexes'][$index->machine_name])) {
      throw new SearchApiException(t('No field settings for index with id @id.', array('@id' => $index->machine_name)));
    }
    $indexed = array();
    foreach ($items as $id => $item) {
      try {
        $this->indexItem($index, $id, $item);
        $indexed[] = $id;
      }
      catch (Exception $e) {
        // We just log the error, hoping we can index the other items.
        watchdog('search_api_db', check_plain($e->getMessage()), NULL, WATCHDOG_WARNING);
      }
    }
    return $indexed;
  }

  /**
   * Indexes a single item on the specified index.
   *
   * Used as a helper method in indexItems().
   *
   * @param SearchApiIndex $index
   *   The index for which the item is being indexed.
   * @param $id
   *   The item's ID.
   * @param array $item
   *   The extracted fields of the item.
   *
   * @throws Exception
   *   Any encountered database (or other) exceptions are passed on, out of this
   *   method.
   */
  protected function indexItem(SearchApiIndex $index, $id, array $item) {
    $fields = $this->getFieldInfo($index);
    $fields_updated = FALSE;
    $txn = $this->connection->startTransaction('search_api_indexing');
    try {
      $inserts = array();
      $text_inserts = array();
      foreach ($item as $name => $field) {
        // Sometimes index changes are not triggering the update hooks
        // correctly. Therefore, to avoid DB errors, we re-check the tables
        // here before indexing.
        if (empty($fields[$name]['table']) && !$fields_updated) {
          unset($this->options['indexes'][$index->machine_name][$name]);
          $this->fieldsUpdated($index);
          $fields_updated = TRUE;
          $fields = $this->options['indexes'][$index->machine_name];
        }
        if (empty($fields[$name]['table'])) {
          watchdog('search_api_db', "Unknown field !field: please check (and re-save) the index's fields settings.",
              array('!field' => $name), WATCHDOG_WARNING);
          continue;
        }
        $table = $fields[$name]['table'];
        $boost = $fields[$name]['boost'];
        $this->connection->delete($table)
          ->condition('item_id', $id)
          ->execute();
        // Don't index null values
        if ($field['value'] === NULL) {
          continue;
        }
        $type = $field['type'];
        $value = $this->convert($field['value'], $type, $field['original_type'], $index);

        if (search_api_is_text_type($type, array('text', 'tokens'))) {
          $words = array();
          foreach ($value as $token) {
            // Taken from core search to reflect less importance of words later
            // in the text.
            // Focus is a decaying value in terms of the amount of unique words
            // up to this point. From 100 words and more, it decays, to e.g. 0.5
            // at 500 words and 0.3 at 1000 words.
            $focus = min(1, .01 + 3.5 / (2 + count($words) * .015));

            $value = &$token['value'];
            if (is_numeric($value)) {
              $value = ltrim($value, '-0');
            }
            elseif (drupal_strlen($value) < $this->options['min_chars']) {
              continue;
            }
            $value = drupal_strtolower($value);
            $token['score'] *= $focus;
            if (!isset($words[$value])) {
              $words[$value] = $token;
            }
            else {
              $words[$value]['score'] += $token['score'];
            }
          }
          if ($words) {
            $field_name = self::getTextFieldName($name);
            foreach ($words as $word) {
              $text_inserts[$table][] = array(
                'item_id'    => $id,
                'field_name' => $field_name,
                'word'       => $word['value'],
                'score'      => (int) round($word['score'] * $boost * self::SCORE_MULTIPLIER),
              );
            }
          }
        }
        elseif (search_api_is_list_type($type)) {
          $values = array();
          if (is_array($value)) {
            foreach ($value as $v) {
              if (isset($v)) {
                $values["$v"] = TRUE;
              }
            }
            $values = array_keys($values);
          }
          elseif (isset($value)) {
            $values[] = $value;
          }
          if ($values) {
            $insert = $this->connection->insert($table)
              ->fields(array('item_id', $fields[$name]['column']));
            foreach ($values as $v) {
              $insert->values(array(
                'item_id' => $id,
                $fields[$name]['column'] => $v,
              ));
            }
            $insert->execute();
          }
        }
        elseif (isset($value)) {
          $inserts[$table][$fields[$name]['column']] = $value;
        }
      }
      foreach ($inserts as $table => $data) {
        $this->connection->insert($table)
          ->fields(array_merge($data, array('item_id' => $id)))
          ->execute();
      }
      foreach ($text_inserts as $table => $data) {
        $query = $this->connection->insert($table)
          ->fields(array('item_id', 'field_name', 'word', 'score'));
        foreach ($data as $row) {
          $query->values($row);
        }
        $query->execute();
      }
    }
    catch (Exception $e) {
      $txn->rollback();
      throw $e;
    }
  }

  /**
   * Trims long field names to fit into the text table's field_name column.
   *
   * @param string $name
   *   The field name.
   *
   * @return string
   *   The field name as stored in the field_name column.
   */
  protected static function getTextFieldName($name) {
    if (strlen($name) > 255) {
      // Replace long field names with something unique and predictable.
      return md5($name);
    }
    else {
      return $name;
    }
  }

  /**
   * Converts a value between two search types.
   *
   * @param $value
   *   The value to convert.
   * @param $type
   *   The type to convert to. One of the keys from
   *   search_api_default_field_types().
   * @param $original_type
   *   The value's original type.
   * @param SearchApiIndex $index
   *   The index for which this conversion takes place.
   *
   * @return mixed
   *   The converted value.
   *
   * @throws SearchApiException
   *   If $type is unknown.
   */
  protected function convert($value, $type, $original_type, SearchApiIndex $index) {
    if (search_api_is_list_type($type)) {
      $type = substr($type, 5, -1);
      $original_type = search_api_extract_inner_type($original_type);
      $ret = array();
      if (is_array($value)) {
        foreach ($value as $v) {
          $v = $this->convert($v, $type, $original_type, $index);

          // Don't add NULL values to the return array. Also, adding an empty
          // array is, of course, a waste of time.
          if (isset($v) && $v !== array()) {
            $ret = array_merge($ret, is_array($v) ? $v : array($v));
          }
        }
      }
      return $ret;
    }
    if (!isset($value)) {
      // For text fields, we have to return an array even if the value is NULL.
      return search_api_is_text_type($type, array('text', 'tokens')) ? array() : NULL;
    }
    switch ($type) {
      case 'text':
        // For dates, splitting the timestamp makes no sense.
        if ($original_type == 'date') {
          $value = format_date($value, 'custom', 'Y y F M n m j d l D');
        }
        $ret = array();
        foreach (preg_split('/[^\p{L}\p{N}]+/u', $value, -1, PREG_SPLIT_NO_EMPTY) as $v) {
          if ($v) {
            $ret[] = array(
              'value' => $v,
              'score' => 1,
            );
          }
        }
        $value = $ret;
        // FALL-THROUGH!
      case 'tokens':
        while (TRUE) {
          foreach ($value as $i => $v) {
            // Check for over-long tokens.
            $score = $v['score'];
            $v = $v['value'];
            if (strlen($v) > 50) {
              $words = preg_split('/[^\p{L}\p{N}]+/u', $v, -1, PREG_SPLIT_NO_EMPTY);
              if (count($words) > 1 && max(array_map('strlen', $words)) <= 50) {
                // Overlong token is due to bad tokenizing.
                // Check for "Tokenizer" preprocessor on index.
                if (empty($index->options['processors']['search_api_tokenizer']['status'])) {
                  watchdog('search_api_db', 'An overlong word (more than 50 characters) was encountered while indexing, due to bad tokenizing. ' .
                      'It is recommended to enable the "Tokenizer" preprocessor for indexes using database servers. ' .
                      'Otherwise, the service class has to use its own, fixed tokenizing.', array(), WATCHDOG_WARNING);
                }
                else {
                  watchdog('search_api_db', 'An overlong word (more than 50 characters) was encountered while indexing, due to bad tokenizing. ' .
                      'Please check your settings for the "Tokenizer" preprocessor to ensure that data is tokenized correctly.',
                      array(), WATCHDOG_WARNING);
                }
              }

              $tokens = array();
              foreach ($words as $word) {
                if (strlen($word) > 50) {
                  watchdog('search_api_db', 'An overlong word (more than 50 characters) was encountered while indexing: %word.<br />' .
                      'Database search servers currently cannot index such words correctly – the word was therefore trimmed to the allowed length.',
                      array('%word' => $word), WATCHDOG_WARNING);
                  $word = self::mbStrcut($word, 0, 50);
                }
                $tokens[] = array(
                  'value' => $word,
                  'score' => $score,
                );
              }
              array_splice($value, $i, 1, $tokens);
              continue 2;
            }
          }
          break;
        }
        return $value;

      case 'string':
      case 'uri':
        // For non-dates, PHP can handle this well enough.
        if ($original_type == 'date') {
          return date('c', $value);
        }
        if (strlen($value) > 255) {
          $value = self::mbStrcut($value, 0, 255);
          watchdog('search_api_db', 'An overlong value (more than 255 characters) was encountered while indexing: %value.<br />' .
              'Database search servers currently cannot index such values correctly – the value was therefore trimmed to the allowed length.',
              array('%value' => $value), WATCHDOG_WARNING);
        }
        return $value;

      case 'integer':
      case 'duration':
      case 'decimal':
        return 0 + $value;

      case 'boolean':
        return $value ? 1 : 0;

      case 'date':
        if (is_numeric($value) || !$value) {
          return 0 + $value;
        }
        return strtotime($value);

      default:
        throw new SearchApiException(t('Unknown field type @type. Database search module might be out of sync with Search API.', array('@type' => $type)));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function deleteItems($ids = 'all', SearchApiIndex $index = NULL) {
    try {
      if (!$index) {
        if (empty($this->options['indexes'])) {
          return;
        }
        $truncated = array();
        foreach ($this->options['indexes'] as $fields) {
          foreach ($fields as $field) {
            if (isset($field['table']) && !isset($truncated[$field['table']])) {
              $this->connection->truncate($field['table'])->execute();
              $truncated[$field['table']] = TRUE;
            }
          }
        }
        return;
      }

      if (empty($this->options['indexes'][$index->machine_name])) {
        return;
      }
      foreach ($this->options['indexes'][$index->machine_name] as $field) {
        if (is_array($ids)) {
          $this->connection->delete($field['table'])
            ->condition('item_id', $ids, 'IN')
            ->execute();
        }
        else {
          $this->connection->truncate($field['table'])->execute();
        }
      }
    }
    // The database operations might throw PDO or other exceptions, so we catch
    // them all and re-wrap them appropriately.
    catch (Exception $e) {
      throw new SearchApiException($e->getMessage());
    }
  }

  /**
   * {@inheritdoc}
   */
  public function search(SearchApiQueryInterface $query) {
    $time_method_called = microtime(TRUE);
    $this->ignored = $this->warnings = array();
    $index = $query->getIndex();
    if (empty($this->options['indexes'][$index->machine_name])) {
      throw new SearchApiException(t('Unknown index @id.', array('@id' => $index->machine_name)));
    }
    $fields = $this->getFieldInfo($index);

    $db_query = $this->createDbQuery($query, $fields);

    // Allow subclasses and other modules to alter the query (before a count
    // query is constructed from it).
    drupal_alter('search_api_db_query', $db_query, $query);
    $this->preQuery($db_query, $query);

    $time_processing_done = microtime(TRUE);
    $results = array();

    $skip_count = $query->getOption('skip result count');
    if (!$skip_count) {
      $count_query = $db_query->countQuery();
      $results['result count'] = $count_query->execute()->fetchField();
    }

    if ($skip_count || $results['result count']) {
      if ($query->getOption('search_api_facets')) {
        $results['search_api_facets'] = $this->getFacets($query, clone $db_query);
      }

      $query_options = $query->getOptions();
      if (isset($query_options['offset']) || isset($query_options['limit'])) {
        $offset = isset($query_options['offset']) ? $query_options['offset'] : 0;
        $limit = isset($query_options['limit']) ? $query_options['limit'] : 1000000;
        $db_query->range($offset, $limit);
      }

      $this->setQuerySort($query, $db_query, $fields);

      $result = $db_query->execute();
      $time_queries_done = microtime(TRUE);

      foreach ($result as $row) {
        $results['results'][$row->item_id] = array(
          'id' => $row->item_id,
          'score' => $row->score / self::SCORE_MULTIPLIER,
        );
      }
      if ($skip_count) {
        $results['result count'] = !empty($results['results']);
      }
    }
    else {
      $time_queries_done = microtime(TRUE);
      $results['results'] = array();
    }

    $results['warnings'] = array_keys($this->warnings);
    $results['ignored'] = array_keys($this->ignored);

    $this->postQuery($results, $query);

    $time_end = microtime(TRUE);
    $results['performance'] = array(
      'complete' => $time_end - $time_method_called,
      'preprocessing' => $time_processing_done - $time_method_called,
      'execution' => $time_queries_done - $time_processing_done,
      'postprocessing' => $time_end - $time_queries_done,
    );

    return $results;
  }

  /**
   * Creates a database query for a search.
   *
   * Used as a helper method in search() and getAutocompleteSuggestions().
   *
   * @param SearchApiQueryInterface $query
   *   The search query for which to create the database query.
   *
   * @param array $fields
   *   The internal field information to use.
   *
   * @return SelectQuery
   *   A database query object which will return the appropriate results (except
   *   for the range setting) for the given search query.
   *
   * @throws SearchApiException
   *   If some illegal query setting (unknown field, etc.) was encountered.
   */
  protected function createDbQuery(SearchApiQueryInterface $query, array $fields) {
    $keys = &$query->getKeys();
    $keys_set = (boolean) $keys;
    $keys = $this->prepareKeys($keys);
    // Special case: if the outermost $keys array has "#negation" set, we can't
    // handle it like other negated subkeys. To avoid additional complexity
    // later, we just wrap $keys so it becomes a subkey.
    if (!empty($keys['#negation'])) {
      $keys = array(
        '#conjunction' => 'AND',
        $keys,
      );
    }
    // Only filter by fulltext keys if there are any real keys present.
    if ($keys && (!is_array($keys) || count($keys) > 2 || (!isset($keys['#negation']) && count($keys) > 1))) {
      $fulltext_fields = $query->getFields();
      if ($fulltext_fields) {
        $_fulltext_fields = $fulltext_fields;
        $fulltext_fields = array();
        foreach ($_fulltext_fields as $name) {
          if (!isset($fields[$name])) {
            throw new SearchApiException(t('Unknown field @field specified as search target.', array('@field' => $name)));
          }
          if (!search_api_is_text_type($fields[$name]['type'])) {
            $types = search_api_field_types();
            $type = $types[$fields[$name]['type']];
            throw new SearchApiException(t('Cannot perform fulltext search on field @field of type @type.', array('@field' => $name, '@type' => $type)));
          }
          $fulltext_fields[$name] = $fields[$name];
        }

        $db_query = $this->createKeysQuery($keys, $fulltext_fields, $fields);
        if (is_array($keys) && !empty($keys['#negation'])) {
          $db_query->addExpression(':score', 'score', array(':score' => self::SCORE_MULTIPLIER));
          $db_query->distinct();
        }
      }
      else {
        $msg = t('Search keys are given but no fulltext fields are defined.');
        watchdog('search_api_db', $msg, NULL, WATCHDOG_WARNING);
        $this->warnings[$msg] = 1;
      }
    }
    elseif ($keys_set) {
      $msg = t('No valid search keys were present in the query.');
      $this->warnings[$msg] = 1;
    }

    if (!isset($db_query)) {
      $db_query = $this->connection->select($fields['search_api_language']['table'], 't');
      $db_query->addField('t', 'item_id', 'item_id');
      $db_query->addExpression(':score', 'score', array(':score' => self::SCORE_MULTIPLIER));
      $db_query->distinct();
    }

    $filter = $query->getFilter();
    if ($filter->getFilters()) {
      $condition = $this->createFilterCondition($filter, $fields, $db_query);
      if ($condition) {
        $db_query->condition($condition);
      }
    }

    $db_query->addTag('search_api_db_search');
    $db_query->addMetaData('search_api_query', $query);
    $db_query->addMetaData('search_api_db_fields', $fields);

    return $db_query;
  }

  /**
   * Removes nested expressions and phrase groupings from the search keys.
   *
   * Used as a helper method in createDbQuery() and createFilterCondition().
   *
   * @param array|string|null $keys
   *   The keys which should be preprocessed.
   *
   * @return array|string|null
   *   The preprocessed keys.
   */
  protected function prepareKeys($keys) {
    if (is_scalar($keys)) {
      $keys = $this->splitKeys($keys);
      return is_array($keys) ? $this->eliminateDuplicates($keys) : $keys;
    }
    elseif (!$keys) {
      return NULL;
    }
    $keys = $this->eliminateDuplicates($this->splitKeys($keys));
    $conj = $keys['#conjunction'];
    $neg = !empty($keys['#negation']);
    foreach ($keys as $i => &$nested) {
      if (is_array($nested)) {
        $nested = $this->prepareKeys($nested);
        if (is_array($nested) && $neg == !empty($nested['#negation'])) {
          if ($nested['#conjunction'] == $conj) {
            unset($nested['#conjunction'], $nested['#negation']);
            foreach ($nested as $renested) {
              $keys[] = $renested;
            }
            unset($keys[$i]);
          }
        }
      }
    }
    $keys = array_filter($keys);
    if (($count = count($keys)) <= 2) {
      if ($count < 2 || isset($keys['#negation'])) {
        $keys = NULL;
      }
      else {
        unset($keys['#conjunction']);
        $keys = reset($keys);
      }
    }
    return $keys;
  }

  /**
   * Splits a keyword expression into separate words.
   *
   * Used as a helper method in prepareKeys().
   *
   * @param array|string|null $keys
   *   The keys to split.
   *
   * @return array|string|null
   *   The keys split into separate words.
   */
  protected function splitKeys($keys) {
    if (is_scalar($keys)) {
      $proc = drupal_strtolower(trim($keys));
      if (is_numeric($proc)) {
        return ltrim($proc, '-0');
      }
      elseif (drupal_strlen($proc) < $this->options['min_chars']) {
        $this->ignored[$keys] = 1;
        return NULL;
      }
      $words = preg_split('/[^\p{L}\p{N}]+/u', $proc, -1, PREG_SPLIT_NO_EMPTY);
      if (count($words) > 1) {
        $proc = $this->splitKeys($words);
        $proc['#conjunction'] = 'AND';
      }
      return $proc;
    }
    foreach ($keys as $i => $key) {
      if (element_child($i)) {
        $keys[$i] = $this->splitKeys($key);
      }
    }
    return array_filter($keys);
  }

  /**
   * Eliminates duplicate keys from a keyword array.
   *
   * Used as a helper method in prepareKeys().
   *
   * @param array $keys
   *   The keywords to parse.
   * @param array $words
   *   (optional) A cache of all encountered words so far, used internally for
   *   recursive invocations.
   *
   * @return array
   *   The processed keywords.
   */
  protected function eliminateDuplicates($keys, &$words = array()) {
    foreach ($keys as $i => $word) {
      if (!element_child($i)) {
        continue;
      }
      if (is_scalar($word)) {
        if (isset($words[$word])) {
          unset($keys[$i]);
        }
        else {
          $words[$word] = TRUE;
        }
      }
      else {
        $keys[$i] = $this->eliminateDuplicates($word, $words);
      }
    }
    return $keys;
  }

  /**
   * Creates a SELECT query for given search keys.
   *
   * Used as a helper method in createDbQuery() and createFilterCondition().
   *
   * @param $keys
   *   The search keys, formatted like the return value of
   *   SearchApiQueryInterface::getKeys(), but preprocessed according to
   *   internal requirements.
   * @param array $fields
   *   The fulltext fields on which to search, with their names as keys mapped
   *   to internal information about them.
   * @param array $all_fields
   *   Internal information about all indexed fields on the index.
   *
   * @return SelectQueryInterface
   *   A SELECT query returning item_id and score (or only item_id, if
   *   $keys['#negation'] is set).
   */
  protected function createKeysQuery($keys, array $fields, array $all_fields) {
    if (!is_array($keys)) {
      $keys = array(
        '#conjunction' => 'AND',
        $keys,
      );
    }

    $neg = !empty($keys['#negation']);
    $conj = $keys['#conjunction'];
    $words = array();
    $nested = array();
    $negated = array();
    $db_query = NULL;
    $word_hits = array();
    $neg_nested = $neg && $conj == 'AND';

    foreach ($keys as $i => $key) {
      if (!element_child($i)) {
        continue;
      }
      if (is_scalar($key)) {
        $words[] = $key;
      }
      elseif (empty($key['#negation'])) {
        if ($neg) {
          // If this query is negated, we also only need item_ids from
          // subqueries.
          $key['#negation'] = TRUE;
        }
        $nested[] = $key;
      }
      else {
        $negated[] = $key;
      }
    }
    $subs = count($words) + count($nested);
    $mul_words = count($words) > 1;
    $not_nested = ($subs <= 1 && !$mul_words) || ($neg && $conj == 'OR' && !$negated);

    if ($words) {
      // All text fields in the index share a table. Get name from the first.
      $field = reset($fields);
      $db_query = $this->connection->select($field['table'], 't');
      if ($neg_nested) {
        $db_query->fields('t', array('item_id', 'word'));
      }
      elseif ($neg) {
        $db_query->fields('t', array('item_id'));
      }
      elseif ($not_nested) {
        $db_query->fields('t', array('item_id'));
        $db_query->addExpression('SUM(score)', 'score');
        $db_query->groupBy('item_id');
      }
      else {
        $db_query->fields('t', array('item_id', 'word'));
        $db_query->addExpression('SUM(score)', 'score');
        $db_query->groupBy('item_id');
        $db_query->groupBy('word');
      }

      if (empty($this->options['partial_matches'])) {
        $db_query->condition('word', $words, 'IN');
      }
      else {
        $db_or = db_or();
        // GROUP BY all existing non-grouped, non-aggregated columns – except
        // "word", which we remove since it will be useless to us in this case.
        $columns = &$db_query->getFields();
        unset($columns['word']);
        foreach (array_keys($columns) as $column) {
          $db_query->groupBy($column);
        }

        foreach ($words as $i => $word) {
          $db_or->condition('t.word', '%' . $this->connection->escapeLike($word) . '%', 'LIKE');

          // Add an expression for each keyword that shows whether the indexed
          // word matches that particular keyword. That way we don't return a
          // result multiple times if a single indexed word (partially) matches
          // multiple keywords. We also remember the column name so we can
          // afterwards verify that each word matched at least once.
          $alias = 'w' . $i;
          $alias = $db_query->addExpression("t.word LIKE '%" . $this->connection->escapeLike($word) . "%'", $alias);
          $db_query->groupBy($alias);
          $word_hits[] = $alias;
        }
        $db_query->condition($db_or);
      }
      $db_query->condition('field_name', array_map(array(__CLASS__, 'getTextFieldName'), array_keys($fields)), 'IN');
    }

    if ($nested) {
      $word = '';
      foreach ($nested as $k) {
        $query = $this->createKeysQuery($k, $fields, $all_fields);
        if (!$neg) {
          $word .= ' ';
          $var = ':word' . strlen($word);
          $query->addExpression($var, 'word', array($var => $word));
        }
        if (!isset($db_query)) {
          $db_query = $query;
        }
        elseif ($not_nested) {
          $db_query->union($query, 'UNION');
        }
        else {
          $db_query->union($query, 'UNION ALL');
        }
      }
    }

    if (isset($db_query) && !$not_nested) {
      $db_query = $this->connection->select($db_query, 't');
      $db_query->addField('t', 'item_id', 'item_id');
      if (!$neg) {
        $db_query->addExpression('SUM(t.score)', 'score');
        $db_query->groupBy('t.item_id');
      }
      if ($conj == 'AND' && $subs > 1) {
        $var = ':subs' . ((int) $subs);
        if (!$db_query->getGroupBy()) {
          $db_query->groupBy('t.item_id');
        }
        if ($word_hits) {
          // Simply check whether each word matched at least once.
          foreach ($word_hits as $column) {
            $db_query->having("SUM($column) >= 1");
          }
        }
        elseif ($mul_words) {
          $db_query->having('COUNT(DISTINCT t.word) >= ' . $var, array($var => $subs));
        }
        else {
          $db_query->having('COUNT(t.word) >= ' . $var, array($var => $subs));
        }
      }
    }

    if ($negated) {
      if (!isset($db_query) || $conj == 'OR') {
        if (isset($db_query)) {
          // We are in a rather bizarre case where the keys are something like
          // "a OR (NOT b)".
          $old_query = $db_query;
        }
        // We use this table because all items should be contained exactly once.
        $db_query = $this->connection->select($all_fields['search_api_language']['table'], 't');
        $db_query->addField('t', 'item_id', 'item_id');
        if (!$neg) {
          $db_query->addExpression(':score', 'score', array(':score' => self::SCORE_MULTIPLIER));
          $db_query->distinct();
        }
      }

      if ($conj == 'AND') {
        foreach ($negated as $k) {
          $db_query->condition('t.item_id', $this->createKeysQuery($k, $fields, $all_fields), 'NOT IN');
        }
      }
      else {
        $or = db_or();
        foreach ($negated as $k) {
          $or->condition('t.item_id', $this->createKeysQuery($k, $fields, $all_fields), 'NOT IN');
        }
        if (isset($old_query)) {
          $or->condition('t.item_id', $old_query, 'NOT IN');
        }
        $db_query->condition($or);
      }
    }

    if ($neg_nested) {
      $db_query = $this->connection->select($db_query, 't')->fields('t', array('item_id'));
    }

    return $db_query;
  }

  /**
   * Creates a database query condition for a given search filter.
   *
   * Used as a helper method in createDbQuery().
   *
   * @param SearchApiQueryFilterInterface $filter
   *   The filter for which a condition should be created.
   * @param array $fields
   *   Internal information about the index's fields.
   * @param SelectQueryInterface $db_query
   *   The database query to which the condition will be added.
   *
   * @return DatabaseCondition|null
   *   The condition to set on the query, or NULL if none is necessary.
   *
   * @throws SearchApiException
   *   If an unknown field was used in the filter.
   */
  protected function createFilterCondition(SearchApiQueryFilterInterface $filter, array $fields, SelectQueryInterface $db_query) {
    $cond = db_condition($filter->getConjunction());
    // Store whether a JOIN alrady occurred for a field, so we don't JOIN
    // repeatedly for OR filters.
    $first_join = array();
    // Store the table aliases for the fields in this condition group.
    $tables = array();
    foreach ($filter->getFilters() as $f) {
      if (is_object($f)) {
        $c = $this->createFilterCondition($f, $fields, $db_query);
        if ($c) {
          $cond->condition($c);
        }
      }
      else {
        if (!isset($fields[$f[0]])) {
          throw new SearchApiException(t('Unknown field in filter clause: @field.', array('@field' => $f[0])));
        }
        $field = $fields[$f[0]];
        // If the field is in its own table, we have to check for NULL values in
        // a special way (i.e., check for missing entries in that table).
        if ($f[1] === NULL && $field['column'] === 'value') {
          $query = $this->connection->select($field['table'], 't')
            ->fields('t', array('item_id'));
          $cond->condition('t.item_id', $query, $f[2] == '<>' || $f[2] == '!=' ? 'IN' : 'NOT IN');
          continue;
        }
        if (search_api_is_text_type($field['type'])) {
          $keys = $this->prepareKeys($f[1]);
          if (!isset($keys)) {
            continue;
          }
          $query = $this->createKeysQuery($keys, array($f[0] => $field), $fields);
          // We only want the item IDs, so we use the keys query as a nested query.
          $query = $this->connection->select($query, 't')->fields('t', array('item_id'));
          $cond->condition('t.item_id', $query, $f[2] == '<>' || $f[2] == '!=' ? 'NOT IN' : 'IN');
        }
        else {
          $new_join = search_api_is_list_type($field['type'])
              && ($filter->getConjunction() == 'AND'
                  || empty($first_join[$f[0]]));
          if ($new_join || empty($tables[$f[0]])) {
            $tables[$f[0]] = $this->getTableAlias($field, $db_query, $new_join);
            $first_join[$f[0]] = TRUE;
          }
          $column = $tables[$f[0]] . '.' . $field['column'];
          if ($f[1] !== NULL) {
            $cond->condition($column, $f[1], $f[2]);
          }
          else {
            $method = ($f[2] == '=') ? 'isNull' : 'isNotNull';
            $cond->$method($column);
          }
        }
      }
    }
    return count($cond->conditions()) > 1 ? $cond : NULL;
  }

  /**
   * Joins a field's table into a database select query.
   *
   * @param array $field
   *   The field information array. The "table" key should contain the table
   *   name to which a join should be made.
   * @param SelectQueryInterface $db_query
   *   The database query used.
   * @param bool $newjoin
   *   (optional) If TRUE, a join is done even if the table was already joined
   *   to in the query.
   * @param string $join
   *   (optional) The join method to use. Must be a method of the $db_query.
   *   Normally, "join", "innerJoin", "leftJoin" and "rightJoin" are supported.
   *
   * @return string
   *   The alias for the field's table.
   */
  protected function getTableAlias(array $field, SelectQueryInterface $db_query, $newjoin = FALSE, $join = 'leftJoin') {
    if(!$newjoin) {
      foreach ($db_query->getTables() as $alias => $info) {
        $table = $info['table'];
        if (is_scalar($table) && $table == $field['table']) {
          return $alias;
        }
      }
    }
    return $db_query->$join($field['table'], 't', 't.item_id = %alias.item_id');
  }

  /**
   * Preprocesses a search's database query before it is executed.
   *
   * This allows subclasses to apply custom changes before the query (and the
   * count query) is executed.
   *
   * @param SelectQueryInterface $db_query
   *   The database query to be executed for the search. Will have "item_id" and
   *   "score" columns in its result.
   * @param SearchApiQueryInterface $query
   *   The search query that is being executed.
   *
   * @see hook_search_api_db_query_alter()
   */
  protected function preQuery(SelectQueryInterface &$db_query, SearchApiQueryInterface $query) {
  }

  /**
   * Postprocess search results.
   *
   * This allows subclasses to apply custom changes before the results are
   * returned.
   *
   * @param array $results
   *   The results array that will be returned for the search, in the format
   *   defined by SearchApiQueryInterface::execute().
   * @param SearchApiQueryInterface $query
   *   The executed search query.
   */
  protected function postQuery(array &$results, SearchApiQueryInterface $query) {
  }

  /**
   * Adds the query sort to a search database query.
   *
   * @param SearchApiQueryInterface $query
   *   The search query whose sorts should be applied.
   * @param SelectQueryInterface $db_query
   *   The database query used for the search.
   * @param array $fields
   *   An array containing information about the internal server storage of the
   *   indexed fields.
   *
   * @throws SearchApiException
   *   If an illegal sort was specified.
   */
  protected function setQuerySort(SearchApiQueryInterface $query, SelectQueryInterface $db_query, array $fields) {
    $sort = $query->getSort();
    if ($sort) {
      foreach ($sort as $field_name => $order) {
        if ($order != 'ASC' && $order != 'DESC') {
          $msg = t('Unknown sort order @order. Assuming "ASC".', array('@order' => $order));
          $this->warnings[$msg] = $msg;
          $order = 'ASC';
        }
        if ($field_name == 'search_api_relevance') {
          $db_query->orderBy('score', $order);
          continue;
        }
        if ($field_name == 'search_api_id') {
          $db_query->orderBy('item_id', $order);
          continue;
        }
        if (!isset($fields[$field_name])) {
          throw new SearchApiException(t('Trying to sort on unknown field @field.', array('@field' => $field_name)));
        }
        $field = $fields[$field_name];
        if (search_api_is_list_type($field['type'])) {
          throw new SearchApiException(t('Cannot sort on field @field of a list type.', array('@field' => $field_name)));
        }
        if (search_api_is_text_type($field['type'])) {
          throw new SearchApiException(t('Cannot sort on fulltext field @field.', array('@field' => $field_name)));
        }
        $alias = $this->getTableAlias($field, $db_query);
        $db_query->orderBy($alias . '.' . $fields[$field_name]['column'], $order);
        // PostgreSQL automatically adds a field to the SELECT list when sorting
        // on it. Therefore, if we have aggregrations present we also have to
        // add the field to the GROUP BY (since Drupal won't do it for us).
        // However, if no aggregations are present, a GROUP BY would lead to
        // another error. Therefore, we only add it if there is already a GROUP
        // BY.
        if ($db_query->getGroupBy()) {
          $db_query->groupBy($alias . '.' . $fields[$field_name]['column']);
        }
      }
    }
    else {
      $db_query->orderBy('score', 'DESC');
    }
  }

  /**
   * Computes facets for a search query.
   *
   * @param SearchApiQueryInterface $query
   *   The search query for which facets should be computed.
   * @param SelectQueryInterface $db_query
   *   A database select query which returns all results of that search query.
   *
   * @return array
   *   An array of facets, as specified by the search_api_facets feature.
   */
  protected function getFacets(SearchApiQueryInterface $query, SelectQueryInterface $db_query) {
    try {
      // Add a tag to the database query to identify it as a facet base query.
      $db_query->addTag('search_api_db_facets_base');

      // Store the results of the query in a temporary table to run facet
      // queries on it afterwards.
      $table = $this->getTemporaryResultsTable($db_query);
      if (!$table) {
        return array();
      }

      $fields = $this->getFieldInfo($query->getIndex());
      $ret = array();
      foreach ($query->getOption('search_api_facets') as $key => $facet) {
        if (empty($fields[$facet['field']])) {
          $this->warnings[] = t('Unknown facet field @field.', array('@field' => $facet['field']));
          continue;
        }
        $field = $fields[$facet['field']];

        if (empty($facet['operator']) || $facet['operator'] != 'or') {
          // All the AND facets can use the main query.
          $select = $this->connection->select($table, 't');
        }
        else {
          // For OR facets, we need to build a different base query that
          // excludes the facet filters applied to the facet.
          $or_query = clone $query;
          $filters = &$or_query->getFilter()->getFilters();
          $tag = 'facet:' . $facet['field'];
          foreach ($filters as $filter_id => $filter) {
            if ($filter instanceof SearchApiQueryFilterInterface && $filter->hasTag($tag)) {
              unset($filters[$filter_id]);
            }
          }
          $or_db_query = $this->createDbQuery($or_query, $fields);
          $select = $this->connection->select($or_db_query, 't');
        }

        // Add tags and metadata.
        $select->addTag('search_api_db_facet');
        $select->addMetaData('search_api_query', $query);
        $select->addMetaData('search_api_db_fields', $fields);
        $select->addMetaData('search_api_db_facet', $facet);

        // If "Include missing facet" is disabled, we use an INNER JOIN and add
        // IS NOT NULL for shared tables.
        $alias = $this->getTableAlias($field, $select, TRUE, $facet['missing'] ? 'leftJoin' : 'innerJoin');
        $select->addField($alias, search_api_is_text_type($field['type']) ? 'word' : $field['column'], 'value');
        if (!$facet['missing'] && !search_api_is_text_type($field['type'])) {
          $select->isNotNull($alias . '.' . $field['column']);
        }
        $select->addExpression('COUNT(DISTINCT t.item_id)', 'num');
        $select->groupBy('value');
        $select->orderBy('num', 'DESC');

        $limit = $facet['limit'];
        if ((int) $limit > 0) {
          $select->range(0, $limit);
        }
        if ($facet['min_count'] > 1) {
          $select->having('COUNT(DISTINCT t.item_id) >= :count', array(':count' => $facet['min_count']));
        }

        $terms = array();
        $values = array();
        $has_missing = FALSE;
        foreach ($select->execute() as $row) {
          $terms[] = array(
            'count' => $row->num,
            'filter' => isset($row->value) ? '"' . $row->value . '"' : '!',
          );
          if (isset($row->value)) {
            $values[] = $row->value;
          }
          else {
            $has_missing = TRUE;
          }
        }

        // If 'Minimum facet count' is set to 0 in the display options for this
        // facet, we need to retrieve all facets, even ones that aren't matched
        // in our search result set above. Here we SELECT all DISTINCT facets,
        // and add in those facets that weren't added above.
        if ($facet['min_count'] < 1) {
          $select = $this->connection->select($field['table'], 't');
          $select->addField('t', $field['column'], 'value');
          $select->distinct();
          if ($values) {
            $select->condition($field['column'], $values, 'NOT IN');
          }
          $select->isNotNull($field['column']);

          // Add tags and metadata.
          $select->addTag('search_api_db_facet_all');
          $select->addMetaData('search_api_query', $query);
          $select->addMetaData('search_api_db_fields', $fields);
          $select->addMetaData('search_api_db_facet', $facet);

          foreach ($select->execute() as $row) {
            $terms[] = array(
              'count' => 0,
              'filter' => '"' . $row->value . '"',
            );
          }
          if ($facet['missing'] && !$has_missing) {
            $terms[] = array(
              'count' => 0,
              'filter' => '!',
            );
          }
        }

        $ret[$key] = $terms;
      }
      return $ret;
    }
    catch (PDOException $e) {
      watchdog_exception('search_api_db', $e, '%type while trying to calculate facets: !message in %function (line %line of %file).');
      return array();
    }
  }

  /**
   * Creates a temporary table from a SelectQuery.
   *
   * Will return the name of a table containing the item IDs of all results, or
   * FALSE on failure.
   *
   * @param SelectQueryInterface $db_query
   *   The select query whose results should be stored in the temporary table.
   *
   * @return string|false
   *   The name of the temporary table, or FALSE on failure.
   */
  protected function getTemporaryResultsTable(SelectQueryInterface $db_query) {
    // We only need the id field, not the score.
    $fields = &$db_query->getFields();
    unset($fields['score']);
    if (count($fields) != 1 || !isset($fields['item_id'])) {
      watchdog('search_api_db', 'Error while adding facets: only "item_id" field should be used, used are: @fields.',
          array('@fields' => implode(', ', array_keys($fields))), WATCHDOG_WARNING);
      return FALSE;
    }
    $expressions = &$db_query->getExpressions();
    $expressions = array();

    // If there's a GROUP BY for item_id, we leave that, all others need to be
    // discarded.
    $group_by = &$db_query->getGroupBy();
    $group_by = array_intersect_key($group_by, array('t.item_id' => TRUE));

    $db_query->distinct();
    if (!$db_query->preExecute()) {
      return FALSE;
    }
    $args = $db_query->getArguments();
    return $this->connection->queryTemporary((string) $db_query, $args);
  }

  /**
   * Implements SearchApiAutocompleteInterface::getAutocompleteSuggestions().
   */
  public function getAutocompleteSuggestions(SearchApiQueryInterface $query, SearchApiAutocompleteSearch $search, $incomplete_key, $user_input) {
    $settings = isset($this->options['autocomplete']) ? $this->options['autocomplete'] : array();
    $settings += array(
      'suggest_suffix' => TRUE,
      'suggest_words' => TRUE,
    );
    // If none of these options is checked, the user apparently chose a very
    // roundabout way of telling us he doesn't want autocompletion.
    if (!array_filter($settings)) {
      return array();
    }

    $index = $query->getIndex();
    if (empty($this->options['indexes'][$index->machine_name])) {
      throw new SearchApiException(t('Unknown index @id.', array('@id' => $index->machine_name)));
    }
    $fields = $this->getFieldInfo($index);

    $suggestions = array();
    $passes = array();

    // Make the input lowercase as the indexed data is also all lowercase.
    $user_input = drupal_strtolower($user_input);
    $incomplete_key = drupal_strtolower($incomplete_key);

    // Decide which methods we want to use.
    if ($incomplete_key && $settings['suggest_suffix']) {
      $passes[] = 1;
      $incomplete_like = $this->connection->escapeLike($incomplete_key) . '%';
    }
    if ($settings['suggest_words']
        && (!$incomplete_key || strlen($incomplete_key) >= $this->options['min_chars'])) {
      $passes[] = 2;
    }

    // We want about half of the suggestions from each enabled method.
    $limit = $query->getOption('limit', 10);
    $limit /= count($passes);

    // Also collect all keywords already contained in the query so we don't
    // suggest them.
    $keys = drupal_map_assoc(preg_split('/[^\p{L}\p{N}]+/u', $user_input, -1, PREG_SPLIT_NO_EMPTY));
    if ($incomplete_key) {
      $keys[$incomplete_key] = $incomplete_key;
    }

    foreach ($passes as $pass) {
      if ($pass == 2 && $incomplete_key) {
        $query->keys($user_input);
      }
      $db_query = $this->createDbQuery($query, $fields);

      // Add additional tags and metadata.
      $db_query->addTag('search_api_db_autocomplete');
      $db_query->addMetaData('search_api_db_autocomplete', array(
        'search' => $search,
        'incomplete_key' => $incomplete_key,
        'user_input' => $user_input,
        'pass' => $pass,
      ));

      // We need a list of all current results to match the suggestions against.
      // However, since MySQL doesn't allow using a temporary table multiple
      // times in one query, we regrettably have to do it this way.
      if (count($query->getFields()) > 1) {
        $all_results = $db_query->execute()->fetchCol();
        // Compute the total number of results so we can later sort out matches
        // that occur too often.
        $total = count($all_results);
      }
      else {
        $table = $this->getTemporaryResultsTable($db_query);
        if (!$table) {
          return NULL;
        }
        $all_results = $this->connection->select($table, 't')
          ->fields('t', array('item_id'));
        $total = $this->connection->query("SELECT COUNT(item_id) FROM {{$table}}")->fetchField();
      }
      $max_occurrences = max(1, floor($total * variable_get('search_api_db_autocomplete_max_occurrences', 0.9)));

      if (!$total) {
        if ($pass == 1) {
          return NULL;
        }
        continue;
      }

      foreach ($query->getFields() as $field) {
        if (!isset($fields[$field]) || !search_api_is_text_type($fields[$field]['type'])) {
          continue;
        }
        $field_query = $this->connection->select($fields[$field]['table'], 't')
          ->fields('t', array('word', 'item_id'))
          ->condition('item_id', $all_results, 'IN');
        if ($pass == 1) {
          $field_query->condition('word', $incomplete_like, 'LIKE')
            ->condition('word', $keys, 'NOT IN');
        }
        if (!isset($word_query)) {
          $word_query = $field_query;
        }
        else {
          $word_query->union($field_query);
        }
      }
      $db_query = $this->connection->select($word_query, 't');
      $db_query->addExpression('COUNT(DISTINCT item_id)', 'results');
      $db_query->fields('t', array('word'))
        ->groupBy('word')
        ->having('COUNT(DISTINCT item_id) <= :max', array(':max' => $max_occurrences))
        ->orderBy('results', 'DESC')
        ->range(0, ceil($limit));
      $incomp_len = strlen($incomplete_key);
      foreach ($db_query->execute() as $row) {
        $suffix = ($pass == 1) ? substr($row->word, $incomp_len) : ' ' . $row->word;
        $suggestions[] = array(
          'suggestion_suffix' => $suffix,
          'results' => $row->results,
        );
      }
    }

    return $suggestions;
  }

  /**
   * Retrieves the internal field information.
   *
   * @param SearchApiIndex $index
   *   The index whose fields should be retrieved.
   *
   * @return array $fields
   *   An array of arrays. The outer array is keyed by field name. Each value
   *   is an associative array with information on the field.
   */
  protected function getFieldInfo(SearchApiIndex $index) {
    $fields = $this->options['indexes'][$index->machine_name];
    foreach ($fields as $key => $field) {
      // Legacy fields do not have column set.
      if (!isset($field['column'])) {
        $fields[$key]['column'] = 'value';
      }
    }
    return $fields;
  }

  /**
   * Emulates self::mbStrcut() if that is not available.
   *
   * Though the Mbstring PHP extension is recommended for running Drupal, it is
   * not required. Therefore, we have to wrap calls to its functions.
   *
   * @param string $str
   *   The string being cut.
   * @param int $start
   *   Starting position in bytes.
   * @param int|null $length
   *   (optional) Length in bytes. If NULL is passed, extract all bytes to the
   *   end of the string.
   *
   * @return string
   *   The portion of $str specified by the $start and $length parameters.
   */
  protected static function mbStrcut($str, $start, $length = NULL) {
    global $multibyte;
    if ($multibyte == UNICODE_MULTIBYTE) {
      return mb_strcut($str, $start, $length);
    }
    return substr($str, $start, $length);
  }

}
