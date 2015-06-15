Project Description
================================================================================

Overview
--------------------------------------------------------------------------------
The Services Documentation module generates API documentation for your site’s Services resources. Most of the documentation is generated automatically, using the resource information contained provided by Services. However, this module provides you with the ability to easily extend that documentation.

We make the following assumptions about your usage of the Services module:

* You are providing an API to a third party, which allows them to access your data via Services.
* Your API will be organized by version. E.g., API version 1.x, 2.x, etc.
* You would like to provide a page to API consumers that documents your API endpoints, resources, etc.
* You have enough familiarity with Drupal to create a module and implement a hook.

Documentation will be made available on your Drupal site at /developers. It will be organized by API version (arbitrarily defined by you), and subsequently displayed hierarchically according to resource, method type, method name, arguments, etc.

Usage
--------------------------------------------------------------------------------
A full example implementation can be found in services_documentation/services_documentation.api.php

* Create a new, custom module. Let’s call it myapi.module.
* Implement hook_services_resources_alter($resources);
* For each resource method that you would like to document, add two new rows to the $resource array: ‘documentation callback’ and 'documentation versions'.
<pre>
  $resources['user']['index']['documentation callback'] = '_myapi_uses_index_doc';
  $resources['user']['index']['documentation versions'] = array(1000);
</pre>

Define the documentation callback, which should return an element in the follow format:
<pre>
/**
 * Documentation callback for index operation of users resource.
 */
function _myapi_uses_index_doc() {
  $element = array(
    '#name' => t('name'),
    '#description' => t('desc'),
    // Example request. E.g., a request URL, headers, and a JSON array.
    '#request' => t('request'),
    // Example response. E.g., a JSON array.
    '#response' => t('response'),
  );

  return $element;
}
</pre>

After enabling this module, a list of API version numbers will appear at
/developers. Clicking on the version number will display a list of resources,
their arguments, endpoints, etc.




Contributing
--------------------------------------------------------------------------------
Anyone is encouraged to contribute to this project.
By contributing to this project, you grant a world-wide, royalty-free, perpetual, irrevocable, non-exclusive, transferable license to all users under the terms of the Gnu General Public License v2 or later.
All comments, messages, pull requests, and other submissions received through official White House pages including this Drupal.org page are subject to the Presidential Records Act and may be archived. Learn more http://WhiteHouse.gov/privacy


License
--------------------------------------------------------------------------------
This project constitutes a work of the United States Government and is not subject to domestic copyright protection under 17 USC Â§ 105.
The project utilizes code licensed under the terms of the GNU General Public License and therefore is licensed under GPL v2 or later.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.
