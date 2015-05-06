
PREFACE
-------
This module provides flexible voting widgets for nodes and comments.
Administrators can add multiple widgets and define an unlimited number of
buttons.

CONTENTS
--------
1. Installation
2. Configuration
2.1. Widget configuration
2.2. Options
2.3. Bot detection
3. Widget types
4. Theming
5. Voting results
6. Views integration
7. Expiration (close voting on a specified date)
8. Using rate in blocks or panels
9. Hooks

1. Installation
--------------------------------------------------------------------------------
Before installing Rate, you need VotingAPI. If not already installed, download
the latest stable version at http://drupal.org/project/votingapi
Please follow the readme file provided by VotingAPI on how to install.

Copy Rate into your modules directory (i.e. sites/all/modules) and enable Rate
(on admin/modules).

Optional modules:

* Chart
  To view the charts in the vote results tab, you also need to install the "chart"
  module, which you can get at http://drupal.org/project/chart.
* Date
  The date module is a requirement for the Rate Expiration module.

2. Configuration
--------------------------------------------------------------------------------
After installation, the configuration page will be available at
admin/structure/rate. This page shows a list with available widgets. Each widget
have an edit and delete link. You can add a tab on the form below 'Add widget'.
In this form you have to choose a widget type. See section 3 for more
information on this topic. Both editing and adding a widget leads to the widget
configuration (see §2.1).

If you want users other than admin to access the voting results page, you need
to give them the "view rate results page" permission on admin/user/permissions.

2.1. Widget configuration
-------------------------
The elements on the widget configuration form are explained in this paragraph.
Note that some elements may not be available depending on the widget type you
use, these are "Value type", "Options" and "Translate options".

* Title
  The title is only used in the admin section. Use a recognizable name.
* Machine readable name
  Name used for technical purposes. You may only contain alphanumeric characters
  and underscores.
* Tag
  This is the tag used by VotingAPI to store the voting results. Voting results
  from different tags are never merged. You can use this to allow multiple
  ratings on the same node (i.e. for "comfort", "location", "services" etc.).
  The default tag for ratings is "vote". Use this value if you do not allow
  ratings on specific aspects.
* Value type
  This determines how vote results are totaled. VotingAPI supports three value
  types by default: 'Percentage' votes are averaged, 'Points' votes are summed
  up, and 'Options' votes get a count of votes cast for each specific option.
  Typical usages are:
  * Thumbs up / down: use points
  * Bad / mediocre / good: use percentage
  * Makes me funny / boring / mad / angry: use options
* Options
  These are the options displayed to the user. Each option has a value and a
  label. See §2.2 for more information on how to configure options.
* Translate options
  This checkbox determines if the labels used for the options should be
  translated.
* Node types
  Check the node types on which a rate widget should be available. There are
  separate columns for nodes and comments in this table.
* Node display
  Determines where the widget will be placed. There are three options:
  * Do not add automatically: The widget is added in the node object, but not
    within the content array. This only makes sense if you place the widget
    somewere by hand in the node theming.
  * Above the content: The content will be prepended by the widget.
  * Below the content: Selected by default. The widget is appended to the
    content.
  * Within the links: Add the widget inside the links section.
* Display in teaser
  Check this box if you want the widget to be visible in the node teaser.
* Appearance in full node
  Display mode when full node is displayed. Options are:
  * Full widget: Display the full, clickable widget.
  * Display only: Display the full widget in disabled state (links are not
    clickable).
  * Display only, compact: Compact widget in disabled state.
  * Compact: Clickable widget, but without information line.
* Appearance in teaser
  Display mode when node teaser is displayed.
* Comment display
  Same as node display, but for comments.
* Display mode when displayed for comments.
* Description
  This is an optional description which is displayed under the rate widget.
* Display description in compact mode
* Which rating should be displayed?
  Determines which rating to display. Options are:
  * Average rating: Always display the average rating.
  * Users vote if available, empty otherwise: Display the users vote. If the
    user has not voted already, there is no voting result displayed.
  * Users vote if available, average otherwise: Display the users vote. If the
    user has not voted already, the average vote is displayed.
