<?php

/**
 * @file
 * Integrate Wysiwyg editors into Drupal.
 */

/**
 * Form builder for Wysiwyg profile form.
 */
function wysiwyg_profile_form($form, &$form_state, $profile) {
  // Merge in defaults.
  $profile = (array) $profile;
  $profile += array(
    'format' => '',
    'editor' => '',
  );
  if (empty($profile['settings'])) {
    $profile['settings'] = array();
  }
  $settings = &$profile['settings'];
  $settings += array(
    'default' => TRUE,
    'user_choose' => FALSE,
    'show_toggle' => TRUE,
    'add_to_summaries' => TRUE,
    'theme' => '',
    'language' => 'en',
    'buttons' => array(),
    'css_setting' => 'theme',
    'css_path' => NULL,
  );
  $profile = (object) $profile;

  $formats = filter_formats();
  $editor = wysiwyg_get_editor($profile->editor);
  drupal_set_title(t('%editor profile for %format', array('%editor' => $editor['title'], '%format' => $formats[$profile->format]->name)), PASS_THROUGH);

  $form['format'] = array('#type' => 'value', '#value' => $profile->format);
  $form['input_format'] = array('#type' => 'value', '#value' => $formats[$profile->format]->name);
  $form['editor'] = array('#type' => 'value', '#value' => $profile->editor);

  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic setup'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );

  $form['basic']['default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled by default'),
    '#default_value' => $settings['default'],
    '#return_value' => 1,
    '#description' => t('The default editor state for users having access to this profile. Users are able to override this state if the next option is enabled.'),
  );

  $form['basic']['user_choose'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow users to choose default'),
    '#default_value' => $settings['user_choose'],
    '#return_value' => 1,
    '#description' => t('If allowed, users will be able to choose their own editor default state in their user account settings.'),
  );

  $form['basic']['show_toggle'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show <em>enable/disable rich text</em> toggle link'),
    '#default_value' => $settings['show_toggle'],
    '#return_value' => 1,
    '#description' => t('Whether or not to show the <em>enable/disable rich text</em> toggle link below a textarea. If disabled, the user setting or global default is used (see above).'),
  );

  $form['basic']['add_to_summaries'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable on summary fields'),
    '#default_value' => $settings['add_to_summaries'],
    '#return_value' => 1,
    '#description' => t('Attach the editor to summary fields of textareas.'),
  );

  $form['basic']['theme'] = array(
    '#type' => 'hidden',
    '#value' => $settings['theme'],
  );

  $form['basic']['language'] = array(
    '#type' => 'select',
    '#title' => t('Interface language'),
    '#default_value' => $settings['language'],
  );
  // @see _locale_prepare_predefined_list()
  require_once DRUPAL_ROOT . '/includes/iso.inc';
  $predefined = _locale_get_predefined_list();
  foreach ($predefined as $key => $value) {
    // Include native name in output, if possible
    if (count($value) > 1) {
      $tname = t($value[0]);
      $predefined[$key] = ($tname == $value[1]) ? $tname : "$tname ($value[1])";
    }
    else {
      $predefined[$key] = t($value[0]);
    }
  }
  asort($predefined);
  $form['basic']['language']['#options'] = $predefined;

  $form['buttons'] = array(
    '#type' => 'fieldset',
    '#title' => t('Buttons and plugins'),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
    '#theme' => 'wysiwyg_admin_button_table',
  );

  $plugins = wysiwyg_get_plugins($profile->editor);
  // Generate the button list.
  foreach ($plugins as $name => $meta) {
    if (isset($meta['buttons']) && is_array($meta['buttons'])) {
      foreach ($meta['buttons'] as $button => $title) {
        $icon = '';
        if (!empty($meta['path'])) {
          // @todo Button icon locations are different in editors, editor versions,
          //   and contrib/custom plugins (like Image Assist, f.e.).
          $img_src = $meta['path'] . "/images/$name.gif";
          // Handle plugins that have more than one button.
          if (!file_exists($img_src)) {
            $img_src = $meta['path'] . "/images/$button.gif";
          }
          $icon = file_exists($img_src) ? '<img src="' . base_path() . $img_src . '" title="' . $button . '" style="border: 1px solid grey; vertical-align: middle;" />' : '';
        }
        $title = (!empty($icon) ? $icon . ' ' . check_plain($title) : check_plain($title));
        $form['buttons'][$name][$button] = array(
          '#type' => 'checkbox',
          '#title' => $title,
          '#default_value' => !empty($settings['buttons'][$name][$button]) ? $settings['buttons'][$name][$button] : FALSE,
          '#description' => isset($meta['url']) ? l($meta['url'], $meta['url']) : NULL,
        );
      }
    }
    elseif (isset($meta['extensions']) && is_array($meta['extensions'])) {
      foreach ($meta['extensions'] as $extension => $title) {
        $form['buttons'][$name][$extension] = array(
          '#type' => 'checkbox',
          '#title' => check_plain($title),
          '#default_value' => !empty($settings['buttons'][$name][$extension]) ? $settings['buttons'][$name][$extension] : FALSE,
          '#description' => isset($meta['url']) ? l($meta['url'], $meta['url']) : NULL,
        );
      }
    }
  }

  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Editor appearance'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );

  $form['output'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cleanup and output'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );

  $form['css'] = array(
    '#type' => 'fieldset',
    '#title' => t('CSS'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'advanced',
  );

  $form['css']['css_setting'] = array(
    '#type' => 'select',
    '#title' => t('Editor CSS'),
    '#default_value' => $settings['css_setting'],
    '#options' => array('theme' => t('Use theme CSS'), 'self' => t('Define CSS'), 'none' => t('Editor default CSS')),
    '#description' => t('Defines the CSS to be used in the editor area.<br />Use theme CSS - loads stylesheets from current site theme.<br/>Define CSS - enter path for stylesheet files below.<br />Editor default CSS - uses default stylesheets from editor.'),
  );

  $form['css']['css_path'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS path'),
    '#default_value' => $settings['css_path'],
    '#size' => 40,
    '#maxlength' => 255,
    '#description' => t('If "Define CSS" was selected above, enter path to a CSS file or a list of CSS files separated by a comma.') . '<br />' . t('Available tokens: <code>%b</code> (base path, eg: <code>/</code>), <code>%t</code> (path to theme, eg: <code>themes/garland</code>)') . '<br />' . t('Example:') . ' css/editor.css,/themes/garland/style.css,%b%t/style.css,http://example.com/external.css',
  );

  $form['advanced'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 50,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/content/wysiwyg',
  );

  // Supply contextual information for other callbacks and handlers.
  // @todo Modernize this form for D7+ and declare these earlier.
  // $profile is the primary object of this form, and as an entity, usually
  // expected to live in $form_state[$entity_type].
  $form_state['wysiwyg_profile'] = $profile;
  $form_state['wysiwyg']['editor'] = $editor;
  $form_state['wysiwyg']['plugins'] = $plugins;

  // Allow editor library specific changes to be made to the form.
  if (isset($editor['settings form callback'])) {
    $editor['settings form callback']($form, $form_state);
  }

  foreach (array('appearance', 'output') as $fieldset) {
    if (count(element_children($form[$fieldset])) == 0) {
      $form[$fieldset]['#access'] = FALSE;
    }
  }

  return $form;
}

/**
 * Submit callback for Wysiwyg profile form.
 *
 * @see wysiwyg_profile_form()
 */
function wysiwyg_profile_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if (isset($values['buttons'])) {
    // Store only enabled buttons for each plugin.
    foreach ($values['buttons'] as $plugin => $buttons) {
      $values['buttons'][$plugin] = array_filter($values['buttons'][$plugin]);
    }
    // Store only enabled plugins.
    $values['buttons'] = array_filter($values['buttons']);
  }

  // Remove input format name.
  $format = $values['format'];
  $input_format = $values['input_format'];
  $editor = $values['editor'];
  unset($values['format'], $values['input_format'], $values['editor']);

  // Remove FAPI values.
  // @see system_settings_form_submit()
  unset($values['submit'], $values['form_id'], $values['op'], $values['form_token'], $values['form_build_id']);

  // Insert new profile data.
  db_merge('wysiwyg')
    ->key(array('format' => $format))
    ->fields(array(
      'editor' => $editor,
      'settings' => serialize($values),
    ))
    ->execute();
  wysiwyg_profile_cache_clear();

  drupal_set_message(t('Wysiwyg profile for %format has been saved.', array('%format' => $input_format)));

  $form_state['redirect'] = 'admin/config/content/wysiwyg';
}

