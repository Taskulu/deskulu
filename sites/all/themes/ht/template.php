<?php

/**
 * @file
 * template.php
 */

/**
 * Implements hook_preprocess_page().
 *
 * @see page.tpl.php
 */
function ht_preprocess_page(&$variables) {
  // Add information about the number of sidebars.
  if (!empty($variables['page']['sidebar'])) {
    $variables['content_column_class'] = ' class="col-sm-8"';
  }
  else {
    $variables['content_column_class'] = ' class="col-sm-12"';
  }

  $variables['actions_nav'] = FALSE;
  $variables['actions_nav'] = i18n_menu_translated_tree('menu-actions');
  $children = element_children($variables['actions_nav']);
  foreach($children as $index => $id) {
    $classes = ['btn', 'navbar-btn'];
    if ($index == count($children) - 1) {
      $classes[] = 'btn-primary';
    }
    else {
      $classes[] = 'btn-info';
    }
    $variables['actions_nav'][$id]['#localized_options']['attributes']['class'] = $classes;
  }
  $variables['actions_nav']['#theme_wrappers'] = array('menu_tree__actions');

  if (isset($variables['node'])) {
    $variables['theme_hook_suggestions'][] = 'page__node_' . $variables['node']->type;
  }
}


function ht_theme($existing, $type, $theme, $path) {
  $items['ticket_node_form'] = array(
    'render element' => 'form',
    'template' => 'node-form--ticket',
    'path' => drupal_get_path('theme', 'ht') . '/templates/forms',
  );

  $items['helpdesk_new_ticket_form'] = array(
    'render element' => 'form',
    'template' => 'ticket-form',
    'path' => drupal_get_path('theme', 'ht') . '/templates/forms',
  );
  return $items;
}

function ht_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'views_exposed_form') {
    if ($form['#id'] == 'views-exposed-form-search-page') {
      $form['search_api_views_fulltext']['#attributes']['placeholder'] = t('Search for articles and topics');
      $form['#attributes']['class'][] = 'navbar-form';
      $form['submit']['#value'] = '<i class="glyphicon glyphicon-search"></i>';
      $form['submit']['#attributes']['class'][] = 'btn-white';
    }
    elseif ($form['#id'] == 'views-exposed-form-advanced-forum-topic-list-default') {
      foreach ($form['field_feature_request_status_value']['#options'] as &$option) {
        $option = t($option);
      }
    }
  }
  elseif($form_id == 'comment_node_ticket_form') {
    $form['author']['_author']['#title'] = '';
    $form['author']['_author']['#type'] = 'markup';

    global $user;

    $form['author']['_author']['#markup'] = theme('user_picture', array('account' => $user));
    $author = user_load($form['#node']->uid);
    $form['actions']['submit']['#value'] = t('Send');
    $form['actions']['submit']['#attributes']['class'][] = 'btn-primary';
    $links = theme('links', [
      'links' => [
        'cc' => [
          'title' => t('CC'),
          'href' => '#',
        ],
        'bcc' => [
          'title' => t('BCC'),
          'href' => '',
        ],
      ],
      'attributes' => ['class' => ['list-inline']],
    ]);
    $form['form_title'] = array(
      '#markup' => '<div class="sender-name">' . t('Reply to: <span class="name">!name &lt;@email&gt;</span>', ['!name' => theme('username', ['account' => $author]), '@email' => $author->mail]) . '<div class="pull-right flip">' . $links . '</div></div>',
      '#weight' => -100,
    );

    $form['from'] = array(
      '#markup' => '<div class="sender-name">' . t('Reply From: <span class="name">!name &lt;@email&gt;</span>', ['!name' => theme('username', ['account' => $user]), '@email' => $user->mail]) . '</div>',
      '#weight' => -101,
    );

    $form['field_email_cc'][LANGUAGE_NONE][0]['value']['#attributes']['placeholder'] = t('Comma separated list of emails');
    $form['field_email_cc'][LANGUAGE_NONE][0]['value']['#prefix'] = '<div id="field_email_cc_wrapper" class="hidden">';
    $form['field_email_cc'][LANGUAGE_NONE][0]['value']['#suffix'] = '</div>';

    $form['field_email_bcc'][LANGUAGE_NONE][0]['value']['#attributes']['placeholder'] = t('Comma separated list of emails');
    $form['field_email_bcc'][LANGUAGE_NONE][0]['value']['#prefix'] = '<div id="field_email_bcc_wrapper" class="hidden">';
    $form['field_email_bcc'][LANGUAGE_NONE][0]['value']['#suffix'] = '</div>';
    $form['#attached']['js'] = array(
      drupal_get_path('theme', 'ht') . '/js/ticket-reply-form.js',
    );
    $form['comment_body'][$form['language']['#value']][0]['#title'] = NULL;
  }

}

