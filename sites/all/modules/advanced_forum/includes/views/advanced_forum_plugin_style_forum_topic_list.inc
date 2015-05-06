<?php
/**
 * @file
 * Contains the topic list style plugin.
 */

/**
 * Style plugin to render each item as a row in a table.
 *
 * @ingroup views_style_plugins
 */
// @codingStandardsIgnoreStart
class advanced_forum_plugin_style_forum_topic_list extends views_plugin_style_table {
  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['tid'] = array('default' => '');
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = array('' => t('None'));

    $arguments = $this->display->handler->get_handlers('argument');
    foreach ($arguments as $id => $argument) {
      $options['argument.' . $id] = $argument->ui_name();

    }
    $filters = $this->display->handler->get_handlers('filter');
    foreach ($filters as $id => $filter) {
      $options['filter.' . $id] = $filter->ui_name();
    }

    $form['tid'] = array(
      '#type' => 'select',
      '#title' => t('Source of forum ID'),
      '#options' => $options,
      '#default_value' => $this->options['tid'],
    );
  }

  /**
   * {@inheritdoc}
   *
   * Add a couple of fields to the query that we can later use. We are going to
   * specificly alias them because this style is not meant to be used on relationships.
   */
  public function query() {
    $this->view->query->add_field('node', 'sticky', 'topic_is_sticky');
    $this->view->query->add_field('forum', 'tid', 'topic_actual_forum');
  }

  /**
   * Figure out what the forum ID is. It could have come from an argument or a filter or nowhere.
   *
   * This source would be set by the user in the options.
   */
  public function get_forum_ids() {
    $where = $this->options['tid'];
    if (empty($where)) {
      return;
    }

    $term = '';

    list($type, $id) = explode('.', $where, 2);
    $handler = $this->display->handler->get_handler($type, $id);
    if ($type == 'argument') {
      return empty($handler->argument) ? array() : array($handler->argument);
    }
    else {
      $terms = is_array($handler->value) ? $handler->value : array($handler->value);
      if (isset($handler->options['depth'])) {
        foreach ($terms as $tid) {
          $term = taxonomy_term_load($tid);
          $tree = taxonomy_get_tree($term->vid, $tid, -1, $handler->options['depth']);
          $terms = array_merge($terms, array_map('_taxonomy_get_tid_from_term', $tree));
        }
      }
      return $terms;
    }
  }
}
// @codingStandardsIgnoreEnd
