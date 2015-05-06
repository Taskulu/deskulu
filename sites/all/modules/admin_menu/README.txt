
-- SUMMARY --

The Administration menu module displays the entire administrative menu tree (and
most local tasks) in a drop-down menu, providing administrators one- or
two-click access to most pages.  Other modules may also add menu links to the
menu using hook_admin_menu_output_alter().

For a full description of the module, visit the project page:
  http://drupal.org/project/admin_menu

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/admin_menu


-- REQUIREMENTS --

None.


-- INSTALLATION --

* Install as usual, see http://drupal.org/node/895232 for further information.

* You likely want to disable Toolbar module, since its output clashes with
  Administration menu.


-- CONFIGURATION --

* Configure user permissions in Administration » People » Permissions:

  - Use the administration pages and help (System module)

    The top-level administration categories require this permission to be
    accessible. The administration menu will be empty unless this permission is
    granted.

  - Access administration menu

    Users in roles with the "Access administration menu" permission will see
    the administration menu at the top of each page.

  - Display Drupal links

    Users in roles with the "Display drupal links" permission will receive
    links to drupal.org issue queues for all enabled contributed modules. The
    issue queue links appear under the administration menu icon.

  Note that the menu items displayed in the administration menu depend on the
  actual permissions of the viewing user. For example, the "People" menu item
  is not displayed to a user who is not a member of a role with the "Administer
  users" permission.

* Customize the menu settings in Administration » Configuration and modules »
  Administration » Administration menu.

* To prevent administrative menu items from appearing twice, you may hide the
  "Management" menu block.


-- CUSTOMIZATION --

* To override the default administration menu icon, you may:

  1) Disable it via CSS in your theme:

     body #admin-menu-icon { display: none; }

  2) Alter the image by overriding the theme function:

     Copy the entire theme_admin_menu_icon() function into your template.php,
     rename it to phptemplate_admin_menu_icon() or THEMENAME_admin_menu_icon(),
     and customize the output according to your needs.

  Remember that the output of the administration menu is cached. To see changes
  from your theme override function, you must clear your site cache (via
  the "Flush all caches" link on the menu).

* To override the font size, add the following line to your theme's stylesheet:

  body #admin-menu { font-size: 10px; }


-- TROUBLESHOOTING --

* If the menu does not display, check the following:

  - Are the "Access administration menu" and "Use the administration pages and help"
    permissions enabled for the appropriate roles?

  - Does html.tpl.php of your theme output the $page_bottom variable?

* If the menu is rendered behind a Flash movie object, add this property to your
  Flash object(s):

  <param name="wmode" value="transparent" />

  See http://drupal.org/node/195386 for further information.


-- FAQ --

Q: When the administration menu module is enabled, blank space is added to the
   bottom of my theme. Why?

A: This is caused by a long list of links to module issue queues at Drupal.org.
   Use Administer >> User management >> Permissions to disable the "display
   drupal links" permission for all appropriate roles. Note that since UID 1
   automatically receives all permissions, the list of issue queue links cannot
   be disabled for UID 1.


Q: After upgrading to 6.x-1.x, the menu disappeared. Why?

A: You may need to regenerate your menu. Visit
   http://example.com/admin/build/modules to regenerate your menu (substitute
   your site name for example.com).


Q: Can I configure the administration menu module to display another menu (like
   the Navigation menu, for instance)?

A: No. As the name implies, administration menu module is for administrative
   menu links only. However, you can copy and paste the contents of
   admin_menu.css into your theme's stylesheet and replace #admin-menu with any
   other menu block id (#block-menu-1, for example).


Q: Sometimes, the user counter displays a lot of anonymous users, but no spike
   of users or requests appear in Google Analytics or other tracking tools.

A: If your site was concurrently spidered by search-engine robots, it may have
   a significant number of anonymous users for a short time. Most web tracking
   tools like Google Analytics automatically filter out these requests.


Q: I enabled "Aggregate and compress CSS files", but admin_menu.css is still
   there. Is this normal?

A: Yes, this is the intended behavior. the administration menu module only loads
   its stylesheet as needed (i.e., on page requests by logged-on, administrative
   users).


Q: Why are sub-menus not visible in Opera?

A: In the Opera browser preferences under "web pages" there is an option to fit
   to width. By disabling this option, sub-menus in the administration menu
   should appear.


Q: How can the administration menu be hidden on certain pages?

A: You can suppress it by simply calling the following function in PHP:

     module_invoke('admin_menu', 'suppress');

   However, this needs to happen as early as possible in the page request, so
   placing it in the theming layer (resp. a page template file) is too late.
   Ideally, the function is called in hook_init() in a custom module.  If you do
   not have a custom module, placing it into some conditional code at the top of
   template.php may work out, too.


-- CONTACT --

Current maintainers:
* Daniel F. Kudwien (sun) - http://drupal.org/user/54136
* Peter Wolanin (pwolanin) - http://drupal.org/user/49851
* Stefan M. Kudwien (smk-ka) - http://drupal.org/user/48898
* Dave Reid (Dave Reid) - http://drupal.org/user/53892

Major rewrite for Drupal 6 by Peter Wolanin (pwolanin).

This project has been sponsored by:
* UNLEASHED MIND
  Specialized in consulting and planning of Drupal powered sites, UNLEASHED
  MIND offers installation, development, theming, customization, and hosting
  to get you started. Visit http://www.unleashedmind.com for more information.

* Lullabot
  Friendly Drupal experts providing professional consulting & education
  services. Visit http://www.lullabot.com for more information.

* Acquia
  Commercially Supported Drupal. Visit http://acquia.com for more information.

