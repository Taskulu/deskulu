Email Registration allows users to register and login with their e-mail address
instead of using a separate username in addition to the e-mail address. It will
automatically generate a username based on the e-mail address but that behavior
can be overridden with a custom hook implementation in a site specific module.

INSTALLATION
============

Required step:

1. Enable the module as you normally would.


Optional steps:

2. You will probably want to change the welcome e-mail
    (Administer -> User Management -> User Settings) and replace instances of
    the token !username with !mailto

3. This automatically generated username is still displayed name for posts,
    comments, etc. You can allow your useres to change their username by
    going to: (Administer -> User Management -> Access Control) and granting
    the permission to "change own username"
    This privilege allows a user to change their username in "My Account".

4. If a user enters an invalid email or password they will see a message:
 "Sorry, unrecognized username or password. Have you forgotten your password?"
    That message is confusing because it mentions username when all other
    language on the page mentions entering their E-mail. This can be easily
    overridden in your settings.php file with an entry like this:

$conf['locale_custom_strings_en'][''] = array(
  'Sorry, unrecognized username or password. <a href="@password">Have you forgotten your password?</a>' => 'Sorry, unrecognized e-mail or password. <a href="@password">Have you forgotten your password?</a>',
);


BUGS, FEATURES, QUESTIONS
=========================
Post any bugs, features or questions to the issue queue:

http://drupal.org/project/issues/email_registration
