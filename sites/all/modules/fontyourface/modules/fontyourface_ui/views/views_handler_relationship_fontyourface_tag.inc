<?php

/**
 * @file
 * Views handler.
 */

class views_handler_relationship_fontyourface_tag extends views_handler_relationship {

  /**
   * Called to implement a relationship in a query.
   */
  function query() {

    $this->ensure_my_table();

    $def = $this->definition;
    $def['table'] = 'fontyourface_tag';

    if (!empty($this->options['required'])) {

      $tag_font = $this->query->add_table('fontyourface_tag_font', $this->relationship);
      $def['left_table'] = $tag_font;
      $def['left_field'] = 'tid';
      $def['field'] = 'tid';

      if (!empty($this->options['required'])) {
        $def['type'] = 'INNER';
      } // if

    } // if
    else {

      // If the join is optional, join a subselect that will emulate fontyourface_tag_font table instead.
      $def['left_table'] = $this->table_alias;
      $def['left_field'] = 'fid';
      $def['field'] = 'font';
      $def['table formula'] = "(
        SELECT t.*, tf.fid AS font
        FROM {fontyourface_tag} t
        INNER JOIN {fontyourface_tag_font} tf
          ON t.tid = tf.tid
      )";

    } // else

    $join = new views_join();

    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;

    // use a short alias for this:
    $alias = $def['table'] . '_' . $this->table;
    $this->alias = $this->query->add_relationship($alias, $join, 'fontyourface_tag', $this->relationship);

  } // query

} // views_handler_relationship_fontyourface_tag_font
