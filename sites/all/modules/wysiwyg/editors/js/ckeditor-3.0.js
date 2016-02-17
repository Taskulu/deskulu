(function($) {

CKEDITOR.disableAutoInline = true;

// Exclude every id starting with 'cke_' in ajax_html_ids during AJAX requests.
Drupal.wysiwyg.excludeIdSelectors.wysiwyg_ckeditor = ['[id^="cke_"]'];

// Keeps track of private instance data.
var instanceMap;

/**
 * Initialize the editor library.
 *
 * This method is called once the first time a library is needed. If new
 * WYSIWYG fieldsare added later, update() will be called instead.
 *
 * @param settings
 *   An object containing editor settings for each input format.
 * @param pluginInfo
 *   An object containing global plugin configuration.
 */
Drupal.wysiwyg.editor.init.ckeditor = function(settings, pluginInfo) {
  instanceMap = {};
  // Nothing to do here other than register new plugins etc.
  Drupal.wysiwyg.editor.update.ckeditor(settings, pluginInfo);
};

/**
 * Update the editor library when new settings are available.
 *
 * This method is called instead of init() when at least one new WYSIWYG field
 * has been added to the document and the library has already been initialized.
 *
 * $param settings
 *   An object containing editor settings for each input format.
 * $param pluginInfo
 *   An object containing global plugin configuration.
 */
Drupal.wysiwyg.editor.update.ckeditor = function(settings, pluginInfo) {
  // Register native external plugins.
  // Array syntax required; 'native' is a predefined token in JavaScript.
  for (var pluginId in pluginInfo['native']) {
    if (pluginInfo['native'].hasOwnProperty(pluginId) && (!CKEDITOR.plugins.externals || !CKEDITOR.plugins.externals[pluginId])) {
      var plugin = pluginInfo['native'][pluginId];
      CKEDITOR.plugins.addExternal(pluginId, plugin.path, plugin.fileName);
    }
  }
  // Build and register Drupal plugin wrappers.
  for (var pluginId in pluginInfo.drupal) {
    if (pluginInfo.drupal.hasOwnProperty(pluginId) && (!CKEDITOR.plugins.registered || !CKEDITOR.plugins.registered[pluginId])) {
      Drupal.wysiwyg.editor.instance.ckeditor.addPlugin(pluginId, pluginInfo.drupal[pluginId]);
    }
  }
  // Register Font styles (versions 3.2.1 and above).
  for (var format in settings) {
    if (settings[format].stylesSet && (!CKEDITOR.stylesSet || !CKEDITOR.stylesSet.registered[format])) {
      CKEDITOR.stylesSet.add(format, settings[format].stylesSet);
    }
  }
};

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.ckeditor = function(context, params, settings) {
  // Apply editor instance settings.
  CKEDITOR.config.customConfig = '';

  var $drupalToolbars = $('#toolbar, #admin-menu', Drupal.overlayChild ? window.parent.document : document);
  if (!settings.height) {
    settings.height = $('#' + params.field).height();
  }
  settings.on = {
    instanceReady: function(ev) {
      var editor = ev.editor;
      // Get a list of block, list and table tags from CKEditor's XHTML DTD.
      // @see http://docs.cksource.com/CKEditor_3.x/Developers_Guide/Output_Formatting.
      var dtd = CKEDITOR.dtd;
      var tags = CKEDITOR.tools.extend({}, dtd.$block, dtd.$listItem, dtd.$tableContent);
      // Set source formatting rules for each listed tag except <pre>.
      // Linebreaks can be inserted before or after opening and closing tags.
      if (settings.simple_source_formatting) {
        // Mimic FCKeditor output, by breaking lines between tags.
        for (var tag in tags) {
          if (tag == 'pre') {
            continue;
          }
          this.dataProcessor.writer.setRules(tag, {
            indent: true,
            breakBeforeOpen: true,
            breakAfterOpen: false,
            breakBeforeClose: false,
            breakAfterClose: true
          });
        }
      }
      else {
        // CKEditor adds default formatting to <br>, so we want to remove that
        // here too.
        tags.br = 1;
        // No indents or linebreaks;
        for (var tag in tags) {
          if (tag == 'pre') {
            continue;
          }
          this.dataProcessor.writer.setRules(tag, {
            indent: false,
            breakBeforeOpen: false,
            breakAfterOpen: false,
            breakBeforeClose: false,
            breakAfterClose: false
          });
        }
      }
    },

    pluginsLoaded: function(ev) {
      var wysiwygInstance = instanceMap[this.name];
      var enabledPlugins = wysiwygInstance.pluginInfo.instances.drupal;
      // Override the conversion methods to let Drupal plugins modify the data.
      var editor = ev.editor;
      if (editor.dataProcessor && enabledPlugins) {
        editor.dataProcessor.toHtml = CKEDITOR.tools.override(editor.dataProcessor.toHtml, function(originalToHtml) {
          // Convert raw data for display in WYSIWYG mode.
          return function(data, fixForBody) {
            for (var plugin in enabledPlugins) {
              if (typeof Drupal.wysiwyg.plugins[plugin].attach == 'function') {
                data = Drupal.wysiwyg.plugins[plugin].attach(data, wysiwygInstance.pluginInfo.global.drupal[plugin], editor.name);
                data = wysiwygInstance.prepareContent(data);
              }
            }
            return originalToHtml.call(this, data, fixForBody);
          };
        });
        editor.dataProcessor.toDataFormat = CKEDITOR.tools.override(editor.dataProcessor.toDataFormat, function(originalToDataFormat) {
          // Convert WYSIWYG mode content to raw data.
          return function(data, fixForBody) {
            data = originalToDataFormat.call(this, data, fixForBody);
            for (var plugin in enabledPlugins) {
              if (typeof Drupal.wysiwyg.plugins[plugin].detach == 'function') {
                data = Drupal.wysiwyg.plugins[plugin].detach(data, wysiwygInstance.pluginInfo.global.drupal[plugin], editor.name);
              }
            }
            return data;
          };
        });
      }
    },

    selectionChange: function (event) {
      var wysiwygInstance = instanceMap[this.name];
      var enabledPlugins = wysiwygInstance.pluginInfo.instances.drupal;
      for (var name in enabledPlugins) {
        var plugin = Drupal.wysiwyg.plugins[name];
        if ($.isFunction(plugin.isNode)) {
          var node = event.data.selection.getSelectedElement();
          var state = plugin.isNode(node ? node.$ : null) ? CKEDITOR.TRISTATE_ON : CKEDITOR.TRISTATE_OFF;
          event.editor.getCommand(name).setState(state);
        }
      }
    },

    focus: function(ev) {
      Drupal.wysiwyg.activeId = ev.editor.name;
    },

    afterCommandExec: function(ev) {
      // Fix Drupal toolbar obscuring editor toolbar in fullscreen mode.
      if (ev.data.name != 'maximize') {
        return;
      }
      if (ev.data.command.state == CKEDITOR.TRISTATE_ON) {
        $drupalToolbars.hide();
      }
      else {
        $drupalToolbars.show();
      }
    },

    destroy: function (event) {
      // Free our reference to the private instance to not risk memory leaks.
      delete instanceMap[this.name];
    }
  };
  instanceMap[params.field] = this;
  // Attach editor.
  var editorInstance = CKEDITOR.replace(params.field, settings);
};

/**
 * Detach a single or all editors.
 *
 * @todo 3.x: editor.prototype.getInstances() should always return an array
 *   containing all instances or the passed in params.field instance, but
 *   always return an array to simplify all detach functions.
 */
Drupal.wysiwyg.editor.detach.ckeditor = function (context, params, trigger) {
  var method = (trigger == 'serialize') ? 'updateElement' : 'destroy';
  if (typeof params != 'undefined') {
    var instance = CKEDITOR.instances[params.field];
    if (instance) {
      instance[method]();
    }
  }
  else {
    for (var instanceName in CKEDITOR.instances) {
      if (CKEDITOR.instances.hasOwnProperty(instanceName)) {
        CKEDITOR.instances[instanceName][method]();
      }
    }
  }
};

Drupal.wysiwyg.editor.instance.ckeditor = {
  addPlugin: function (pluginName, pluginSettings) {
    CKEDITOR.plugins.add(pluginName, {
      // Wrap Drupal plugin in a proxy pluygin.
      init: function(editor) {
        if (pluginSettings.css) {
          editor.on('mode', function(ev) {
            if (ev.editor.mode == 'wysiwyg') {
              // Inject CSS files directly into the editing area head tag.
              var iframe = $('#cke_contents_' + ev.editor.name + ' iframe, #' + ev.editor.id + '_contents iframe');
              $('head', iframe.eq(0).contents()).append('<link rel="stylesheet" href="' + pluginSettings.css + '" type="text/css" >');
            }
          });
        }
        if (typeof Drupal.wysiwyg.plugins[pluginName].invoke == 'function') {
          var pluginCommand = {
            exec: function (editor) {
              var data = { format: 'html', node: null, content: '' };
              var selection = editor.getSelection();
              if (selection) {
                data.node = selection.getSelectedElement();
                if (data.node) {
                  data.node = data.node.$;
                }
                if (selection.getType() == CKEDITOR.SELECTION_TEXT) {
                  data.content = selection.getSelectedText();
                }
                else if (data.node) {
                  // content is supposed to contain the "outerHTML".
                  data.content = data.node.parentNode.innerHTML;
                }
              }
              Drupal.wysiwyg.plugins[pluginName].invoke(data, pluginSettings, editor.name);
            }
          };
          editor.addCommand(pluginName, pluginCommand);
        }
        editor.ui.addButton(pluginName, {
          label: pluginSettings.title,
          command: pluginName,
          icon: pluginSettings.icon
        });

        // @todo Add button state handling.
      }
    });
  },
  prepareContent: function(content) {
    // @todo Don't know if we need this yet.
    return content;
  },

  insert: function(content) {
    content = this.prepareContent(content);
    if (CKEDITOR.env.webkit || CKEDITOR.env.chrome || CKEDITOR.env.opera || CKEDITOR.env.safari) {
      // Works around a WebKit bug which removes wrapper elements.
      // @see https://drupal.org/node/1927968
      var tmp = new CKEDITOR.dom.element('div'), children, skip = 0, item;
      tmp.setHtml(content);
      children = tmp.getChildren();
      skip = 0;
      while (children.count() > skip) {
        item = children.getItem(skip);
        switch(item.type) {
          case 1:
            CKEDITOR.instances[this.field].insertElement(item);
            break;
          case 3:
            CKEDITOR.instances[this.field].insertText(item.getText());
            skip++;
            break;
          case 8:
            CKEDITOR.instances[this.field].insertHtml(item.getOuterHtml());
            skip++;
            break;
        }
      }
    }
    else {
      CKEDITOR.instances[this.field].insertHtml(content);
    }
  },

  setContent: function (content) {
    CKEDITOR.instances[this.field].setData(content);
  },

  getContent: function () {
    return CKEDITOR.instances[this.field].getData();
  },

  isFullscreen: function () {
    var cmd = CKEDITOR.instances[this.field].commands.maximize;
    return !!(cmd && cmd.state == CKEDITOR.TRISTATE_ON);
  }
};

})(jQuery);
