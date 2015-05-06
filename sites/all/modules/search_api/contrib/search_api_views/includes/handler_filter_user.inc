<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterUser.
 */

/**
 * Views filter handler class for handling user entities.
 *
 * Based on views_handler_filter_user_name.
 */
class SearchApiViewsHandlerFilterUser extends SearchApiViewsHandlerFilterEntity {

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    // Set autocompletion.
    $path = $this->isMultiValued() ? 'admin/views/ajax/autocomplete/user' : 'user/autocomplete';
    $form['value']['#autocomplete_path'] = $path;
  }

  /**
   * {@inheritdoc}
   */
  protected function ids_to_strings(array $ids) {
    $names = array();
    $args[':uids'] = array_filter($ids);
    $result = db_query("SELECT uid, name FROM {users} u WHERE uid IN (:uids)", $args);
    $result = $result->fetchAllKeyed();
    foreach ($ids as $uid) {
      if (!$uid) {
        $names[] = variable_get('anonymous', t('Anonymous'));
      }
      elseif (isset($result[$uid])) {
        $names[] = $result[$uid];
      }
    }
    return implode(', ', $names);
  }

  /**
   * {@inheritdoc}
   */
  protected function validate_entity_strings(array &$form, array $values) {
    $uids = array();
    $missing = array();
    foreach ($values as $value) {
      if (drupal_strtolower($value) === drupal_strtolower(variable_get('anonymous', t('Anonymous')))) {
        $uids[] = 0;
      }
      else {
        $missing[strtolower($value)] = $value;
      }
    }

    if (!$missing) {
      return $uids;
    }

    $result = db_query("SELECT * FROM {users} WHERE name IN (:names)", array(':names' => array_values($missing)));
    foreach ($result as $account) {
      unset($missing[strtolower($account->name)]);
      $uids[] = $account->uid;
    }

    if ($missing) {
      form_error($form, format_plural(count($missing), 'Unable to find user: @users', 'Unable to find users: @users', array('@users' => implode(', ', $missing))));
    }

    return $uids;
  }

}
