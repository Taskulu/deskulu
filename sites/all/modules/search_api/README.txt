Search API
----------

This module provides a framework for easily creating searches on any entity
known to Drupal, using any kind of search engine. For site administrators, it is
a great alternative to other search solutions, since it already incorporates
facetting support and the ability to use the Views module for displaying search
results, filters, etc. Also, with the Apache Solr integration [1], a
high-performance search engine is available for use with the Search API.

If you need help with the module, please post to the project's issue queue [2].

[1] http://drupal.org/project/search_api_solr
[2] http://drupal.org/project/issues/search_api


Content:
 - Glossary
 - Information for users
 - Information for developers
 - Included components


Glossary
--------

Terms as used in this module.

- Service class:
  A type of search engine, e.g. using the database, Apache Solr,
  Sphinx or any other professional or simple indexing mechanism. Takes care of
  the details of all operations, especially indexing or searching content.
- Server:
  One specific place for indexing data, using a set service class. Can
  e.g. be some tables in a database, a connection to a Solr server or other
  external services, etc.
- Index:
  A configuration object for indexing data of a specific type. What and how data
  is indexed is determined by its settings. Also keeps track of which items
  still need to be indexed (or re-indexed, if they were updated). Needs to lie
  on a server in order to be really used (although configuration is independent
  of a server).
- Item type:
  A type of data which can be indexed (i.e., for which indexes can be created).
  Most entity types (like Content, User, Taxonomy term, etc.) are available, but
  possibly also other types provided by contrib modules.
- Entity:
  One object of data, usually stored in the database. Might for example
  be a node, a user or a file.
- Field:
  A defined property of an entity, like a node's title or a user's mail address.
  All fields have defined datatypes. However, for indexing purposes the user
  might choose to index a property under a different data type than defined.
- Data type:
  Determines how a field is indexed. While "Fulltext" fields can be completely
  searched for keywords, other fields can only be used for filtering. They will
  also be converted to fit their respective value ranges.
  How types other than "Fulltext" are handled depends on the service class used.
  Its documentation should state how the type-selection affect the indexed
  content. However, service classes will always be able to handle all data
  types, it is just possible that the type doesn't affect the indexing at all
  (apart from "Fulltext vs. the rest").
- Boost:
  Number determining how important a certain field is, when searching for
  fulltext keywords. The higher the value is, the more important is the field.
  E.g., when the node title has a boost of 5.0 and the node body a boost of 1.0,
  keywords found in the title will increase the score as much as five keywords
  found in the body. Of course, this has only an effect when the score is used
  (for sorting or other purposes). It has no effect on other parts of the search
  result.
- Data alteration:
  A component that is used when indexing data. It can add additional fields to
  the indexed entity or prevent certain entities from being indexed. Fields
  added by callbacks have to be enabled on the "Fields" page to be of any use,
  but this is done by default.
- Processor:
  An object that is used for preprocessing indexed data as well as search
  queries, and for postprocessing search results. Usually only work on fulltext
  fields to control how content is indexed and searched. E.g., processors can be
  used to make searches case-insensitive, to filter markup out of indexed
  content, etc.


Information for users
---------------------

IMPORTANT: Access checks
  In general, the Search API doesn't contain any access checks for search
  results. It is your responsibility to ensure that only accessible search
  results are displayed – either by only indexing such items, or by filtering
  appropriately at search time.
  For search on general site content (item type "Node"), this is already
  supported by the Search API. To enable this, go to the index's "Filters" tab
  and activate the "Node access" data alteration. This will add the necessary
  field, "Node access information", to the index (which you have to leave as
  "indexed"). If both this field and "Published" are set to be indexed, access
  checks will automatically be executed at search time, showing only those
  results that a user can view. Some search types (e.g., search views) also
  provide the option to disable these access checks for individual searches.
  Please note, however, that these access checks use the indexed data, while
  usually the current data is displayed to users. Therefore, users might still
  see inappropriate content as long as items aren't indexed in their latest
  state. If you can't allow this for your site, please use the index's "Index
  immediately" feature (explained below) or possibly custom solutions for
  specific search types, if available.

