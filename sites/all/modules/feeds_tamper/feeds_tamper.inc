<?php

/**
 * @file
 * Version agnostic parts of feeds_tamper.module.
 */


/**
 * @defgroup feeds_tamper_api Feeds Tamper API
 * @{
 * API functions for dealing with plugins and plugin instances.
 */

/**
 * Create a new plugin instance with default values filled in.
 *
 * @return stdClass
 *   A new plugin instance object.
 */
function feeds_tamper_new_instance() {
  ctools_include('export');
  return ctools_export_crud_new('feeds_tamper');
}

/**
 * Load all plugin instances.
 *
 * @param bool $disabled
 *   (optional) If TRUE, load disabled plugin instances. Defaults to FALSE.
 * @param bool $reset
 *   (optional) If TRUE, the static cache of all objects will be flushed prior
 *   to loading. Defaults to FALSE.
 *
 * @return array
 *   An associative array of plugin instances keyed to their id.
 */
function feeds_tamper_load_all_instances($disabled = FALSE, $reset = FALSE) {
  $instances = array();
  ctools_include('export');
  $configs = ctools_export_crud_load_all('feeds_tamper', $reset);
  foreach ($configs as $instance_id => $config) {
    if (!empty($config->id) && ($disabled || empty($config->disabled))) {
      $instances[$instance_id] = $config;
    }
  }
  return $instances;
}

/**
 * Load a plugin instance by id.
 *
 * @param string $id
 *   The id of the plugin instance.
 *
 * @return stdClass
 *   A plugin instance object.
 */
function feeds_tamper_load_instance($id) {
  $id = drupal_substr($id, 0, 127);
  ctools_include('export');
  return ctools_export_crud_load('feeds_tamper', $id);
}

/**
 * Save a plugin instance.
 *
 * @param stdClass $instance
 *   A plugin instance object.
 *
 * @return mixed
 *   If the plugin save failed, returns FALSE. If it succeeded, returns
 *   SAVED_NEW or SAVED_UPDATED, depending on the operation performed.
 */
function feeds_tamper_save_instance($instance) {
  $instance->id = drupal_substr($instance->id, 0, 127);

  ctools_include('export');
  // It's a new instance, give it the heaviest weight.
  if (!isset($instance->weight)) {
    $conditions = array('importer' => $instance->importer, 'source' => $instance->source);
    $all = ctools_export_load_object('feeds_tamper', 'conditions', $conditions);
    $weight = 0;
    foreach ($all as $i) {
      if ($i->weight >= $weight) {
        $weight = $i->weight + 1;
      }
    }
    $instance->weight = $weight;
  }
  $disabled = variable_get('default_feeds_tamper', array());
  $disabled[$instance->id] = !empty($instance->disabled) ? TRUE : FALSE;
  variable_set('default_feeds_tamper', $disabled);
  return ctools_export_crud_save('feeds_tamper', $instance);
}

/**
 * Removes plugin instances whos source was removed.
 *
 * @param FeedsImporter $importer
 *   The importer to rectify against.
 */
function feeds_tamper_rectify_instances(FeedsImporter $importer) {
  ctools_include('export');
  $instances = ctools_export_load_object('feeds_tamper', 'conditions', array('importer' => $importer->id));
  $sources = feeds_tamper_get_unique_source_list($importer, FALSE);

  foreach ($instances as $instance) {
    if (in_array($instance->source, $sources)) {
      continue;
    }

    if ($instance->export_type == EXPORT_IN_DATABASE) {
      feeds_tamper_delete_instance($instance);
    }
    else {
      $instance->disabled = TRUE;
      feeds_tamper_save_instance($instance);
    }
  }
}

/**
 * Delete a single plugin instance.
 *
 * @param string|object $instance
 *   A plugin instance object or the id of a plugin instance.
 */
function feeds_tamper_delete_instance($instance) {
  // Allow for string id or plugin object.
  if (is_scalar($instance)) {
    $instance = feeds_tamper_load_instance($instance);
  }
  ctools_include('export');
  ctools_export_crud_delete('feeds_tamper', $instance);
}

/**
 * Load plugin instances by importer id.
 *
 * @param string|FeedsImporter $importer
 *   The importer id, or object to reference.
 * @param bool $disabled
 *   (optional) If TRUE load disabled plugin instances. Defaults to FALSE.
 *
 * @return array
 *   An associative array of plugin instances, keyed by source.
 */
function feeds_tamper_load_by_importer($importer, $disabled = FALSE) {
  if (is_scalar($importer)) {
    $importer = feeds_importer($importer);
  }

  ctools_include('export');
  $t = ctools_export_load_object('feeds_tamper', 'conditions', array('importer' => $importer->id));

  $sources = feeds_tamper_get_unique_source_list($importer, FALSE);

  $plugins = feeds_tamper_get_plugins();

  $instances = array();
  foreach ($t as $i) {
    // Add an empty settings array, if the plugin instance doesn't provide any settings itself.
    if (empty($i->settings)) {
      $i->settings = array();
    }
    // Filter disabled and only pull plugins that actually have a corresponding
    // source since we don't delete them with a mapping.
    if (($disabled || empty($i->disabled)) && in_array($i->source, $sources) && function_exists($plugins[$i->plugin_id]['callback'])) {
      $instances[$i->source][] = $i;
    }
  }
  foreach ($instances as &$instance_list) {
    usort($instance_list, '_feeds_tamper_cmp');
  }

  // Sort the plugins by the order they appear on the mapping page.
  $return = array();
  foreach ($sources as $source) {
    if (isset($instances[$source])) {
      $return[$source] = $instances[$source];
    }
  }

  return $return;
}

