<?php
/**
 * @file
 * Variable information
 */

/**
 * Implements hook_variable_info().
 */
function i18n_select_variable_info($options = array()) {
  $variables['i18n_select_nodes'] = array(
    'title' => t('Select nodes by language', array(), $options),
    'type' => 'boolean',
    'default' => TRUE,
    'group' => 'i18n',
  );
  $variables['i18n_select_taxonomy'] = array(
    'title' => t('Select taxonomy terms by language', array(), $options),
    'type' => 'boolean',
    'default' => TRUE,
    'group' => 'i18n',
    'element' => array('#disabled' => !module_exists('i18n_taxonomy')),
  );

  // Enable / disable for specific pages
  $description = t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'), $options);
  if (module_exists('php')) {
    $title = t('Pages or PHP code');
    $description .= ' ' . t('If the PHP option is chosen, enter PHP code between %php. Note that executing incorrect PHP code can break your Drupal site.', array('%php' => '<?php ?>'), $options);
  }
  else {
    $title = t('Pages', array(), $options);
  }
  $variables['i18n_select_page_mode'] = array(
    'type' => 'select',
    'options callback' => 'i18n_select_variable_option_list',
    'default' => I18N_SELECT_PAGE_NOTLISTED,
  );
  $variables['i18n_select_page_list'] = array(
    'type' => 'text',
    'title' => $title,
    'default' => 'admin/*',
    'description' => $description,
  );
  $variables['i18n_select_page_block'] = array(
    'type' => 'boolean',
    'title' => t('Enable always for block content though it may be disabled for the page', array(), $options),
    'default' => TRUE,
  );
  $variables['i18n_select_skip_tags'] = array(
    'title' => t('Skip tags', array(), $options),
    'type' => 'string',
    'default' => 'views',
    'group' => 'i18n',
    'description' => t('Skip queries with these tags. Enter a list of tags separated by commas.'),
    'localize' => FALSE,
  );
  return $variables;
}

/**
 * Options for page selection mode
 */
function i18n_select_variable_option_list($variable, $options = array()) {
  $options = array(
    I18N_SELECT_PAGE_NOTLISTED => t('All pages except those listed', array(), $options),
    I18N_SELECT_PAGE_LISTED => t('Only the listed pages', array(), $options),
  );
  if (module_exists('php')) {
    $options += array(I18N_SELECT_PAGE_PHP => t('Pages on which this PHP code returns <code>TRUE</code> (experts only)'));
  }
  return $options;
}