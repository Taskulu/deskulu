<?php
/**
 * @file
 * Contains the 'Taxonomy term (i18n)' argument validator plugin.
 */

/**
 * Validate whether an argument is a localized term.
 */
class i18nviews_plugin_argument_validate_i18n_taxonomy_term extends views_plugin_argument_validate_taxonomy_term {

  function option_definition() {
    $options = parent::option_definition();
    $options['type']['default'] = 'i18n_name';

    return $options;
  }

  function convert_options(&$options) {
    if (!isset($options['vids']) && !empty($this->argument->options['validate_argument_vocabulary_i18n'])) {
      $options['vids'] = $this->argument->options['validate_argument_vocabulary_i18n'];
      $options['type'] = $this->argument->options['validate_argument_type_i18n'];
      $options['transform'] = $this->argument->options['validate_argument_transform_i18n'];
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $options = array();
    // Get the localized vocabularies.
    foreach (taxonomy_get_vocabularies() as $vocab) {
      if (i18n_taxonomy_vocabulary_mode($vocab->vid, I18N_MODE_LOCALIZE)) {
        $options[$vocab->machine_name] = check_plain($vocab->name);
      }
    }

    $form['vocabularies']['#description'] = t('Limit this validator to vocabularies that have been localized');
    $form['vocabularies']['#options'] = $options;

    $form['type']['#options'] = array(
      'i18n_tid' => t('Term ID'),
      'i18n_tids' => t('Term IDs separated by , or +'),
      'i18n_name' => t('Localised Term name or synonym'),
      'i18n_convert' => t('Localised Term name/synonym converted to Term ID'),
    );
    $form['type']['#description'] = t('Select the form of this argument; if using term name, it is generally more efficient to convert it to a term ID and use Taxonomy: Term ID rather than Taxonomy: Term Name" as an argument.');
  }

  function validate_argument($argument) {
    $vocabularies = $this->options['vocabularies'];
    $type = $this->options['type'];
    $transform = $this->options['transform'];
    switch ($type) {
      case 'i18n_tid':
      case 'i18n_tids':
      case 'i18n_name':
      case 'i18n_convert':
        // Check to see if the term is in fact localised
        $localised = i18nviews_locale_source($argument, 'taxonomy');
        if (!empty($localised)) {
          // If it is localised I set the $argument to the orginal and tell the view that the argument has been localized and to use the source
          $argument = $localised->source;
          $this->argument->argument = $localised->source;
        }

        if ($type == 'i18n_tid' || $type == 'i18n_tids') {
          return $this->{"validate_argument_$type"}($argument, $vocabularies);
        }

        $query = db_select('taxonomy_term_data', 'td');
        $query->leftJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
        $query->fields('td');
        $query->fields('tv', array('machine_name'));
        if (!empty($vocabularies)) {
          $query->condition('tv.machine_name', $vocabularies);
        }
        if ($transform) {
          $query->where("replace(td.name, ' ', '-') = :name", array(':name' => $argument));
        }
        else {
          $query->condition('td.name', $argument);
        }
        $term = $query->execute()->fetchObject();

        if ($term && (empty($vocabularies) || !empty($vocabularies[$term->machine_name]))) {
          if ($type == 'i18n_convert') {
            $this->argument->argument = $term->tid;
          }
          // ToDo: check vocabulary translation mode (if localization needed)
          $this->argument->validated_title = check_plain(i18n_taxonomy_term_name($term));
          return TRUE;
        }
        return FALSE;
    }
  }

  /**
   * Validate taxonomy terms - case i18n_tid
   */
  function validate_argument_i18n_tid($argument, $vocabularies) {
    if (!is_numeric($argument)) {
      return FALSE;
    }

    $query = db_select('taxonomy_term_data', 'td');
    $query->leftJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
    $query->fields('td');
    $query->fields('tv', array('machine_name'));
    $query->condition('td.tid', $argument);
    $query->addTag('term_access');
    $term = $query->execute()->fetchObject();
    if (!$term) {
      return FALSE;
    }

    $this->argument->validated_title = check_plain(i18n_taxonomy_term_name($term));
    return empty($vocabularies) || !empty($vocabularies[$term->machine_name]);
  }

  /**
   * Validate taxonomy term - case i18n_tids
   */
   function validate_argument_i18n_tids($argument, $vocabularies) {
     // An empty argument is not a term so doesn't pass.
     if (empty($argument)) {
       return FALSE;
     }

     $tids = new stdClass();
     $tids->value = $argument;
     $tids = views_break_phrase($argument, $tids);
     if ($tids->value == array(-1)) {
       return FALSE;
     }

     $test = drupal_map_assoc($tids->value);
     $titles = array();

     // check, if some tids already verified
     static $validated_cache = array();
     foreach ($test as $tid) {
       if (isset($validated_cache[$tid])) {
         if ($validated_cache[$tid] === FALSE) {
           return FALSE;
         }
         else {
           $titles[] = $validated_cache[$tid];
           unset($test[$tid]);
         }
       }
     }

     // if unverified tids left - verify them and cache results
     if (count($test)) {
       $query = db_select('taxonomy_term_data', 'td');
       $query->leftJoin('taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
       $query->fields('td');
       $query->fields('tv', array('machine_name'));
       $query->condition('td.tid', $test);

       $result = $query->execute();

       foreach ($result as $term) {
         if ($vocabularies && empty($vocabularies[$term->machine_name])) {
           $validated_cache[$term->tid] = FALSE;
           return FALSE;
         }

         $titles[] = $validated_cache[$term->tid] = check_plain(i18n_taxonomy_term_name($term));
         unset($test[$term->tid]);
       }
     }

     // Remove duplicate titles
     $titles = array_unique($titles);

     $this->argument->validated_title = implode($tids->operator == 'or' ? ' + ' : ', ', $titles);
     // If this is not empty, we did not find a tid.
     return empty($test);
   }
}
