<?php
/**
 * @file
 * Administrative forms for Mandrill Template module.
 */

/**
 * Return a form for adding/editing a Mandrill template map.
 */
function mandrill_template_map_form($form, &$form_state, MandrillTemplateMap $map = NULL, $op = 'edit') {
  $form_state['op'] = $op;
  if ($form_state['op'] == 'clone') {
    $map->label .= ' (cloned)';
    $map->name = '';
  }
  // Store the existing map for updating on submit.
  if (isset($map)) {
    $form_state['map'] = $map;
  }
  else {
    $form_state['op'] = 'add';
  }

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#description' => t('The name of this Template Mapping.'),
    '#size' => 35,
    '#maxlength' => 32,
    '#default_value' => $map ? $map->label : '',
    '#required' => TRUE,
  );

  // Machine-readable list name.
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($map->name) ? $map->name : '',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'mandrill_template_map_load_entities',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this template map. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['map_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Template Map Settings'),
    '#collapsible' => FALSE,
    '#attributes' => array(
      'id' => array('mandrill-template-mapping'),
    ),
  );

  $templates = mandrill_get_templates();
  $template_names = array();
  foreach ($templates as $template) {
    $template_names[$template['slug']] = $template;
  }
  // Check if the currently configured template still exists.
  if (!empty($map->template_id) && !array_key_exists($map->template_id, $template_names)) {
    drupal_set_message(t('The configured Mandrill template is no longer available, please select a valid one.'), 'warning');
  }
  if (!empty($templates)) {
    $options = array('' => t('-- Select --'));
    foreach ($templates as $template) {
      $options[$template['slug']] = $template['name'];
    }
    $form['map_settings']['template_id'] = array(
      '#type' => 'select',
      '#title' => t('Email Template'),
      '#description' => t('Select a Mandrill template.'),
      '#options' => $options,
      '#default_value' => isset($map->template_id) ? $map->template_id : '',
      '#required' => TRUE,
      '#ajax' => array(
        'callback' => 'mandrill_template_map_form_callback',
        'wrapper' => 'mandrill-template-mapping',
      ),
    );
    $form_template_id = & $form_state['values']['template_id'];

    if (!$form_template_id && isset($map->mandrill_template_map_entity_id)) {
      $form_template_id = $map->template_id;
    }

    if ($form_template_id) {
      $regions = array('' => t('-- Select --')) + _mandrill_parse_regions($template_names[$form_template_id]['publish_code']);
      $form['map_settings']['main_section'] = array(
        '#type' => 'select',
        '#title' => t('Template region'),
        '#description' => t('Select the template region to use for email content. <i>Note that you can populate more regions by attaching an array to your message with the index "mandrill_template_content", using region names as indexes to the content for that region.'),
        '#options' => $regions,
        '#default_value' => isset($map->main_section) ? $map->main_section : '',
        '#required' => TRUE,
      );
    }
    $usable_keys = mandrill_template_map_usage();
    $module_names = mandrill_get_module_key_names();
    $mandrill_in_use = FALSE;
    $available_modules = FALSE;
    $mailsystem_options = array('' => t('-- None --'));
    foreach ($usable_keys as $key => $sys) {
      $mandrill_in_use = TRUE;
      if ($sys === NULL || (isset($map) && $sys == $map->mandrill_template_map_entity_id)) {
        $mailsystem_options[$key] = $module_names[$key];
        $available_modules = TRUE;
      }
    }

    if ($mandrill_in_use) {
      $form['mailsystem_key'] = array(
        '#type' => 'select',
        '#title' => t('Email key'),
        '#description' => t(
          'Select a module and mail key to use this template for outgoing email. Note that if an email has been selected in another Template Mapping, it will not appear in this list. These keys are defined through the !MailSystem interface.',
          array('!MailSystem' => l(t('MailSystem'), 'admin/config/system/mailsystem'))
        ),
        '#options' => $mailsystem_options,
        '#default_value' => isset($map->mailsystem_key) ? $map->mailsystem_key : '',
      );
      if (!$available_modules) {
        drupal_set_message(t("All email-using modules that have been assigned to Mandrill are already assigned to other template maps"), 'warning');
      }
    }

    if (!$mandrill_in_use) {
      drupal_set_message(t("You have not assigned any Modules to use Mandrill: to use this template, make sure Mandrill is assigned in Mailsystem."), 'warning');
    }
  }
  else {
    $form['email_options']['#description'] = t('The template selection is only available if the Mandrill API is correctly configured and available.');
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/services/mandrill/templates',
  );

  return $form;
}

/**
 * Validate handler for mandrill_template_map().
 */
function mandrill_template_map_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  // Test to see if mandrill_template_map value will exceed row length of 255
  // characters
  if (strlen($values['mailsystem_key']) > 255) {
    form_set_error('mailsystem_key', t('The length of the mailsytem_key name is too long.'));
  }
}

/**
 * Submit handler for mandrill_template_map(), creates the entity.
 */
function mandrill_template_map_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if ($form_state['op'] == 'add' || $form_state['op'] == 'clone') {
    $map = entity_create('mandrill_template_map', $values);
  }
  else {
    $map = $form_state['mandrill_template_map'];
    foreach ($values as $key => $val) {
      $map->{$key} = $val;
    }
  }
  $map->save();
  drupal_set_message(t('Mandrill template mapping saved.'));
  $form_state['redirect'] = 'admin/config/services/mandrill/templates';
}

/**
 * Javascript callback for the mandrill_template_map_form.
 */
function mandrill_template_map_form_callback($form, &$form_state) {
  return $form['map_settings'];
}

/**
 * Parse a Mandrill template to extract its regions.
 *
 * The Mandrill API does not provide an elegant data structure
 * containing the mc:edit tags for a given template, but rather
 * a big pile of ugly HTML containing the tags. We need to parse
 * through it.
 */
function _mandrill_parse_regions($html, $tag = 'mc:edit') {
  $instances = array();
  $offset = 0;
  $inst = NULL;
  while ($offset = strpos($html, $tag, $offset)) {
    $start = 1 + strpos($html, '"', $offset);
    $length = strpos($html, '"', $start) - $start;
    $inst = substr($html, $start, $length);
    $instances[$inst] = $inst;
    $offset = $start + $length;
  }
  return $instances;
}
