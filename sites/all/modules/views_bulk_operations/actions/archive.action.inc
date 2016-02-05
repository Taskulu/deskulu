<?php

/**
 * @file
 * Provides an action for creating a zip archive of selected files.
 * An entry in the {file_managed} table is created for the newly created archive,
 * and it is marked as permanent or temporary based on the operation settings.
 */

function views_bulk_operations_archive_action_info() {
  $actions = array();
  if (function_exists('zip_open')) {
    $actions['views_bulk_operations_archive_action'] = array(
      'type' => 'file',
      'label' => t('Create an archive of selected files'),
      // This action only works when invoked through VBO. That's why it's
      // declared as non-configurable to prevent it from being shown in the
      // "Create an advanced action" dropdown on admin/config/system/actions.
      'configurable' => FALSE,
      'vbo_configurable' => TRUE,
      'behavior' => array('views_property'),
      'triggers' => array('any'),
    );
  }
  return $actions;
}

/**
 * Since Drupal's Archiver doesn't abstract properly the archivers it implements
 * (Archive_Tar and ZipArchive), it can't be used here.
 */
function views_bulk_operations_archive_action($file, $context) {
  global $user;
  static $archive_contents = array();

  // Adding a non-existent file to the archive crashes ZipArchive on close().
  if (file_exists($file->uri)) {
    $destination = $context['destination'];
    $zip = new ZipArchive();
    // If the archive already exists, open it. If not, create it.
    if (file_exists($destination)) {
      $opened = $zip->open(drupal_realpath($destination));
    }
    else {
      $opened = $zip->open(drupal_realpath($destination), ZIPARCHIVE::CREATE | ZIPARCHIVE::OVERWRITE);
    }

    if ($opened) {
      // Create a list of all files in the archive. Used for duplicate checking.
      if (empty($archive_contents)) {
        for ($i = 0; $i < $zip->numFiles; $i++) {
          $archive_contents[] = $zip->getNameIndex($i);
        }
      }
      // Make sure that the target filename is unique.
      $filename = _views_bulk_operations_archive_action_create_filename(basename($file->uri), $archive_contents);
      // Note that the actual addition happens on close(), hence the need
      // to open / close the archive each time the action runs.
      $zip->addFile(drupal_realpath($file->uri), $filename);
      $zip->close();
      $archive_contents[] = $filename;
    }
  }

  // The operation is complete, create a file entity and provide a download
  // link to the user.
  if ($context['progress']['current'] == $context['progress']['total']) {
    $archive_file = new stdClass();
    $archive_file->uri      = $destination;
    $archive_file->filename = basename($destination);
    $archive_file->filemime = file_get_mimetype($destination);
    $archive_file->uid      = $user->uid;
    $archive_file->status   = $context['settings']['temporary'] ? FALSE : FILE_STATUS_PERMANENT;
    file_save($archive_file);

    $url = file_create_url($archive_file->uri);
    $url = l($url, $url, array('absolute' => TRUE));
    _views_bulk_operations_log(t('An archive has been created and can be downloaded from: !url', array('!url' => $url)));
  }
}

/**
 * Configuration form shown to the user before the action gets executed.
 */
function views_bulk_operations_archive_action_form($context) {
  // Pass the scheme as a value, so that the submit callback can access it.
  $form['scheme'] = array(
    '#type' => 'value',
    '#value' => $context['settings']['scheme'],
  );

  $form['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('Filename'),
    '#default_value' => 'vbo_archive_' . date('Ymd'),
    '#field_suffix' => '.zip',
    '#description' => t('The name of the archive file.'),
  );
  return $form;
}

/**
 * Assembles a sanitized and unique URI for the archive, and returns it for
 * usage by the action callback (views_bulk_operations_archive_action).
 */
function views_bulk_operations_archive_action_submit($form, $form_state) {
  // Validate the scheme, fallback to public if it's somehow invalid.
  $scheme = $form_state['values']['scheme'];
  if (!file_stream_wrapper_valid_scheme($scheme)) {
    $scheme = 'public';
  }
  $destination = $scheme . '://' . basename($form_state['values']['filename']) . '.zip';
  // If the chosen filename already exists, file_destination() will append
  // an integer to it in order to make it unique.
  $destination = file_destination($destination, FILE_EXISTS_RENAME);

  return array(
    'destination' => $destination,
  );
}

/**
 * Settings form (embedded into the VBO field settings in the Views UI).
 */
function views_bulk_operations_archive_action_views_bulk_operations_form($options) {
  $scheme_options = array();
  foreach (file_get_stream_wrappers(STREAM_WRAPPERS_LOCAL_NORMAL) as $scheme => $stream_wrapper) {
    $scheme_options[$scheme] = $stream_wrapper['name'];
  }
  if (count($scheme_options) > 1) {
    $form['scheme'] = array(
      '#type' => 'radios',
      '#title' => t('Storage'),
      '#options' => $scheme_options,
      '#default_value' => !empty($options['scheme']) ? $options['scheme'] : variable_get('file_default_scheme', 'public'),
      '#description' => t('Select where the archive should be stored. Private file storage has significantly more overhead than public files, but allows restricted access.'),
    );
  }
  else {
    $scheme_option_keys = array_keys($scheme_options);
    $form['scheme'] = array(
      '#type' => 'value',
      '#value' => reset($scheme_option_keys),
    );
  }

  $form['temporary'] = array(
    '#type' => 'checkbox',
    '#title' => t('Temporary'),
    '#default_value' => isset($options['temporary']) ? $options['temporary'] : TRUE,
    '#description' => t('Temporary files older than 6 hours are removed when cron runs.'),
  );
  return $form;
}

/**
 * Create a sanitized and unique version of the provided filename.
 *
 * @param $filename
 *   String filename
 *
 * @return
 *   The new filename.
 */
function _views_bulk_operations_archive_action_create_filename($filename, $archive_list) {
  // Strip control characters (ASCII value < 32). Though these are allowed in
  // some filesystems, not many applications handle them well.
  $filename = preg_replace('/[\x00-\x1F]/u', '_', $filename);
  if (substr(PHP_OS, 0, 3) == 'WIN') {
    // These characters are not allowed in Windows filenames
    $filename = str_replace(array(':', '*', '?', '"', '<', '>', '|'), '_', $filename);
  }

  if (in_array($filename, $archive_list)) {
    // Destination file already exists, generate an alternative.
    $pos = strrpos($filename, '.');
    if ($pos !== FALSE) {
      $name = substr($filename, 0, $pos);
      $ext = substr($filename, $pos);
    }
    else {
      $name = $filename;
      $ext = '';
    }

    $counter = 0;
    do {
      $filename = $name . '_' . $counter++ . $ext;
    } while (in_array($filename, $archive_list));
  }

  return $filename;
}
