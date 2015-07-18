<?php

/**
 * @file
 * CTools plugins declarations.
 */

/**
 * Break out for feeds_feed_plugins().
 */
function _feeds_feeds_plugins() {
  $path = drupal_get_path('module', 'feeds') . '/plugins';

  $info = array();
  $info['FeedsPlugin'] = array(
    'hidden' => TRUE,
    'handler' => array(
      'class' => 'FeedsPlugin',
      'file' => 'FeedsPlugin.inc',
      'path' => $path,
    ),
  );
  $info['FeedsMissingPlugin'] = array(
    'name' => 'Missing plugin',
    'description' => 'There is a problem with your configuration.',
    'handler' => array(
      'class' => 'FeedsMissingPlugin',
      'file' => 'FeedsPlugin.inc',
      'path' => $path,
    ),
  );
  $info['FeedsFetcher'] = array(
    'hidden' => TRUE,
    'handler' => array(
      'parent' => 'FeedsPlugin',
      'class' => 'FeedsFetcher',
      'file' => 'FeedsFetcher.inc',
      'path' => $path,
    ),
  );
  $info['FeedsParser'] = array(
    'hidden' => TRUE,
    'handler' => array(
      'parent' => 'FeedsPlugin',
      'class' => 'FeedsParser',
      'file' => 'FeedsParser.inc',
      'path' => $path,
    ),
  );
  $info['FeedsProcessor'] = array(
    'hidden' => TRUE,
    'handler' => array(
      'parent' => 'FeedsPlugin',
      'class' => 'FeedsProcessor',
      'file' => 'FeedsProcessor.inc',
      'path' => $path,
    ),
  );
  $info['FeedsHTTPFetcher'] = array(
    'name' => 'HTTP Fetcher',
    'description' => 'Download content from a URL.',
    'handler' => array(
      'parent' => 'FeedsFetcher', // This is the key name, not the class name.
      'class' => 'FeedsHTTPFetcher',
      'file' => 'FeedsHTTPFetcher.inc',
      'path' => $path,
    ),
  );
  $info['FeedsFileFetcher'] = array(
    'name' => 'File upload',
    'description' => 'Upload content from a local file.',
    'handler' => array(
      'parent' => 'FeedsFetcher',
      'class' => 'FeedsFileFetcher',
      'file' => 'FeedsFileFetcher.inc',
      'path' => $path,
    ),
  );
  $info['FeedsCSVParser'] = array(
    'name' => 'CSV parser',
    'description' => 'Parse data in Comma Separated Value format.',
    'handler' => array(
      'parent' => 'FeedsParser',
      'class' => 'FeedsCSVParser',
      'file' => 'FeedsCSVParser.inc',
      'path' => $path,
    ),
  );
  if (extension_loaded('SimpleXML')) {
    $info['FeedsSyndicationParser'] = array(
      'name' => 'Common syndication parser',
      'description' => 'Parse RSS and Atom feeds.',
      'help' => 'Parse XML feeds in RSS 1, RSS 2 and Atom format.',
      'handler' => array(
        'parent' => 'FeedsParser',
        'class' => 'FeedsSyndicationParser',
        'file' => 'FeedsSyndicationParser.inc',
        'path' => $path,
      ),
    );
  }
  $info['FeedsOPMLParser'] = array(
    'name' => 'OPML parser',
    'description' => 'Parse OPML files.',
    'handler' => array(
      'parent' => 'FeedsParser',
      'class' => 'FeedsOPMLParser',
      'file' => 'FeedsOPMLParser.inc',
      'path' => $path,
    ),
  );
  if (feeds_simplepie_exists()) {
    $info['FeedsSimplePieParser'] = array(
      'name' => 'SimplePie parser',
      'description' => 'Parse RSS and Atom feeds.',
      'help' => 'Use <a href="http://simplepie.org">SimplePie</a> to parse XML feeds in RSS 1, RSS 2 and Atom format.',
      'handler' => array(
        'parent' => 'FeedsParser',
        'class' => 'FeedsSimplePieParser',
        'file' => 'FeedsSimplePieParser.inc',
        'path' => $path,
      ),
    );
  }
  $info['FeedsSitemapParser'] = array(
    'name' => 'Sitemap parser',
    'description' => 'Parse Sitemap XML format feeds.',
    'handler' => array(
      'parent' => 'FeedsParser',
      'class' => 'FeedsSitemapParser',
      'file' => 'FeedsSitemapParser.inc',
      'path' => $path,
    ),
  );
  $info['FeedsNodeProcessor'] = array(
    'name' => 'Node processor',
    'description' => 'Create and update nodes.',
    'help' => 'Create and update nodes from parsed content.',
    'handler' => array(
      'parent' => 'FeedsProcessor',
      'class' => 'FeedsNodeProcessor',
      'file' => 'FeedsNodeProcessor.inc',
      'path' => $path,
    ),
  );
  $info['FeedsUserProcessor'] = array(
    'name' => 'User processor',
    'description' => 'Create users.',
    'help' => 'Create users from parsed content.',
    'handler' => array(
      'parent' => 'FeedsProcessor',
      'class' => 'FeedsUserProcessor',
      'file' => 'FeedsUserProcessor.inc',
      'path' => $path,
    ),
  );
  if (module_exists('taxonomy')) {
    $info['FeedsTermProcessor'] = array(
      'name' => 'Taxonomy term processor',
      'description' => 'Create taxonomy terms.',
      'help' => 'Create taxonomy terms from parsed content.',
      'handler' => array(
        'parent' => 'FeedsProcessor',
        'class' => 'FeedsTermProcessor',
        'file' => 'FeedsTermProcessor.inc',
        'path' => $path,
      ),
    );
  }

  return $info;
}
