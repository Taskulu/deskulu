<?php

/**
 * @file
 * Field handler to display the topic icon.
 */

// @codingStandardsIgnoreStart
class advanced_forum_handler_field_node_topic_icon extends views_handler_field {
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
    $options['hot_topic_threshold'] = array('default' => 15);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['hot_topic_threshold'] = array(
      '#title' => t('Hot topic threshold'),
      '#description' => t('The number of posts a topic must have to be considered "hot".'),
      '#type' => 'textfield',
      '#default_value' => $this->options['hot_topic_threshold'],
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
    $new_posts = advanced_forum_reply_num_new($values->nid);
    return theme('forum_icon', array(
        'new_posts' => $new_posts,
        'num_posts' => empty($values->node_comment_statistics_comment_count) ? 1 : $values->node_comment_statistics_comment_count + 1,
        'comment_mode' => $values->node_comment,
        'sticky' => $values->node_sticky,
        'node_type' => $values->node_type,
    ));
  }
}
// @codingStandardsIgnoreEnd
