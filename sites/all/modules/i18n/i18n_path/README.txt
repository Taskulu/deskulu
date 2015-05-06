
README.txt
==========
Drupal module: Path translation
==================================

This module provides some basic path translation feature for generic paths.

For paths belonging to objects that have translations, like nodes and taxonomy terms, the system can produce automatic
links for the language switcher.

For the rest of paths, this module allows to define which path is translation of which. Example:

1. We define a new 'path translation set' like
   - English: node/1
   - Spanish: taxonomy/term/3

2. Every time we are on any of these pages, the language switcher will point to the other path for each language.

This module is intended for translation of generic paths that don't have other way of being translated.

Note: path translations must be defined without aliases.

====================================================================
Jose A. Reyero, http://reyero.net