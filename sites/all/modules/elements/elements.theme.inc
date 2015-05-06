<?php

/**
 * @file
 * The theme include file for the elements module.
 *
 * Contains the theme functions for all the elements module elements.
 */

/**
 * Returns HTML for an emailfield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #size, #maxlength,
 *     #placeholder, #required, #attributes, #autocomplete_path.
 *
 * @ingroup themeable
 */
function theme_emailfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'email';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength', 'placeholder'));
  _form_set_class($element, array('form-text', 'form-email'));

  $extra = elements_add_autocomplete($element);
  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Returns HTML for a searchfield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #size, #maxlength,
 *     #placeholder, #required, #attributes, #autocomplete_path.
 *
 * @ingroup themeable
 */
function theme_searchfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'search';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength', 'placeholder'));
  _form_set_class($element, array('form-text', 'form-search'));

  $extra = elements_add_autocomplete($element);
  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Returns HTML for a telfield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #size, #maxlength,
 *     #placeholder, #required, #attributes.
 *
 * @ingroup themeable
 */
function theme_telfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'tel';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength', 'placeholder'));
  _form_set_class($element, array('form-text', 'form-tel'));

  $extra = elements_add_autocomplete($element);
  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Returns HTML for an urlfield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #size, #maxlength,
 *     #placeholder, #required, #attributes, #autocomplete_path.
 *
 * @ingroup themeable
 */
function theme_urlfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'url';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength', 'placeholder'));
  _form_set_class($element, array('form-text', 'form-url'));

  $extra = elements_add_autocomplete($element);
  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Returns HTML for a numberfield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #min, #max, #placeholder,
 *     #required, #attributes, #step.
 *
 * @ingroup themeable
 */
function theme_numberfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'number';
  element_set_attributes($element, array('id', 'name', 'value', 'step', 'min', 'max', 'placeholder'));
  _form_set_class($element, array('form-text', 'form-number'));

  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output;
}

/**
 * Returns HTML for a rangefield form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #value, #description, #min, #max, #attributes,
 *     #step.
 *
 * @ingroup themeable
 */
function theme_rangefield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'range';
  element_set_attributes($element, array('id', 'name', 'value', 'step', 'min', 'max'));
  _form_set_class($element, array('form-text', 'form-range'));

  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output;
}