function ht_user_is_agent($user) {
  $roles = array_intersect($user->roles, array('administrator', 'agent'));
  return !empty($roles);
}

function ht_preprocess_comment_wrapper(&$variables) {
  global $user;
  $is_agent = ht_user_is_agent($user);
  foreach($variables['content']['comments'] as $cid => &$el) {
    $comment = $el['#comment'];
    if ($is_agent && isset($comment->field_private[LANGUAGE_NONE][0]['value']) && $comment->field_private[LANGUAGE_NONE][0]['value']) {
      $el['#printed'] = true;
    }
  }
}

function ht_preprocess_comment(&$variables) {
  $account = user_load($variables['elements']['#comment']->uid);
  $variables['submitted'] = t('!user replied @time ago (@longtime)', array('!user' => theme('username', ['account' => $account]), '@time' => format_interval(REQUEST_TIME - $variables['elements']['#comment']->created), '@longtime' => format_date($variables['elements']['#comment']->created)));
  $to = user_load($variables['elements']['#node']->uid);
  $variables['replied_to'] = t('replied to: @email', array('@email' => $to->mail));
}

function ht_preprocess_node(&$variables) {
  $account = user_load($variables['node']->uid);
  $variables['submitted'] = t('!user reported @time ago (@longtime)', array('!user' => theme('username', ['account' => $account]), '@time' => format_interval(REQUEST_TIME - $variables['node']->created), '@longtime' => format_date($variables['node']->created)));
}

function ht_preprocess_username(&$vars) {

  // Update the username so it's the full name of the user.
  $account = $vars['account'];

  // Revise the name trimming done in template_preprocess_username.
  $name = $vars['name_raw'] = format_username($account);


  // Assign the altered name to $vars['name'].
  $vars['name'] = views_trim_text(['word_boundary' => 1, 'max_length' => 17], $name);
}


function ht_textfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'text';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength'));
  _form_set_class($element, array('form-text'));

  $extra = '';
  if ($element['#autocomplete_path'] && drupal_valid_path($element['#autocomplete_path'])) {
    drupal_add_library('system', 'drupal.autocomplete');
    $element['#attributes']['class'][] = 'form-autocomplete';

    $attributes = array();
    $attributes['type'] = 'hidden';
    $attributes['id'] = $element['#attributes']['id'] . '-autocomplete';
    $attributes['value'] = url($element['#autocomplete_path'], array('absolute' => TRUE));
    $attributes['disabled'] = 'disabled';
    $attributes['class'][] = 'autocomplete';
    $extra = '<input' . drupal_attributes($attributes) . ' />';
  }

  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Implements hook_preprocess_forum_list().
 */
function ht_preprocess_forum_list(&$variables) {
  foreach ($variables['tables'] as &$table) {
    foreach ($table['items'] as $tid => &$item) {
      $term = taxonomy_term_load($tid);
      if (!empty($term->field_icon[LANGUAGE_NONE][0]['value'])) {
        $item->icon = $term->field_icon[LANGUAGE_NONE][0]['value'];
      }
      else {
        $item->icon = 'glyphicon-info-sign';
      }
      $q = new EntityFieldQuery();
      $results = $q->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'forum')
        ->fieldCondition('taxonomy_forums', 'tid', $tid)
        ->range(0, 5)
        ->propertyOrderBy('created', 'DESC')
        ->execute();
      $item->recent_nodes = [];
      if (!empty($results['node'])) {
        $item->recent_nodes = node_load_multiple(array_keys($results['node']));
      }
    }
  }
}

