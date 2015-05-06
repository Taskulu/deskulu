<?php

/**
 * @file
 * Holds the contents of a preprocess function moved into its own file
 * to ease memory requirements and having too much code in one file.
 */

/**
 * Preprocess forum list.
 */
function _advanced_forum_preprocess_forum_list(&$variables) {
  advanced_forum_add_template_suggestions("forum_list", $variables);

  // Added variable for default collapsed configuration.
  $variables['collapsed_main_list'] = variable_get('advanced_forum_default_collapsed_list', array());

  // Add needed items for the collapsible containers.
  $collapsible = variable_get('advanced_forum_collapsible_containers', 'toggle');
  $variables['collapsible'] = FALSE;
  if ($collapsible != 'none') {
    drupal_add_library('system', 'jquery.cookie');
    drupal_add_js(drupal_get_path('module', 'advanced_forum') . '/js/advanced_forum.js');
    $settings['advanced_forum'] = array(
      'modulePath' => drupal_get_path('module', 'advanced_forum'),
      'effect' => $collapsible,
      'default_collapsed_list' => $variables['collapsed_main_list'],
    );

    drupal_add_js($settings, array('type' => 'setting'));
    $variables['collapsible'] = TRUE;
  }

  // The tid is the ID of the container or forum we are in. Assign it to
  // $parent_id for easy reference, assign it to the forum_id template variable
  // to give it a nice name for themers, then get rid of the original variable.
  $parent_id = isset($variables['tid']) ? $variables['tid'] : 0;
  $variables['forum_id'] = $parent_id;

  unset($variables['tid']);

  $variables['use_taxonomy_image'] = (function_exists('taxonomy_image_display') && variable_get('advanced_forum_use_taxonomy_image', TRUE));

  // Process the containers and forums and move them to a new variable.
  $items = array();
  $lowest_depth = 0;
  foreach ($variables['forums'] as $id => $forum) {
    if (empty($forum->container)) {
      $items[$id] = advanced_forum_process_forum($forum);
    }
    else {
      $items[$id] = advanced_forum_process_container($forum);
    }

    // Figure out how deep the hierarchy goes for the next step.
    if ($forum->depth > $lowest_depth) {
      $lowest_depth = $forum->depth;
    }
  }

  // Calculate number of and number of new child posts for each item.
  for ($depth = $lowest_depth + 1; $depth > 0; $depth--) {
    foreach ($items as $id => $item) {
      if ($depth <= $lowest_depth) {
        if (empty($item->depth)) {
          $item->depth = 0;
        }
        if ($item->depth == $depth) {
          $items[$item->parents[0]]->child_total_topics += $item->total_topics + $item->child_total_topics;
          $items[$item->parents[0]]->child_new_topics += $item->new_topics + $item->child_new_topics;
          $items[$item->parents[0]]->child_total_posts += $item->total_posts + $item->child_total_posts;
          $items[$item->parents[0]]->child_new_posts += $item->new_posts + $item->child_new_posts;
        }
      }

      if ($item->depth + 1 == $depth) {
        $parent = $items[$id]->parents[0];
        if (($parent != 0) && (!empty($items[$item->parents[0]]))) {
          $items[$item->parents[0]]->total_posts += $item->total_posts;
          $items[$item->parents[0]]->total_topics += $item->total_topics;
          $items[$item->parents[0]]->new_posts += $item->new_posts;
          $items[$item->parents[0]]->new_topics += $item->new_topics;
          if (isset($items[$parent]->link)) {
            $items[$parent]->new_posts_link = $items[$parent]->link . "#new";
            $items[$parent]->new_topics_link = $items[$parent]->link . "#new";
          }
          $items[$parent]->new_topics_text = $items[$parent]->new_topics . " new";
          $items[$parent]->new_posts_text = $items[$parent]->new_posts . " new";
        }
      }
    }
  }

  // Loop through all the items and fill the $tables variable that will
  // hold all the tables with the containers and forums organized, processed,
  // and ready for the template.
  if ($parent_id) {
    // On a container page. Fake the main table.
    $table_counter = 1;
    $tables[$table_counter]['table_info'] = advanced_forum_process_container($variables['parents'][0]);
  }
  else {
    $table_counter = 0;
  }

  // Workaround for container-less forums on main page.
  $depth_offset = 0;
  foreach ($items as $id => $item) {
    // Get a handle on the parent of this item.
    if ($parent_id && $item->depth == 0) {
      // If we are on a container page, the parent of root items is the main
      // container that is being used for the table info.
      $parent = $tables[$table_counter]['table_info'];
    }
    elseif (!empty($variables['forums'][$item->parents[0]])) {
      // For simplicity, we assume forums/containers have only one parent.
      $parent = $variables['forums'][$item->parents[0]];
    }
    else {
      $depth_offset = 0;
    }

    // If we aren't on the main forum page, we need to bump up the depth.
    $item_depth = ($parent_id) ? $item->depth + 1 : $item->depth;
    $item_depth += $depth_offset;

    if (!empty($item->container)) {
      // CONTAINERS.
      if ($item_depth == 0) {
        // Top level container always starts a new table.
        $table_counter++;
        $tables[$table_counter]['table_info'] = $item;
        $tables[$table_counter]['items'] = array();

        // Reset the striping.
        advanced_forum_stripe(TRUE);
      }
      elseif ($item_depth == 1) {
        // Subcontainer at top level is treated like a forum.
        // We set the forum icon here, rather than in the process_forum
        // function because we need to take into account new content
        // in children.
        if ($item->new_topics || $item->new_posts || $item->child_new_topics || $item->child_new_posts) {
          $item->icon_classes = "forum-list-icon forum-list-icon-new-posts";
          $item->icon_text = t("New posts");
        }
        else {
          $item->icon_classes = "forum-list-icon forum-list-icon-default";
          $item->icon_text = t("No new");
        }

        // Set the variable to control the row striping.
        $item->zebra = advanced_forum_stripe();

        // Add the container info to the table's item list.
        $tables[$table_counter]['items'][$id] = $item;
      }
      elseif ($item_depth == 2) {
        // A container elsewhere gets added to the parent's subcontainer list.
        $tables[$table_counter]['items'][$parent->tid]->subcontainer_list[$id] = $item;
      }
    }
    else {
      // FORUMS.
      // We set the forum icon here, rather than in the process_forum
      // function because we need to take into account new content
      // in children.
      if ($item->new_topics || $item->new_posts || $item->child_new_topics || $item->child_new_posts) {
        $item->icon_classes = "forum-list-icon forum-list-icon-new-posts";
        $item->icon_text = t("New posts");
      }
      else {
        $item->icon_classes = "forum-list-icon forum-list-icon-default";
        $item->icon_text = t("No new");
      }

      if ($item_depth == 0) {
        // This is a forum at the root. If it is the first or the first
        // since the last container, make a new table. Otherwise, put it in
        // the previous table.
        if (empty($table_counter) || !empty($tables[$table_counter]['table_info']->container)) {
          // This is the first root item or the current table belongs to a
          // container. Start a new generic one. We need a tid so just grab
          // the tid of the current forum for it.
          $table_counter++;
          if (!isset($tables[$table_counter]['table_info'])) {
            $tables[$table_counter]['table_info'] = new stdClass();
          }
          $tables[$table_counter]['table_info']->name = t('Forums');
          $tables[$table_counter]['table_info']->description = '';
          $tables[$table_counter]['table_info']->tid = $item->tid;

          // Reset the striping.
          advanced_forum_stripe(TRUE);
        }
        $depth_offset = 1;

        // Set the variable to control the row striping.
        $item->zebra = advanced_forum_stripe();

        // Add the forum info to the table's item list.
        $tables[$table_counter]['items'][$id] = $item;
      }
      elseif ($item_depth == 1) {
        // Main forum. Add it to the item list.
        if (empty($item->container)) {
          $item->zebra = advanced_forum_stripe();
        }

        $tables[$table_counter]['items'][$id] = $item;
      }
      elseif ($item_depth == 2) {
        // Subforum.
        $tables[$table_counter]['items'][$parent->tid]->subforum_list[$id] = $item;
      }
    }
  }

  // Theme subcontainers and subforums.
  foreach ($tables as $table_id => $table) {
    foreach ($table['items'] as $item_id => $item) {
      if (!empty($item->subforum_list)) {
        $tables[$table_id]['items'][$item_id]->subforums = theme('advanced_forum_subforum_list', array('subforum_list' => $item->subforum_list));
      }

      if (!empty($item->subcontainer_list)) {
        $tables[$table_id]['items'][$item_id]->subcontainers = theme('advanced_forum_subcontainer_list', array('subcontainer_list' => $item->subcontainer_list));
      }
    }
  }

  $variables['tables'] = $tables;

  // Remove unneeded variables.
  unset($variables['zebra']);
}