/**
 * Layout for the buttons in the Wysiwyg Editor profile form.
 */
function theme_wysiwyg_admin_button_table($variables) {
  $form = $variables['form'];
  $buttons = array();

  // Flatten forms array.
  foreach (element_children($form) as $name) {
    foreach (element_children($form[$name]) as $button) {
      $buttons[] = drupal_render($form[$name][$button]);
    }
  }

  // Split checkboxes into rows with 3 columns.
  $total = count($buttons);
  $rows = array();
  for ($i = 0; $i < $total; $i += 3) {
    $row = array();
    $row_buttons = array_slice($buttons, $i, 3) + array_fill(0, 3, array());
    foreach ($row_buttons as $row_button) {
      $row[] = array('data' => $row_button);
    }
    $rows[] = $row;
  }

  $output = theme('table', array('rows' => $rows, 'attributes' => array('width' => '100%')));

  return $output;
}

/**
 * Display overview of setup Wysiwyg Editor profiles; menu callback.
 */
function wysiwyg_profile_overview($form, &$form_state) {
  include_once './includes/install.inc';

  // Check which wysiwyg editors are installed.
  $editors = wysiwyg_get_all_editors();

  // Sort editors by installed first, then by title.
  foreach ($editors as $key => $row) {
    $installed[$key] = $row['installed'];
    $title[$key] = drupal_strtolower($row['title']);
  }
  array_multisort($installed, SORT_DESC, $title, SORT_ASC, $editors);

  $count = count($editors);
  $status = array();
  $options = array('' => t('No editor'));

  // D7's seven theme displays links in table headers as block elements.
  drupal_add_css('table.system-status-report th a {display: inline;}', 'inline');

  foreach ($editors as $name => $editor) {
    $status[$name] = array(
      'severity' => (isset($editor['error']) ? REQUIREMENT_ERROR : ($editor['installed'] ? REQUIREMENT_OK : REQUIREMENT_INFO)),
      'title' => t('<a href="!vendor-url">@editor</a> (<a href="!download-url">Download</a>)', array('!vendor-url' => $editor['vendor url'], '@editor' => $editor['title'], '!download-url' => $editor['download url'])),
      'value' => (isset($editor['installed version']) ? $editor['installed version'] : t('Not installed.')),
      'description' => (isset($editor['error']) ? $editor['error'] : ''),
    );
    if ($editor['installed']) {
      $options[$name] = $editor['title'] . (isset($editor['installed version']) ? ' ' . $editor['installed version'] : '');
    }
    else {
      // Build on-site installation instructions.
      // @todo Setup $library in wysiwyg_load_editor() already.
      $library = (isset($editor['library']) ? $editor['library'] : key($editor['libraries']));
      $targs = array(
        '@editor-path' => $editor['editor path'],
        '@library-filepath' => $editor['library path'] . '/' . (isset($editor['libraries'][$library]['files'][0]) ? $editor['libraries'][$library]['files'][0] : key($editor['libraries'][$library]['files'])),
      );
      $instructions = '<p>' . t('Extract the archive and copy its contents into a new folder in the following location:<br /><code>@editor-path</code>', $targs) . '</p>';
      $instructions .= '<p>' . t('So the actual library can be found at:<br /><code>@library-filepath</code>', $targs) . '</p>';

      // Add any install notes.
      if (!empty($editor['install note callback']) && function_exists($editor['install note callback'])) {
        $instructions .= '<div class="editor-install-note">' . $editor['install note callback']() . '</div>';
      }

      $status[$name]['description'] .= $instructions;
      $count--;
    }
    // In case there is an error, always show installation instructions.
    if (isset($editor['error'])) {
      $show_instructions = TRUE;
    }
  }
  if (!$count) {
    $show_instructions = TRUE;
  }
  $form['status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Installation instructions'),
    '#collapsible' => TRUE,
    '#collapsed' => !isset($show_instructions),
    '#description' => (!$count ? t('There are no editor libraries installed currently. The following list contains a list of currently supported editors:') : ''),
    '#weight' => 10,
  );
  $form['status']['report'] = array('#markup' => theme('status_report', array('requirements' => $status)));

  if (!$count) {
    return $form;
  }

  $formats = filter_formats();
  $profiles = wysiwyg_profile_load_all();
  $form['formats'] = array(
    '#type' => 'item',
    '#description' => t('To assign a different editor to a text format, click "delete" to remove the existing first.'),
    '#tree' => TRUE,
  );

  $enable_save = FALSE;
  foreach ($formats as $id => $format) {
    $form['formats'][$id]['name'] = array(
      '#markup' => check_plain($format->name),
    );
    // Only display editor selection for associated input formats to avoid
    // confusion about disabled selection.
    if (isset($profiles[$id]) && !empty($profiles[$id]->editor)) {
      $editor_name = $profiles[$id]->editor;
      $installed = !empty($editors[$editor_name]['installed']);
      $form['formats'][$id]['editor'] = array(
        '#wysiwyg-editor-name' => $editor_name,
      );
      if ($installed) {
        $form['formats'][$id]['editor']['#markup'] = $options[$editor_name];
      }
      else {
        drupal_set_message(t('Missing %editor library for %format format. Re-install the %editor library or delete the editor profile.', array(
          '%editor' => $editors[$editor_name]['title'],
          '%format' => $format->name,
        )), 'warning');
      }
    }
    else {
      $form['formats'][$id]['editor'] = array(
        '#title' => t('Select an editor'),
        '#title_display' => 'invisible',
        '#type' => 'select',
        '#default_value' => '',
        '#options' => $options,
      );
      $enable_save = TRUE;
    }
    if (isset($profiles[$id]) && !empty($profiles[$id]->editor)) {
      $form['formats'][$id]['edit'] = array(
        '#markup' => l(t('Edit'), "admin/config/content/wysiwyg/profile/$id/edit"),
      );
      $form['formats'][$id]['delete'] = array(
        '#markup' => l(t('Delete'), "admin/config/content/wysiwyg/profile/$id/delete"),
      );
    }
  }

  // Submitting the form when no editors can be selected causes errors.
  if ($enable_save) {
    $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  }
  return $form;
}

