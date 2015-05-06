<?php

/**
 * @file Base class for testing the Better Exposed Filters module.
 * @author mikeker
 */

/**
 * Helper functions for Better Exposed Filters tests.
 */
class BEF_TestBase extends DrupalWebTestCase {
  /**
   * User with 'Administrator' role.
   */
  protected $admin_user;

  /**
   * Stores information about the view used in these tests.
   */
  protected $view = array();

  public static function getInfo() {
    return array(
      'name' => 'BEF Basic functionality tests',
      'description' => 'Basic tests for Better Exposed Filters.',
      'group' => 'Better Exposed Filters',
    );
  }

  public function setUp() {
    // For benchmarking.
    $this->start = time();

    // Enable any modules required for the test.
    parent::setUp(
      'better_exposed_filters',
      'date',
      'date_views',
      'list',
      'number',
      'taxonomy',
      'text',
      'views',
      'views_ui'
    );

    // One of these days I'll figure out why Features is breaking all my tests.
    module_enable(array('bef_test_content'));

    // User with edit views perms
    $this->admin_user = $this->drupalCreateUser();
    $role = user_role_load_by_name('administrator');
    $this->assertTrue(!empty($role->rid), 'Found the "administrator" role.');
    user_save($this->admin_user, array('roles' => array($role->rid => $role->rid)));
    $this->drupalLogin($this->admin_user);

    // Build a basic view for use in tests.
    $this->createView();

    // $this->createDisplay('Page', array('path' => array('path' => 'bef_test_page')));

    // Add field to default display
    // $this->addField('node.title');

    // Turn of Better Exposed Filters
    $this->setBefExposedForm();
  }

  public function tearDown() {
    debug('This test run took ' . (time() - $this->start) . ' seconds.');
    unset($this->view);
    parent::tearDown();
  }

  /*******************************************************************************
   * Helper functions
   ******************************************************************************/

  /**
   * Returns the URL for the BEF exposed form settings page.
   */
  protected function getBefSettingsUrl() {
    return 'admin/structure/views/nojs/display/' . $this->view['machine_name'] . '/default/exposed_form_options';
  }

  protected function createView($name = '') {
    if (!empty($this->view)) {
      debug('WARNING: createView called after view has already been created.');
      return;
    }

    if (empty($name)) {
      $name = $this->randomName(8);
    }
    $this->view['name'] = $name;
    $this->view['machine_name'] = strtolower($name);

    $edit = array(
      'human_name' => $this->view['name'],
      'name' => $this->view['machine_name'],
      // Default is to create a page display.
      'page[create]' => FALSE,
    );
    $this->drupalPost('admin/structure/views/add', $edit, 'Save & exit');

    // URL to edit this view.
    $this->view['edit_url'] = 'admin/structure/views/view/' . $this->view['machine_name'] . '/edit';
  }

  /**
   * Creates a display of $type.  Currently supports:
   *    'Page'
   *
   * @todo: support more types...
   */
  protected function createDisplay($type = 'Page', $settings = NULL) {
    if (!isset($this->view['displays'])) {
      $this->view['displays'] = array();
    }

    // Add a display of $type to the view
    $this->drupalPost($this->view['edit_url'], array(), "Add $type");

    // Grab the name of the newly created display and store some info about it.
    $url = $this->getUrl();
    $display_name = substr($url, strrpos($url, '/') + 1);
    $this->view['displays'][$display_name] = array(
      'machine_name' => $display_name,
      'edit_url' => 'admin/structure/views/view/' . $this->view['machine_name'] . '/edit/' . $display_name,
      'settings_base_url' => 'admin/structure/views/nojs/display/' . $this->view['machine_name'] . '/' . $display_name,
    );

    // Settings should be in the form of 'path' => array_of_form_settings. Eg:
    // to set the title for a new display as an override:
    //  'title' => array(
    //    'title' => 'This is an override title',
    //    'override[dropdown]' => display_machine_name_goes_here,
    //  )
    //
    // If you navigate to
    //  admin/structure/views/nojs/display/<view_name>/<display_name>/title
    // you will see the form in question.
    foreach ($settings as $path => $values) {
      $this->drupalPost($this->view['displays'][$display_name]['settings_base_url'] . "/$path", $values, 'Apply');
    }
    $this->saveView();
  }