/**
 * Forum stripe.
 */
function advanced_forum_stripe($reset = FALSE) {
  static $stripe = 'odd';

  if ($reset) {
    $stripe = 'odd';
  }
  else {
    $stripe = ($stripe == 'odd') ? 'even' : 'odd';
  }

  return $stripe;
}

/**
 * Prepare an individual container for display.
 */
function advanced_forum_process_container($container) {
  // Create the link to the container.
  $container->link = url("forum/$container->tid");

  // Sanitise the name and description so they can be safely printed.
  $container->name = check_plain($container->name);
  $container->description = !empty($container->description) ? filter_xss_admin($container->description) : '';

  // Create a variable to check if the item is a container in the template.
  $container->is_container = TRUE;

  // @TODO: Make the icon change if subforums have posts.
  $container->icon_classes = "forum-list-icon forum-list-icon-default";
  $container->icon_text = t("No new");

  // Initialize these variables to avoid notices later.
  $container->total_topics = 0;
  $container->new_topics = 0;
  $container->total_posts = 0;
  $container->new_posts = 0;
  $container->child_total_topics = 0;
  $container->child_new_topics = 0;
  $container->child_total_posts = 0;
  $container->child_new_posts = 0;

  // Rename these to make them more descriptive.
  if (isset($container->num_topics)) {
    $container->total_topics = $container->num_topics;
    unset($container->num_topics);
  }

  if (isset($container->num_posts)) {
    $container->total_posts = $container->num_posts;
    unset($container->num_posts);
  }

  return $container;
}