As stated above, you will need at least one other module to use the Search API,
namely one that defines a service class (e.g., search_api_db ("Database search")
which can be found at [3]).

[3] http://drupal.org/project/search_api_db

- Creating a server
  (Configuration > Search API > Add server)

The most basic thing you have to create is a search server for indexing content.
Go to Configuration > Search API in the administration pages and select
"Add server". Name and description are usually only shown to administrators and
can be used to differentiate between several servers, or to explain a server's
use to other administrators (for larger sites). Disabling a server makes it
unusable for indexing and searching and can e.g. be used if the underlying
search engine is temporarily unavailable.
The "service class" is the most important option here, since it lets you select
which backend the search server will use. This cannot be changed after the
server is created.
Depending on the selected service class, further, service-specific settings will
be available. For details on those settings, consult the respective service's
documentation.

- Creating an index
  (Configuration > Search API > Add index)

For adding a search index, choose "Add index" on the Search API administration
page. Name, description and "enabled" status serve the exact same purpose as
for servers.
The most important option in this form is the indexed entity type. Every index
contains data on only a single type of entities, e.g. nodes, users or taxonomy
terms. This is therefore the only option that cannot be changed afterwards.
The server on which the index lies determines where the data will actually be
indexed. It doesn't affect any other settings of the index and can later be
changed with the only drawback being that the index' content will have to be
indexed again. You can also select a server that is at the moment disabled, or
choose to let the index lie on no server at all, for the time being. Note,
however, that you can only create enabled indexes on an enabled server. Also,
disabling a server will disable all indexes that lie on it.
The "Index items immediately" option specifies that you want items to be
directly re-indexed after being changed, instead of waiting for the next cron
run. Use this if it is important that users see no stale data in searches, and
only when your setup enables relatively fast indexing.
Lastly, the "Cron batch size" option allows you to set whether items will be
indexed when cron runs (as long as the index is enabled), and how many items
will be indexed in a single batch. The best value for this setting depends on
how time-consuming indexing is for your setup, which in turn depends mostly on
the server used and the enabled data alterations. You should set it to a number
of items which can easily be indexed in 10 seconds' time. Items can also be
indexed manually, or directly when they are changed, so even if this is set to
0, the index can still be used.

- Indexed fields
  (Configuration > Search API > [Index name] > Fields)

Here you can select which of the entities' fields will be indexed, and how.
Fields added by (enabled) data alterations will be available here, too.
Without selecting fields to index, the index will be useless and also won't be
available for searches. Select the "Fulltext" data type for fields which you
want search for keywords, and other data types when you want to use the field
for filtering (e.g., as facets). The "Item language" field will always be
indexed as it contains important information for processors and hooks.
You can also add fields of related entities here, via the "Add related fields"
form at the bottom of the page. For instance, you might want to index the
author's username to the indexed data of a node, and you need to add the "Body"
entity to the node when you want to index the actual text it contains.

- Indexing workflow
  (Configuration > Search API > [Index name] > Filters)

This page lets you customize how the created index works, and what metadata will
be available, by selecting data alterations and processors (see the glossary for
further explanations).
Data alterations usually only add one or more fields to the entity and their
order is mostly irrelevant.
The order of processors, however, often is important. Read the processors'
descriptions or consult their documentation for determining how to use them most
effectively.

- Index status
  (Configuration > Search API > [Index name] > Status)

