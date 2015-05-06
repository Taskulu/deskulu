<?php
/**
 * Created by IntelliJ IDEA.
 * User: svip
 * Date: 19.07.13
 * Time: 17:09
 * To change this template use File | Settings | File Templates.
 */

function select2widget_process_callback($element) {
  // #multiple select fields need a special #name.
  if ($element['#multiple']) {
    $element['#attributes']['multiple'] = 'multiple';
    $element['#attributes']['name'] = $element['#name'] . '[]';
  }
  // A non-#multiple select needs special handling to prevent user agents from
  // preselecting the first option without intention. #multiple select lists do
  // not get an empty option, as it would not make sense, user interface-wise.
  else {
    $required = $element['#required'];
    // If the element is required and there is no #default_value, then add an
    // empty option that will fail validation, so that the user is required to
    // make a choice. Also, if there's a value for #empty_value or
    // #empty_option, then add an option that represents emptiness.
    if (($required && !isset($element['#default_value'])) || isset($element['#empty_value']) || isset($element['#empty_option'])) {
      $element += array(
        '#empty_value' => '',
        '#empty_option' => $required ? t('- Select -') : t('- None -'),
      );
      // The empty option is prepended to #options and purposively not merged
      // to prevent another option in #options mistakenly using the same value
      // as #empty_value.
      $empty_option = array($element['#empty_value'] => $element['#empty_option']);
      $element['#options'] = $empty_option + $element['#options'];
    }
  }
  // Send Drupal.settings a reference to this form element.
  $data['select2widget2']['elements'][$element['#id']] = array(
    'id' => $element['#id'],
    'multiple' => $element['#multiple']
  );

  // Attaching library, integration script, and settings array.
  $element['#attached']['library'][] = array('select2widget', 'select2');
  $element['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => $data,
  );

  return $element;
}

/**
 * Form element validation handler for options element.
 */
function select2widget_field_widget_validate($element, &$form_state) {
  if ($element['#required'] && $element['#value'] == '_none') {
    form_error($element, t('@name field is required.', array('@name' => $element['#title'])));
  }
  // Transpose selections from field => delta to delta => field, turning
  // multiple selected options into multiple parent elements.
  $items = _options_form_to_storage($element);
  form_set_value($element, $items, $form_state);
}


/**
 * Describes the preparation steps required by each widget.
 */
function _select2widget_options_properties($type, $multiple, $required, $has_value) {
  $base = array(
    'filter_xss' => FALSE,
    'strip_tags' => FALSE,
    'empty_option' => FALSE,
    'optgroups' => FALSE,
  );

  $properties = array();

  switch ($type) {
    case 'select2widget':
      $properties = array(
        // Select boxes do not support any HTML tag.
        'strip_tags' => TRUE,
        'optgroups' => TRUE,
      );
      if (!$multiple) {
        // Single select: add a 'none' option for non-required fields,
        // and a 'select a value' option for required fields that do not come
        // with a value selected.
        if (!$required) {
          $properties['empty_option'] = 'option_none';
        }
        elseif (!$has_value) {
          $properties['empty_option'] = 'option_select';
        }
      }
      break;
  }

  return $properties + $base;
}



/**
 * Collects the options for a field.
 */
function _select2widget_options_get_options($field, $instance, $properties, $entity_type, $entity) {
  // Get the list of options.
  $options = (array) module_invoke($field['module'], 'options_list', $field, $instance, $entity_type, $entity);

  // Sanitize the options.
  _options_prepare_options($options, $properties);

  if (!$properties['optgroups']) {
    $options = options_array_flatten($options);
  }

  if ($properties['empty_option']) {
    $label = theme('select2widget_options_none', array(
      'instance' => $instance,
      'option' => $properties['empty_option'],
    ));
    $options = array('_none' => $label) + $options;
  }

  return $options;
}