/**
 * Prepare an individual forum for display.
 */
function advanced_forum_process_forum($forum) {
  // Create a variable to check if the item is a container in the template.
  $forum->is_container = FALSE;

  // Create the link to the forum.
  $forum->link = url("forum/$forum->tid");

  // Sanitise the name and description so they can be safely printed.
  // We don't do this for subforum names because that is sent through l()
  // in the theme function which runs it through check_plain().
  $forum->name = empty($forum->parents[0]) ? check_plain($forum->name) : $forum->name;
  $forum->description = !empty($forum->description) ? filter_xss_admin($forum->description) : '';

  // Initialize these variables to avoid notices later since not all forums
  // have new content or even any content at all.
  $forum->total_topics = 0;
  $forum->child_total_topics = 0;

  $forum->new_topics = 0;
  $forum->new_topics_text = '';
  $forum->new_topics_link = '';
  $forum->child_new_topics = 0;

  $forum->total_posts = 0;
  $forum->child_total_posts = 0;

  $forum->new_posts = 0;
  $forum->new_posts_text = '';
  $forum->new_posts_link = '';
  $forum->child_new_posts = 0;

  // Rename these to make them more descriptive.
  if (isset($forum->num_topics)) {
    $forum->total_topics = $forum->num_topics;
    unset($forum->num_topics);
  }

  if (isset($forum->num_posts)) {
    $forum->total_posts = $forum->num_posts;
    unset($forum->num_posts);
  }

  // If the viewer is authenticated, check for new topics and posts.
  global $user;
  if ($user->uid) {
    // New topics.
    $forum->new_topics = _forum_topics_unread($forum->tid, $user->uid);
    if ($forum->new_topics) {
      $forum->new_topics_text = format_plural($forum->new_topics, '1 new', '@count new');
      $forum->new_topics_link = url("forum/$forum->tid", array('fragment' => 'new'));
    }

    // New posts are optional because the query is slow.
    if (variable_get('advanced_forum_get_new_comments', 0)) {
      $forum->new_posts = advanced_forum_unread_replies_in_forum($forum->tid, $user->uid) + $forum->new_topics;

      if ($forum->new_posts) {
        $forum->new_posts_text = format_plural($forum->new_posts, '1 new', '@count new');
        $forum->new_posts_path = "forum/$forum->tid";
        $forum->new_posts_link = url("forum/$forum->tid", array('fragment' => 'new'));
      }
    }
  }

  // Process the "last post" object into a printable string.
  // Trying to copy the string back into the variable directly caused odd bugs
  // so we move it to a temp variable then unset the original.
  // Before doing so, however, we make a copy in case another module implementing
  // hook_TYPE_alter() needs access to the raw data.
  $forum->last_post_obj = empty($forum->last_post) ? new stdClass() : $forum->last_post;
  $last_post = (empty($forum->last_post)) ? '' : $forum->last_post;
  unset($forum->last_post);
  $forum->last_post = theme('forum_submitted', array('topic' => $last_post));

  // Display forum images using Field API.
  $entity = entity_load('taxonomy_term', array($forum->tid));
  $field_name = variable_get('advanced_forum_forum_image_field', '');

  if (!empty($entity[$forum->tid]->$field_name)) {
    $field = $entity[$forum->tid]->$field_name;
    global $language;
    $langcode = isset($field[$language->language]) ? $language->language : LANGUAGE_NONE;

    if (isset($field[$langcode])) {
      $display = array(
        'label' => 'hidden',
        'type' => 'image',
        'settings' => array(
          'image_style' => variable_get('advanced_forum_forum_image_preset', ''),
          'image_link' => '',
        ),
      );
      $output = field_view_value('taxonomy_term', $entity[$forum->tid], $field_name, $field[$langcode][0], $display);
      $forum->forum_image = render($output);
    }
  }

  drupal_alter('advanced_forum', $forum);
  return $forum;
}
