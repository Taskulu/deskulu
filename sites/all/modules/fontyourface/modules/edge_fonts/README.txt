Edge Fonts (edge_fonts) module

SUMMARY

This module lets you use Adobe® Edge Web Fonts (http://www.edgefonts.com/).

PREREQUISITES

Edge Fonts depends on @font-your-face (http://drupal.org/project/fontyourface).

CONFIGURATION

-- SCRIPT URLS

Adobe recommends fonts to be requested using schema-less URLs (e.g.,
//use.edgefonts.net/ubuntu.js); if that does not work for you, you can choose
to use HTTP/HTTPS explicitly.
(See http://www.edgefonts.com/#protocol)

-- SUBSETS TO USE

There are two options: 'default', which, as per Adobe, "contains the Latin-1
character set plus useful typographic marks", and 'all', which "contains every
glyph that’s available in the original font". The former results in smaller
files and hence is likely to have slightly better performace, while the latter
will contain any extended characters if available.
(See http://www.edgefonts.com/#subsets)

KNOWN ISSUES

Edge Fonts do not work in Midori (http://twotoasts.de/index.php/midori/)
browser. This seems to be an issue somewhere between Midori and Adobe, not with
this module or @font-your-face.

MISCELLANEOUS INFORMATION

-- AVAILABLE FONTS

As of November 2012, there were 865 fonts of 502 families available (see
http://www.edgefonts.com/#list-of-available-fonts). There is yet no API to
provide information on available fonts, so a list is hard-coded into the
module. If you notice that the list on Adobe's site has changed, file an issue,
and we will update the module accordingly.

-- FONT URLS

Edge Fonts does not have an individual page or URL for each font; therefore,
font URLs point to the available font list, and '?font=' is just a pseudo-
parameter that allows URLs to be unique.
