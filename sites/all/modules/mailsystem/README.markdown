## [Mail System](http://drupal.org/project/mailsystem)

Provides an Administrative UI and Developers API for safely updating the
[mail_system](http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7)
configuration variable.

### Administrative UI

The administrative interface is at `admin/config/system/mailsystem`.
A [screenshot](http://drupal.org/node/1134044) is available.

### Used by:

* [HTML Mail](http://drupal.org/project/htmlmail)
* [Mime Mail 7.x-1.x-dev](http://drupal.org/project/mimemail)
* [Postmark 7.x-1.x](http://drupal.org/project/postmark)

### Developers API

A module `example` with a
[`MailSystemInterface`](http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7)
implementation called `ExampleMailSystem` should add the following in its
`example.install` file:

    /**
     * Implements hook_enable().
     */
    function example_enable() {
      mailsystem_set(array('example' =\> 'ExampleMailSystem'));
    }

    /**
     * Implements hook_disable().
     */
    function example_disable() {
      mailsystem_clear(array('example' =\> 'ExampleMailSystem'));
    }

The above settings allow mail sent by `example` to use `ExampleMailSystem`.  To make
`ExampleMailSystem` the site-wide default for sending mail:

    mailsystem_set(array(mailsystem_default_id() =\> 'ExampleMailSystem'));

To restore the default mail system:

    mailsystem_set(array(mailsystem_default_id() =\> mailsystem_default_value()));

Or simply:

    mailsystem_set(mailsystem_defaults());

If module `example` relies on dependency `foo` and its `FooMailSystem` class, then
the `example.install` code should like like this:

    /**
     * Implements hook_enable().
     */
    function example_enable() {
      mailsystem_set(array('example' =\> 'FooMailSystem'));
    }

    /**
     * Implements hook_disable().
     */
    function example_disable() {
      mailsystem_clear(array('example' =\> ''));
    }

If module `example` only wants to use `FooMailSystem` when sending emails with a key
of `examail`, then the `example.install` code should look like this:

    /**
     * Implements hook_enable().
     */
    function example_enable() {
      mailsystem_set(array('example_examail' =\> 'FooMailSystem'));
    }

    /**
     * Implements hook_disable().
     */
    function example_disable() {
      mailsystem_clear(array('example_examail' =\> ''));
    }

#### *(New in 2.x branch)*

To change the site-wide defaults to use the `FooMailSystem` for formatting messages and the `BarMailSystem` for sending them:

    mailsystem_set(
      array(
        mailsystem_default_id() => array(
          'format' => 'FooMailSystem',
          'mail' => 'BarMailSystem',
        ),
      )
    );

To change the site-wide defaults to use the `FooMailSystem` for sending messages, while continuing to use the current system for formatting them:

    mailsystem_set(
      array(
        mailsystem_default_id() => array(
          'mail' => 'FooMailsystem',
        ),
      )
    );

### References

**[`drupal_mail_system()` API documentation](http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7)**:
:    [api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7](http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7)

**[`MailSystemInterface` API documentation](http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7)**:
:    [api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7](http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7)

**[Creating HTML formatted mails in Drupal 7](http://drupal.org/node/900794)**:
:    [drupal.org/node/900794](http://drupal.org/node/900794)
