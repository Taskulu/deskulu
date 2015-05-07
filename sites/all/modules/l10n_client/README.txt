
CONTENTS
--------

 * About
 * Installation
 * On-page translation
 * Sharing translations
 * Re-importing translation packages
 * Contributors & sponsors

ABOUT
-----

The main goal of the Localization client module (l10n_client) is to provide you
with an easy way to translate your website's interface. The module includes an
AJAX on-page editor that allows you to translate right on the actual web page
that you are viewing. The Localization client module also offers an overview
of all interface strings of your website. This module only lets you translate
the website's interface, for information on translating your site's content
please check the Drupal handbook page about the Translation module
(http://drupal.org/handbook/modules/translation).

The module can instantly share your translations by sending them to
localization servers such as localize.drupal.org. See also the Localization
server project  (http://drupal.org/project/l10n_server).

Finally the module includes a translation package re-import tool, which
simplifies importing new and changed translations, especially when upgrading
or developing modules. This functionality is similar to the Drupal 5.x
Autolocale module (http://drupal.org/project/autolocale).

The translation sharing and re-import tools are only available in the 6.x
version of the module because these functionalities are based on features only
available in Drupal 6.x. Therefore it is impossible to backport these
functionalities to the 5.x version without modifying Drupal itself.

 * Project page: http://drupal.org/project/l10n_client
 * Support queue: http://drupal.org/project/issues/l10n_client

INSTALLATION
------------

 1. Enable the Localization client module at Administer > Modules
 2. Enable two or more languages at Administer > Configuration > Regional and
    Language > Languages. For help with building a multilingual website please
    check the Drupal handbook page about the Locale module at
    http://drupal.org/handbook/modules/locale
 3. Join a language team on localize.drupal.org (required to be able to submit
    translation updates)
 4. Assign the appropriate permissions to the user roles under the section
    "Localization client module" at Administer > People > Permissions

ON-PAGE TRANSLATION
-------------------

Users with the permission "Use on-page translation" can translate interface
strings right on the page that they are viewing.

 1. Switch the website's language to one that is not English.
 2. Browse to the webpage that contains the interface string you want to translate
 3. Click the "TRANSLATE TEXT" button in the right bottom corner of the webpage
    The on-page translation pane appears, showing all strings available on the
    current webpage in the left column. Already translated strings are marked
    green, yet untranslated strings are shown in white. You can filter the list
    using the input field in the left bottom corner of the page.
 4. Select a string in the left column to see the source text and the
    translation (if available). You can add your own translation or edit the
    existing translation in the column at the right. Your changes will be saved
    to your local database. The translations can be shared (see below).

Open the list of all the website's interface strings by selecting Translate
Strings from the menu (http://www.example.com/locale). Click "TRANSLATE TEXT"
to start translating.

Note: If you are using the Overlay module, Localization client won't be able to
translate string in the overlay, we recommend that you disable the module or if
need be, access the administration pages directly (remove the '#overlay=' part
in url)

SHARING TRANSLATIONS
--------------------

Localization client can instantly share your translations by sending them to a
localization server. For this a user needs the "Submit translations to
localization server" permission. To be able to share translations a user needs
an API key from the localization server.

 1. Enable translation sharing at Administer > Configuration > Regional and
    Language > Languages > Sharing
 2. Enter a localization server, e.g. "http://localize.drupal.org"
 3. Join a language team on localize.drupal.org (required to be able to submit
    translation updates)
 3. Enter your localization server API key at My account > Edit
    (The form field has a link to obtain the key from the set localization server)
 4. Start translating interface strings.

RE-IMPORTING TRANSLATION PACKAGES
-----------------------------------------------------------------

To re-import translation all files should be already uncompressed to the
Drupal directories.

Choose the languages for which you want to re-import translations at
Administer > Configuration > Regional and language > Translate interface >
Import > Reimport packages.

See also http://drupal.org/project/l10n_update.

CONTRIBUTORS & SPONSORS
--------------------------------------------------------------------------------

 * Gábor Hojtsy http://drupal.org/user/4166 (original author)
 * Young Hahn / Development Seed - http://developmentseed.org/ (friendly user interface)

Initial development was sponsored by Google Summer of Code 2007,
user interface sponsored by Development Seed / Young Hahn.
