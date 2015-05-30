<?php
/**
 * @file
 * Default theme implementation to display a single Drupal page.
 *
 * The doctype, html, head and body tags are not in this template. Instead they
 * can be found in the html.tpl.php template in this directory.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['content']: The main content of the current page.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['header']: Items for the header region.
 * - $page['footer']: Items for the footer region.
 *
 * @see bootstrap_preprocess_page()
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see bootstrap_process_page()
 * @see template_process()
 * @see html.tpl.php
 *
 * @ingroup themeable
 */
$node = menu_get_object('node');
?>
<?php require_once drupal_get_path('theme', 'ht') . '/templates/header.tpl.php'; ?>
<div class="main-container container">

  <div class="row">
    <section<?php print $content_column_class; ?>>
      <div class="">
        <a id="main-content"></a>
        <?php if ($messages): ?>
          <?php print $messages; ?>
        <?php endif; ?>
        <?php if (!empty($tabs)): ?>
          <?php print render($tabs); ?>
        <?php endif; ?>
        <div class="conversation">
          <div class="conversation-msg well">
            <h3 class="ticket-title"><?php print $title; ?></h3>
            <?php
            $comments = $page['content']['system_main']['nodes'][$node->nid]['comments'];
            unset($page['content']['system_main']['nodes'][$node->nid]['comments']);
            print render($page['content']);
            ?>
          </div>
          <?php print render($comments); ?>
          <?php
          if (user_is_anonymous()):
          ?>
          <div class="reply-link"><?php echo t('Please !login to post a comment', ['!login' => l(t('Login'), 'user/login')]); ?></div>
          <?php endif; ?>
        </div>
      </div>
    </section>

    <?php if (!empty($page['sidebar'])): ?>
      <aside class="col-sm-4" role="complementary">
        <?php print render($page['sidebar']); ?>
      </aside>  <!-- /#sidebar-first -->
    <?php endif; ?>
  </div>
</div>
<footer class="footer container">
  <?php print render($page['footer']); ?>
</footer>
