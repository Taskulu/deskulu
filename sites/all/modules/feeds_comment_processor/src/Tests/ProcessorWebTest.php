<?php

/**
 * @file
 * Contains \Drupal\feeds_comment_processor\Tests\ProcessorWebTest.
 */

namespace Drupal\feeds_comment_processor\Tests;

/**
 * Basic behavior tests for feeds_comment_processor.
 */
class ProcessorWebTest extends \FeedsWebTestCase {

  public static function getInfo() {
    return array(
      'name' => 'Tests for comment processor',
      'description' => 'Tests that comments can be imported.',
      'group' => 'Feeds comment processor',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function setUp() {
    parent::setUp('comment', 'feeds_comment_processor');

    // Create an importer configuration.
    $this->createImporterConfiguration('Test', 'comment');

    // Set and configure plugins.
    $this->setPlugin('comment', 'FeedsCSVParser');
    $this->setPlugin('comment', 'FeedsCommentProcessor');

    $edit = array(
      'bundle' => 'comment_node_article',
    );
    $this->setSettings('comment', 'FeedsCommentProcessor', $edit);

    $this->addMappings('comment', array(
      0 => array(
        'source' => 'subject',
        'target' => 'subject',
      ),
      1 => array(
        'source' => 'guid',
        'target' => 'nid_by_guid',
      ),
      2 => array(
        'source' => 'body',
        'target' => 'comment_body',
        'format' => 'plain_text',
      ),
      3 => array(
        'source' => 'status',
        'target' => 'status',
      ),
      4 => array(
        'source' => 'hostname',
        'target' => 'hostname',
      ),
    ));

    $parent = (object) array('title' => 'Parent', 'type' => 'article');
    node_save($parent);

    // Insert a feeds_item record.
    $item = (object) array(
      'guid' => 10,
      'url' => '',
      'entity_type' => 'node',
      'entity_id' => $parent->nid,
      'feed_nid' => 0,
      'id' => 'node_importer',
    );
    drupal_write_record('feeds_item', $item);
  }

  /**
   * Tests a very basic import.
   */
  public function test() {
    $url = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_comment_processor') . '/tests/test.csv';
    $nid = $this->createFeedNode('comment', $url, 'Comment test');

    $this->assertText('Created 1 comment');

    $comment = comment_load(1);
    $this->assertEqual(1, $comment->nid);
    $this->assertEqual('test subject', $comment->subject);
    $this->assertEqual('example.com', $comment->hostname);
    $this->assertEqual('test body text', $comment->comment_body[LANGUAGE_NONE][0]['value']);
    $this->assertEqual('plain_text', $comment->comment_body[LANGUAGE_NONE][0]['format']);
  }

  /**
   * Tests authorization.
   */
  public function testAuthorize() {
    // Create a user with limited permissions. We can't use
    // $this->drupalCreateUser here because we need to to set a specific user
    // name.
    $edit = array(
      'name' => 'Poor user',
      'mail' => 'poor@example.com',
      'pass' => user_password(),
      'status' => 1,
    );

    $account = user_save(drupal_anonymous_user(), $edit);

    // // Adding a mapping to the user_name will invoke authorization.
    $this->addMappings('comment', array(
      5 => array(
        'source' => 'mail',
        'target' => 'user_mail',
      ),
    ));

    $url = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_comment_processor') . '/tests/test.csv';
    $nid = $this->createFeedNode('comment', $url, 'Comment test');

    $this->assertText('Failed importing 1 comment');
    $this->assertText('User ' . $account->name . ' is not permitted to post comments.');
    $this->assertEqual(0, db_query("SELECT COUNT(*) FROM {comment}")->fetchField());

    user_role_change_permissions(2, array('post comments' => TRUE));

    $this->drupalPost("node/$nid/import", array(), 'Import');
    $this->assertText('Created 1 comment.');
    $this->assertEqual(1, db_query("SELECT COUNT(*) FROM {comment}")->fetchField());
    $comment = comment_load(1);
    $this->assertEqual(0, $comment->status);
  }

  /**
   * Tests importing existing cids.
   */
  public function testMappingCid() {
    // // Adding a mapping to the user_name will invoke authorization.
    $this->addMappings('comment', array(
      5 => array(
        'source' => 'guid',
        'target' => 'cid',
      ),
    ));

    $url = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_comment_processor') . '/tests/test.csv';
    $nid = $this->createFeedNode('comment', $url, 'Comment test');

    $this->assertText('Created 1 comment.');
    $this->assertEqual(1, db_query("SELECT COUNT(*) FROM {comment}")->fetchField());
    $comment = comment_load(10);
    $this->assertEqual(10, $comment->cid);
    $this->assertEqual(1, $comment->nid);
    $this->assertEqual('01/', $comment->thread);
  }

  /**
   * Tests mapping to node by title.
   */
  public function testMappingByTitle() {
    $this->removeMappings('comment', array(
      1 => array(
        'source' => 'guid',
        'target' => 'nid_by_guid',
      ),
    ));

    $this->addMappings('comment', array(
      4 => array(
        'source' => 'title',
        'target' => 'nid_by_title',
      ),
    ));

    $url = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_comment_processor') . '/tests/test.csv';
    $nid = $this->createFeedNode('comment', $url, 'Comment test');

    $this->assertText('Created 1 comment');

    $comment = comment_load(1);
    $this->assertEqual(1, $comment->nid);
    $this->assertEqual('test subject', $comment->subject);
    $this->assertEqual('example.com', $comment->hostname);
    $this->assertEqual('test body text', $comment->comment_body[LANGUAGE_NONE][0]['value']);
    $this->assertEqual('plain_text', $comment->comment_body[LANGUAGE_NONE][0]['format']);
  }

}