* Which rating should be displayed when the user just voted?
  Same as previous question. Applies directly after the user has voted. It is
  recommended to set this to "Users vote" to provide a visible feedback after
  a user has voted.
* Roles
  Check the roles which are allowed to vote using this widget. All roles are
  allowed to vote if no roles are checked.
* Behaviour when user has no permissions to vote
  If the user may not vote on the widget, what needs to be done? Options are:
  * Redirect to login and show message.
    The widget is visible just as if you may vote on in. If the user clicks on
    a button, the user is redirected to the user page which shows the message
    'You must login before you can vote.'. The user is redirected back to the
    page with the rate widget after login.
  * Redirect to login but do not show a message.
    This behaviour is the same as the first option, except for the fact that it
    does not display a message.
  * Show a disabled widget (with non clickable buttons).
    The user is able to see the widget, but cannot click on it.
  * Hide widget
    The widget is not visible to the user if he does not have the permission to
    vote.

2.2. Options
------------
Options are the "buttons" dispayed in the widget. These can be visually
different, depending on the theming. Options are generated as HTML links by
default.

Each option has a value and a label. Only the label is visible for the user, but
the value is what he actually votes for when clicking the button.

Values have to be configured according to the following rules:
* Values must be integers (may be negative). thus '1', '2', '0', '-3' are all
  right, but '2.3' is wrong.
* Values must be unique across all options within the same widget.

Which value you should use depends on the value type setting. When using points,
these are the points which will be added when clicking that button. So "thumbs
up" must have the value '1', "thumbs down" the value '-1' and "neutral" '0'. For
'Percentage' you have to use whole numbers between 0 and 100. When using
'Options', you may use any number as long as they are unique. It doesn't have to
make sense as they are only used for storage.

2.3. Bot detection
--------------------------------------------------------------------------------
The Rate module is able to detect bots in three ways:

* Based on user agent string
* Using an threshold. The IP-address is blocked when there are more votes from
  the same IP within a given time than the threshold. There are thresholds for
  1 minute and 1 hour.
* Lookup the IP-address in the BotScout.com database. This requires you to
  obtain an API-key.

The thresholds and API-key can be configured at the settings form found on
admin/structure/rate/settings. The default thresholds are 25 and 250. They are
too high for many sites, but you should make sure that no real users get
blocked. On the other hand, lower thresholds will identify more bots and will
identify them faster. A value of 10 / 50 is a better setting for most sites.

Bad user agents cannot be configured via the admin at this moment. You can add
bad strings in the 'rate_bot_agent' table. Percent signs ("%") can be used as
wildcards. A string can be for example "%GoogleBot%" or just "%bot%". Patterns
are case insensitive. The id field is for identification and has no meaning.

3. Widget types
--------------------------------------------------------------------------------
Technically, widget types are sets of "value types" and "options" (see §2.1).
They are called 'templates' in code.

Some widget types have the option to let the user customize the options, others
don't allow the user to do that. But all widget types have a predefined set of
options.

You may create widgets without choosing a "template" by selecting the 'Custom'
type. By using 'Custom', you have to add the theming for this widget (see
section 4).

Widget types can be extended by 3rd party modules. The following widget types
are provided by the rate module:

* Thumbs up
* Thumbs up / down
* Number up / down
* Fivestar
* Emotion
* Yes / no

4. Theming
--------------------------------------------------------------------------------
Default templates for theming are:

* rate-widget.tpl.php
  This is the default template for all custom widgets.
* rate-widget--NAME.tpl.php
  This is a widget specific template. Use the machine name for NAME. Replace
  underscores by dashes in this name. This template is only available for custom
  widgets.

Theming for non-custom widget types are defined in the module which provides
the widget type.

You may use the following snippets in the template:

