<?php

/**
 * @file
 * Administrative interface for the mail_system variable.
 */

/**
 * Form constructor for the mailsystem settings form.
 */
function mailsystem_admin_settings() {
  $args = array(
    '!interface' => url('http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7'),
    '@interface' => 'MailSystemInterface',
    '!format' => url('http://api.drupal.org/api/drupal/includes--mail.inc/function/MailSystemInterface%3A%3Aformat/7'),
    '@format' => 'format()',
    '!mail' => url('http://api.drupal.org/api/drupal/includes--mail.inc/function/MailSystemInterface%3A%3Amail/7'),
    '!hook_mail' => 'http://api.drupal.org/api/drupal/modules%21system%21system.api.php/function/hook_mail/7',
    '@mail' => 'mail()',
    '!default_class' => url('http://api.drupal.org/api/drupal/modules--system--system.mail.inc/class/DefaultMailSystem/7'),
    '@default_class' => mailsystem_default_value(),
    '%module' => 'module',
    '%key' => 'key',
  );

  $mail_system = mailsystem_read_settings();
  $delivery_classes = mailsystem_admin_get_delivery_classes();
  $formatter_classes = mailsystem_admin_get_formatter_classes();
  $mail_modules = mailsystem_admin_get_mail_modules();

  $form['mailsystem'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail System Settings'),
    '#description' => t('Drupal provides a default <a href="!interface"><code>@interface</code></a> class called <a href="!default_class"><code>@default_class</code></a>. Modules may provide additional classes. Each <a href="!interface"><code>@interface</code></a> class may be associated with one or more identifiers, composed of a %module and an optional %key. Each email being sent also has a %module and a %key. To decide which class to use, Drupal uses the following search order: <ol><li>The class associated with the %module and %key, if any.</li><li>The class associated with the %module, if any.</li><li>The site-wide default <a href="!interface"><code>@interface</code></a> class.</li></ol>', $args),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );

  // Generate a list of themes which may used to render emails.
  $theme_options = array('current' => t('Current'), 'default' => t('Default'));
  if (module_exists('domain_theme')) {
    $theme_options['domain'] = t('Domain Theme');
  }
  // Get a list of all themes.
  $themes = list_themes();
  foreach ($themes as $name => $theme) {
    if ($theme->status == 1) {
      $theme_options[$name] = $theme->info['name'];
    }
  }
  $form['mailsystem']['mailsystem_theme'] = array(
      '#type' => 'select',
      '#title' => t('Theme to render the emails'),
      '#description' => t('Select the theme that will be used to render the emails. This can be either the current theme, the default theme, the domain theme or any active theme.'),
      '#options' => $theme_options,
      '#default_value' => variable_get('mailsystem_theme', 'current'),
  );

  $form['mailsystem'][mailsystem_default_id()] = array(
    '#type' => 'fieldset',
    '#title' => t('Site-wide default mail system'),
  );
  $form['mailsystem'][mailsystem_default_id()]['mail'] = array(
    '#type' => 'select',
    '#title' => t('Delivery'),
    '#options' => $delivery_classes,
    '#default_value' => $mail_system[mailsystem_default_id()]['mail'],
    '#description' => t('Class used to send the mail'),
  );
  $form['mailsystem'][mailsystem_default_id()]['format'] = array(
    '#type' => 'select',
    '#title' => t('Formatting'),
    '#options' => $formatter_classes,
    '#default_value' => $mail_system[mailsystem_default_id()]['format'],
    '#description' => t('Class used to format the body of the mail'),
  );

  unset($mail_system[mailsystem_default_id()]);

  foreach ($mail_system as $id => $settings) {
    $form['mailsystem'][$id] = array(
      '#type' => 'fieldset',
      '#title' => t('Custom settings for mail-id %id', array('%id' => $id)),
    );
    $form['mailsystem'][$id]['mail'] = array(
      '#type' => 'select',
      '#title' => t('Delivery'),
      '#options' => $delivery_classes,
      '#default_value' => $settings['mail'],
      '#description' => t('Class used to send the mail'),
    );
    $form['mailsystem'][$id]['format'] = array(
      '#type' => 'select',
      '#title' => t('Formatting'),
      '#options' => $formatter_classes,
      '#default_value' => $settings['format'],
      '#description' => t('Class used to format the body of the mail'),
    );
    $form['mailsystem'][$id]['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Remove custom settings for mail-id @id', array('@id' => $id)),
      '#submit' => array('mailsystem_admin_remove_setting_submit'),
      '#limit_validation_errors' => array(
        array('mailsystem', $id),
      ),
    );
  }

  $form['mailsystem']['add-custom-settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add custom settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#element_validate' => array('mailsystem_admin_add_setting_validate'),
  );
  $form['mailsystem']['add-custom-settings']['module'] = array(
    '#type' => 'select',
    '#title' => t('Module'),
    '#options' => $mail_modules,
  );
  $form['mailsystem']['add-custom-settings']['key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#size' => 30,
    '#description' => t('An optional key which further specifies the mail in question. You may have to examine the source code of the <a href="!hook_mail">hook_mail</a> implementation of the module in question in order to find an appropriate value', $args),
  );
  $form['mailsystem']['add-custom-settings']['mail'] = array(
    '#type' => 'select',
    '#title' => t('Delivery'),
    '#options' => $delivery_classes,
    '#description' => t('Class used to send the mail'),
  );
  $form['mailsystem']['add-custom-settings']['format'] = array(
    '#type' => 'select',
    '#title' => t('Formatting'),
    '#options' => $formatter_classes,
    '#description' => t('Class used to format the body of the mail'),
  );
  $form['mailsystem']['add-custom-settings']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add custom settings'),
    '#submit' => array('mailsystem_admin_add_setting_submit'),
    '#limit_validation_errors' => array(
      array('mailsystem', 'add-custom-settings'),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
  return $form;
}

