Search API Views integration
----------------------------

This module integrates the Search API with the popular Views module [1],
allowing users to create views with filters, arguments, sorts and fields based
on any search index.

[1] http://drupal.org/project/views

"More like this" feature
------------------------
This module defines the "More like this" feature (feature key: "search_api_mlt")
that search service classes can implement. With a server supporting this, you
can use the „More like this“ contextual filter to display a list of items
related to a given item (usually, nodes similar to the node currently viewed).

For developers:
A service class that wants to support this feature has to check for a
"search_api_mlt" option in the search() method. When present, it will be an
array containing two keys:
- id: The entity ID of the item to which related items should be searched.
- fields: An array of indexed fields to use for testing the similarity of items.
When these are present, the normal keywords should be ignored and the related
items be returned as results instead. Sorting, filtering and range restriction
should all work normally.

"Random sort" feature
---------------------
This module defines the "Random sort" feature (feature key:
"search_api_random_sort") that allows to randomly sort the results returned by a
search. With a server supporting this, you can use the "Global: Random" sort to
sort the view's results randomly. Every time the query is run a different
sorting will be provided.

For developers:
A service class that wants to support this feature has to check for a
"search_api_random" field in the search query's sorts and insert a random sort
in that position. If the query is sorted in this way, then the
"search_api_random_sort" query option can contain additional options for the
random sort, as an associative array with any of the following keys:
- seed: A numeric seed value to use for the random sort.

"Facets block" display
----------------------
Most features should be clear to users of Views. However, the module also
provides a new display type, "Facets block", that might need some explanation.
This display type is only available, if the „Search facets“ module is also
enabled.

The basic use of the block is to provide a list of links to the most popular
filter terms (i.e., the ones with the most results) for a certain category. For
example, you could provide a block listing the most popular authors, or taxonomy
terms, linking to searches for those, to provide some kind of landing page.

Please note that, due to limitations in Views, this display mode is shown for
views of all base tables, even though it only works for views based on Search
API indexes. For views of other base tables, this will just print an error
message.
The display will also always ignore the view's "Style" setting, selected fields
and sorts, etc.

To use the display, specify the base path of the search you want to link to
(this enables you to also link to searches that aren't based on Views) and the
facet field to use (any indexed field can be used here, there needn't be a facet
defined for it). You'll then have the block available in the blocks
administration and can enable and move it at leisure.
Note, however, that the facet in question has to be enabled for the search page
linked to for the filter to have an effect.

Since the block will trigger a search on pages where it is set to appear, you
can also enable additional „normal“ facet blocks for that search, via the
„Facets“ tab for the index. They will automatically also point to the same
search that you specified for the display.
If you want to use only the normal facets and not display anything at all in
the Views block, just activate the display's „Hide block“ option.

Note: If you want to display the block not only on a few pages, you should in
any case take care that it isn't displayed on the search page, since that might
confuse users.

Access features
---------------
Search views created with this module contain two query settings (located in
the "Advanced" fieldset) which let you control the access checks executed for
search results displayed in the view.

- Bypass access checks
This option allows you to deactivate access filters that would otherwise be
added to the search, if the index supports this. This is, for instance, the case
for indexes on the "Node" item type, when the "Node access" data alteration is
activated.
Use this either to slightly speed up searches where additional checks are
unnecessary (e.g., because you already filter on "Node: Published") and there is
no other node access mechanism on your site) or to show certain data that users
normally wouldn't have access to (e.g., a list of all matching node titles,
published or not).

- Additional access checks on result entities
When this option is activated, all result entities will be passed to an
additional access check, even if search-time access checks are available for
this index. The advantage is that access rules are guaranteed to be enforced –
stale data in the index, which might make other access checks incorrect, won't
influence this access check. You can also use it for item types for which no
other access mechanisms are available.
However, note that results filtered out this way will mess up paging, result
counts and possibly other things too (like facet counts), as the result row is
only hidden from display after the search has been executed. Where possible,
you should therefore only use this in combination with appropriate filter
settings ensuring that only when the index isn't up-to-date items will be
filtered out this way.
This option is only available for indexes on entity types.

Other features
--------------
- Change parse mode
You can determine how search keys entered by the user will be parsed by going to
"Advanced" > "Query settings" within your View's settings. "Direct" can be
useful, e.g., when you want to give users the full power of Solr. In other
cases, "Multiple terms" is usually what you want / what users expect.
Caution: For letting users use fulltext searches, always use the "Search:
Fulltext search" filter or contextual filter – using a normal filter on a
fulltext field won't parse the search keys, which means multiple words will only
be found when they appear as that exact phrase.

FAQ: Why „*Indexed* Node“?
--------------------------
The group name used for the search result itself (in fields, filters, etc.) is
prefixed with „Indexed“ in order to be distinguishable from fields on referenced
nodes (or other entities). The data displayed normally still comes from the
entity, not from the search index.
