<?php

/**
 * @file
 * Rating admin
 */

/**
 * Menu callback.
 *
 * Generates the admin page which contains the widget listing.
 */
function rate_admin_page() {
  $widgets = variable_get(RATE_VAR_WIDGETS, array());
  uasort($widgets, '_rate_sort');

  $header = array(t('Title'), t('Name'), t('Operations'));
  $rows = array();
  foreach ($widgets as $id => $widget) {
    $edit = l(t('Edit'), str_replace('%', $id, RATE_PATH_ADMIN_PAGE_EDIT));
    $delete = l(t('Delete'), str_replace('%', $id, RATE_PATH_ADMIN_PAGE_DELETE));
    $rows[] = array(
      $widget->title,
      $widget->name,
      "$edit $delete",
    );
  }

  $output = '';
  $output .= '<h3>' . t('Add widget') . '</h3>';
  $add_form = drupal_get_form('rate_choose_template_form');
  $output .= drupal_render($add_form);
  if ($rows) {
    $output .= theme('table', array('header' => $header, 'rows' => $rows));
  }
  else {
    $text = t('You do not have any rate widgets defined yet.');
    $output .= '<p>' . check_plain($text) . '</p>';
  }

  return $output;
}

/**
 * Callback for uasort().
 *
 * @see rate_admin_page()
 * @see http://php.net/uasort
 */
function _rate_sort($a, $b) {
  if ($a->title == $b->title) {
    return 0;
  }
  return $a->title < $b->title ? -1 : 1;
}

/**
 * Form for choosing a template before the user goes to the add widget form.
 */
