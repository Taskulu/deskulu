<?php

function views_bulk_operations_script_action_info() {
  $actions = array();
  $actions['views_bulk_operations_script_action'] = array(
    'type' => 'entity',
    'label' => t('Execute arbitrary PHP script'),
    'configurable' => TRUE,
    'triggers' => array('any'),
  );
  // Provide a strict default permission if actions_permissions is disabled.
  if (!module_exists('actions_permissions')) {
    $actions['views_bulk_operations_script_action']['permissions'] = array('administer site configuration');
  }

  return $actions;
}

function views_bulk_operations_script_action($entity, $context) {
  $return = eval($context['script']);
  if ($return === FALSE) {
    $msg = 'Error in script.';
    $arg = array();
    $error = error_get_last();
    if ($error) {
      $msg = '!err in script: !msg in line \'%line\'.';
      $arg = array(
        '!msg' => $error['message'],
        '%line' => _views_bulk_operations_script_action_error_line($context['script'], $error['line']),
        '!err' => _views_bulk_operations_script_action_error_type($error['type']),
      );
    }
    drupal_set_message(t($msg, $arg), 'error', FALSE);
    watchdog('actions', $msg, $arg, WATCHDOG_ERROR);
  }
}

function views_bulk_operations_script_action_form($context) {
  $form['script'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP script'),
    '#description' => t('Type the PHP snippet that will run upon execution of this action. You can use variables <code>$entity</code> and <code>$context</code> in your snippet.
                         Note that it is up to the script to save the $entity once it\'s done modifying it.'),
    '#default_value' => @$context['script'],
  );
  return $form;
}

function views_bulk_operations_script_action_validate($form, $form_state) {
}

function views_bulk_operations_script_action_submit($form, $form_state) {
  return array(
    'script' => $form_state['values']['script'],
  );
}

function _views_bulk_operations_script_action_error_line($script, $line) {
  $lines = preg_split("/(\r?\n)/", $script);
  if (isset($lines[$line-1])) {
    return $lines[$line-1];
  }
  else {
    return t('Line !line', array('!line' => $line));
  }
}

function _views_bulk_operations_script_action_error_type($type) {
  $types = array(
    E_ERROR              => 'Error',
    E_WARNING            => 'Warning',
    E_PARSE              => 'Parsing Error',
    E_NOTICE             => 'Notice',
    E_CORE_ERROR         => 'Core Error',
    E_CORE_WARNING       => 'Core Warning',
    E_COMPILE_ERROR      => 'Compile Error',
    E_COMPILE_WARNING    => 'Compile Warning',
    E_USER_ERROR         => 'User Error',
    E_USER_WARNING       => 'User Warning',
    E_USER_NOTICE        => 'User Notice',
    E_STRICT             => 'Runtime Notice',
    E_RECOVERABLE_ERROR  => 'Catchable Fatal Error',
  );
  if (version_compare(PHP_VERSION, '5.3.0') >= 0) {
    $types += array(
      E_DEPRECATED         => 'Deprecated Notice',
      E_USER_DEPRECATED    => 'User Deprecated Notice',
    );
  }

  return t($types[$type]);
}
