<?php

/**
 * @file
 * Tests for FeedsXPathParserXML.inc.
 */

/**
 * Test single feeds.
 */
class FeedsXPathParserWebTestCase extends FeedsWebTestCase {

  /**
   * Set up test.
   */
  public function setUp() {
    parent::setUp('feeds_xpathparser');

    // Set the front page to show 30 nodes so we can easily see what is aggregated.
    $edit = array('default_nodes_main' => 30);
    $this->drupalPost('admin/config/system/site-information', $edit, 'Save configuration');

    // Set the teaser length display to unlimited otherwise tests looking for
    // text on nodes will fail.
    $edit = array('fields[body][type]' => 'text_default');
    $this->drupalPost('admin/structure/types/manage/article/display/teaser', $edit, 'Save');

    // Generalize across my version of feeds and the standard one.
    $items = feeds_ui_menu();
    if (isset($items['admin/structure/feeds/%feeds_importer/edit'])) {
      $this->feeds_base = 'admin/structure/feeds';
    }
    else {
      $this->feeds_base = 'admin/structure/feeds/edit';
    }
  }

  function postAndCheck($url, $edit, $button, $saved_text) {
    $this->drupalPost($url, $edit, $button);
    $this->assertText($saved_text);
    $this->drupalGet($url);
    foreach ($edit as $key => $value) {
      $this->assertFieldByName($key, $value);
    }
  }
}
