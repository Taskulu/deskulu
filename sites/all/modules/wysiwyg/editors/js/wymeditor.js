(function($) {

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.wymeditor = function (context, params, settings) {
  // Prepend basePath to wymPath.
  settings.wymPath = settings.basePath + settings.wymPath;
  // Update activeId on focus.
  settings.postInit = function (instance) {
    $(instance._doc).focus(function () {
      Drupal.wysiwyg.activeId = params.field;
    });
  };
  // Attach editor.
  $('#' + params.field).wymeditor(settings);
};

/**
 * Detach a single or all editors.
 */
Drupal.wysiwyg.editor.detach.wymeditor = function (context, params, trigger) {
  if (typeof params != 'undefined') {
    var $field = $('#' + params.field);
    var index = $field.data(WYMeditor.WYM_INDEX);
    if (typeof index != 'undefined') {
      var instance = WYMeditor.INSTANCES[index];
      instance.update();
      if (trigger != 'serialize') {
        $(instance._box).remove();
        $(instance._element).show();
        delete instance;
      }
    }
    if (trigger != 'serialize') {
      $field.show();
    }
  }
  else {
    jQuery.each(WYMeditor.INSTANCES, function () {
      this.update();
      if (trigger != 'serialize') {
        $(this._box).remove();
        $(this._element).show();
        delete this;
      }
    });
  }
};

Drupal.wysiwyg.editor.instance.wymeditor = {
  insert: function (content) {
    this.getInstance().insert(content);
  },

  setContent: function (content) {
    this.getInstance().html(content);
  },

  getContent: function () {
    return this.getInstance().xhtml();
  },

  getInstance: function () {
    var $field = $('#' + this.field);
    var index = $field.data(WYMeditor.WYM_INDEX);
    if (typeof index != 'undefined') {
      return WYMeditor.INSTANCES[index];
    }
    return null;
  }
};

})(jQuery);
