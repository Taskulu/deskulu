
README.txt
==========
Drupal module: Translation set API
==================================

This is a generic API to handle translation sets. It is being used for now
for path translation and taxonomy term translation inside i18n package.

Translation sets can hold a collection of entities or other objects. A translation set is itself
an Entity thus leveraging all the power of the Entity API.

It also provides some basic storage for translation sets and a generator of new translation set id.
However, each module is responsible for storing which objects belong to which translation set for which
it needs to verride some methods of the base i18n_translation_set class.

- load_translations()
- save_translations()
- clean_translations()
- delete_translations()

Once these are implemented, to get the objects belonging to a translation set, indexed by language code,
you can invoke this method on a translation set object:

- get_translations()

To define a new type of translation set, a module must implement hook_i18n_translation_set_info() 
as in this example:

/**
 * Implements hook_i18n_translation_set_info().
 */
function i18n_path_i18n_translation_set_info() {
  return array(
    'path' => array(
      'title' => t('Paths'),
      'class' => 'i18n_path_translation_set',
    )
  );
}

See examples of overriding and extending this API:
- i18n_path/i18n_path.inc
- i18n_taxonomy/i18n_taxonomy.inc

====================================================================
Jose A. Reyero, http://reyero.net