<?php

/**
 * @file
 * Home of the FeedsFileFetcher and related classes.
 */

/**
 * Definition of the import batch object created on the fetching stage by
 * FeedsFileFetcher.
 */
class FeedsFileFetcherResult extends FeedsFetcherResult {
  /**
   * Constructor.
   */
  public function __construct($file_path) {
    parent::__construct('');
    $this->file_path = $file_path;
  }

  /**
   * Overrides parent::getRaw().
   */
  public function getRaw() {
    return $this->sanitizeRaw(file_get_contents($this->file_path));
  }

  /**
   * Overrides parent::getFilePath().
   */
  public function getFilePath() {
    if (!is_readable($this->file_path)) {
      throw new Exception(t('File @filepath is not accessible.', array('@filepath' => $this->file_path)));
    }
    return $this->sanitizeFile($this->file_path);
  }
}

/**
 * Fetches data via HTTP.
 */
class FeedsFileFetcher extends FeedsFetcher {

  /**
   * Implements FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    // Just return a file fetcher result if this is a file.
    if (is_file($source_config['source'])) {
      return new FeedsFileFetcherResult($source_config['source']);
    }

    // Batch if this is a directory.
    $state = $source->state(FEEDS_FETCH);
    $files = array();
    if (!isset($state->files)) {
      $state->files = $this->listFiles($source_config['source']);
      $state->total = count($state->files);
    }
    if (count($state->files)) {
      $file = array_shift($state->files);
      $state->progress($state->total, $state->total - count($state->files));
      return new FeedsFileFetcherResult($file);
    }

    throw new Exception(t('Resource is not a file or it is an empty directory: %source', array('%source' => $source_config['source'])));
  }

  /**
   * Returns an array of files in a directory.
   *
   * @param string $dir
   *   A stream wreapper URI that is a directory.
   *
   * @return array
   *   An array of stream wrapper URIs pointing to files. The array is empty if
   *   no files could be found. Never contains directories.
   */
  protected function listFiles($dir) {
    $dir = file_stream_wrapper_uri_normalize($dir);
    $files = array();
    if ($items = @scandir($dir)) {
      foreach ($items as $item) {
        if (is_file("$dir/$item") && strpos($item, '.') !== 0) {
          $files[] = "$dir/$item";
        }
      }
    }
    return $files;
  }

