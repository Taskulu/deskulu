CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Installation / Configuration
 * How to use it

INTRODUCTION
------------

This module provides two widgets for editing fields that allows users to use
Select2 jQuery plugin. Select2 is a jQuery based replacement for select boxes.
It supports searching, remote data sets, and infinite scrolling of results.
Select2 comes with AJAX/JSONP support built in, which is implemented in
Select2WidgetAjax widget. Select2 also supports multi-value select boxes.

For further informations you can visit: http://ivaynberg.github.io/select2/


INSTALLATION / CONFIGURATION
-------------------

 * Download Select2 from http://ivaynberg.github.io/select2/.
    Unzip it in the sites/all/libraries/select2 directory
    (select2.js should be located in sites/all/libraries/select2).

* Install as you would normally install a contributed drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.


HOW TO USE IT
-------------------

This module is adding two widgets:

1. Select2Widget
  - supported by the following field types:
      list,
      list_text,
      list_number,
      node_reference,
      taxonomy_term_reference,
      user_reference,
      entityreference

2. Select2WidgetAjax

  - This widget loads data via AJAX and can be used for taxonomy_term_reference
  and entityreference.

  I. Taxonomy term reference widget settings:
    1. Search matching
        - Starts with
        - Contains
      Note: the search match "Contains" can cause performance issues
        on sites with thousands of nodes.

    2. Separator

    3. Allow new terms
        a) Allow and insert new terms
        b) Allow new terms and insert them into a separate vocabulary
        c) Deny any new terms

      IMPORTANT: If option 2 is selected, you will have to save
        this settings form 2 times (the "separate vocabulary" will appear
        only after you saved the first time)

    4. Taxonomy term level
      If this is checked only terms from the last level
      of taxonomy hierarchy are shown.

    5. Number of minimum characters
      The number of minimum characters to be typed in order
      to start a search and show hints.

  II. Entity reference widget settings:
    1. View modes
      Select a view mode for the target entities. (This view mode is used
      to render entities when you search between them with select2)

    2. Search matching
        - Starts with
        - Contains

    3. Width of field (px)



IMPORTANT:
-------------------

  If termstatus module is enabled, the terms must have a status equal to 1,
  otherwise the term does not appear in select2 fields.


Version 2.x of this module was sponsored by:

* CYLEX - http://cylex-international.com
