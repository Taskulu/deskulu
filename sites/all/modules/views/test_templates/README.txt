Workaround for:

- https://www.drupal.org/node/2450447
- https://www.drupal.org/node/2415991


Files of this folder cannot be included inside the views/tests directory because
they are included as tests cases and make testbot crash.

This files could be moved to tests/templates once
https://www.drupal.org/node/2415991 be properly fixed.
