<?php

/**
 * Implements hook_token_info()
 */
function token_test_token_info() {
  $info['tokens']['node']['colons:in:name'] = array(
    'name' => t('A test token with colons in the name'),
    'description' => NULL,
  );

  return $info;
}
