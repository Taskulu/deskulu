
Drupal = window.parent.Drupal;

/**
 * Fetch and provide original editor settings as local variable.
 *
 * FCKeditor does not support to pass complex variable types to the editor.
 * Instance settings passed to FCKinstance.Config are temporarily stored in
 * FCKConfig.PageConfig.
 */
// Fetch the private instance and make sure nothing can tamper with it.
var $field = window.parent.jQuery(FCK.LinkedField);
var wysiwygInstance = $field.data('wysiwygInstance');
$field.removeData('wysiwygInstance');
var wysiwygSettings = wysiwygInstance.editorSettings;
var pluginInfo = wysiwygInstance.pluginInfo;

/**
 * Apply format-specific settings.
 */
for (var setting in wysiwygSettings) {
  if (setting == 'buttons') {
    // Apply custom Wysiwyg toolbar for this format.
    // FCKConfig.ToolbarSets['Wysiwyg'] = wysiwygSettings.buttons;

    // Temporarily stack buttons into multiple button groups and remove
    // separators until #277954 is solved.
    FCKConfig.ToolbarSets['Wysiwyg'] = [];
    for (var i = 0; i < wysiwygSettings.buttons[0].length; i++) {
      FCKConfig.ToolbarSets['Wysiwyg'].push([wysiwygSettings.buttons[0][i]]);
    }
    FCKTools.AppendStyleSheet(document, '#xToolbar .TB_Start { display:none; }');
    // Set valid height of select element in silver and office2003 skins.
    if (FCKConfig.SkinPath.match(/\/office2003\/$/)) {
      FCKTools.AppendStyleSheet(document, '#xToolbar .SC_FieldCaption { height: 24px; } #xToolbar .TB_End { display: none; }');
    }
    else if (FCKConfig.SkinPath.match(/\/silver\/$/)) {
      FCKTools.AppendStyleSheet(document, '#xToolbar .SC_FieldCaption { height: 27px; }');
    }
  }
  else {
    FCKConfig[setting] = wysiwygSettings[setting];
  }
}

// Fix Drupal toolbar obscuring editor toolbar in fullscreen mode.
var oldFitWindowExecute = FCKFitWindow.prototype.Execute;
var $drupalToolbars = window.parent.jQuery('#toolbar, #admin-menu', Drupal.overlayChild ? window.parent.window.parent.document : window.parent.document);
FCKFitWindow.prototype.Execute = function() {
  oldFitWindowExecute.apply(this, arguments);
  if (this.IsMaximized) {
    $drupalToolbars.hide();
  }
  else {
    $drupalToolbars.show();
  }
}

/**
 * Initialize this editor instance.
 */
wysiwygInstance.init(window);

/**
 * Register native plugins for this input format.
 *
 * Parameters to Plugins.Add are:
 * - Plugin name.
 * - Languages the plugin is available in.
 * - Location of the plugin folder; <plugin_name>/fckplugin.js is appended.
 */
for (var pluginId in pluginInfo.instances['native']) {
  // Languages and path may be undefined for internal plugins.
  FCKConfig.Plugins.Add(pluginId, pluginInfo.global['native'][pluginId].languages, pluginInfo.global['native'][pluginId].path);
}

/**
 * Register Drupal plugins for this input format.
 *
 * Parameters to addPlugin() are:
 * - Plugin name.
 * - Format specific plugin settings.
 * - General plugin settings.
 * - A reference to this window so the plugin setup can access FCKConfig.
 */
for (var pluginId in pluginInfo.instances.drupal) {
  var plugin = pluginInfo.instances.drupal[pluginId];
  Drupal.wysiwyg.editor.instance.fckeditor.addPlugin(pluginId, pluginInfo.global.drupal[pluginId], window);
}

