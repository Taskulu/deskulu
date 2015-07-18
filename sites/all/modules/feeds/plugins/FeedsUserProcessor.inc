<?php

/**
 * @file
 * Contains FeedsUserProcessor.
 */

/**
 * Option to block users not found in the feed.
 *
 * @var string
 */
define('FEEDS_BLOCK_NON_EXISTENT', 'block');

/**
 * Feeds processor plugin. Create users from feed items.
 */
class FeedsUserProcessor extends FeedsProcessor {

  /**
   * Define entity type.
   */
  public function entityType() {
    return 'user';
  }

  /**
   * Implements parent::entityInfo().
   */
  protected function entityInfo() {
    $info = parent::entityInfo();
    $info['label plural'] = t('Users');
    return $info;
  }

  /**
   * Creates a new user account in memory and returns it.
   */
  protected function newEntity(FeedsSource $source) {
    $account = new stdClass();
    $account->uid = 0;
    $account->roles = array_filter($this->config['roles']);
    $account->status = $this->config['status'];
    return $account;
  }

  /**
   * Loads an existing user.
   */
  protected function entityLoad(FeedsSource $source, $uid) {
    $user = parent::entityLoad($source, $uid);

    // Copy the password so that we can compare it again at save.
    $user->feeds_original_pass = $user->pass;
    return $user;
  }

  /**
   * Validates a user account.
   */
  protected function entityValidate($account) {
    if (empty($account->name) || empty($account->mail) || !valid_email_address($account->mail)) {
      throw new FeedsValidationException(t('User name missing or email not valid.'));
    }
  }

  /**
   * Save a user account.
   */
  protected function entitySave($account) {
    if ($this->config['defuse_mail']) {
      $account->mail = $account->mail . '_test';
    }

    $edit = (array) $account;

    // Remove pass from $edit if the password is unchanged.
    if (isset($account->feeds_original_pass) && $account->pass == $account->feeds_original_pass) {
      unset($edit['pass']);
    }

    user_save($account, $edit);
    if ($account->uid && !empty($account->openid)) {
      $authmap = array(
        'uid' => $account->uid,
        'module' => 'openid',
        'authname' => $account->openid,
      );
      if (SAVED_UPDATED != drupal_write_record('authmap', $authmap, array('uid', 'module'))) {
        drupal_write_record('authmap', $authmap);
      }
    }
  }

  /**
   * Delete multiple user accounts.
   */
  protected function entityDeleteMultiple($uids) {
    user_delete_multiple($uids);
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'roles' => array(),
      'status' => 1,
      'defuse_mail' => FALSE,
    ) + parent::configDefaults();
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    $form['status'] = array(
      '#type' => 'radios',
      '#title' => t('Status'),
      '#description' => t('Select whether users should be imported active or blocked.'),
      '#options' => array(0 => t('Blocked'), 1 => t('Active')),
      '#default_value' => $this->config['status'],
    );

    $roles = user_roles(TRUE);
    unset($roles[2]);
    if (count($roles)) {
      $form['roles'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Additional roles'),
        '#description' => t('Every user is assigned the "authenticated user" role. Select additional roles here.'),
        '#default_value' => $this->config['roles'],
        '#options' => $roles,
      );
    }
    $form['defuse_mail'] = array(
      '#type' => 'checkbox',
      '#title' => t('Defuse e-mail addresses'),
      '#description' => t('This appends _test to all imported e-mail addresses to ensure they cannot be used as recipients.'),
      '#default_value' => $this->config['defuse_mail'],
    );
    $form['update_non_existent']['#options'][FEEDS_BLOCK_NON_EXISTENT] = t('Block non-existent users');
    return $form;
  }

  /**
   * Override setTargetElement to operate on a target item that is a node.
   */
  public function setTargetElement(FeedsSource $source, $target_user, $target_element, $value) {
    switch ($target_element) {
      case 'created':
        $target_user->created = feeds_to_unixtime($value, REQUEST_TIME);
        break;
      case 'language':
        $target_user->language = strtolower($value);
        break;
      default:
        parent::setTargetElement($source, $target_user, $target_element, $value);
        break;
    }
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $targets = parent::getMappingTargets();
    $targets += array(
      'name' => array(
        'name' => t('User name'),
        'description' => t('Name of the user.'),
        'optional_unique' => TRUE,
       ),
      'mail' => array(
        'name' => t('Email address'),
        'description' => t('Email address of the user.'),
        'optional_unique' => TRUE,
       ),
      'created' => array(
        'name' => t('Created date'),
        'description' => t('The created (e. g. joined) data of the user.'),
       ),
      'pass' => array(
        'name' => t('Unencrypted Password'),
        'description' => t('The unencrypted user password.'),
      ),
      'status' => array(
        'name' => t('Account status'),
        'description' => t('Whether a user is active or not. 1 stands for active, 0 for blocked.'),
      ),
      'language' => array(
        'name' => t('User language'),
        'description' => t('Default language for the user.'),
      ),
    );
    if (module_exists('openid')) {
      $targets['openid'] = array(
        'name' => t('OpenID identifier'),
        'description' => t('The OpenID identifier of the user. <strong>CAUTION:</strong> Use only for migration purposes, misconfiguration of the OpenID identifier can lead to severe security breaches like users gaining access to accounts other than their own.'),
        'optional_unique' => TRUE,
       );
    }

    $this->getHookTargets($targets);

    return $targets;
  }

  /**
   * Get id of an existing feed item term if available.
   */
  protected function existingEntityId(FeedsSource $source, FeedsParserResult $result) {
    if ($uid = parent::existingEntityId($source, $result)) {
      return $uid;
    }

    // Iterate through all unique targets and try to find a user for the
    // target's value.
    foreach ($this->uniqueTargets($source, $result) as $target => $value) {
      switch ($target) {
        case 'name':
          $uid = db_query("SELECT uid FROM {users} WHERE name = :name", array(':name' => $value))->fetchField();
          break;
        case 'mail':
          $uid = db_query("SELECT uid FROM {users} WHERE mail = :mail", array(':mail' => $value))->fetchField();
          break;
        case 'openid':
          $uid = db_query("SELECT uid FROM {authmap} WHERE authname = :authname AND module = 'openid'", array(':authname' => $value))->fetchField();
          break;
      }
      if ($uid) {
        // Return with the first nid found.
        return $uid;
      }
    }
    return 0;
  }

  /**
   * Overrides FeedsProcessor::clean().
   *
   * Block users instead of deleting them.
   *
   * @param FeedsState $state
   *   The FeedsState object for the given stage.
   */
  protected function clean(FeedsState $state) {
    // Delegate to parent if not blocking or option not set.
    if (!isset($this->config['update_non_existent']) || $this->config['update_non_existent'] !== FEEDS_BLOCK_NON_EXISTENT) {
      return parent::clean($state);
    }

    if (!empty($state->removeList)) {
      // @see user_user_operations_block().
      // The following foreach is copied from above function but with an added
      // counter to count blocked users.
      foreach (user_load_multiple($state->removeList) as $account) {
        $this->loadItemInfo($account);
        $account->feeds_item->hash = $this->config['update_non_existent'];
        // For efficiency manually save the original account before applying any
        // changes.
        $account->original = clone $account;
        user_save($account, array('status' => 0));
        $state->blocked++;
      }
    }
  }

}
