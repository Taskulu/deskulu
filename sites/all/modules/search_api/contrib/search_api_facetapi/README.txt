Search facets
-------------

This module allows you to create facetted searches for any search executed via
the Search API, no matter if executed by a search page, a view or any other
module. The only thing you'll need is a search service class that supports the
"search_api_facets" feature. Currently, the "Database search" and "Solr search"
modules supports this.

This module is built on the Facet API [1], which is needed for this module to
work.

[1] http://drupal.org/project/facetapi


Information for site builders
-----------------------------

For creating a facetted search, you first need a search. Create or find some
page displaying Search API search results, either via a search page, a view or
by any other means. Now go to the configuration page for the index on which
this search is executed.
If the index lies on a server supporting facets (and if this module is enabled),
you'll notice a "Facets" tab. Click it and it will take you to the index' facet
configuration page. You'll see a table containing all indexed fields and options
for enabling and configuring facets for them.
For a detailed explanation of the available options, please refer to the Facet
API documentation.

- Creating facets via the URL

Facets can be added to a search (for which facets are activated) by passing
appropriate GET parameters in the URL. Assuming you have an indexed field with
the machine name "field_price", you can filter on it in the following ways:

- Filter for a specific value. For finding only results that have a price of
  exactly 100, pass the following $options to url() or l():

  $options['query']['f'][] = 'field_price:100';

  Or manually append the following GET parameter to a URL:

  ?f[0]=field_price:100

- Search for values in a specified range. The following example will only return
  items that have a price greater than or equal to 100 and lower than 500.

  Code: $options['query']['f'][] = 'field_price:[100 TO 500]';
  URL:  ?f[0]=field_price%3A%5B100%20TO%20500%5D

- Search for values above a value. The next example will find results which have
  a price greater than or equal to 100. The asterisk (*) stands for "unlimited",
  meaning that there is no upper limit. Filtering for values lower than a
  certain value works equivalently.

  Code: $options['query']['f'][] = 'field_price:[100 TO *]';
  URL:  ?f[0]=field_price%3A%5B100%20TO%20%2A%5D

- Search for missing values. This example will filter out all items which have
  any value at all in the price field, and will therefore only list items on
  which this field was omitted. (This naturally only makes sense for fields
  that aren't required.)

  Code: $options['query']['f'][] = 'field_price:!';
  URL:  ?f[0]=field_price%3A%21

- Search for present values. The following example will only return items which
  have the price field set (regardless of the actual value). You can see that it
  is actually just a range filter with unlimited lower and upper bound.

  Code: $options['query']['f'][] = 'field_price:[* TO *]';
  URL:  ?f[0]=field_price%3A%5B%2A%20TO%20%2A%5D

Note: When filtering a field whose machine name contains a colon (e.g.,
"author:roles"), you'll have to additionally URL-encode the field name in these
filter values:
  Code: $options['query']['f'][] = rawurlencode('author:roles') . ':100';
  URL:  ?f[0]=author%253Aroles%3A100

- Issues

If you find any bugs or shortcomings while using this module, please file an
issue in the project's issue queue [1], using the "Facets" component.

[1] http://drupal.org/project/issues/search_api


Information for developers
--------------------------

- Features

If you are the developer of a SearchApiServiceInterface implementation and want
to support facets with your service class, too, you'll have to support the
"search_api_facets" feature. You can find details about the necessary additions
to your class in the example_servive.php file. In short, you'll just, when
executing a query, have to return facet terms and counts according to the
query's "search_api_facets" option, if present.
In order for the module to be able to tell that your server supports facets,
you will also have to change your service's supportsFeature() method to
something like the following:
  public function supportsFeature($feature) {
    return $feature == 'search_api_facets';
  }

There is also a second feature defined by this module, namely
"search_api_facets_operator_or", for supporting "OR" facets. The requirements
for this feature are also explained in the example_servive.php file.

- Query option

The facets created follow the "search_api_base_path" option on the search query.
If set, this path will be used as the base path from which facet links will be
created. This can be used to show facets on pages without searches – e.g., as a
landing page.

- Hidden variable

The module uses one hidden variable, "search_api_facets_search_ids", to keep
track of the search IDs of searches executed for a given index. It is only
updated when a facet is displayed for the respective search, so isn't really a
reliable measure for this.
In any case, if you e.g. did some test searches and now don't want them to show
up in the block configuration forever after, just clear the variable:
  variable_del("search_api_facets_search_ids")
