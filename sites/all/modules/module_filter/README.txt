Description
-----------
This module provides a method for filtering modules on the modules page as well
as for filtering projects on the update status report.

The supplied filter is simpler than using your browsers find feature which
searches the entire page. The provided filter will filter modules/projects that
do not meet your input.

Along with the filter textfield there are additional
checkboxes that help to narrow the search more. The modules page contains four
checkboxes: Enabled, Disabled, Required, and Unavailable. While the first two
are self-explanatory, the latter two can take an explanation. The Required
checkbox affects visibility of modules that are enabled and have other
module(s) that require it also enabled. The Unavailable checkbox affects
visibility of modules that are disabled and depend on module(s) that are
missing.

The update status report filter also contains four checkboxes: Up-to-Date,
Update availabe, Security update, and Unknown. These directly affect the
visibilty of each project; whether it is up-to-date, there is an update
available, a security update is available, or the status is unknown.

Installation
------------
To install this module, do the following:

1. Extract the tar ball that you downloaded from Drupal.org.

2. Upload the entire directory and all its contents to your modules directory.

Configuration
-------------
To enable and configure this module do the following:

1. Go to Admin -> Modules, and enable Module Filter.

2. Go to Admin -> Configuration -> User interface -> Module filter, and make
   any necessary configuration changes. 

Tabs
----
By default Module Filter alters the modules page into tabs (Can be disabled on
configuration page). In the tabs view, each package is converted to a vertical
tab rather than a fieldset which greatly increases the ability to browse them.

There are several benefits to using the tabs view over the standard view for
the modules page. I've listed the key benefits below as well as additional
information that pertains to each.

1.  The increased ease of browsing between packages.

2.  Allows all modules to be listed alphabetically outside of their package,
    making it all the easier to find the module by name rather than package it
    happens to be in.

3.  The operations for a module are moved within the description column giving
    the description more "elbow room".

4.  Filtering is restricted to within the active tab or globally when no tab is
    selected. By default no tab is selected which will list all modules. When a
    tab is active and you want to get back to the 'all' state click on the
    active tab to deselect it.

5.  The number of enabled modules per tab is shown on the active tab. (Can be
    disabled on configuration page)

6.  Nice visual aids become available showing what modules are to be
    enabled/disabled and the number of matching modules in each tab when
    filtering. (Can be disabled on configuration page)

7.  The save configuration button becomes more accessible, either staying at
    the bottom of the window when the tabs exceed past the bottom and at the
    top when scrolling past the tabs. (Can be disabled on configuration page)

8.  When filtering, tabs that do not contain matches can be hidden. (Can be
    enabled on configuration page)

9.  Tab states are remembered like individual pages allowing you to move
    forward and backward within your selections via your browsers
    forward/backward buttons.

10. When viewing all modules (no active tab) and mousing over modules it's tab
    becomes highlighted to signify which tab it belongs to.

Filter operators
----------------
The modules page's filter has three filter operators available. Filter
operators allow alternative filtering techniques. A filter operator is applied
by typing within the filter textfield 'operator:' (where operator is the
operator type) followed immediately with the string to pass to the operator
function (e.g. 'requires:block'). The available operators are:

description:
   Filter based on a module's description.

requiredBy:
   Filter based on what a module is required by.

requires:
   Filter based on what a module requires.

Multiple filters (or queries) can be applied by space delimiting. For example,
the filter string 'description:ctools views' would filter down to modules with
"ctools" in the description and "views" within the module's name. To pass a
space within a single query wrap it within double quotes (e.g. 'requires:"chaos
tools"' or '"bulk export"').
