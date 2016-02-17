
var wysiwygWhizzywig = { currentField: null, fields: {} };
var buttonPath = null;

/**
 * Override Whizzywig's document.write() function.
 *
 * Whizzywig uses document.write() by default, which leads to a blank page when
 * invoked in jQuery.ready().  Luckily, Whizzywig developers implemented a
 * shorthand w() substitute function that we can override to redirect the output
 * into the global wysiwygWhizzywig variable.
 *
 * @see o()
 */
var w = function (string) {
  if (string) {
    wysiwygWhizzywig.fields[wysiwygWhizzywig.currentField] += string;
  }
  return wysiwygWhizzywig.fields[wysiwygWhizzywig.currentField];
};

/**
 * Override Whizzywig's document.getElementById() function.
 *
 * Since we redirect the output of w() into a temporary string upon attaching
 * an editor, we also have to override the o() shorthand substitute function
 * for document.getElementById() to search in the document or our container.
 * This override function also inserts the editor instance when Whizzywig
 * tries to access its IFRAME, so it has access to the full/regular window
 * object.
 *
 * @see w()
 */
var o = function (id) {
  // Upon first access to "whizzy" + id, Whizzywig tries to access its IFRAME,
  // so we need to insert the editor into the DOM.
  if (id == 'whizzy' + wysiwygWhizzywig.currentField && wysiwygWhizzywig.fields[wysiwygWhizzywig.currentField]) {
    jQuery('#' + wysiwygWhizzywig.currentField).after('<div id="' + wysiwygWhizzywig.currentField + '-whizzywig"></div>');
    // Iframe's .contentWindow becomes null in Webkit if inserted via .after().
    jQuery('#' + wysiwygWhizzywig.currentField + '-whizzywig').html(w());
    // Prevent subsequent invocations from inserting the editor multiple times.
    wysiwygWhizzywig.fields[wysiwygWhizzywig.currentField] = '';
  }
  // If id exists in the regular window.document, return it.
  if (jQuery('#' + id).size()) {
    return jQuery('#' + id).get(0);
  }
  // Otherwise return id from our container.
  return jQuery('#' + id, w()).get(0);
};

(function($) {

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.whizzywig = function(context, params, settings) {
  // Previous versions used per-button images found in this location,
  // now it is only used for custom buttons.
  if (settings.buttonPath) {
    window.buttonPath = settings.buttonPath;
  }
  // Assign the toolbar image path used for native buttons, if available.
  if (settings.toolbarImagePath) {
    btn._f = settings.toolbarImagePath;
  }
  // Fall back to text labels for all buttons.
  else {
    window.buttonPath = 'textbuttons';
  }
  // Create Whizzywig container.
  wysiwygWhizzywig.currentField = params.field;
  wysiwygWhizzywig.fields[wysiwygWhizzywig.currentField] = '';
  // Whizzywig needs to have the width set 'inline'.
  var $field = $('#' + params.field);
  this.originalStyle = $field.attr('style');
  $field.css('width', $field.width() + 'px');

  // Attach editor.
  makeWhizzyWig(params.field, (settings.buttons ? settings.buttons : 'all'));
  // Whizzywig fails to detect and set initial textarea contents.
  $('#whizzy' + params.field).contents().find('body').html(tidyD($field.val()));
};

/**
 * Detach a single or all editors.
 */
Drupal.wysiwyg.editor.detach.whizzywig = function (context, params, trigger) {
  var instance = this;
  var detach = function (index) {
    var id = whizzies[index], $field = $('#' + id);

    // Save contents of editor back into textarea.
    $field.val(instance.getContent());
    // If the editor is just being serialized (not detached), our work is done.
    if (trigger == 'serialize') {
      return;
    }
    // Remove editor instance.
    $('#' + id + '-whizzywig').remove();
    whizzies.splice(index, 1);

    // Restore original textarea styling.
    if ('originalStyle' in instance) {
      $field.removeAttr('style').attr('style', instance.originalStyle);
    }
  };

  if (typeof params != 'undefined') {
    for (var i = 0; i < whizzies.length; i++) {
      if (whizzies[i] == params.field) {
        detach(i);
        break;
      }
    }
  }
  else {
    while (whizzies.length > 0) {
      detach(0);
    }
  }
};

/**
 * Instance methods for Whizzywig.
 */
Drupal.wysiwyg.editor.instance.whizzywig = {
  insert: function (content) {
    // Whizzywig executes any string beginning with 'js:'.
    insHTML(content.replace(/^js:/, 'js&colon;'));
  },

  setContent: function (content) {
    // Whizzywig shows the original textarea in source mode.
    if ($field.css('display') == 'block') {
      $('#' + this.field).val(content);
    }
    else {
      var doc = $('#whizzy' + this.field).contents()[0];
      doc.open();
      doc.write(content);
      doc.close();
    }
  },

  getContent: function () {
    var $field = $('#' + this.field);
    // Whizzywig's tidyH() expects a document node. Clone the editing iframe's
    // document so tidyH() won't mess with it if this gets called while editing.
    var clone = $($('#whizzy' + this.field).contents()[0].documentElement).clone()[0].ownerDocument;
    // Whizzywig shows the original textarea in source mode so update the body.
    if ($field.css('display') == 'block') {
     clone.body.innerHTML = $field.val();
    }
    return tidyH(clone);
  }
};
})(jQuery);
