<?php

/**
 * @file
 * Field handler to display the topic pager.
 */

// @codingStandardsIgnoreStart
class advanced_forum_handler_field_node_topic_pager extends views_handler_field {
  /**
   * {@inheritdoc}
   */
  public function construct() {
    parent::construct();
    $this->additional_fields = array('nid' => 'nid', 'type' => 'type');
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['total_pages_shown'] = array('default' => 6);

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['total_pages_shown'] = array(
      '#title' => t('Total pages shown'),
      '#description' => t('Total number of page numbers to show, including initial numbers and one final. 0 hides pager.'),
      '#type' => 'textfield',
      '#default_value' => $this->options['total_pages_shown'],
    );
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
    $this->field_alias = $this->table . '_' . $this->field;
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    // Make a fake topic object with the information that the core one has.
    $topic = new stdClass();
    $topic->nid = $values->nid;
    $topic->type = $values->node_type;
    $topic->comment_count = $values->node_comment_statistics_comment_count;

    return theme('advanced_forum_topic_pager', array(
      'pagecount' => $this->options['total_pages_shown'],
      'topic' => $topic,
    ));
  }
}
// @codingStandardsIgnoreEnd
