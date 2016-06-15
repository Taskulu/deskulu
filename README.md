#Intro
Deskulu is an opensource helpdesk and ticketing system based on Drupal 7, developed as a weekend project by [Taskulu](https://taskulu.com) - a task management tool for enterprise. We've been using Deskulu as our own [helpdesk](https://help.taskulu.com) since the initial development and have been fixing bugs and making improvements. It's quite production ready at this point.

![Knowledge base](/screenshots/knowledgebase.png?raw=true "Knowledge base")Knowledge base

![Agent Dashboard](/screenshots/dashboard.png?raw=true "Agent Dashboard")Agent dashboard

![Ticket](/screenshots/ticket.png?raw=true "Ticket")Ticket

##Features
Deskulu is based on Drupal and inherits all its flexibility and [plethora of modules](https://www.drupal.org/project/project_module) that allow you to add more functionality. 
Here's what Deskulu offers by default:
* All existing features offered by Drupal (Role-based user management, customizable fields on all entities, etc), Views (easily visualize information in any format you want) and Rules (automate workflows without any coding).   
* Knowledge base
* Discussions (public product forums)
* Ticketing system
  * Ticket submission by both anonymous and logged in users.
  * Email integration for replying to tickets (for both users and support agents).
  * Assign tickets to support agents.
  * Configurable ticket status (open/closed/etc).
  * Configurable ticket categories (feature request/bug report/technical support/etc)
  * Configurable ticket priorities (urgent/high/low/etc).
  * Ticket tagging.
* Multi-lingual support
  * Farsi and English are available by default.
  * Default theme supports RTL layout.
* RESTful API for integration with your own product using the [Services](https://www.drupal.org/project/services) module.

## Installation
Before you continue please make sure that you have all the necessary [requirements](https://www.drupal.org/requirements). You also need a [Mandrill](http://mandrill.com/) account for the email integration to work.
For the rest of this guide I'll be assuming that deskulu will be accessible from http://yoursite.com/.

### Configure Mandrill for sending/receiving emails
1. Open your Mandrill account and go to https://mandrillapp.com/settings/sending-domains.
2. Add a new sending domain for Deskulu (for example helpmail.yoursite.com).
3. Visit https://mandrillapp.com/inbound and add helpmail.yoursite.com domain.
4. On the same page click on helpmail.yoursite.com in the table.
5. Click on the "+Add New Route" button.
  * When The Receiving Email Address Matches: support@helpmail.yoursite.com
  * Post To URL: http://yoursite.com/tickets/incoming/new
6. Add another route by clicking on the "+Add New Route" button again.
  * When The Receiving Email Address Matches: ticket-*-*@helpmail.yoursite.com
  * Post To URL: http://yoursite.com/tickets/incoming/new
7. Visit https://mandrillapp.com/settings/index and generate a new API key for the helpdesk. You'll need this API key later.
8. Open your own mail administration tool and create an Alias to forward all emails sent to support@yoursite.com to support@mail.taskulu.com.


### Install and configure Deskulu
1. Clone the repository (or download the [zip file](https://github.com/Taskulu/deskulu/archive/master.zip) and extrac it) in your webserver's documentroot.
2. Create a new database and import deskulu.sql.gz
3. Adjust your [PHP configuration](https://www.drupal.org/requirements/php) to make sure it meets Druapl requirements.
4. Configure your web server
  * [Nginx configuration](https://github.com/perusio/drupal-with-nginx)
  * [Apache configuration](https://www.digitalocean.com/community/tutorials/how-to-install-drupal-on-an-ubuntu-14-04-server-with-apache)
5. Open sites/default/settings.php, replace DB_NAME, USERNAME and PASSWORD with your database name, username and password.
6. Open the website in your browser and login
  * Username: admin
  * Password: admin
7. Visit http://yoursite.com/user/1/edit and change your password.
8. Visit http://yoursite.com/admin/config/system/site-information (Configuration > System > Site Information in the top menu) and change Site name and Email address.
9. Visit http://yoursite.com/admin/appearance/settings/ht (Appearance > Settings > Helpdesk theme), scroll down "Override Global Settings" select "Logo image settings" and upload a new logo.
10. Visit http://yoursite.com/admin/config/regional/language (Configuration > System > Regional and language > Languages)
  * If you don't need the Persian language, delete it, otherwise Enable it.
  * If you need any other languages, see the Localization section below.
  * If you don't need the multiple languages at all, visit http://yoursite.com/admin/modules (Modules in the top menu) and disable all modules related to "Multilingual", "Multilingual - Entity Translation" and "Multilingual - Internationalization". Also disable "Calendar Systems" and "Locale" modules (Please note that you need to disable modules that depend on these modules first).
11. Open http://yoursite.com/admin/config/workflow/rules/reaction/manage/rules_send_thank_you_email_for_new_tickets/edit/3
  * Find SENDER E-MAIL ADDRESS and change its value to ticket-[node:nid]-[node:field-security-token]@helpmail.yoursite.com
12. Open http://yoursite.com/admin/config/workflow/rules/reaction/manage/rules_send_email_to_support_agent/edit/4
  * Find SENDER E-MAIL ADDRESS and change its value to ticket-[node:nid]-[node:field-security-token]@helpmail.yoursite.com
13. Open http://yoursite.com/admin/config/workflow/rules/reaction/manage/rules_notify_agents_about_new_comments/edit/3
  * Find SENDER E-MAIL ADDRESS and change its value to ticket-[comment:node:nid]-[comment:node:field-security-token]@helpmail.yoursite.com
14. Open http://yoursite.com/admin/config/workflow/rules/reaction/manage/rules_send_notification_email_to_site_owners/edit/3
  * Find REPLY E-MAIL ADDRESS and change its value to ticket-[node:nid]-[node:field_security_token]@helpmail.yoursite.com
15. Open http://yoursite.com/admin/config/services/mandrill and set the Mandrill API Key.

**Note:** You can see all automation Rules and add/remove/edit them here: http://yoursite.com/admin/config/workflow/rules (Configuration > Workflow > Rules).

###Localization
You can enable as many languages as you need. To do that you first need to add thos languages from http://yoursite.com/admin/config/regional/language (Configuration > Regional and language > Languages). Most of languages have translations available in https://localize.drupal.org/. You can download translations from this site and import them in http://yoursite.com/admin/config/regional/translate/import (Configuration > Regional and language > Translate interface > Import). If there are strings that are not translated after importing, you can use the form at http://yoursite.com/admin/config/regional/translate/translate (Configuration > Regional and language > Translate interface > Translate) to translate them yourself.

###Adding agents
If you want to add agents use the form at http://yoursite.com/admin/people/create (People > Add user) and give them the "agent" role. If you want to change what agents can and cannot do, checkout this page: http://yoursite.com/admin/people/permissions (People > Permissions).

###Changing heldesk article and ticket categories, priorities and statuses and discussion forums. 
If you need to change categories, priorities or statuses, visit http://yoursite.com/admin/structure/taxonomy (Structure > Taxonomy). Click on List terms in from of each Vocabulary to see/edit the list of available options.

###Changing Menues
You probably want to change the menus. You can do that from here: http://yoursite.com/admin/structure/menu (Structre > Menu).

###Changing fields on helpdesk articles, tickets and discussion forums
If you need to add/remove fields to helpdesk articles or tickets visit this page: http://yoursite.com/admin/structure/types (Structure > Content types).

###Using the RESTful API
Please see the documentation of [Services](https://www.drupal.org/project/services) module.

##Roadmap
* Add a more minimal theme.
* Create a Drupal distribution that automates installation and initial configuration.
* Add support for SendGrid and other transactional mail systems.
* Add configuration forms and introduce tokens to make it possible to change Rules configs from a single form.

##License
GPLv2. See LICENSE.txt.