  /**
   * Adds a filter to a view display.
   *
   * $field: string in the form of node.status or
   *   field_data_field_example.field_example_value
   * $settings: (array) Settings on the "Configure filter criterion" dialog.
   *   NOTE: called after the "Expose filter" button is pressed if $exposed
   *   is TRUE so you can set things like "Allow multiple items" or grouped
   *   filter options.
   * $additional: (array) settings for any additional configuration forms such
   *   as taxonomy term settings.
   * $display: machine name of the display to add this filter to. NOTE:
   *   Currently only allows filters on the master display, no overrides.
   *   @todo: fix that, if needed.
   * $exposed: (bool) (optional, default: TRUE) Is this an exposed filter?
   *
   * Note: This routine expects the caller to save the view, as needed.
   */
  protected function addFilter($field, $settings = array(), $additional = array(), $exposed = TRUE, $display = 'default') {
    $edit = array(
      "name[$field]" => TRUE,
    );
    $url = 'admin/structure/views/nojs/add-item/' . $this->view['machine_name'] . "/$display/filter";
    $this->drupalPost($url, $edit, 'Add and configure filter criteria');

    if (!empty($additional)) {
      // Handle filter-specific options screen.
      $this->drupalPost(NULL, $additional, 'Apply');
    }

    if ($exposed) {
      $this->drupalPost(NULL, array(), 'Expose filter');
    }
    $this->drupalPost(NULL, $settings, 'Apply');
  }

  /**
   * Edits an existing filter in the current view. See addFilter for param
   * definitions.
   */
  protected function editFilter($field, $settings, $additional = array(), $display = 'default') {
    if (FALSE !== ($pos = strpos($field, '.'))) {
      $field = substr($field, $pos + 1);
    }
    $url = 'admin/structure/views/nojs/config-item/' . $this->view['machine_name'] . "/$display/filter/$field";
    $this->drupalPost($url, $settings, 'Apply');

    if (!empty($additional)) {
      // Handle filter-specific options screen.
      $this->drupalPost(NULL, $additional, 'Apply');
    }
  }

  /**
   * Adds a sort to a view display. See addFilter for parameter options.
   *
   * Note: This routine expects the caller to save the view, as needed.
   */
  protected function addSort($field, $settings = array(), $additional = array(), $exposed = TRUE, $display = 'default') {
    $edit = array(
      "name[$field]" => TRUE,
    );
    $url = 'admin/structure/views/nojs/add-item/' . $this->view['machine_name'] . "/$display/sort";
    $this->drupalPost($url, $edit, 'Add and configure sort criteria');

    if (!empty($additional)) {
      // Handle filter-specific options screen.
      $this->drupalPost(NULL, $additional, 'Apply');
    }

    if ($exposed) {
      $this->drupalPost(NULL, array(), 'Expose sort');
    }
    $this->drupalPost(NULL, $settings, 'Apply');
  }

  /**
   * Adds a field to a view display. See addFilter for parameter options.
   *
   * Note: This routine expects the caller to save the view, as needed.
   */
  protected function addField($field, $settings = array(), $display = 'default') {
    $edit = array(
      "name[$field]" => TRUE,
    );
    $url = 'admin/structure/views/nojs/add-item/' . $this->view['machine_name'] . "/$display/field";
    $this->drupalPost($url, $edit, 'Add and configure fields');
    $this->drupalPost(NULL, $settings, 'Apply');
  }

  /**
   * Ensures that BEF is selected as the exposed form option
   *
   * Note: This routine expects the caller to save the view, as needed.
   */
  protected function setBefExposedForm($display = 'default') {
    $edit = array(
      "exposed_form[type]" => 'better_exposed_filters',
    );
    $url = 'admin/structure/views/nojs/display/' . $this->view['machine_name'] . "/$display/exposed_form";
    $this->drupalPost($url, $edit, 'Apply');

    // BEF settings is covered under setBefSettings() so we just accept the
    // default values and move on.
    $this->drupalPost(NULL, array(), 'Apply');
  }

  /**
   * Sets various BEF exposed form settings. If $error is specified it also
   * asserts that the error text apepars when trying to apply $settings.
   *
   * Note: This routine expects the caller to save the view, as needed.
   */
  protected function setBefSettings($settings, $error = '') {
    $this->drupalPost($this->getBefSettingsUrl(), $settings, 'Apply');
    if (!empty($error)) {
      $this->assertText($error);
    }
  }

  /**
   * Saves the view
   */
  protected function saveView() {
    $this->drupalPost($this->view['edit_url'], array(), 'Save');
  }
}