/**
 * Theme wrapper for the main menu.
 */
function ht_menu_tree__main_menu(&$variables) {
  return '<ul class="menu nav navbar-nav">' . $variables['tree'] . '</ul>';
}

/**
 * Theme wrapper for the main menu.
 */
function ht_menu_tree__user_menu(&$variables) {
  return '<ul class="menu nav navbar-nav secondary">' . $variables['tree'] . '</ul>';
}

/**
 * Bootstrap theme wrapper function for the primary menu links.
 */
function ht_menu_tree__primary(&$variables) {
  $variables['tree'] = i18n_menu_translated_tree('main-menu');
  return render($variables['tree']);
}

/**
 * Theme wrapper function for actions menu in navbar.
 */
function ht_menu_tree__actions(&$variables) {
  return '<ul class="menu nav navbar-nav actions">' . render($variables['tree']) . '</ul>';
}

/**
 * Theme wrapper function for actions menu in navbar.
 */
function ht_menu_tree__menu_footer(&$variables) {
  return '<ul class="menu list-inline text-center">' . render($variables['tree']) . '</ul>';
}


/**
 * Bootstrap theme wrapper function for the secondary menu links.
 */
function ht_menu_tree__secondary(&$variables) {
  $variables['tree'] = i18n_menu_translated_tree('user-menu');
  if (user_is_anonymous()) {
    $children = element_children($variables['tree']);
    foreach($children as $index => $id) {
      $classes = ['btn', 'navbar-btn'];
      if ($index == count($children) - 1) {
        $classes[] = 'btn-material-cyan-900';
        $variables['tree'][$id]['#localized_options']['attributes']['class'] = $classes;
      }
    }
  }
  return render($variables['tree']);
}

/**
 * Allow to translate views page title
 * @param $view
 */
function ht_views_pre_render(&$view)  {
  $view->set_title(t($view->get_title()));
}

/**
 * Overrides theme_button().
 */
function ht_button($variables) {
  $element = $variables['element'];
  $label = $element['#value'];
  element_set_attributes($element, array('id', 'name', 'value', 'type'));

  // If a button type class isn't present then add in default.
  $button_classes = array(
    'btn-default',
    'btn-primary',
    'btn-success',
    'btn-info',
    'btn-warning',
    'btn-danger',
    'btn-link',
    'btn-white',
    'btn-black',
  );
  $class_intersection = array_intersect($button_classes, $element['#attributes']['class']);
  if (empty($class_intersection)) {
    $element['#attributes']['class'][] = 'btn-default';
  }

  // Add in the button type class.
  $element['#attributes']['class'][] = 'form-' . $element['#button_type'];

  // This line break adds inherent margin between multiple buttons.
  return '<button' . drupal_attributes($element['#attributes']) . '>' . $label . "</button>\n";
}

/**
 * Implements hook_preprocess_button().
 */
function ht_preprocess_button(&$vars) {
  $vars['element']['#attributes']['class'][] = 'btn';
  $skip = false;
  foreach ($vars['element']['#attributes']['class'] as $class) {
    if (strpos($class, 'btn-') === 0) {
      $skip = true;
    }
  }
  if (!$skip && isset($vars['element']['#value'])) {
    if ($class = _bootstrap_colorize_button($vars['element']['#value'])) {
      $vars['element']['#attributes']['class'][] = $class;
    }
  }
}

/**
 * Updates:
 *  - Add actions menu
 *  - Remove new ticket link from main menu
 *  - Update related articles view: Chane icon to mdi-editor-insert-comment
 */