function rate_choose_template_form($form, &$form_state, $id = NULL) {
  if ($id) {
    $form['#widget_id'] = $id;
    $widgets = variable_get(RATE_VAR_WIDGETS, array());
    $widget = $widgets[$id];
    isset($widget->template) or $widget->template = 'custom';
    $selected = $widget->template;
  }
  else {
    $selected = 'custom';
  }

  $options = array('custom' => t('Custom'));
  foreach (module_implements('rate_templates') as $module) {
    foreach (module_invoke($module, 'rate_templates') as $name => $widget) {
      $options[$name] = $widget->template_title;
    }
  }
  $form['template'] = array(
    '#type' => 'select',
    '#title' => t('Widget type'),
    '#options' => $options,
    '#default_value' => $selected,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * Form submit callback for the choose_template_form.
 */
function rate_choose_template_form_submit($form, &$form_state) {
  $template = $form_state['values']['template'];
  $query = array('template' => $template);
  if (isset($form['#widget_id'])) {
    /**
     * @todo: Copy over the options array from the template to the widget.
     */
    $path = str_replace('%', $form['#widget_id'], RATE_PATH_ADMIN_PAGE_EDIT);
  }
  else {
    $path = RATE_PATH_ADMIN_PAGE_ADD;
  }
  drupal_goto($path, array('query' => $query));
}

/**
 * Form for adding and editing widgets.
 */
function rate_widget_form($form, &$form_state, $id = NULL) {
  // Workaround core issue #591696 in AHAH forms.
  // See http://drupal.org/node/591696#comment-3369036 for details.
  if ($id) {
    $form['#action'] = url(str_replace('%', $id, RATE_PATH_ADMIN_PAGE_EDIT));
  }
  else {
    $form['#action'] = url(RATE_PATH_ADMIN_PAGE_ADD);
  }

  if ($id) {
    $widgets = variable_get(RATE_VAR_WIDGETS, array());
    if (!isset($widgets[$id])) {
      drupal_not_found();
      module_invoke_all('exit') && exit;
    }
    $widget = $widgets[$id];
  }
  else {
    $widget = new stdClass();
    $widget->name = '';
    $widget->tag = 'vote';
    $widget->title = '';
    $widget->node_types = array();
    $widget->comment_types = array();
    $widget->value_type = 'percent';
    $widget->options = array();
    $widget->template = 'custom';
    $widget->node_display = RATE_DISPLAY_BELOW_CONTENT;
    $widget->teaser_display = FALSE;
    $widget->node_display_mode = RATE_FULL;
    $widget->teaser_display_mode = RATE_FULL;
    $widget->comment_display_mode = RATE_FULL;
    $widget->comment_display = RATE_DISPLAY_BELOW_CONTENT;
    $widget->roles = array();
    $widget->allow_voting_by_author = TRUE;
    $widget->noperm_behaviour = RATE_NOPERM_REDIRECT_WITH_MESSAGE;
    $widget->displayed = RATE_AVERAGE;
    $widget->displayed_just_voted = RATE_USER;
    $widget->description = '';
    $widget->description_in_compact = TRUE;
    $widget->delete_vote_on_second_click = 0;
    $widget->use_source_translation = TRUE;
  }
  
  // Determine the template from GET (which is only available before the form
  // was post) or from post values (only available after the form was posted).
  if (isset($form_state['post']['widget_template'])) {
    $widget->template = $form_state['post']['widget_template'];
  }
  elseif (isset($_GET['template'])) {
    // The template can be overridden (also used for changing templates).
    $widget->template = $_GET['template'];
  }

  // Check widget for missing values (legacy).
  _rate_check_widget($widget);

  if ($widget->template != 'custom') {
    if (!$template = _rate_get_template($widget->template)) {
      $form['error'] = array(
        '#markup' => t('You cannot edit this widget because it was built using the template %template which does not exists anymore.', array('%template' => $widget->template)),
      );
      return $form;
    }
    $widget->customizable = isset($template->customizable) ? $template->customizable : TRUE;
    if ($widget->customizable && !$id) {
      $widget->options = $template->options;
      $widget->value_type = $template->value_type;
    }
  }
  
  if ($id) {
    $form['#widget_id'] = $id;
  }

  // The GET value for the template is not available anymore after posting,
  // so we need to add the template as a hidden value.
  $form['widget_template'] = array(
    '#type' => 'hidden',
    '#default_value' => $widget->template,
  );
  
  if ($id) {
    $link = l(t('change the widget type'), str_replace('%', $id, RATE_PATH_ADMIN_PAGE_TEMPLATE));
    if ($widget->template == 'custom') {
      $text = t('This is a custom widget. You may !change.', array('!change' => $link));
    }
    else {
      $template = _rate_get_template($widget->template);
      $text = t('This is a %type widget. You may !change.', array('%type' => $template->template_title, '!change' => $link));
    }
    $text = filter_xss($text);
    $form['template'] = array(
      '#type' => 'fieldset',
      '#title' => t('Widget type'),
    );
    $form['template']['switch'] = array(
      '#markup' => $text,
    );
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $widget->title,
    '#required' => TRUE,
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine readable name'),
    '#default_value' => $widget->name,
    '#required' => TRUE,
    '#machine_name' => array(
      'exists' => 'rate_machine_name_exists',
      'source' => array('title'),
    ),
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('Tag'),
    '#default_value' => $widget->tag,
    '#description' => t('Tag used for VotingAPI. Widgets with the same tag share their voting results.'),
    '#required' => TRUE,
  );
  
  if ($widget->customizable) {
    $options = array(
      'percent' => t('Percentage'),
      'points' => t('Points'),
      'option' => t('Options'),
    );
    $form['value_type'] = array(
      '#type' => 'radios',
      '#title' => t('Value type'),
      '#options' => $options,
      '#default_value' => $widget->value_type,
      '#required' => TRUE,
    );

    $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Options'),
      '#description' => t('Define the available voting buttons.'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form['options']['options'] = array(
      '#theme' => 'rate_admin_options',
      '#prefix' => "<div id=\"rate-options\">",
      '#suffix' => '</div>',
    );

    if ($form_state['submitted']) {
      $option_count = (int) $form_state['values']['option_count'];
      if ($form_state['triggering_element']['#value'] == t('Add another option')) {
        ++$option_count;
      }
    }
    else {
      $option_count = max(2, count($widget->options));
    }

    $id = 0;
    $c = $option_count;
    for ($i = 0; $i < $c; $i++) {
      $form['options']['options']['option' . $id] = array();
      if (isset($form_state['values']['value' . $i])) {
        $default_value = $form_state['values']['value' . $i];
        $default_label = $form_state['values']['label' . $i];
      }
      elseif (isset($widget->options[$i]) && !$form_state['submitted']) {
        $default_value = $widget->options[$i][0];
        $default_label = $widget->options[$i][1];
      }
      else {
        $default_value = '';
        $default_label = '';
      }
      $form['options']['options']['option' . $id]['value' . $id] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#default_value' => $default_value,
        '#size' => 16,
      );
      $form['options']['options']['option' . $id]['label' . $id] = array(
        '#type' => 'textfield',
        '#title' => t('Label'),
        '#default_value' => $default_label,
        '#size' => 40,
      );
      $form['options']['options']['option' . $id]['delete' . $id] = array(
        '#type' => 'checkbox',
        '#title' => t('Delete'),
        '#default_value' => FALSE,
      );
      ++$id;
    }

    $form['options']['option_count'] = array(
      '#type' => 'hidden',
      '#value' => $option_count,
    );

    $form['options']['add_another'] = array(
      '#type' => 'submit',
      '#value' => t('Add another option'),
      '#ajax' => array(
        'callback' => 'rate_widget_form_ajax',
        'wrapper' => 'rate-options',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    );

    $form['translate'] = array(
      '#type' => 'checkbox',
      '#title' => t('Translate options'),
      '#default_value' => $widget->translate,
    );
  }

  $form['types'] = array(
    '#type' => 'fieldset',
    '#title' => t('Node types'),
    '#description' => t('Select the node types on which to enable this widget.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#theme' => 'rate_admin_types',
  );
  foreach (node_type_get_names() as $type_name => $type_title) {
    $form['types'][$type_name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($type_title),
    );
    $form['types'][$type_name]['node_type_' . $type_name] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($type_title),
      '#default_value' => in_array($type_name, $widget->node_types),
    );
    $form['types'][$type_name]['comment_type_' . $type_name] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($type_title),
      '#default_value' => in_array($type_name, $widget->comment_types),
    );
  }

  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $display_options = array(
    RATE_DISPLAY_DISABLE => t('Do not add automatically'),
    RATE_DISPLAY_ABOVE_CONTENT => t('Above the content'),
    RATE_DISPLAY_BELOW_CONTENT => t('Below the content'),
  );
  $form['display']['node_display'] = array(
    '#type' => 'radios',
    '#title' => t('Node display'),
    '#options' => $display_options,
    '#default_value' => $widget->node_display,
  );
  $form['display']['teaser_display'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display in teaser'),
    '#default_value' => $widget->teaser_display,
  );

  $display_mode_options = array(
    RATE_FULL => t('Full widget'),
    RATE_DISABLED => t('Display only'),
    RATE_COMPACT_DISABLED => t('Display only, compact'),
    RATE_COMPACT => t('Compact'),
  );
  $form['display']['node_display_mode'] = array(
    '#type' => 'select',
    '#title' => t('Appearance in full node'),
    '#options' => $display_mode_options,
    '#default_value' => $widget->node_display_mode,
  );
  $form['display']['teaser_display_mode'] = array(
    '#type' => 'select',
    '#title' => t('Appearance in teaser'),
    '#options' => $display_mode_options,
    '#default_value' => $widget->teaser_display_mode,
  );

  $form['display']['comment_display'] = array(
    '#type' => 'radios',
    '#title' => t('Comment display'),
    '#options' => $display_options,
    '#default_value' => $widget->comment_display,
  );
  $form['display']['comment_display_mode'] = array(
    '#type' => 'select',
    '#title' => t('Appearance in comments'),
    '#options' => $display_mode_options,
    '#default_value' => $widget->comment_display_mode,
  );
  $form['display']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => $widget->description,
    '#description' => t('Optional description which will be visible on the rate widget.'),
  );
  $form['display']['description_in_compact'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display in compact mode'),
    '#default_value' => $widget->description_in_compact,
  );

  $form['interaction'] = array(
    '#type' => 'fieldset',
    '#title' => t('Interaction'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Note that these settings do not apply for rate widgets inside views. Widgets in views will display the average voting when a relationship to the voting results is used and the users vote in case of a relationship to the votes.'),
  );
  
  $options = array(
    RATE_AVERAGE => t('Average rating'),
    RATE_USER => t('Users vote if available, empty otherwise'),
    RATE_USER_OR_AVERAGE => t('Users vote if available, average otherwise'),
  );
  $form['interaction']['displayed'] = array(
    '#type' => 'radios',
    '#title' => t('Which rating should be displayed?'),
    '#options' => $options,
    '#default_value' => $widget->displayed,
  );
  $options = array(
    RATE_AVERAGE => t('Average rating'),
    RATE_USER => t('Users vote'),
  );

  $form['interaction']['displayed_just_voted'] = array(
    '#type' => 'radios',
    '#title' => t('Which rating should be displayed when the user just voted?'),
    '#options' => $options,
    '#default_value' => $widget->displayed_just_voted,
  );

  $click_options = array(
    0 => t('No'),
    1 => t('Yes'),
  );
  $form['interaction']['delete_vote_on_second_click'] = array(
    '#type' => 'radios',
    '#title' => t('Should a second click on the same button delete the vote?'),
    '#options' => $click_options,
    '#default_value' => $widget->delete_vote_on_second_click,
    '#description' => t('Example: the Facebook "Like" button.'),
  );

  $form['permissions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Permissions'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $res = db_select('role', 'r')
    ->fields('r', array('rid', 'name'))
    ->orderBy('name', 'asc')
    ->execute();
  $options = array();
  foreach ($res as $rec) {
    $options[$rec->rid] = $rec->name;
  }
  $form['permissions']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles'),
    '#options' => $options,
    '#default_value' => $widget->roles,
    '#description' => t('Allow voting only for the selected role(s). If you select no roles, all users are allowed to vote.'),
  );
  $form['permissions']['allow_voting_by_author'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow author to rate his / her own content'),
    '#default_value' => $widget->allow_voting_by_author,
    '#description' => t('Will change the state to disabled. Always true for anonymous users.'),
  );
  $options = array(
    RATE_NOPERM_REDIRECT_WITH_MESSAGE => t('Redirect to login and show message'),
    RATE_NOPERM_REDIRECT_WITHOUT_MESSAGE => t('Redirect to login but do not show a message'),
    RATE_NOPERM_SHOW_DISABLED_WIDGET => t('Show a disabled widget (with non clickable buttons)'),
    RATE_NOPERM_HIDE_WIDGET => t('Hide widget'),
  );
  $form['permissions']['noperm_behaviour'] = array(
    '#type' => 'radios',
    '#title' => t('Behaviour when user has no permission to vote'),
    '#options' => $options,
    '#default_value' => $widget->noperm_behaviour,
    '#description' => t('Choose an action what will happen if a user clicks on the widget but doesn\'t have the permission to vote.'),
  );
  
  if (module_exists('translation')) {
    $form['translations'] = array(
      '#type' => 'fieldset',
      '#title' => t('Translations'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['translations']['use_source_translation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use source translation'),
      '#default_value' => $widget->use_source_translation,
      '#description' => t('Save votes to the source translation. When this option is checked, every translation will show the same rating. Uncheck to separate rating per language.'),
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 50,
  );

  return $form;
}

/**
 * Form validate.
 */
function rate_widget_form_validate($form, &$form_state) {
  // Validate machine readable name
  if (!preg_match('/^[a-z\\_0-9]+$/i', $form_state['values']['name'])) {
    form_set_error('name', t('The machine readable name may only contain alphanumeric characters and underscores.'));
  }

  // Validate vote tag
  if (!preg_match('/^[a-z\\_0-9]+$/i', $form_state['values']['tag'])) {
    form_set_error('tag', t('The vote tag may only contain alphanumeric characters and underscores.'));
  }
  
  $customizable = TRUE;
  if ($form_state['values']['widget_template'] != 'custom') {
    if ($template = _rate_get_template($form_state['values']['widget_template'])) {
      $customizable = isset($template->customizable) ? $template->customizable : TRUE;
    }
  }

  if ($customizable) {
    // Validate option values
    $used_values = array();
    foreach ($form_state['values'] as $name => $value) {
      if (preg_match('/^value([0-9]+)$/', $name, $match)) {
        if (empty($value)) {
          continue;
        }
        if (!preg_match('/^\\-?[0-9]+$/', $value)) {
          form_set_error($name, t('Values must be integers'));
          continue;
        }
        if (in_array($value, $used_values)) {
          form_set_error($name, t('You may not use the same value twice.'));
        }
        $used_values[] = $value;
        if ($form_state['values']['value_type'] == 'percent') {
          if ($value < 0 || $value > 100) {
            form_set_error($name, t('Percentages must be between 0 and 100'));
          }
        }
      }
    }
    if (!count($used_values)) {
      form_set_error('', t('Each widget must have at least one option.'));
    }
  }
}

/**
 * Form submit handler.
 */
function rate_widget_form_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Add another option')) {
    $form_state['rebuild'] = TRUE;
    return $form_state['values'];
  }

  $widget = new stdClass();
  $widget->name = $form_state['values']['name'];
  $widget->tag = filter_xss($form_state['values']['tag']);
  $widget->title = filter_xss($form_state['values']['title']);
  $widget->node_types = array();
  $widget->comment_types = array();
  $widget->options = array();
  $widget->template = $form_state['values']['widget_template'];
  $widget->node_display = $form_state['values']['node_display'];
  $widget->teaser_display = (bool) $form_state['values']['teaser_display'];
  $widget->comment_display = $form_state['values']['comment_display'];
  $widget->node_display_mode = $form_state['values']['node_display_mode'];
  $widget->teaser_display_mode = $form_state['values']['teaser_display_mode'];
  $widget->comment_display_mode = $form_state['values']['comment_display_mode'];
  $widget->roles = $form_state['values']['roles'];
  $widget->allow_voting_by_author = $form_state['values']['allow_voting_by_author'];
  $widget->noperm_behaviour = $form_state['values']['noperm_behaviour'];
  $widget->displayed = $form_state['values']['displayed'];
  $widget->displayed_just_voted = $form_state['values']['displayed_just_voted'];
  $widget->description = $form_state['values']['description'];
  $widget->description_in_compact = (bool) $form_state['values']['description_in_compact'];
  $widget->delete_vote_on_second_click = $form_state['values']['delete_vote_on_second_click'];
  if (module_exists('translation')) {
    $widget->use_source_translation = !empty($form_state['values']['use_source_translation']);
  }
  else {
    $widget->use_source_translation = TRUE;
  }
  
  if ($widget->template != 'custom') {
    // Take over the values from the template.
    $template = _rate_get_template($widget->template);
    if (!$template->customizable) {
      $widget->options = $template->options;
    }
    $widget->value_type = $template->value_type;
    !isset($template->theme) or $widget->theme = $template->theme;
    !isset($template->css) or $widget->css = $template->css;
    !isset($template->js) or $widget->js = $template->js;
    $widget->translate = $template->translate;
    if (isset($form_state['values']['translate'])) {
      $widget->translate = (bool) $form_state['values']['translate'];
    }
  }
  else {
    $widget->value_type = $form_state['values']['value_type'];
    $widget->translate = (bool) $form_state['values']['translate'];
  }

  foreach ($form_state['values'] as $name => $value) {
    if (preg_match('/^value([0-9]+)$/', $name, $match) && preg_match('/^\\-?[0-9]+$/', $value)) {
      if (!empty($form_state['values']['delete' . $match[1]])) {
        // The delete option is checked.
        continue;
      }
      $widget->options[] = array(
        $value,
        filter_xss($form_state['values']['label' . $match[1]]),
      );
    }
    if (preg_match('/^node_type_(.+)$/', $name, $match) && $value) {
      $widget->node_types[] = $match[1];
    }
    if (preg_match('/^comment_type_(.+)$/', $name, $match) && $value) {
      $widget->comment_types[] = $match[1];
    }
  }

  // Let other modules alter the rate widget.
  $hook = 'rate_widget_' . (empty($form['#widget_id']) ? 'insert' : 'update');
  module_invoke_all($hook, $widget, $form_state['values']);

  // Save the widget.
  $widgets = variable_get(RATE_VAR_WIDGETS, array());
  if (!empty($form['#widget_id'])) {
    $id = (int) $form['#widget_id'];
  }
  else {
    $id = 1;
    while (isset($widgets[$id])) {
      ++$id;
    }
  }
  $widgets[$id] = $widget;
  variable_set(RATE_VAR_WIDGETS, $widgets);
  drupal_set_message(t('The widget has been saved.'));

  $form_state['redirect'] = RATE_PATH_ADMIN_PAGE;
}

/**
 * AHAH callback for widget form.
 */
function rate_widget_form_ajax($form, $form_state) {
  return $form['options']['options'];
}

/**
 * Theming function for the node / comment types list in the widget form.
 *
 * @ingroup themeable
 */
function theme_rate_admin_types($variables) {
  $element = $variables['element'];

  $header = array(
    t('Name'),
    t('Node'),
    t('Comment'),
  );
  $rows = array();
  foreach ($element as $name => $subelement) {
    if ($name[0] != '#') {
      unset($subelement['node_type_' . $name]['#title']);
      unset($subelement['comment_type_' . $name]['#title']);
      $rows[] = array(
        check_plain($subelement['#title']),
        drupal_render($subelement['node_type_' . $name]),
        drupal_render($subelement['comment_type_' . $name]),
      );
    }
  }
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Theme the options list in the widget form.
 *
 * @ingroup themeable
 */
function theme_rate_admin_options($variables) {
  $element = $variables['element'];

  $header = array(
    t('Value'),
    t('Label'),
    t('Delete'),
  );
  $rows = array();
  foreach ($element as $name => $subelement) {
    if (preg_match('/^option([0-9]+)$/', $name, $match)) {
      $id = $match[1];
      unset($subelement['value' . $id]['#title']);
      unset($subelement['label' . $id]['#title']);
      unset($subelement['delete' . $id]['#title']);
      $rows[] = array(
        drupal_render($subelement['value' . $id]),
        drupal_render($subelement['label' . $id]),
        drupal_render($subelement['delete' . $id]),
      );
    }
  }
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form for deleting widgets.
 */
function rate_widget_delete_form($form, &$form_state, $id) {
  $form = array();

  $widgets = variable_get(RATE_VAR_WIDGETS, array());

  $form['#widget_id'] = $id;

  if (!isset($widgets[$id])) {
    drupal_not_found();
    module_invoke_all('exit') && exit;
  }

  $title = $widgets[$id]->title;

  $form['info'] = array(
    '#markup' => '<p>' . t('Are you sure you want to delete widget %title?', array('%title' => $title)) . '</p>'
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  $form['buttons']['cancel'] = array(
    '#markup' => l(t('Cancel'), RATE_PATH_ADMIN_PAGE),
  );

  return $form;
}

/**
 * Submit handler.
 */
function rate_widget_delete_form_submit($form, &$form_state) {
  $widgets = variable_get(RATE_VAR_WIDGETS, array());
  if (isset($widgets[$form['#widget_id']])) {
    $widget = $widgets[$form['#widget_id']];

    unset($widgets[$form['#widget_id']]);
    variable_set(RATE_VAR_WIDGETS, $widgets);

    drupal_set_message(t('Widget %title has been deleted.', array('%title' => $widget->title)));

    // Let other modules act on this action.
    module_invoke_all('rate_widget_delete', $widget);
  }
  $form_state['redirect'] = RATE_PATH_ADMIN_PAGE;
}

/**
 * Admin settings form.
 */
function rate_settings_form($form, &$form_state) {
  
  $form['bot'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bot detection'),
    '#description' => t('Bots can be automatically banned from voting if they rate more than a given amount of votes within one minute or hour. This threshold is configurable below. Votes from the same IP-address will be ignored forever after reaching this limit.'),
    '#collapsbile' => FALSE,
    '#collapsed' => FALSE,
  );
  
  $options = drupal_map_assoc(array(0, 10, 25, 50, 100, 250, 500, 1000));
  $options[0] = t('disable');
  
  $form['bot'][RATE_VAR_BOT_MINUTE_THRESHOLD] = array(
    '#type' => 'select',
    '#title' => t('1 minute threshold'),
    '#options' => $options,
    '#default_value' => variable_get(RATE_VAR_BOT_MINUTE_THRESHOLD, 25),
  );
  
  $form['bot'][RATE_VAR_BOT_HOUR_THRESHOLD] = array(
    '#type' => 'select',
    '#title' => t('1 hour threshold'),
    '#options' => $options,
    '#default_value' => variable_get(RATE_VAR_BOT_HOUR_THRESHOLD, 250),
  );
  
  /* Feature planned for 1.5 release
  $form['bot'][RATE_VAR_BOT_RETROACTIVE] = array(
    '#type' => 'checkbox',
    '#title' => t('Retroactive delete votes'),
    '#default_value' => variable_get(RATE_VAR_BOT_RETROACTIVE, TRUE),
    '#description' => t('The bot detection is a retrospective process. When enabled all votes done from the bots IP will be deleted when the bot hits the threshold.'),
  );
  */
  
  $form['bot'][RATE_VAR_BOT_BOTSCOUT_KEY] = array(
    '#type' => 'textfield',
    '#title' => t('BotScout.com API key'),
    '#default_value' => variable_get(RATE_VAR_BOT_BOTSCOUT_KEY, ''),
    '#description' => t('Rate will check the voters IP against the BotScout database if it has an API key. You can request a key at %url.', array('%url' => 'http://botscout.com/getkey.htm')),
  );
  
  return system_settings_form($form);
}

/**
 * Form validate callback for the settings form.
 */
function rate_settings_form_validate($form, &$form_state) {
  if (!empty($form_state['values'][RATE_VAR_BOT_BOTSCOUT_KEY])) {
    $url = "http://botscout.com/test/?ip=84.16.230.111&key=" . $form_state['values'][RATE_VAR_BOT_BOTSCOUT_KEY];
    $data = drupal_http_request($url, array('timeout' => 5));
    if ($data->code == 200) {
      if ($data->data{0} == 'Y' || $data->data{0} == 'N') {
        drupal_set_message(t('Rate has succesfully contacted the BotScout server.'));
      }
      else {
        form_set_error(RATE_VAR_BOT_BOTSCOUT_KEY, t('Invalid API-key.'));
      }
    }
    else {
      form_set_error(RATE_VAR_BOT_BOTSCOUT_KEY, t('Rate was unable to contact the BotScout server.'));
    }
  }
}
