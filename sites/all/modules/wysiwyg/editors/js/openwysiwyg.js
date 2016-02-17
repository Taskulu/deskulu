
// Backup $ and reset it to jQuery.
Drupal.wysiwyg._openwysiwyg = $;
$ = jQuery;

// Wrap openWYSIWYG's methods to temporarily use its version of $.
jQuery.each(WYSIWYG, function (key, value) {
  if (jQuery.isFunction(value)) {
    WYSIWYG[key] = function () {
      var old$ = $;
      $ = Drupal.wysiwyg._openwysiwyg;
      var result = value.apply(this, arguments);
      $ = old$;
      return result;
    };
  }
});

// Override editor functions.
WYSIWYG.getEditor = function (n) {
  return Drupal.wysiwyg._openwysiwyg("wysiwyg" + n);
};

(function($) {

// Fix Drupal toolbar obscuring editor toolbar in fullscreen mode.
var oldMaximize = WYSIWYG.maximize;
WYSIWYG.maximize = function (n) {
var $drupalToolbar = $('#toolbar', Drupal.overlayChild ? window.parent.document : document);
  oldMaximize.apply(this, arguments);
  if (this.maximized[n]) {
    $drupalToolbar.hide();
  }
  else {
    $drupalToolbar.show();
  }
}

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.openwysiwyg = function(context, params, settings) {
  // Initialize settings.
  settings.ImagesDir = settings.path + 'images/';
  settings.PopupsDir = settings.path + 'popups/';
  settings.CSSFile = settings.path + 'styles/wysiwyg.css';
  //settings.DropDowns = [];
  var config = new WYSIWYG.Settings();
  for (var setting in settings) {
    config[setting] = settings[setting];
  }
  // Attach editor.
  WYSIWYG.setSettings(params.field, config);
  WYSIWYG_Core.includeCSS(WYSIWYG.config[params.field].CSSFile);
  WYSIWYG._generate(params.field, config);
};

/**
 * Detach a single or all editors.
 */
Drupal.wysiwyg.editor.detach.openwysiwyg = function (context, params, trigger) {
  if (typeof params != 'undefined') {
    var instance = WYSIWYG.config[params.field];
    if (typeof instance != 'undefined') {
      WYSIWYG.updateTextArea(params.field);
      if (trigger != 'serialize') {
        jQuery('#wysiwyg_div_' + params.field).remove();
        delete instance;
      }
    }
    if (trigger != 'serialize') {
      jQuery('#' + params.field).show();
    }
  }
  else {
    jQuery.each(WYSIWYG.config, function(field) {
      WYSIWYG.updateTextArea(field);
      if (trigger != 'serialize') {
        jQuery('#wysiwyg_div_' + field).remove();
        delete this;
        jQuery('#' + field).show();
      }
    });
  }
};

/**
 * Instance methods for openWYSIWYG.
 */
Drupal.wysiwyg.editor.instance.openwysiwyg = {
  insert: function (content) {
    // If IE has dropped focus content will be inserted at the top of the page.
    $('#wysiwyg' + this.field).contents().find('body').focus();
    WYSIWYG.insertHTML(content, this.field);
  },

  setContent: function (content) {
    // Based on openWYSIWYG's _generate() method.
    var doc = WYSIWYG.getEditorWindow(this.field).document;
    if (WYSIWYG.config[this.field].ReplaceLineBreaks) {
      content = content.replace(/\n\r|\n/ig, '<br />');
    }
    if (WYSIWYG.viewTextMode[this.field]) {
      var html = document.createTextNode(content);
      doc.body.innerHTML = '';
      doc.body.appendChild(html);
    }
    else {
      doc.open();
      doc.write(content);
      doc.close();
    }
  },

  getContent: function () {
    // Based on openWYSIWYG's updateTextarea() method.
    var content = '';
    var doc = WYSIWYG.getEditorWindow(this.field).document;
    if (WYSIWYG.viewTextMode[this.field]) {
      if (WYSIWYG_Core.isMSIE) {
        content = doc.body.innerText;
      }
      else {
        var range = doc.body.ownerDocument.createRange();
        range.selectNodeContents(doc.body);
        content = range.toString();
      }
    }
    else {
      content = doc.body.innerHTML;
    }
    content = WYSIWYG.stripURLPath(this.field, content);
    content = WYSIWYG_Core.replaceRGBWithHexColor(content);
    if (WYSIWYG.config[this.field].ReplaceLineBreaks) {
      content = content.replace(/(\r\n)|(\n)/ig, '');
    }
    return content;
  }
};

})(jQuery);
