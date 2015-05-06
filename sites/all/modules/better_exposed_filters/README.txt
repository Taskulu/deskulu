$Id

-- SUMMARY --

The Better Exposed Filters module replaces the Views' default single- 
or multi-select boxes with radio buttons or checkboxes, respectively.

Views Filters are a powerful tool to limit the results of a given view.
When you expose a filter, you allow the user to interact with the view
making it easy to build a customized advanced search.  For example, 
exposing a taxonomy filter lets your site visitor search for articles
with specific tags.  Better Exposed Filters gives you greater control
over the rendering of exposed filters. 

For a full description of the module, visit the project page:
  http://drupal.org/project/bef
  
A handbook page has been started to document some use cases, provide
additional documentation and tips:
  http://drupal.org/node/766974 

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/bef
  
For more information on Views filters, see the Advanced Help documentation
that comes with Views or visit the online version:
  http://views-help.doc.logrus.com/help/views/filter 


-- REQUIREMENTS --

This module requires the Views module:
  http://drupal.org/project/views 


-- INSTALLATION --

Install as usual, see http://drupal.org/node/70151 for further instructions.


-- CONFIGURATION --

When adding a filter to a view, you are given several options in the 
"Configure filter" screen.  At the top of this screen is the option
to expose the filter (button in the upper-right).  Clicking this button
adds more customization options to the screen.

Better Exposed Filters adds the option to render the filter using the 
default Views display or as radio buttons/checkboxes.  If "Force single"
is selected radio buttons will be used, otherwise checkboxes are displayed.

When adding a CCK-field based filter, be sure to use the "Allowed values"
option, otherwise the filter is rendered as an auto-complete textbox.

In Views 3.x, the BEF configuration options have been moved to the Exposed
Form dialog.  Set the "Exposed form style" to "Better Exposed Filters" and
make your configuration changes in the resulting dialog.  Each filter is 
listed based on the label given in the exposed filter dialog.

Views 3.x also introduces the idea of the exposed form allowing site builders
to expose other options such as sort and pager.  BEF allows you to customize
those settings as well.


-- CUSTOMIZATION --

Themers can override the theme_select_as_checkboxes() routine to allow for 
addition markup in the exposed filter.  However, this routine is updated 
often with bug fixes and enhancements.  If you have suggestions on how to
improve Better Exposed Filters, please add them to the issue queue:
  http://drupal.org/project/issues/bef
  
  
-- TROUBLESHOOTING --

* I don't see the "Display exposed filter as" option when I click the Expose button.

  - Make sure this filter isn't already displayed as a checkbox/radio button

  - If this is a CCK-based field (field title starts with "Content:") make sure
    you're filtering on the "Allowed values" option.
    
  - You're using Views 3.x.  The BEF settings have been moved to the Exposed Form
    dialog.

* Wow, a really long URL is generated when using exposed filters!  How can I make it 
  a more reasonable length?
  
  - Exposed Views filters (with or without BEF enabled) store their current selection
    in the URL query string.  This allows for easy bookmarking of search results as well
    as "return to search results" links.  However, it can make for long URLs when using
    Views' default settings, especially with CCK-based fields.  To shorten the key used
    for a given filter, configure the filter and change the value in the "Filter indentifier"
    textbox.  Make sure that your indentifiers are unique within your view!  

-- FAQ --

Q: What was the motivation to build this module?

A: I find multi-select boxes to be a horrible user experience.  Imagine telling a
   a client that they should click on an option, then scroll to the next option and
   ctrl+click on it.  Make sure not to just click as you'll lose your first selection
   and ctrl+click again to unselect an option...  Yeah, not user friendly.
   
Q: Can't you just use hook_form_alter() to flip the exposed filter type from 
   'select' to 'checkboxes'?
   
A: Yes, that will get you checkboxes displayed but they won't act as you would expect.
   For example, leaving all checkboxes unchecked (often the default state) returns
   zero results.
   
Q: So, how does Better Exposed Filters work?

A: BEF changes the display of an exposed filter at the theme level.  This also
   allows a designer to customize the display by overriding BEF's 
   theme_select_as_checkboxes().
   
Q: What is the "Hidden" option used for?

A: You can use the "Hidden" option to build multi-page search.  For example, on the first 
   page the user selects a country, on the next page they select a region, etc.  To build
   this using BEF create a view with only the country field as an exposed filter.  On the
   second page, expose filters for both the region and country fields but set the display
   of the country filter to "Hidden".  More details can be found here: 
   http://drupal.org/node/645348#comment-2336516  


-- CONTACT --

The maintainer for this project is Mike Keran (mikeker - http://drupal.org/user/192273)
He can be contacted through his personal web site (http://MikeKeran.com) for work on this 
module or other custom projects.


-- CREDIT --

Thanks to Ben Buckman (http://echodittolabs.org/) for the original concept.