On this page you can view how much of the entities are already indexed and also
control indexing. With the "Index now" button (displayed only when there are
still unindexed items) you can directly index a certain number of "dirty" items
(i.e., items not yet indexed in their current state). Setting "-1" as the number
will index all of those items, similar to the cron batch size setting.
When you change settings that could affect indexing, and the index is not
automatically marked for re-indexing, you can do this manually with the
"Re-index content" button. All items in the index will be marked as dirty and be
re-indexed when subsequently indexing items (either manually or via cron runs).
Until all content is re-indexed, the old data will still show up in searches.
This is different with the "Clear index" button. All items will be marked as
dirty and additionally all data will be removed from the index. Therefore,
searches won't show any results until items are re-indexed, after clearing an
index. Use this only if completely wrong data has been indexed. It is also done
automatically when the index scheme or server settings change too drastically to
keep on using the old data.

- Hidden settings

search_api_index_worker_callback_runtime:
  By changing this variable, you can determine the time (in seconds) the Search
  API will spend indexing (for all indexes combined) in each cron run. The
  default is 15 seconds.


Information for developers
--------------------------

 | NOTE:
 | For modules providing new entities: In order for your entities to become
 | searchable with the Search API, your module will need to implement
 | hook_entity_property_info() in addition to the normal hook_entity_info().
 | hook_entity_property_info() is documented in the entity module.
 | For making certain non-entities searchable, see "Item type" below.
 | For custom field types to be available for indexing, provide a
 | "property_type" key in hook_field_info(), and optionally a callback at the
 | "property_callbacks" key.
 | Both processes are explained in [4].
 |
 | [4] http://drupal.org/node/1021466