/**
 * Form validation handler for mailsystem settings form.
 *
 * Add an additional custom setting for a module/key.
 */
function mailsystem_admin_add_setting_validate($fieldset, &$form_state) {
  // Only perform this validation if we're adding a custom setting.
  if (!isset($form_state['submit_handlers']) || !in_array('mailsystem_admin_add_setting_submit', $form_state['submit_handlers'])) {
    return;
  }

  $module = $fieldset['module']['#value'];
  $key = $fieldset['key']['#value'];
  $id = $module;
  if (!empty($key)) {
    $id .= '_' . $key;
   }

  $mail_system = mailsystem_get();
  if (isset($mail_system[$id])) {
    form_error($fieldset['module'], t('A setting already exists for this combination of module and key'));
  }

  $form_state['new_settings_id'] = $id;
}

/**
 * Form API submit callback: Add an additional custom setting for a module/key.
 */
function mailsystem_admin_add_setting_submit($form, &$form_state) {
  $values = $form_state['values']['mailsystem']['add-custom-settings'];

  $id = $form_state['new_settings_id'];
  mailsystem_admin_save_custom_setting($id, $values);

  drupal_set_message(t('Added new custom setting for mail-id %id',
    array('%id' => $id)));
}

/**
 * Form API submit callback: Remove a custom setting.
 */
function mailsystem_admin_remove_setting_submit($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents); // submit
  $id = array_pop($parents); // fieldset

  mailsystem_clear(array($id => $id));

  drupal_set_message(t('Removed custom setting for mail-id %id',
    array('%id' => $id)));
}

/**
 * Form API submit callback: save settings.
 */
function mailsystem_admin_settings_submit($form, &$form_state) {
  variable_set('mailsystem_theme', $form_state['values']['mailsystem']['mailsystem_theme']);
  // Rebuild the theme registry to make changes needed by theme rendering.
  drupal_theme_rebuild();
  unset($form_state['values']['mailsystem']['mailsystem_theme']);
  unset($form_state['values']['mailsystem']['add-custom-settings']);

  foreach ($form_state['values']['mailsystem'] as $id => $values) {
    mailsystem_admin_save_custom_setting($id, $values);
  }

  drupal_set_message(t('Saved settings for mailsystem'));
}


/**
 * Helper function. Writes the setting values for a given mail-id.
 */
function mailsystem_admin_save_custom_setting($id, $values) {
  if ($values['format'] != $values['mail']) {
    $value = array(
      'mail' => $values['mail'],
      'format' => $values['format'],
    );
  }
  else {
    $value = $values['mail'];
  }
  mailsystem_set(array($id => $value));
}

/**
 * Retrieves a list of modules implementing hook_mail().
 *
 * @return array
 *   An associative array of module descriptions keyed by the corresponding
 *   module machine name.
 */
function mailsystem_admin_get_mail_modules() {
  $modules = module_implements('mail');
  $result = array();

  foreach ($modules as $module) {
    $info = system_get_info('module', $module);
    $description = empty($info['package']) ? t('Other') : $info['package'];
    $description .= ' » ' . $info['name'];
    $result[$module] = $description;
  }

  return $result;
}

/**
 * Returns a list of classes suitable for sending mail.
 *
 * @return array
 *   List of the names of classes implementing MailSystemInterface.
 */
function mailsystem_admin_get_delivery_classes() {
  $delivery_classes = mailsystem_get_classes();
  unset($delivery_classes['MailsystemDelegateMailSystem']);
  return $delivery_classes;
}

/**
 * Returns a list of classes suitable for formatting email.
 *
 * @todo: Currently we consider all classes capable of both mail delivery and
 * formatting. If required in the future the mechanism could be extended such
 * that specific classes only serve either as formatter or delivery-system if
 * appropriate.
 *
 * @return array
 *   List of the names of classes implementing MailSystemInterface.
 */
function mailsystem_admin_get_formatter_classes() {
  return mailsystem_admin_get_delivery_classes();
}