/**
 * Returns a unique list of sources in order.
 *
 * @param FeedsImporter $importer
 *   The importer.
 *
 * @return array
 *   A list of sources.
 */
function feeds_tamper_get_unique_source_list($importer, $lower = TRUE) {
  $sources = array();
  $is_csv = $lower && ($importer->parser instanceof FeedsCSVParser);

  foreach ($importer->processor->getMappings() as $mapping) {
    if ($is_csv) {
      $sources[] = drupal_strtolower($mapping['source']);
    }
    else {
      $sources[] = $mapping['source'];
    }
  }

  return array_unique($sources);
}

/**
 * Comparison callback that sorts by weight, then alphabetically by id.
 */
function _feeds_tamper_cmp($a, $b) {
  // If weights are equal compare id's.
  if ($a->weight == $b->weight) {
    $tmp = array($a->id, $b->id);
    sort($tmp);
    if ($tmp[0] == $a->id) {
      return -1;
    }
    return 1;
  }
  return ($a->weight < $b->weight) ? -1 : 1;
}

/**
 * Get all available plugins.
 *
 * @return array
 *   An associative array where the keys are the plugin keys and the values are
 *   the plugin info arrays as defined in a plugin include file.
 */
function feeds_tamper_get_plugins() {
  ctools_include('plugins');
  return ctools_get_plugins('feeds_tamper', 'plugins');
}

/**
 * Get a single plugin.
 *
 * @param string $id
 *   The id of a plugin.
 *
 * @return stdClass
 *   A plugin object.
 */
function feeds_tamper_get_plugin($id) {
  ctools_include('plugins');
  return ctools_get_plugins('feeds_tamper', 'plugins', $id);
}

/**
 * Return a machine name safe version of a string.
 *
 * @param string $string
 *   String to get machine nameized.
 *
 * @return string
 *   A lowercase string with all values not in [a-zA-Z0-9] replaced with an
 *   underscore and shortened to 128 characters.
 */
function feeds_tamper_make_machine($string) {
  return drupal_substr(preg_replace('/[^a-z0-9-]/u', '_', drupal_strtolower($string)), 0, 127);
}

/**
 * @} End of "feeds_tamper_api".
 */

/**
 * Menu access callback.
 *
 * @param string|FeedsImporter $importer
 *   The importer or importer id being tampered with.
 * @param string|stdClass $instance
 *   (optional) If set, the importer attached to $instance will be tried first.
 *   Defaults to NULL.
 *
 * @return bool
 *   TRUE if the user has acces, FALSE if not.
 */
function feeds_tamper_access($importer, $instance = NULL) {
  if (isset($instance)) {
    if (is_object($instance)) {
      $importer = $instance->importer;
    }
    else {
      $importer = feeds_tamper_load_instance($instance)->importer;
    }
  }
  elseif (is_object($importer)) {
    $importer = $importer->id;
  }

  // Verify actual input if above failed.
  if ($importer) {
    // Check for permissions, otherwise return FALSE.
    if (user_access('administer feeds_tamper') || user_access('tamper ' . $importer)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Implements hook_form_feeds_ui_mapping_form_alter().
 *
 * This is an interesting bit of work. Each source name has to be unique,
 * but we have no idea how many to create with
 * feeds_tamper_feeds_parser_sources_alter() because we don't know how many
 * targets there are going to be.
 *
 * The solution is to keep track in the form how many have been added.
 */
function feeds_tamper_form_feeds_ui_mapping_form_alter(array &$form, array &$form_state) {
  $form['#submit'][] = 'feeds_tamper_form_feeds_ui_mapping_form_submit';

  // Don't alter the form for parsers that use manual input.
  if ($form['source']['#type'] == 'textfield') {
    return;
  }
  $max_source = 0;
  $max_target = 0;
  foreach ($form['#mappings'] as $mapping) {
    if (strpos($mapping['source'], 'Blank source ') === 0) {
      list(, , $source_value) = explode(' ', $mapping['source']);
      if ($source_value > $max_source) {
        $max_source = $source_value;
      }
    }
    if (strpos($mapping['target'], 'Temporary target ') === 0) {
      list(, , $target_value) = explode(' ', $mapping['target']);
      if ($target_value > $max_target) {
        $max_target = $target_value;
      }
    }
  }
  if ($max_source) {
    unset($form['source']['#options']['Blank source 1']);
    $form['source']['#options']['Blank source ' . ++$max_source] = 'Blank source';
  }
  else {
    $form['source']['#options']['Blank source 1'] = 'Blank source';
  }
  if ($max_target) {
    unset($form['target']['#options']['Temporary target 1']);
    $form['target']['#options']['Temporary target ' . ++$max_target] = 'Temporary target';
  }
  else {
    $form['target']['#options']['Temporary target 1'] = 'Temporary target';
  }
}

/**
 * Submit callback for feeds_ui_mapping_form().
 */
function feeds_tamper_form_feeds_ui_mapping_form_submit(array &$form, array &$form_state) {
  feeds_tamper_rectify_instances(feeds_importer($form['#importer']));
}

/**
 * Implements hook_ctools_plugin_api().
 */
function feeds_tamper_ctools_plugin_api($owner, $api) {
  if ($owner == 'feeds_tamper' && $api == 'plugins') {
    return array('version' => 2);
  }
}

/**
 * Implements hook_ctools_plugin_directory().
 */
function feeds_tamper_ctools_plugin_directory($module, $plugin) {
  if ($module == 'feeds_tamper') {
    return 'plugins';
  }
}