Apart from improving the module itself, developers can extend search
capabilities provided by the Search API by providing implementations for one (or
several) of the following classes. Detailed documentation on the methods that
need to be implemented are always available as doc comments in the respective
interface definition (all found in their respective files in the includes/
directory). The details for hooks can be looked up in the search_api.api.php
file. Note that all hooks provided by the Search API use the "search_api" hook
group. Therefore, implementations of the hook can be moved into a
MODULE.search_api.inc file in your module's directory.
For all interfaces there are handy base classes which can (but don't need to) be
used to ease custom implementations, since they provide sensible generic
implementations for many methods. They, too, should be documented well enough
with doc comments for a developer to find the right methods to override or
implement.

- Service class
  Interface: SearchApiServiceInterface
  Base class: SearchApiAbstractService
  Hook: hook_search_api_service_info()

The service classes are the heart of the API, since they allow data to be
indexed on different search servers. Since these are quite some work to get
right, you should probably make sure a service class for a specific search
engine doesn't exist already before programming it yourself.
When your module supplies a service class, please make sure to provide
documentation (at least a README.txt) that clearly states the datatypes it
supports (and in what manner), how a direct query (a query where the keys are
a single string, instead of an array) is parsed and possible limitations of the
service class.
The central methods here are the indexItems() and the search() methods, which
always have to be overridden manually. The configurationForm() method allows
services to provide custom settings for the user.
See the SearchApiDbService class provided by [5] for an example implementation.

[5] http://drupal.org/project/search_api_db

- Query class
  Interface: SearchApiQueryInterface
  Base class: SearchApiQuery

You can also override the query class' behaviour for your service class. You
can, for example, change key parsing behaviour, add additional parse modes
specific to your service, or override methods so the information is stored more
suitable for your service.
For the query class to become available (other than through manual creation),
you need a custom service class where you override the query() method to return
an instance of your query class.

- Item type
  Interface: SearchApiDataSourceControllerInterface
  Base class: SearchApiAbstractDataSourceController
  Hook: hook_search_api_item_type_info()

If you want to index some data which is not defined as an entity, you can
specify it as a new item type here. For defining a new item type, you have to
create a data source controller for the type and track new, changed and deleted
items of the type by calling the search_api_track_item_*() functions.
An instance of the data source controller class will then be used by indexes
when handling items of your newly-defined type.

If you want to make external data that is indexed on some search server
available to the Search API, there is a handy base class for your data source
controller (SearchApiExternalDataSourceController in
includes/datasource_external.inc) which you can extend. For a minimal use case,
you will then only have to define the available fields that can be retrieved by
the server.

- Data type
  Hook: hook_search_api_data_type_info()

You can specify new data types for indexing fields. These new types can then be
selected on indexes' „Fields“ tabs. You just have to implement the hook,
returning some information on your data type, and specify in your module's
documentation the format of your data type and how it should be used.

For a custom data type to have an effect, in most cases the server's service
class has to support that data type. A service class can advertize its support
of a data type by declaring support for the "search_api_data_type_TYPE" feature
in its supportsFeature() method. If this support isn't declared, a fallback data
type is automatically used instead of the custom one.

If a field is indexed with a custom data type, its entry in the index's options
array will have the selected type in "real_type", while "type" contains the
fallback type (which is always one of the default data types, as returned by
search_api_default_field_types().

- Data-alter callbacks
  Interface: SearchApiAlterCallbackInterface
  Base class: SearchApiAbstractAlterCallback
  Hook: hook_search_api_alter_callback_info()

Data alter callbacks can be used to change the field data of indexed items, or
to prevent certain items from being indexed. They are only used when indexing,
or when selecting the fields to index. For adding additional information to
search results, you have to use a processor.
Data-alter callbacks are called "data alterations" in the UI.

- Processors
  Interface: SearchApiProcessorInterface
  Base class: SearchApiAbstractProcessor
  Hook: hook_search_api_processor_info()

Processors are used for altering the data when indexing or searching. The exact
specifications are available in the interface's doc comments. Just note that the
processor description should clearly state assumptions or restrictions on input
types (e.g. only tokenized text), item language, etc. and explain concisely what
effect it will have on searches.
See the processors in includes/processor.inc for examples.


Included components
-------------------

- Data alterations

  * URL field
    Provides a field with the URL for displaying the entity.
  * Aggregated fields
    Offers the ability to add additional fields to the entity, containing the
    data from one or more other fields. Use this, e.g., to have a single field
    containing all data that should be searchable, or to make the text from a
    string field, like a taxonomy term, also fulltext-searchable.
    The type of aggregation can be selected from a set of values: you can, e.g.,
    collect the text data of all contained fields, or add them up, count their
    values, etc.
  * Bundle filter
    Enables the admin to prevent entities from being indexed based on their
    bundle (content type for nodes, vocabulary for taxonomy terms, etc.).
  * Complete entity view
    Adds a field containing the whole HTML content of the entity as it is viewed
    on the site. The view mode used can be selected.
    Note, however, that this might not work for entities of all types. All core
    entities except files are supported, though.
  * Index hierarchy
    Allows to index a hierarchical field along with all its parents. Most
    importantly, this can be used to index taxonomy term references along with
    all parent terms. This way, when an item, e.g., has the term "New York", it
    will also be matched when filtering for "USA" or "North America".

- Processors

  * Ignore case
    Makes all fulltext searches (and, optionally, also filters on string values)
    case-insensitive. Some servers might do this automatically, for others this
    should probably always be activated.
  * HTML filter
    Strips HTML tags from fulltext fields and decodes HTML entities. If you are
    indexing HTML content (like node bodies) and the search server doesn't
    handle HTML on its own, this should be activated to avoid indexing HTML
    tags, as well as to give e.g. terms appearing in a heading a higher boost.
  * Tokenizer
    This processor allows you to specify how indexed fulltext content is split
    into seperate tokens – which characters are ignored and which treated as
    white-space that seperates words.
  * Stopwords
    Enables the admin to specify a stopwords file, the words contained in which
    will be filtered out of the text data indexed. This can be used to exclude
    too common words from indexing, for servers not supporting this natively.

- Additional modules

  * Search views
    This integrates the Search API with the Views module [6], enabling the user
    to create views which display search results from any Search API index.
  * Search facets
    For service classes supporting this feature (e.g. Solr search), this module
    automatically provides configurable facet blocks on pages that execute
    a search query.

[6] http://drupal.org/project/views
