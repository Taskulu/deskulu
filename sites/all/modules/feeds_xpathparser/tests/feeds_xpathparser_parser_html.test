<?php

/**
 * @file
 * Tests for FeedsXPathParser.inc.
 */

/**
 * Test single feeds.
 */
class FeedsXPathParseHTMLTestCase extends FeedsXPathParserWebTestCase {

  /**
   * Describe this test.
   */
  public static function getInfo() {
    return array(
      'name' => 'HTML Parser',
      'description' => 'Regression tests for Feeds XPath HTML parser.',
      'group' => 'Feeds XPath Parser',
    );
  }

  /**
   * Run tests.
   */
  public function test() {
    $this->createImporterConfiguration('XPath', 'xpath');

    $this->setPlugin('xpath', 'FeedsXPathParserHTML');
    $this->addMappings('xpath', array(
      0 => array(
        'source' => 'xpathparser:0',
        'target' => 'title',
        'unique' => FALSE,
      ),
      1 => array(
        'source' => 'xpathparser:1',
        'target' => 'url',
        'unique' => TRUE,
      ),
    ));
    // Set importer default settings.
    $importer_url = $this->feeds_base . '/xpath/settings/FeedsXPathParserHTML';
    $edit = array(
      'xpath[context]' => '//tr[starts-with(@class, "odd ") or starts-with(@class, "even ")]',
      'xpath[sources][xpathparser:0]' => 'td[1]/a',
      'xpath[sources][xpathparser:1]' => 'td[1]/a/@href',
    );
    $this->postAndCheck($importer_url, $edit, t('Save'), t('Your changes have been saved.'));

    // Test import.
    // Set batch limit to 5 to force batching.
    variable_set('feeds_process_limit', 5);
    $path = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_xpathparser') . '/tests/feeds_xpathparser/';
    $nid = $this->createFeedNode('xpath', $path . 'issues_drupal.org.htm', 'Testing XPath HTML Parser');
    $feed_node_edit_url = 'node/' . $nid . '/edit';
    $this->assertText(t('Created 29 nodes'));

    // Import again, this verifies url field was mapped correctly.
    $this->drupalPost('node/' . $nid . '/import', array(), t('Import'));
    $this->assertText(t('There are no new nodes'));

    // Assert accuracy of aggregated content. I find humor in using our own
    // issue queue to run tests against.
    $this->drupalGet('node');
    $this->assertText('Xpath Functions');
    $this->assertText('Unable to upload .html files');
    $this->assertText('Import to multiple content types');
    $this->assertText('Parser includes tags in mapped output');
    $this->assertText('Errors');
    $this->assertText('Loop through HTML - all data is in one node?');
    $this->assertText('Patch: add encoding options for PHP tidy feature');
    $this->assertText('Import and Maintain 1300+ Node Items');
    $this->assertText('Documentation update');
    $this->assertText('An HTTP error 404 occured');
    $this->assertText('Does it work with Feeds Image Grabber');
    $this->assertText('Node published date not being correctly mapped (set to 1 Jan 1970)');
    $this->assertText('fields to fill xpath not displayed in importer interface except for &quot;body&quot;');
    $this->assertText('parsing link field');
    $this->assertText('Error when switching to XML Parser');
    $this->assertText('Duplicate content even if &quot;unique target&quot; is set');
    $this->assertText('Labels/field names become meaningless with Data Processor');
    $this->assertText('Xpath namespace help');
    $this->assertText('warning: mysql_real_escape_string()');
    $this->assertText('Feeds XPath Parser: warning: Invalid argument');
    $this->assertText('What am I missing? FeedsXPathParser: No mappings are defined.');
    $this->assertText('CDATA in tag not producing text');
    $this->assertText('Cant map empty fields');
    $this->assertText('Support literal XPath expressions');
    $this->assertText('adding a prefix to a parsed xml value.');
    $this->assertText('Mapping on import');
    $this->assertText('Feeds XPath Parser: HTML parser example for number expressions');
    $this->assertText("I dont want to define any field queries");
    $this->assertText("Document // and other syntax for this module a little better");

    // Test debugging.
    $edit = array(
      'feeds[FeedsXPathParserHTML][xpath][exp][debug][xpathparser:0]' => TRUE,
    );
    $this->postAndCheck($feed_node_edit_url, $edit, t('Save'), 'Basic page Testing XPath HTML Parser has been updated.');
    $this->drupalPost('node/' . $nid . '/import', array(), t('Import'));
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/976478&quot;&gt;Xpath Functions&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1048030&quot;&gt;Unable to upload .html files&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1050310&quot;&gt;Import to multiple content types&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1047788&quot;&gt;Parser includes tags in mapped output&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1043608&quot;&gt;Errors&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1044546&quot;&gt;Loop through HTML - all data is in one node?&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1043728&quot;&gt;Patch: add encoding options for PHP tidy feature&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1040132&quot;&gt;Import and Maintain 1300+ Node Items&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1043604&quot;&gt;Documentation update&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1039492&quot;&gt;An HTTP error 404 occured&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1042048&quot;&gt;Does it work with Feeds Image Grabber&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/961158&quot;&gt;Node published date not being correctly mapped (set to 1 Jan 1970)&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1021474&quot;&gt;fields to fill xpath not displayed in importer interface except for &quot;body&quot;&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1040530&quot;&gt;parsing link field&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1038912&quot;&gt;Error when switching to XML Parser&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1032340&quot;&gt;Duplicate content even if &quot;unique target&quot; is set&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/982102&quot;&gt;Labels/field names become meaningless with Data Processor&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/1034758&quot;&gt;Xpath namespace help&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/908458&quot;&gt;warning: mysql_real_escape_string()&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/869076&quot;&gt;Feeds XPath Parser: warning: Invalid argument&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/991386&quot;&gt;What am I missing? FeedsXPathParser: No mappings are defined.&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/990972&quot;&gt;CDATA in tag not producing text&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/989948&quot;&gt;Cant map empty fields&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/973324&quot;&gt;Support literal XPath expressions&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/958344&quot;&gt;adding a prefix to a parsed xml value.&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/914216&quot;&gt;Mapping on import&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/863714&quot;&gt;Feeds XPath Parser: HTML parser example for number expressions&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/915856&quot;&gt;I dont want to define any field queries&lt;/a&gt;');
    $this->assertText('&lt;a href=&quot;http://drupal.org/node/950150&quot;&gt;Document // and other syntax for this module a little better&lt;/a&gt;');
    $this->assertText(t('There are no new nodes'));
    // Turn debugging off.
    $edit = array(
      'feeds[FeedsXPathParserHTML][xpath][exp][debug][xpathparser:0]' => FALSE,
    );
    $this->postAndCheck($feed_node_edit_url, $edit, t('Save'), 'Basic page Testing XPath HTML Parser has been updated.');

    // Test that overriding default settings works.
    $edit = array(
      'feeds[FeedsXPathParserHTML][xpath][context]' => '/foo',
      'feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:0]' => 'bar',
      'feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:1]' => 'baz',
    );

    $this->postAndCheck($feed_node_edit_url, $edit, t('Save'), 'Basic page Testing XPath HTML Parser has been updated.');

    // Assert the we don't create an empty node when XPath values don't return anything.
    // That happened at one point.
    $this->drupalPost('node/' . $nid . '/import', array(), t('Import'));
    $this->assertText(t('There are no new nodes'));

    // Test that validation works.
    $edit = array(
      'feeds[FeedsXPathParserHTML][xpath][context]' => 'sdf asf',
      'feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:0]' => 'asdf[sadfas asdf]',
    );
    $this->drupalPost($feed_node_edit_url, $edit, 'Save');
    // Check for valid error messages.
    $this->assertText('There was an error with the XPath selector: Invalid expression');
    $this->assertText('There was an error with the XPath selector: Invalid predicate');
    // Make sure the fields are errored out correctly. I.e. we have red outlines.
    $this->assertFieldByXPath('//input[@id="edit-feeds-feedsxpathparserhtml-xpath-context"][1]/@class', 'form-text required error');
    $this->assertFieldByXPath('//input[@id="edit-feeds-feedsxpathparserhtml-xpath-sources-xpathparser0"][1]/@class', 'form-text error');

    // Put the values back so we can test inheritance if the form was changed
    // and then changed back.
    $edit = array(
      'feeds[FeedsXPathParserHTML][xpath][context]' => '//tr[starts-with(@class, "odd ") or starts-with(@class, "even ")]',
      'feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:0]' => 'td[1]/a',
      'feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:1]' => 'td[1]/a/@href',
    );
    $this->postAndCheck($feed_node_edit_url, $edit, t('Save'), t('Basic page Testing XPath HTML Parser has been updated.'));

    // Change importer defaults.
    $edit = array(
      'xpath[context]' => '//tr',
      'xpath[sources][xpathparser:0]' => 'booya',
      'xpath[sources][xpathparser:1]' => 'boyz',
    );
    $this->postAndCheck($importer_url, $edit, t('Save'), t('Your changes have been saved.'));

    // Make sure the changes propigated.
    $this->drupalGet($feed_node_edit_url);
    $this->assertFieldByName('feeds[FeedsXPathParserHTML][xpath][context]', '//tr');
    $this->assertFieldByName('feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:0]', 'booya');
    $this->assertFieldByName('feeds[FeedsXPathParserHTML][xpath][sources][xpathparser:1]', 'boyz');

    //Cleanup
    $this->drupalPost("node/$nid/delete-items", array(), t('Delete'));
    $this->assertText(t('Deleted 29 nodes'));

    $this->_testGetRaw($importer_url);
  }


  public function _testGetRaw($importer_url) {
    $this->addMappings('xpath', array(
      2 => array(
        'source' => 'xpathparser:2',
        'target' => 'body',
      ),
    ));
    // Change importer defaults.
    $edit = array(
      'xpath[context]' => '/html',
      'xpath[sources][xpathparser:0]' => 'head/title',
      'xpath[sources][xpathparser:2]' => 'body',
      'xpath[rawXML][xpathparser:2]' => TRUE,
    );
    $this->postAndCheck($importer_url, $edit, t('Save'), t('Your changes have been saved.'));
    $path = $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'feeds_xpathparser') . '/tests/feeds_xpathparser/';
    $nid = $this->createFeedNode('xpath', $path . 'simple.html', 'Testing GetRaw');
    $feed_node_edit_url = "node/$nid/edit";
    $this->assertText(t('Created 1 node'));
    $url = 'node/' . ++$nid . '/edit';
    $this->drupalGet($url);
    $this->assertFieldByName('body[und][0][value]', '<body><div>bla bla</div></body>');
  }

}
