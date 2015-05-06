<?php
/**
 * @file
 * A virtual Mandrill Templates API implementation for use in testing.
 */

class Mandrill_TemplatesTest extends Mandrill_Templates {
  public function __construct(DrupalMandrillTest $master) {
    parent::__construct($master);
  }

  /**
   * @see Mandrill_Templates::getList()
   */
  public function getList($label = NULL) {
    $templates = $this->getTestTemplatesData();

    if (empty($label)) {
      $matched_templates = $templates;
    }
    else {
      $matched_templates = array();

      foreach ($templates as $template) {
        if (in_array($label, $template->labels)) {
          $matched_templates[] = $template;
        }
      }
    }

    return $matched_templates;
  }

  /**
   * Gets an array of templates used in tests.
   */
  protected function getTestTemplatesData() {
    $templates = array();

    $template = array(
      'slug' => 'test-template',
      'name' => 'Test Template',
      'labels' => array(
        'test-label'
      ),
      'code' => '<div>editable content</div>',
      'subject' => 'Test Subject',
      'from_email' => 'admin@example.com',
      'from_name' => 'Admin',
      'text' => 'Test text',
      'publish_name' => 'Test Template',
      'publish_code' => '<div>different than draft content</div>',
      'publish_subject' => 'Test Publish Subject',
      'publish_from_email' => 'admin@example.com',
      'publish_from_name' => 'Test Publish Name',
      'publish_text' => 'Test publish text',
      'published_at' => '2013-01-01 15:30:40',
      'created_at' => '2013-01-01 15:30:27',
      'updated_at' => '2013-01-01 15:30:49',
    );

    $templates[] = $template;

    return $templates;
  }
}