* Print a button for a single option:

    <?php
    print theme('rate_button', array(
      'text' => $links[0]['text'],
      'href' => $links[0]['href'],
      'class' => "extra-class")
    );
    ?>

  '0' is the first option (see §2.1). For a thumbs up / down
  configuration you will have:

    <?php
    print theme('rate_button', array(
      'text' => $links[0]['text'],
      'href' => $links[0]['href'],
      'class' => "extra-class")
    );
    print theme('rate_button', array(
      'text' => $links[1]['text'],
      'href' => $links[1]['href'],
      'class' => "extra-class")
    );
    ?>

* Print the rating when using value type 'percentage' or 'points':

    <?php print $results['rating']; ?>

* Print the number of votes for a specific option (only available when using
  value type 'options'):

    <?php print $links[0]['votes']; ?>

  '0' is the first option (see §2.1).

* Print the total number of votes:

  <?php print $results['count']; ?>

* For thumbs up / down widgets, there are 2 special variables available which
  provides the percentage of votes for up and down.

  <?php print $results['up_percentage']; ?>
  <?php print $results['down_percentage']; ?>

You can choose to not automatically add the widget to the node template. In that
case, the widget can be used as:

<?php print $node->rate_NAME['#markup']; ?>

Replace NAME by the widget's machine readable name.

5. Voting results
--------------------------------------------------------------------------------
Voting results are available on the voting results page. You can get there by
clicking the "Voting results" tab on the node page. Note that this tab is hidden
if the node does not have any rate widgets or if you do not have the
"view rate results" permission.

When the chart module is enabled, you will find charts of the results in the
last 30 days on this page. The chart may show less than 30 days if there was no
activity on all days.

The voting results page is only available for nodes.

6. Views integration
--------------------------------------------------------------------------------
This module provides views integration via the VotingAPI module. To add a rate
widget in your view, first add a relation to "Node: Vote results" for nodes or
"Comment: Vote results" for comments. You have to configure a few options here.
The "Value type" and "Vote tag" needs to be the same as used for the widget
(see §2.1). The "aggregate" function must be "Number of votes".

After adding the relationship, you can add the field "Vote results: Value" to
your view. In the "Appearance" box you may choose one of the following:

* Rate widget (display only)
  This shows a disabled widget. Uses are allowed to see the results, but cannot
  click the buttons.
* Rate widget (compact)
  This shows a compact widget. This is the basic widget without the textual
  information.
* Rate widget
  This shows the full widget (as on the node page).

When using a view on nodes, you are advised to add the "Node: Type" field to
your view fields. If you do not, an additional query will be executed per row.
You may exclude this field from display.

7. Expiration (close voting on a specified date)
--------------------------------------------------------------------------------
The optional Rate Expiration module allows you to close voting on a specified
date. When adding or editing a rate widget, you will find the following options:

* Disable voting after this period
  When set, voting is closed when the configured period has ellapsed since node
  creation. Users are not able to click the buttons when voting is closed.
* Allow override
  When checked, the start- en enddates for voting can be set in the node edit
  form.

8. Using rate in blocks or panels
--------------------------------------------------------------------------------
You can place the rate widget on a node page in a block or (mini) panel. Add
a custom block with the PHP code input filter or a panel with PHP code and use
the following code:

<?php
if (arg(0) == 'node' && is_numeric(arg(1)) && ($node = node_load(arg(1)))) {
  print rate_embed($node, 'NAME');
}
?>

Replace NAME by the widget's machine readable name. If you already have a loaded
node object, you just need the "print rate_embed" line.

You may also use different build modes:

print rate_embed($node, 'NAME', RATE_FULL);
print rate_embed($node, 'NAME', RATE_COMPACT);
print rate_embed($node, 'NAME', RATE_DISABLED);
print rate_embed($node, 'NAME', RATE_CLOSED);

9. Hooks
--------------------------------------------------------------------------------
Hooks for modules are documented in rate.hooks.inc.

There are two Javascript hooks available; eventBeforeRate and eventAfterRate.
This hook has an argument 'data'. This is an object which contains the variables
'content_type', 'content_id', 'widget_id' and 'widget_mode'. Example of use:

$(document).bind('eventAfterRate', function(event, data)
{
  alert('eventAfterRate called');
});
