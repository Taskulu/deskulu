<?php

/**
 * @file
 * Wysiwyg dialog page handling functions.
 */

/**
 * Page callback; Outputs a dialog page for a wysiwyg plugin.
 *
 * A Wysiwyg dialog is a bare minimum, simple HTML page; presented in a
 * modal/popup window, triggered via JavaScript.
 *
 * However, Drupal core does not support such a concept, at all.
 * Insanity happens on two separate layers:
 * - All HTML pages go through the default delivery callback of
 *   drupal_deliver_html_page(), which calls into drupal_render_page(), which
 *   in turn *unconditionally* invokes hook_page_build() implementations. Thus,
 *   block_page_build() and similar implementations add the entirety of their
 *   page regions and blocks to our simple dialog page.
 *   Obviously, we don't want that.
 * - There is a nice default 'page' theme template implementation, which
 *   performs all the heavy-lifting that is required for outputting a sane HTML
 *   page through preprocess and process functions. The theme system does not
 *   support to "inherit" preprocess and process hooks to alternative
 *   implementations. Even a very basic HTML page requires almost all of that.
 *   However, the default page template (normally overridden by a theme)
 *   contains too many regions and usually also huge a header and footer.
 *   Obviously, we don't want that.
 *
 * The poor workaround would be to follow the Overlay module's implementation in
 * core: override the theme, build everything, and after doing all of that,
 * strip away what isn't needed. Obviously, we don't want that.
 *
 * Instead, we bend Drupal to sane rules:
 * - This page callback returns the actual main content.
 * - wysiwyg_menu() defines a custom delivery callback that replaces
 *   drupal_deliver_html_page(), just because we need to replace
 *   drupal_render_page().
 * - Our replacement for drupal_render_page() builds a $page that does not use
 *   #type 'page' but #type 'wysiwyg_dialog_page' instead.
 * - #type 'wysiwyg_dialog_page' is defined like #type 'page' in
 *   system_element_info(), but is required, because there's no way to inherit
 *   a theme definition but override the page template file to be used.
 * - As a consequence, #type 'wysiwyg_dialog_page' uses
 *   #theme 'wysiwyg_dialog_page', for which we have to implement stub
 *   preprocess and process callbacks in order to call into the ones for
 *   #theme 'page'.
 *
 * As a result we get:
 * - A HTML response.
 * - A HTML page wrapped into html.tpl.php.
 * - A page title, title prefix/suffix, messages, help, etc.pp.
 * - A simple page without regions and blocks (neither built nor rendered).
 *
 * @see wysiwyg_menu()
 * @see wysiwyg_deliver_dialog_page
 * @see wysiwyg_render_dialog_page()
 * @see wysiwyg_element_info()
 * @see wysiwyg_theme()
 * @see template_preprocess_wysiwyg_dialog_page()
 * @see template_process_wysiwyg_dialog_page()
 *
 * @see drupal_deliver_page()
 * @see drupal_deliver_html_page()
 * @see drupal_render_page()
 * @see system_element_info()
 * @see drupal_common_theme()
 * @see template_preprocess_page()
 * @see template_process_page()
 */
function wysiwyg_dialog($plugin, $instance) {
  $plugins = wysiwyg_get_all_plugins();
  if (!isset($plugins[$plugin])) {
    return drupal_access_denied();
  }
  $callback = $plugin . '_wysiwyg_dialog';
  if (!function_exists($callback)) {
    return drupal_not_found();
  }

  // Suppress admin menu.
  module_invoke('admin_menu', 'suppress');
  // Add editor instance id to Drupal.settings.
  $settings = array(
    'plugin' => $plugin,
    'instance' => $instance,
  );
  drupal_add_js(array('wysiwyg' => $settings), 'setting');

  $build = $callback($instance);
  if (!is_array($build)) {
    $build = array('#markup' => $build);
  }
  $build += array(
    '#instance' => $instance,
    '#plugin' => $plugin,
  );
  return $build;
}

/**
 * @see drupal_deliver_html_page()
 */
function wysiwyg_deliver_dialog_page($page_callback_result) {
  // Menu status constants are integers; page content is a string or array.
  if (is_int($page_callback_result)) {
    return drupal_deliver_html_page($page_callback_result);
  }

  // Emit the correct charset HTTP header, but not if the page callback
  // result is NULL, since that likely indicates that it printed something
  // in which case, no further headers may be sent, and not if code running
  // for this page request has already set the content type header.
  if (isset($page_callback_result) && is_null(drupal_get_http_header('Content-Type'))) {
    drupal_add_http_header('Content-Type', 'text/html; charset=utf-8');
  }

  // Send appropriate HTTP-Header for browsers and search engines.
  global $language;
  drupal_add_http_header('Content-Language', $language->language);

  if (isset($page_callback_result)) {
    // Print anything besides a menu constant, assuming it's not NULL or
    // undefined.
    print wysiwyg_render_dialog_page($page_callback_result);
  }

  // Perform end-of-request tasks.
  drupal_page_footer();
}

/**
 * @see drupal_render_page()
 */
function wysiwyg_render_dialog_page($page) {
  $main_content_display = &drupal_static('system_main_content_added', FALSE);

  // Allow menu callbacks to return strings or arbitrary arrays to render.
  // If the array returned is not of #type page directly, we need to fill
  // in the page with defaults.
  if (is_string($page) || (is_array($page) && (!isset($page['#type']) || ($page['#type'] != 'page')))) {
    drupal_set_page_content($page);
    $page = element_info('wysiwyg_dialog_page');
  }

  // Modules alter the $page as needed. Blocks are populated into regions like
  // 'sidebar_first', 'footer', etc.
  drupal_alter(array('wysiwyg_dialog_page', 'page'), $page);

  // If no module has taken care of the main content, add it to the page now.
  // This allows the site to still be usable even if no modules that
  // control page regions (for example, the Block module) are enabled.
  if (!$main_content_display) {
    $page['content']['system_main'] = drupal_set_page_content();
  }

  return drupal_render($page);
}

/**
 * Template preprocess function for theme_wysiwyg_dialog_page().
 *
 * @see wysiwyg_dialog()
 * @see wysiwyg-dialog-page.tpl.php
 * @see template_preprocess_page()
 */
function template_preprocess_wysiwyg_dialog_page(&$variables) {
  template_preprocess_page($variables);
}


/**
 * Template process function for theme_wysiwyg_dialog_page().
 *
 * @see wysiwyg_dialog()
 * @see wysiwyg-dialog-page.tpl.php
 * @see template_process_page()
 */
function template_process_wysiwyg_dialog_page(&$variables) {
  template_process_page($variables);
}

