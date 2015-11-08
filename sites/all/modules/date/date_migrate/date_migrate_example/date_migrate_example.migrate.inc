<?php

/**
 * @file
 * Examples and test folder for migration into date fields.
 */

/**
 * Migration class to test import of various date fields.
 */
class DateExampleMigration extends XMLMigration {

  /**
   * Sets up the migration.
   */
  public function __construct() {
    parent::__construct();
    $this->description = t('Example migration into date fields');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Date ID',
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Source fields available in the XML file.
    $fields = array(
      'id' => t('Source id'),
      'title' => t('Title'),
      'body' => t('Description'),
      'date' => t('A simple date'),
      'date_range_from' => t('Start value for a date range'),
      'datestamp' => t('Simple datestamp'),
      'datestamp_range_from' => t('Start value for a datestamp range'),
      'datetime' => t('Simple datetime'),
      'datetime_range_from' => t('Start value for a datetime range'),
      'date_repeat' => t('Sample of a repeating date field'),
    );

    // Our test data is in an XML file.
    $xml_folder = drupal_get_path('module', 'date_migrate_example');
    $items_url = $xml_folder . '/date_migrate_example.xml';
    $item_xpath = '/source_data/item';
    $item_id_xpath = 'id';
    $items_class = new MigrateItemsXML($items_url, $item_xpath, $item_id_xpath);
    $this->source = new MigrateSourceMultiItems($items_class, $fields);
    $this->destination = new MigrateDestinationNode('date_migrate_example');

    // Basic fields.
    $this->addFieldMapping('title', 'title')
      ->xpath('title');
    $this->addFieldMapping('uid')
      ->defaultValue(1);
    $this->addFieldMapping('body', 'body')
      ->xpath('body');

    // For simple date fields, we just need the xpath.
    $this->addFieldMapping('field_date', 'date')
      ->xpath('date');

    // For date ranges, we add the "end" value in prepareRow() below.
    $this->addFieldMapping('field_date_range', 'date_range_from');
    $this->addFieldMapping('field_date_range:to', 'date_range_to');

    // RRULEs on repeat fields are also done in prepareRow().
    $this->addFieldMapping('field_date_repeat', 'date_repeat');
    $this->addFieldMapping('field_date_repeat:rrule', 'date_repeat_rrule');

    $this->addFieldMapping('field_datestamp', 'datestamp')
      ->xpath('datestamp');
    $this->addFieldMapping('field_datestamp_range', 'datestamp_range_from');
    $this->addFieldMapping('field_datestamp_range:to', 'datestamp_range_to');

    // You can specify a timezone to be applied to all values going into the
    // field (Tokyo is UTC+9, no DST).
    $this->addFieldMapping('field_datetime', 'datetime')
      ->xpath('datetime');
    $this->addFieldMapping('field_datetime:timezone')
      ->defaultValue('Asia/Tokyo');

    // You can also get the timezone from the source data - it can be different
    // for each instance of the field. Like To and RRULE values, it is added
    // in prepareRow().
    $this->addFieldMapping('field_datetime_range', 'datetime_range_from');
    $this->addFieldMapping('field_datetime_range:to', 'datetime_range_to');
    $this->addFieldMapping('field_datetime_range:timezone', 'datetime_range_timezone');

    // Unmapped destination fields.
    $this->addUnmigratedDestinations(array('is_new', 'status', 'promote',
      'revision', 'language', 'sticky', 'created', 'changed', 'revision_uid'));
  }

  /**
   * Transforms the raw migration data into the expected date formats.
   *
   * An advanced feature of the date field handler is that in addition to the
   * basic (Start) date itself, we can add additional properties like timezone,
   * encapsulating them as JSON.
   */
  public function prepareRow($current_row) {
    // The date range field can have multiple values.
    $current_row->date_range_from = array();
    foreach ($current_row->xml->date_range as $range) {
      $current_row->date_range_from[] = (string) $range->from[0];
      $current_row->date_range_to[] = (string) $range->to[0];
    }

    $current_row->datestamp_range_from
      = (string) $current_row->xml->datestamp_range->from[0];
    $current_row->datestamp_range_to
      = (string) $current_row->xml->datestamp_range->to[0];

    $current_row->datetime_range_from
      = (string) $current_row->xml->datetime_range->from[0];
    $current_row->datetime_range_to
      = (string) $current_row->xml->datetime_range->to[0];
    $current_row->datetime_range_timezone
      = (string) $current_row->xml->datetime_range->timezone[0];

    $current_row->date_repeat
      = (string) $current_row->xml->date_repeat->date[0];
    $current_row->date_repeat_rrule
      = (string) $current_row->xml->date_repeat->rule[0];
  }
}