  /**
   * Source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['fid'] = array(
      '#type' => 'value',
      '#value' => empty($source_config['fid']) ? 0 : $source_config['fid'],
    );
    if (empty($this->config['direct'])) {
      $form['source'] = array(
        '#type' => 'value',
        '#value' => empty($source_config['source']) ? '' : $source_config['source'],
      );
      $form['upload'] = array(
        '#type' => 'file',
        '#title' => empty($this->config['direct']) ? t('File') : NULL,
        '#description' => empty($source_config['source']) ? t('Select a file from your local system.') : t('Select a different file from your local system.'),
        '#theme_wrappers' => array('feeds_upload'),
        '#file_info' => empty($source_config['fid']) ? NULL : file_load($source_config['fid']),
        '#size' => 10,
      );
    }
    else {
      $form['source'] = array(
        '#type' => 'textfield',
        '#title' => t('File'),
        '#description' => t('Specify a path to a file or a directory. Prefix the path with a scheme. Available schemes: @schemes.', array('@schemes' => implode(', ', $this->config['allowed_schemes']))),
        '#default_value' => empty($source_config['source']) ? '' : $source_config['source'],
      );
    }
    return $form;
  }

  /**
   * Overrides parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
    $values['source'] = trim($values['source']);

    if (empty($this->config['direct'])) {

      $feed_dir = $this->config['directory'];

      if (!file_prepare_directory($feed_dir, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
        if (user_access('administer feeds')) {
          $plugin_key = feeds_importer($this->id)->config[$this->pluginType()]['plugin_key'];
          $link = url('admin/structure/feeds/' . $this->id . '/settings/' . $plugin_key);
          form_set_error('feeds][FeedsFileFetcher][source', t('Upload failed. Please check the upload <a href="@link">settings.</a>', array('@link' => $link)));
        }
        else {
          form_set_error('feeds][FeedsFileFetcher][source', t('Upload failed. Please contact your site administrator.'));
        }
        watchdog('feeds', 'The upload directory %directory required by a feed could not be created or is not accessible. A newly uploaded file could not be saved in this directory as a consequence, and the upload was canceled.', array('%directory' => $feed_dir));
      }
      // Validate and save uploaded file.
      elseif ($file = file_save_upload('feeds', array('file_validate_extensions' => array(0 => $this->config['allowed_extensions'])), $feed_dir)) {
        $values['source'] = $file->uri;
        $values['file'] = $file;
      }
      elseif (empty($values['source'])) {
        form_set_error('feeds][FeedsFileFetcher][source', t('Please upload a file.'));
      }
      else {
        // File present from previous upload. Nothing to validate.
      }
    }
    else {
      // Check if chosen url scheme is allowed.
      $scheme = file_uri_scheme($values['source']);
      if (!$scheme || !in_array($scheme, $this->config['allowed_schemes'])) {
        form_set_error('feeds][FeedsFileFetcher][source', t("The file needs to reside within the site's files directory, its path needs to start with scheme://. Available schemes: @schemes.", array('@schemes' => implode(', ', $this->config['allowed_schemes']))));
      }
      // Check whether the given path is readable.
      elseif (!is_readable($values['source'])) {
        form_set_error('feeds][FeedsFileFetcher][source', t('The specified file or directory does not exist.'));
      }
    }
  }

  /**
   * Overrides parent::sourceSave().
   */
  public function sourceSave(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    // If a new file is present, delete the old one and replace it with the new
    // one.
    if (isset($source_config['file'])) {
      $file = $source_config['file'];
      if (isset($source_config['fid'])) {
        $this->deleteFile($source_config['fid'], $source->feed_nid);
      }
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'feeds', get_class($this), $source->feed_nid);

      $source_config['fid'] = $file->fid;
      unset($source_config['file']);
      $source->setConfigFor($this, $source_config);
    }
  }

  /**
   * Overrides parent::sourceDelete().
   */
  public function sourceDelete(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    if (isset($source_config['fid'])) {
      $this->deleteFile($source_config['fid'], $source->feed_nid);
    }
  }

  /**
   * Overrides parent::configDefaults().
   */
  public function configDefaults() {
    $schemes = $this->getSchemes();
    $scheme = in_array('private', $schemes) ? 'private' : 'public';

    return array(
      'allowed_extensions' => 'txt csv tsv xml opml',
      'direct' => FALSE,
      'directory' => $scheme . '://feeds',
      'allowed_schemes' => $schemes,
    );
  }

  /**
   * Overrides parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['allowed_extensions'] = array(
      '#type' => 'textfield',
      '#title' => t('Allowed file extensions'),
      '#description' => t('Allowed file extensions for upload.'),
      '#default_value' => $this->config['allowed_extensions'],
    );
    $form['direct'] = array(
      '#type' => 'checkbox',
      '#title' => t('Supply path to file or directory directly'),
      '#description' => t('For experts. Lets users specify a path to a file <em>or a directory of files</em> directly,
        instead of a file upload through the browser. This is useful when the files that need to be imported
        are already on the server.'),
      '#default_value' => $this->config['direct'],
    );
    $form['directory'] = array(
      '#type' => 'textfield',
      '#title' => t('Upload directory'),
      '#description' => t('Directory where uploaded files get stored. Prefix the path with a scheme. Available schemes: @schemes.', array('@schemes' => implode(', ', $this->getSchemes()))),
      '#default_value' => $this->config['directory'],
      '#states' => array(
        'visible' => array(':input[name="direct"]' => array('checked' => FALSE)),
        'required' => array(':input[name="direct"]' => array('checked' => FALSE)),
      ),
    );
    if ($options = $this->getSchemeOptions()) {
      $form['allowed_schemes'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Allowed schemes'),
        '#default_value' => $this->config['allowed_schemes'],
        '#options' => $options,
        '#description' => t('Select the schemes you want to allow for direct upload.'),
        '#states' => array(
          'visible' => array(':input[name="direct"]' => array('checked' => TRUE)),
        ),
      );
    }

    return $form;
  }

  /**
   * Overrides parent::configFormValidate().
   *
   * Ensure that the chosen directory is accessible.
   */
  public function configFormValidate(&$values) {

    $values['directory'] = trim($values['directory']);
    $values['allowed_schemes'] = array_filter($values['allowed_schemes']);

    if (!$values['direct']) {
      // Ensure that the upload directory field is not empty when not in
      // direct-mode.
      if (!$values['directory']) {
        form_set_error('directory', t('Please specify an upload directory.'));
        // Do not continue validating the directory if none was specified.
        return;
      }

      // Validate the URI scheme of the upload directory.
      $scheme = file_uri_scheme($values['directory']);
      if (!$scheme || !in_array($scheme, $this->getSchemes())) {
        form_set_error('directory', t('Please enter a valid scheme into the directory location.'));

        // Return here so that attempts to create the directory below don't
        // throw warnings.
        return;
      }

      // Ensure that the upload directory exists.
      if (!file_prepare_directory($values['directory'], FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
        form_set_error('directory', t('The chosen directory does not exist and attempts to create it failed.'));
      }
    }
  }

  /**
   * Deletes a file.
   *
   * @param int $fid
   *   The file id.
   * @param int $feed_nid
   *   The feed node's id, or 0 if a standalone feed.
   *
   * @return bool|array
   *   TRUE for success, FALSE in the event of an error, or an array if the file
   *   is being used by any modules.
   *
   * @see file_delete()
   */
  protected function deleteFile($fid, $feed_nid) {
    if ($file = file_load($fid)) {
      file_usage_delete($file, 'feeds', get_class($this), $feed_nid);
      return file_delete($file);
    }
    return FALSE;
  }

  /**
   * Returns available schemes.
   *
   * @return array
   *   The available schemes.
   */
  protected function getSchemes() {
    return array_keys(file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE));
  }

  /**
   * Returns available scheme options for use in checkboxes or select list.
   *
   * @return array
   *   The available scheme array keyed scheme => description
   */
  protected function getSchemeOptions() {
    $options = array();
    foreach (file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE) as $scheme => $info) {
      $options[$scheme] = check_plain($scheme . ': ' . $info['description']);
    }
    return $options;
  }

}
