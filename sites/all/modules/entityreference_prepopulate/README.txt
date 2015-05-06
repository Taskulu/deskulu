Description
===========
Allows the contents of an "Entity Reference" field to be pre-populated by
taking a parameter from the URL path.

Install
=======
1. Download and enable the module.
2. Visit admin/structure/types/manage/[ENTITY-TYPE]/fields/[FIELD-NAME]
3. Enable "Entity reference prepopulate" under the instance settings.


Configuration
=============
Enable Entity reference prepopulate:
  Check this to enable Entity reference prepopulate on this field.
Action
  Using the select box choose the action to take if the entity reference
  field is pre-populated.
Fallback behaviour
  Select what to do if the URL path does NOT contain a parameter to
  pre-populate the field.
Skip access permission
  This is a fallback override, the fallback behaviour will not be followed
  for users with the specified permission.

Usage
=====
In order to pre-populate an entity reference field you have to supply the
parameter in the URL.

The structure is
node/add/article?[field_ref]=[id]

Where [field_ref] is the name of the entity reference field and [id] is
the id of the entity being referenced.

Examples:
node/add/article?field_foo=1
node/add/page?field_bar=1,2,3


