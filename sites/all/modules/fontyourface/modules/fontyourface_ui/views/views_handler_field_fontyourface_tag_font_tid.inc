<?php

/**
 * @file
 * Views handler.
 */

/**
 * Field handler for tags.
 * Largely copied from views_handler_field_term_node_tid.inc
 */
class views_handler_field_fontyourface_tag_font_tid extends views_handler_field_prerender_list {

  function init(&$view, &$options) {

    parent::init($view, $options);
    $this->additional_fields['fid'] = array('table' => 'fontyourface_font', 'field' => 'fid');

  } // init

  function option_definition() {

    $options = parent::option_definition();

    $options['link_to_tag'] = array('default' => TRUE);

    return $options;

  } // option_definition

  /**
   * Provide "link to term" option.
   */
  function options_form(&$form, &$form_state) {

    parent::options_form($form, $form_state);

    $form['link_to_tag'] = array(
      '#title' => t('Link this field to its tag page'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_tag']),
    );

  } // options_form

  /**
   * Add this term to the query
   */
  function query() {

    $this->add_additional_fields();

  } // query

  function pre_render(&$values) {

    $this->field_alias = $this->aliases['fid'];
    $fids = array();

    foreach ($values as $result) {
      if (!empty($result->{$this->aliases['fid']})) {
        $fids[] = $result->{$this->aliases['fid']};
      } // if
    } // foreach

    if ($fids) {

      $query = db_select('fontyourface_tag', 't');
      $tag_font_alias = $query->join('fontyourface_tag_font', 'tf', 't.tid = tf.tid');
      $query
        ->condition($tag_font_alias . '.fid', $fids, 'IN')
        ->fields('t')
        ->orderBy('t.name');
      $font_fid_field = $query->addField($tag_font_alias, 'fid', 'font_fid');

      $result = $query->execute();

      foreach ($result as $tag) {

        $this->items[$tag->font_fid][$tag->tid]['name'] = check_plain($tag->name);
        $this->items[$tag->font_fid][$tag->tid]['tid'] = $tag->tid;

        if (isset($tag->fid)) {
          $this->items[$tag->font_fid][$tag->tid]['fid'] = $tag->fid;
        } // if

        if (!empty($this->options['link_to_tag'])) {

          $this->items[$tag->font_fid][$tag->tid]['make_link'] = TRUE;
          $this->items[$tag->font_fid][$tag->tid]['path'] = 'admin/appearance/fontyourface/browse?tid[]=' . $tag->tid;

        } // if

      } // foreach

    } // if

  } // pre_render

  function render_item($count, $item) {

    return $item['name'];

  } // render_item

} // views_handler_field_fontyourface_tag_font_tid
