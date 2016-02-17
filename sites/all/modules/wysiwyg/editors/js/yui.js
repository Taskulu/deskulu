(function($) {

/**
 * Attach this editor to a target element.
 *
 * Since buttons must be added before the editor is rendered, we add plugins
 * buttons on attach event rather than in init.
 */
Drupal.wysiwyg.editor.attach.yui = function(context, params, settings) {
  // Apply theme.
  $('#' + params.field).parent().addClass('yui-skin-' + settings.theme);

  var wysiwygInstance = this;
  var enabledPlugins = wysiwygInstance.pluginInfo.instances;
  // Load plugins stylesheet.
  for (var pluginId in enabledPlugins.drupal) {
    if (wysiwygInstance.pluginInfo.global.drupal[pluginId].css) {
      settings.extracss += ' @import "' + wysiwygInstance.pluginInfo.global.drupal[pluginId].css + '"; ';
    }
  }

  // Attach editor.
  var editor = new YAHOO.widget.Editor(params.field, settings);

  if (enabledPlugins) {
    editor.on('toolbarLoaded', function() {
      // 'this' will reference the toolbar while inside the event handler.
      var instanceId = params.field;
      // Load Drupal plugins.
      for (var plugin in enabledPlugins.drupal) {
        wysiwygInstance.addPlugin(plugin, wysiwygInstance.pluginInfo.global.drupal[plugin]);
      }
    });

    // Allow plugins to act on setEditorHTML.
    var oldSetEditorHTML = editor.setEditorHTML;
    editor.setEditorHTML = function (content) {
      for (var plugin in enabledPlugins.drupal) {
        var pluginSettings = wysiwygInstance.pluginInfo.global.drupal[plugin];
        if (typeof Drupal.wysiwyg.plugins[plugin].attach == 'function') {
          content = Drupal.wysiwyg.plugins[plugin].attach(content, pluginSettings, params.field);
          content = wysiwygInstance.prepareContent(content);
        }
      }
      oldSetEditorHTML.call(this, content);
    };

    // Allow plugins to act on getEditorHTML.
    var oldGetEditorHTML = editor.getEditorHTML;
    editor.getEditorHTML = function () {
      var content = oldGetEditorHTML.call(this);
      for (var plugin in enabledPlugins.drupal) {
        var pluginSettings = wysiwygInstance.pluginInfo.global.drupal[plugin];
        if (typeof Drupal.wysiwyg.plugins[plugin].detach == 'function') {
          content = Drupal.wysiwyg.plugins[plugin].detach(content, pluginSettings, params.field);
        }
      }
      return content;
    }
  }

  // Reload the editor contents to give Drupal plugins a chance to act.
  editor.on('editorContentLoaded', function (e) {
    e.target.setEditorHTML(oldGetEditorHTML.call(e.target));
  });

  editor.on('afterNodeChange', function (e) {
    for (var plugin in enabledPlugins.drupal) {
      if (typeof Drupal.wysiwyg.plugins[plugin].isNode == 'function') {
        if (Drupal.wysiwyg.plugins[plugin].isNode(e.target._getSelectedElement())) {
          this.toolbar.selectButton(plugin);
        }
      }
    }
  });

  editor.render();
  // This event never gets fired if loaded into a dialog, harmless otherwise.
  editor.fireEvent('contentReady');
};

/**
 * Detach a single or all editors.
 *
 * See Drupal.wysiwyg.editor.detach.none() for a full desciption of this hook.
 */
Drupal.wysiwyg.editor.detach.yui = function (context, params, trigger) {
  var method = (trigger && trigger == 'serialize') ? 'saveHTML' : 'destroy';
  if (typeof params != 'undefined') {
    var instance = YAHOO.widget.EditorInfo._instances[params.field];
    if (instance) {
      instance[method]();
      if (method == 'destroy') {
        delete YAHOO.widget.EditorInfo._instances[params.field];
      }
    }
  }
  else {
    for (var e in YAHOO.widget.EditorInfo._instances) {
      // Save contents of all editors back into textareas.
      var instance = YAHOO.widget.EditorInfo._instances[e];
      instance[method]();
      if (method == 'destroy') {
        delete YAHOO.widget.EditorInfo._instances[e];
      }
    }
  }
};

/**
 * Instance methods for YUI Editor.
 */
Drupal.wysiwyg.editor.instance.yui = {
  addPlugin: function (plugin, pluginSettings) {
    if (typeof Drupal.wysiwyg.plugins[plugin] != 'object') {
      return;
    }
    var editor = YAHOO.widget.EditorInfo.getEditorById(this.field);
    var button = editor.toolbar.getButtonByValue(plugin);
    $(button._button).parent().css('background', 'transparent url(' + pluginSettings.icon + ') no-repeat center');
    // 'this' will reference the toolbar while inside the event handler.
    var instanceId = this.field;
    editor.toolbar.on(plugin + 'Click', function (e) {
      var selectedElement = editor._getSelectedElement();
      // @todo Using .html() will cause XTHML vs HTML conflicts.
      var data = {
        format: 'html',
        node: selectedElement,
        content: $(selectedElement).html()
      };
      Drupal.wysiwyg.plugins[plugin].invoke(data, pluginSettings, instanceId);
    });
  },

  prepareContent: function (content) {
    var editor = YAHOO.widget.EditorInfo.getEditorById(this.field);
    content = editor.cleanHTML(content);
    return content;
  },

  insert: function (content) {
    YAHOO.widget.EditorInfo.getEditorById(this.field).execCommand('inserthtml', content);
  },

  setContent: function (content) {
    YAHOO.widget.EditorInfo.getEditorById(this.field).setEditorHTML(content);
  },

  getContent: function () {
    var instance = YAHOO.widget.EditorInfo.getEditorById(this.field);
    return instance.cleanHTML(instance.getEditorHTML(content));
  }
};

})(jQuery);
