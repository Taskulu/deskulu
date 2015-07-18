<?php

/**
 * @file
 * File fetcher tests.
 */

/**
 * File fetcher test class.
 */
class FeedsFileFetcherTestCase extends FeedsWebTestCase {
  public static function getInfo() {
    return array(
      'name' => 'File fetcher',
      'description' => 'Tests for file fetcher plugin.',
      'group' => 'Feeds',
    );
  }

  /**
   * Test scheduling on cron.
   */
  public function testPublicFiles() {
    // Set up an importer.
    $this->createImporterConfiguration('Node import', 'node');
    // Set and configure plugins and mappings.
    $this->setSettings('node', NULL, array('content_type' => ''));
    $this->setPlugin('node', 'FeedsFileFetcher');
    $this->setPlugin('node', 'FeedsCSVParser');
    $mappings = array(
      '0' => array(
        'source' => 'title',
        'target' => 'title',
      ),
    );
    $this->addMappings('node', $mappings);
    // Straight up upload is covered in other tests, focus on direct mode
    // and file batching here.
    $settings = array(
      'direct' => TRUE,
      'directory' => 'public://feeds',
    );
    $this->setSettings('node', 'FeedsFileFetcher', $settings);

    // Verify that invalid paths are not accepted.
    foreach (array('/tmp/') as $path) {
      $edit = array(
        'feeds[FeedsFileFetcher][source]' => $path,
      );
      $this->drupalPost('import/node', $edit, t('Import'));
      $this->assertText("The file needs to reside within the site's files directory, its path needs to start with scheme://. Available schemes:");
      $count = db_query("SELECT COUNT(*) FROM {feeds_source} WHERE feed_nid = 0")->fetchField();
      $this->assertEqual($count, 0);
    }

    // Verify batching through directories.
    // Copy directory of files.
    $dir = 'public://batchtest';
    $this->copyDir($this->absolutePath() . '/tests/feeds/batch', $dir);

    // Ingest directory of files. Set limit to 5 to force processor to batch,
    // too.
    variable_set('feeds_process_limit', 5);
    $edit = array(
      'feeds[FeedsFileFetcher][source]' => $dir,
    );
    $this->drupalPost('import/node', $edit, t('Import'));
    $this->assertText('Created 18 nodes');
  }

  /**
   * Test uploading private files.
   */
  public function testPrivateFiles() {
    // Set up an importer.
    $this->createImporterConfiguration('Node import', 'node');
    // Set and configure plugins and mappings.
    $edit = array(
      'content_type' => '',
    );
    $this->drupalPost('admin/structure/feeds/node/settings', $edit, 'Save');
    $this->setPlugin('node', 'FeedsFileFetcher');
    $this->setPlugin('node', 'FeedsCSVParser');
    $mappings = array(
      '0' => array(
        'source' => 'title',
        'target' => 'title',
      ),
    );
    $this->addMappings('node', $mappings);
    // Straight up upload is covered in other tests, focus on direct mode
    // and file batching here.
    $settings = array(
      'direct' => TRUE,
      'directory' => 'private://feeds',
    );
    $this->setSettings('node', 'FeedsFileFetcher', $settings);

    // Verify batching through directories.
    // Copy directory of files.
    $dir = 'private://batchtest';
    $this->copyDir($this->absolutePath() . '/tests/feeds/batch', $dir);

    // Ingest directory of files. Set limit to 5 to force processor to batch,
    // too.
    variable_set('feeds_process_limit', 5);
    $edit = array(
      'feeds[FeedsFileFetcher][source]' => $dir,
    );
    $this->drupalPost('import/node', $edit, t('Import'));
    $this->assertText('Created 18 nodes');
  }

}
