<?php
/**
 * @file
 * Contains the SearchApiAlterCommentAccess class.
 */

/**
 * Adds node access information to comment indexes.
 */
class SearchApiAlterCommentAccess extends SearchApiAlterNodeAccess {

  /**
   * Overrides SearchApiAlterNodeAccess::supportsIndex().
   *
   * Returns TRUE only for indexes on comments.
   */
  public function supportsIndex(SearchApiIndex $index) {
    return $index->getEntityType() === 'comment';
  }

  /**
   * Overrides SearchApiAlterNodeAccess::getNode().
   *
   * Returns the comment's node, instead of the item (i.e., the comment) itself.
   */
  protected function getNode($item) {
    return node_load($item->nid);
  }

  /**
   * Overrides SearchApiAlterNodeAccess::configurationFormSubmit().
   *
   * Doesn't index the comment's "Author".
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $old_status = !empty($form_state['index']->options['data_alter_callbacks']['search_api_alter_comment_access']['status']);
    $new_status = !empty($form_state['values']['callbacks']['search_api_alter_comment_access']['status']);

    if (!$old_status && $new_status) {
      $form_state['index']->options['fields']['status']['type'] = 'boolean';
    }

    return parent::configurationFormSubmit($form, $values, $form_state);
  }

}