/**
 * Return HTML for the Wysiwyg profile overview form.
 */
function theme_wysiwyg_profile_overview($variables) {
  $form = $variables['form'];
  if (!isset($form['formats'])) {
    return;
  }
  $editors = wysiwyg_get_all_editors();
  $output = '';
  $header = array(t('Text format'), t('Editor'), array('data' => t('Operations'), 'colspan' => 2));
  $rows = array();
  foreach (element_children($form['formats']) as $item) {
    $format = &$form['formats'][$item];
    $row = array(
      'data' => array(
        drupal_render($format['name']),
        drupal_render($format['editor']),
        isset($format['edit']) ? drupal_render($format['edit']) : '',
        isset($format['delete']) ? drupal_render($format['delete']) : '',
      ),
    );
    if (empty($row['data'][1])) {
      $row['data'][1] = array(
        'data' => t('Missing library: @library', array('@library' => $editors[$format['editor']['#wysiwyg-editor-name']]['title'])),
        'class' => 'error',
      );
      $row['class'] = array('error');
    }
    $rows[] = $row;
  }
  $form['formats']['table']['#markup'] = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Submit callback for Wysiwyg profile overview form.
 */
function wysiwyg_profile_overview_submit($form, &$form_state) {
  foreach ($form_state['values']['formats'] as $format => $values) {
    db_merge('wysiwyg')
      ->key(array('format' => $format))
      ->fields(array(
        'editor' => $values['editor'],
      ))
      ->execute();
  }
  wysiwyg_profile_cache_clear();
  if (module_exists('admin_menu')) {
    admin_menu_flush_caches();
  }
}

/**
 * Delete editor profile confirmation form.
 */
function wysiwyg_profile_delete_confirm($form, &$form_state, $profile) {
  $formats = filter_formats();
  $format = $formats[$profile->format];
  $form['format'] = array('#type' => 'value', '#value' => $format);
  return confirm_form(
    $form,
    t('Are you sure you want to remove the profile for %name?', array('%name' => $format->name)),
    'admin/config/content/wysiwyg',
    t('This action cannot be undone.'), t('Remove'), t('Cancel')
  );
}

/**
 * Submit callback for Wysiwyg profile delete form.
 *
 * @see wysiwyg_profile_delete_confirm()
 */
function wysiwyg_profile_delete_confirm_submit($form, &$form_state) {
  $format = $form_state['values']['format'];
  wysiwyg_profile_delete($format->format);
  wysiwyg_profile_cache_clear();
  if (module_exists('admin_menu')) {
    admin_menu_flush_caches();
  }

  drupal_set_message(t('Wysiwyg profile for %name has been deleted.', array('%name' => $format->name)));
  $form_state['redirect'] = 'admin/config/content/wysiwyg';
}
