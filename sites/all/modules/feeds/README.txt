

"It feeds"


FEEDS
=====

An import and aggregation framework for Drupal.
http://drupal.org/project/feeds

Features
========

- Pluggable import configurations consisting of fetchers (get data) parsers
  (read and transform data) and processors (create content on Drupal).
-- HTTP upload (with optional PubSubHubbub support).
-- File upload.
-- CSV, RSS, Atom parsing.
-- Creates nodes or terms.
-- Creates lightweight database records if Data module is installed.
   http://drupal.org/project/data
-- Additional fetchers/parsers or processors can be added by an object oriented
   plugin system.
-- Granular mapping of parsed data to content elements.
- Import configurations can be piggy backed on nodes (thus using nodes to track
  subscriptions to feeds) or they can be used on a standalone form.
- Unlimited number of import configurations.
- Export import configurations to code.
- Optional libraries module support.

Requirements
============

- CTools 1.x
  http://drupal.org/project/ctools
- Job Scheduler
  http://drupal.org/project/job_scheduler
- Drupal 7.x
  http://drupal.org/project/drupal
- PHP safe mode is not supported, depending on your Feeds Importer configuration
  safe mode may cause no problems though.

Installation
============

- Install Feeds, Feeds Admin UI.
- To get started quick, install one or all of the following Feature modules:
  Feeds News, Feeds Import, Feeds Fast News (more info below).
- Make sure cron is correctly configured http://drupal.org/cron
- Go to import/ to import data.

SimplePie Installation
======================

- To install the SimplePie parser plugin, complete the following steps:
  1. Download SimplePie from http://simplepie.org/downloads. The recommended
     version is: 1.3.
  2. Decompress the downloaded zip file.
  3. Rename the uncompressed folder to 'simplepie'.
     For example rename 'simplepie-simplepie-e9472a1' to 'simplepie'.
  4. Move the folder to sites/all/libraries. The final directory structure
     should be sites/all/libraries/simplepie.
  5. Flush the Drupal cache.
  6. The SimplePie parser should be available now in the list of parsers.

Feature modules
===============

Feeds ships with three feature modules that can be enabled on
admin/build/modules or - if you are using Features - on admin/build/features.
http://drupal.org/project/features

The purpose of these modules is to provide a quick start for using Feeds. You
can either use them out of the box as they come or you can take them as samples
to learn how to build import or aggregation functionality with Feeds.

The feature modules merely contain sets of configurations using Feeds and in
some cases the modules Node, Views or Data. If the default configurations do not
fit your use case you can change them on the respective configuration pages for
Feeds, Node, Views or Data.

Here is a description of the provided feature modules:

- Feeds News -

This feature is a news aggregator. It provides a content type "Feed" that can
be used to subscribe to RSS or Atom feeds. Every item on such a feed is
aggregated as a node of the type "Feed item", also provided by the module.

What's neat about Feeds News is that it comes with a configured View that shows
a list of news items with every feed on the feed node's "View items" tab. It
also comes with an OPML importer filter that can be accessed under /import.

- Feeds Fast News -

This feature is very similar to Feeds News. The big difference is that instead
of aggregating a node for every item on a feed, it creates a database record
in a single table, thus significantly improving performance. This approach
especially starts to save resources when many items are being aggregated and
expired (= deleted) on a site.

- Feeds Import -

This feature is an example illustrating Feeds' import capabilities. It contains
a node importer and a user importer that can be accessed under /import. Both
accept CSV or TSV files as imports.

PubSubHubbub support
====================

Feeds supports the PubSubHubbub publish/subscribe protocol. Follow these steps
to set it up for your site.
http://code.google.com/p/pubsubhubbub/

- Go to admin/build/feeds and edit (override) the importer configuration you
  would like to use for PubSubHubbub.
- Choose the HTTP Fetcher if it is not already selected.
- On the HTTP Fetcher, click on 'settings' and check "Use PubSubHubbub".
- Optionally you can use a designated hub such as http://superfeedr.com/ or your
  own. If a designated hub is specified, every feed on this importer
  configuration will be subscribed to this hub, no matter what the feed itself
  specifies.

Libraries support
=================

If you are using Libraries module, you can place external libraries in the
Libraries module's search path (for instance sites/all/libraries. The only
external library used at the moment is SimplePie.

Libraries found in the libraries search path are preferred over libraries in
feeds/libraries/.

Transliteration support
=======================

If you plan to store files with Feeds - for instance when storing podcasts
or images from syndication feeds - it is recommended to enable the
Transliteration module to avoid issues with non-ASCII characters in file names.
http://drupal.org/project/transliteration

API Overview
============

See "The developer's guide to Feeds":
http://drupal.org/node/622700

Testing
=======

See "The developer's guide to Feeds":
http://drupal.org/node/622700

Debugging
=========

Set the Drupal variable 'feeds_debug' to TRUE (i. e. using drush). This will
create a file /tmp/feeds_[my_site_location].log. Use "tail -f" on the command
line to get a live view of debug output.

Note: at the moment, only PubSubHubbub related actions are logged.

Performance
===========

See "The site builder's guide to Feeds":
http://drupal.org/node/622698

Hidden settings
===============

Hidden settings are variables that you can define by adding them to the $conf
array in your settings.php file.

Name:        feeds_debug
Default:     FALSE
Description: Set to TRUE for enabling debug output to
             /DRUPALTMPDIR/feeds_[sitename].log

Name:        feeds_importer_class
Default:     'FeedsImporter'
Description: The class to use for importing feeds.

Name:        feeds_source_class
Default:     'FeedsSource'
Description: The class to use for handling feed sources.

Name:        feeds_data_$importer_id
Default:     feeds_data_$importer_id
Description: The table used by FeedsDataProcessor to store feed items. Usually a
             FeedsDataProcessor builds a table name from a prefix (feeds_data_)
             and the importer's id ($importer_id). This default table name can
             be overridden by defining a variable with the same name.

Name:        feeds_process_limit
Default:     50
             The number of nodes feed node processor creates or deletes in one
             page load.

Name:        http_request_timeout
Default:     15
Description: Timeout in seconds to wait for an HTTP get request to finish.
Note:        This setting could be overridden per importer in admin UI :
             admin/structure/feeds/<your_importer>/settings/<your_fetcher> page.

Name:        feeds_never_use_curl
Default:     FALSE
Description: Flag to stop feeds from using its cURL for http requests. See
             http_request_use_curl().

Glossary
========

See "Feeds glossary":
http://drupal.org/node/622710
