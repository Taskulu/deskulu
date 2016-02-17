(function($) {

// Check if this file has already been loaded.
if (typeof Drupal.wysiwygAttach !== 'undefined') {
  return;
}

// Keeps track of editor status during AJAX operations, active format and more.
// Always use getFieldInfo() to get a valid reference to the correct data.
var _fieldInfoStorage = {};
// Keeps track of information relevant to each format, such as editor settings.
// Always use getFormatInfo() to get a reference to a format's data.
var _formatInfoStorage = {};

// Keeps track of global and per format plugin configurations.
// Always use getPluginInfo() tog get a valid reference to the correct data.
var _pluginInfoStorage = {'global': {'drupal': {}, 'native': {}}};

// Keeps track of private instance information.
var _internalInstances = {};

// Keeps track of initialized editor libraries.
var _initializedLibraries = {};

// Keeps a map between format selectboxes and fields.
var _selectToField = {};

/**
 * Returns field specific editor data.
 *
 * @throws Error
 *   Exception thrown if data for an unknown field is requested.
 *   Summary fields are expected to use the same data as the main field.
 *
 * If a field id contains the delimiter '--', anything after that is dropped and
 * the remainder is assumed to be the id of an original field replaced by an
 * AJAX operation, due to how Drupal generates unique ids.
 * @see drupal_html_id()
 *
 * Do not modify the returned object unless you really know what you're doing.
 * No external code should need access to this, and it may likely change in the
 * future.
 *
 * @param fieldId
 *   The id of the field to get data for.
 *
 * @returns
 *   A reference to an object with the following properties:
 *   - activeFormat: A string with the active format id.
 *   - enabled: A boolean, true if the editor is attached.
 *   - formats: An object with one sub-object for each available format, holding
 *     format specific state data for this field.
 *   - summary: An optional string with the id of a corresponding summary field.
 *   - trigger: A string with the id of the format selector for the field.
 *   - getFormatInfo: Shortcut method to getFormatInfo(fieldInfo.activeFormat).
 */
function getFieldInfo(fieldId) {
  if (_fieldInfoStorage[fieldId]) {
    return _fieldInfoStorage[fieldId];
  }
  var baseFieldId = (fieldId.indexOf('--') === -1 ? fieldId : fieldId.substr(0, fieldId.indexOf('--')));
  if (_fieldInfoStorage[baseFieldId]) {
    return _fieldInfoStorage[baseFieldId];
  }
  throw new Error('Wysiwyg module has no information about field "' + fieldId + '"');
}

/**
 * Returns format specific editor data.
 *
 * Do not modify the returned object unless you really know what you're doing.
 * No external code should need access to this, and it may likely change in the
 * future.
 *
 * @param formatId
 *   The id of a format to get data for.
 *
 * @returns
 *   A reference to an object with the following properties:
 *   - editor: A string with the id of the editor attached to the format.
 *     'none' if no editor profile is associated with the format.
 *   - enabled: True if the editor is active.
 *   - toggle: True if the editor can be toggled on/off by the user.
 *   - editorSettings: A structure holding editor settings for this format.
 *   - getPluginInfo: Shortcut method to get plugin config for the this format.
 */
function getFormatInfo(formatId) {
  if (_formatInfoStorage[formatId]) {
    return _formatInfoStorage[formatId];
  }
  return {
    editor: 'none',
    getPluginInfo: function () {
      return getPluginInfo(formatId);
    }
  };
}

/**
 * Returns plugin configuration for a specific format, or the global values.
 *
 * @param formatId
 *   The id of a format to get data for, or 'global' to get data common to all
 *   formats and editors. Use 'global:editorname' to limit it to one editor.
 *
 * @return
 *   The returned object will have the sub-objects 'drupal' and 'native', each
 *   with properties matching names of plugins.
 *   Global data for Drupal (cross-editor) plugins will have the following keys:
 *   - title: A human readable name for the button.
 *   - internalName: The unique name of a native plugin wrapper, used in editor
 *     profiles and when registering the plugin with the editor API to avoid
 *     possible id conflicts with native plugins.
 *   - css: A stylesheet needed by the plugin.
 *   - icon path: The path where button icons are stored.
 *   - path: The path to the plugin's main folder.
 *   - buttons: An object with button data, keyed by name with the properties:
 *     - description: A human readable string describing the button's function.
 *     - title: A human readable string with the name of the button.
 *     - icon: An object with one or more of the following properties:
 *       - src: An absolute (begins with '/') or relative path to the icon.
 *       - path: An absolute path to a folder containing the button.
 *
 *   When formatId matched a format with an assigned editor, values for plugins
 *   match the return value of the editor integration's [proxy] plugin settings
 *   callbacks.
 *
 *   @see Drupal.wysiwyg.utilities.getPluginInfo()
 *   @see Drupal.wyswiyg.utilities.extractButtonSettings()
 */
function getPluginInfo(formatId) {
  var match, editor;
  if ((match = formatId.match(/^global:(\w+)$/))) {
    formatId = 'global';
    editor = match[1];
  }
  if (!_pluginInfoStorage[formatId]) {
    return {};
  }
  if (formatId === 'global' && typeof editor !== 'undefined') {
    return { 'drupal': _pluginInfoStorage.global.drupal, 'native': (_pluginInfoStorage.global['native'][editor]) };
  }
  return _pluginInfoStorage[formatId];
}

/**
 * Attach editors to input formats and target elements (f.e. textareas).
 *
 * This behavior searches for input format selectors and formatting guidelines
 * that have been preprocessed by Wysiwyg API. All CSS classes of those elements
 * with the prefix 'wysiwyg-' are parsed into input format parameters, defining
 * the input format, configured editor, target element id, and variable other
 * properties, which are passed to the attach/detach hooks of the corresponding
 * editor.
 *
 * Furthermore, an "enable/disable rich-text" toggle link is added after the
 * target element to allow users to alter its contents in plain text.
 *
 * This is executed once, while editor attach/detach hooks can be invoked
 * multiple times.
 *
 * @param context
 *   A DOM element, supplied by Drupal.attachBehaviors().
 */
Drupal.behaviors.attachWysiwyg = {
  attach: function (context, settings) {
    // This breaks in Konqueror. Prevent it from running.
    if (/KDE/.test(navigator.vendor)) {
      return;
    }
    var wysiwygs = $('.wysiwyg:input', context);
    if (!wysiwygs.length) {
      // No new fields, nothing to update.
      return;
    }
    updateInternalState(settings.wysiwyg, context);
    wysiwygs.once('wysiwyg', function () {
      // Skip processing if the element is unknown or does not exist in this
      // document. Can happen after a form was removed but Drupal.ajax keeps a
      // lingering reference to the form and calls Drupal.attachBehaviors().
      var $this = $('#' + this.id, document);
      if (!$this.length) {
        return;
      }
      // Directly attach this editor, if the input format is enabled or there is
      // only one input format at all.
      Drupal.wysiwygAttach(context, this.id);
    })
    .closest('form').submit(function (event) {
      // Detach any editor when the containing form is submitted.
      // Do not detach if the event was cancelled.
      if (event.isDefaultPrevented()) {
        return;
      }
      var form = this;
      $('.wysiwyg:input', this).each(function () {
        Drupal.wysiwygDetach(form, this.id, 'serialize');
      });
    });
  },

  detach: function (context, settings, trigger) {
    var wysiwygs;
    // The 'serialize' trigger indicates that we should simply update the
    // underlying element with the new text, without destroying the editor.
    if (trigger == 'serialize') {
      // Removing the wysiwyg-processed class guarantees that the editor will
      // be reattached. Only do this if we're planning to destroy the editor.
      wysiwygs = $('.wysiwyg-processed:input', context);
    }
    else {
      wysiwygs = $('.wysiwyg:input', context).removeOnce('wysiwyg');
    }
    wysiwygs.each(function () {
      Drupal.wysiwygDetach(context, this.id, trigger);
    });
  }
};

/**
 * Attach an editor to a target element.
 *
 * Detaches any existing instance for the field before attaching a new instance
 * based on the current state of the field. Editor settings and state
 * information is fetched based on the element id and get cloned first, so they
 * cannot be overridden. After attaching the editor, the toggle link is shown
 * again, except in case we are attaching no editor.
 *
 * Also attaches editors to the summary field, if available.
 *
 * @param context
 *   A DOM element, supplied by Drupal.attachBehaviors().
 * @param fieldId
 *   The id of an element to attach an editor to.
 */
Drupal.wysiwygAttach = function(context, fieldId) {
  var fieldInfo = getFieldInfo(fieldId),
      formatInfo = fieldInfo.getFormatInfo(),
      editorSettings = formatInfo.editorSettings,
      editor = formatInfo.editor,
      previousStatus = false,
      previousEditor = 'none',
      doSummary = (fieldInfo.summary && (!fieldInfo.formats[fieldInfo.activeFormat] || !fieldInfo.formats[fieldInfo.activeFormat].skip_summary));
  if (_internalInstances[fieldId]) {
    previousStatus = _internalInstances[fieldId]['status'];
    previousEditor = _internalInstances[fieldId].editor;
  }
  // Detach any previous editor instance if enabled, else remove the grippie.
  detachFromField(context, {'editor': previousEditor, 'status': previousStatus, 'field': fieldId, 'resizable': fieldInfo.resizable}, 'unload');
  if (doSummary) {
    // Summary instances may have a different status if no real editor was
    // attached yet because the field was hidden.
    if (_internalInstances[fieldInfo.summary]) {
      previousStatus = _internalInstances[fieldInfo.summary]['status'];
    }
    detachFromField(context, {'editor': previousEditor, 'status': previousStatus, 'field': fieldInfo.summary, 'resizable': fieldInfo.resizable}, 'unload');
  }
  // Store this field id, so (external) plugins can use it.
  // @todo Wrong point in time. Probably can only supported by editors which
  //   support an onFocus() or similar event.
  Drupal.wysiwyg.activeId = fieldId;
  // Attach or update toggle link, if enabled.
  Drupal.wysiwygAttachToggleLink(context, fieldId);
  // Attach to main field.
  attachToField(context, {'status': fieldInfo.enabled, 'editor': editor, 'field': fieldId, 'format': fieldInfo.activeFormat, 'resizable': fieldInfo.resizable}, editorSettings);
  // Attach to summary field.
  if (doSummary) {
    // If the summary wrapper is visible, attach immediately.
    if ($('#' + fieldInfo.summary).parents('.text-summary-wrapper').is(':visible')) {
      attachToField(context, {'status': fieldInfo.enabled, 'editor': editor, 'field': fieldInfo.summary, 'format': fieldInfo.activeFormat, 'resizable': fieldInfo.resizable}, editorSettings);
    }
    else {
      // Attach an instance of the 'none' editor to have consistency while the
      // summary is hidden, then switch to a real editor instance when shown.
      attachToField(context, {'status': false, 'editor': editor, 'field': fieldInfo.summary, 'format': fieldInfo.activeFormat, 'resizable': fieldInfo.resizable}, editorSettings);
      // Unbind any existing click handler to avoid double toggling.
      $('#' + fieldId).parents('.text-format-wrapper').find('.link-edit-summary').unbind('click.wysiwyg').bind('click.wysiwyg', function () {
        detachFromField(context, {'status': false, 'editor': editor, 'field': fieldInfo.summary, 'format': fieldInfo.activeFormat, 'resizable': fieldInfo.resizable}, editorSettings);
        attachToField(context, {'status': fieldInfo.enabled, 'editor': editor, 'field': fieldInfo.summary, 'format': fieldInfo.activeFormat, 'resizable': fieldInfo.resizable}, editorSettings);
        $(this).unbind('click.wysiwyg');
      });
    }
  }
};

/**
 * The public API exposed for an editor-enabled field.
 *
 * Properties should be treated as read-only state and changing them will not
 * have any effect on how the instance behaves.
 *
 * Note: The attach() and detach() methods are not part of the public API and
 * should not be called directly to avoid synchronization issues.
 * Use Drupal.wysiwygAttach() and Drupal.wysiwygDetach() to activate or
 * deactivate editor instances. Externally switching the active editor is not
 * supported other than changing the format using the select element.
 */
function WysiwygInstance(internalInstance) {
  // The id of the field the instance manipulates.
  this.field = internalInstance.field;
  // The internal name of the attached editor.
  this.editor = internalInstance.editor;
  // If the editor is currently enabled or not.
  this['status'] = internalInstance['status'];
  // The id of the text format the editor is attached to.
  this.format = internalInstance.format;
  // If the field is resizable without an editor attached.
  this.resizable = internalInstance.resizable;

  // Methods below here redirect to the 'none' editor which handles plain text
  // fields when the editor is disabled.

   /**
    * Insert content at the cursor position.
    *
    * @param content
    *   An HTML markup string.
    */
  this.insert = function (content) {
    return internalInstance['status'] ? internalInstance.insert(content) : Drupal.wysiwyg.editor.instance.none.insert.call(internalInstance, content);
  }

  /**
   * Get all content from the editor.
   *
   * @return
   *   An HTML markup string.
   */
  this.getContent = function () {
    return internalInstance['status'] ? internalInstance.getContent() : Drupal.wysiwyg.editor.instance.none.getContent.call(internalInstance);
  }

  /**
   * Replace all content in the editor.
   *
   * @param content
   *   An HTML markup string.
   */
  this.setContent = function (content) {
    return internalInstance['status'] ? internalInstance.setContent(content) : Drupal.wysiwyg.editor.instance.none.setContent.call(internalInstance, content);
  }

  /**
   * Check if the editor is in fullscreen mode.
   *
   * @return bool
   *  True if the editor is considered to be in fullscreen mode.
   */
  this.isFullscreen = function (content) {
    return internalInstance['status'] && $.isFunction(internalInstance.isFullscreen) ? internalInstance.isFullscreen() : false;
  }

  // @todo The methods below only work for TinyMCE, deprecate?

  /**
   * Open a native editor dialog.
   *
   * Use of this method i not recomended due to limited editor support.
   *
   * @param dialog
   *   An object with dialog settings. Keys used:
   *   - url: The url of the dialog template.
   *   - width: Width in pixels.
   *   - height: Height in pixels.
   */
  this.openDialog = function (dialog, params) {
    if ($.isFunction(internalInstance.openDialog)) {
      return internalInstance.openDialog(dialog, params)
    }
  }

  /**
   * Close an opened dialog.
   *
   * @param dialog
   *   Same options as for opening a dialog.
   */
  this.closeDialog = function (dialog) {
    if ($.isFunction(internalInstance.closeDialog)) {
      return internalInstance.closeDialog(dialog)
    }
  }
}

/**
 * The private base for editor instances.
 *
 * An instance of this object is used as the context for all calls into the
 * editor instances (including attach() and detach() when only one instance is
 * asked to detach).
 *
 * Anything added to Drupal.wysiwyg.editor.instance[editorName] is cloned into
 * an instance of this function.
 *
 * Editor state parameters are cloned into the instance after that.
 */
function WysiwygInternalInstance(params) {
  $.extend(true, this, Drupal.wysiwyg.editor.instance[params.editor]);
  $.extend(true, this, params);
  this.pluginInfo = {
    'global': getPluginInfo('global:' + params.editor),
    'instances': getPluginInfo(params.format)
  };
  // Keep track of the public face to keep it synced.
  this.publicInstance = new WysiwygInstance(this);
}

/**
 * Updates internal settings and state caches with new information.
 *
 * Attaches selection change handler to format selector to track state changes.
 *
 * @param settings
 *   A structure like Drupal.settigns.wysiwyg.
 * @param context
 *   The context given from Drupal.attachBehaviors().
 */
function updateInternalState(settings, context) {
  var pluginData = settings.plugins;
  for (var plugin in pluginData.drupal) {
    if (!(plugin in _pluginInfoStorage.global.drupal)) {
      _pluginInfoStorage.global.drupal[plugin] = pluginData.drupal[plugin];
    }
  }
  // To make sure we don't rely on Drupal.settings, uncomment these for testing.
  //pluginData.drupal = {};
  for (var editorId in pluginData['native']) {
    for (var plugin in pluginData['native'][editorId]) {
      _pluginInfoStorage.global['native'][editorId] = (_pluginInfoStorage.global['native'][editorId] || {});
      if (!(plugin in _pluginInfoStorage.global['native'][editorId])) {
        _pluginInfoStorage.global['native'][editorId][plugin] = pluginData['native'][editorId][plugin];
      }
    }
  }
  //pluginData['native'] = {};
  for (var fmatId in pluginData) {
    if (fmatId.substr(0, 6) !== 'format') {
      continue;
    }
    _pluginInfoStorage[fmatId] = (_pluginInfoStorage[fmatId] || {'drupal': {}, 'native': {}});
    for (var plugin in pluginData[fmatId].drupal) {
      if (!(plugin in _pluginInfoStorage[fmatId].drupal)) {
        _pluginInfoStorage[fmatId].drupal[plugin] = pluginData[fmatId].drupal[plugin];
      }
    }
    for (var plugin in pluginData[fmatId]['native']) {
      if (!(plugin in _pluginInfoStorage[fmatId]['native'])) {
        _pluginInfoStorage[fmatId]['native'][plugin] = pluginData[fmatId]['native'][plugin];
      }
    }
    delete pluginData[fmatId];
  }
  // Build the cache of format/profile settings.
  for (var editor in settings.configs) {
    if (!settings.configs.hasOwnProperty(editor)) {
      continue;
    }
    for (var format in settings.configs[editor]) {
      if (_formatInfoStorage[format] || !settings.configs[editor].hasOwnProperty(format)) {
        continue;
      }
      _formatInfoStorage[format] = {
        editor: editor,
        toggle: true, // Overridden by triggers.
        editorSettings: processObjectTypes(settings.configs[editor][format])
      };
    }
    // Initialize editor libraries if not already done.
    if (!_initializedLibraries[editor] && typeof Drupal.wysiwyg.editor.init[editor] === 'function') {
      // Clone, so original settings are not overwritten.
      Drupal.wysiwyg.editor.init[editor](jQuery.extend(true, {}, settings.configs[editor]), getPluginInfo('global:' + editor));
      _initializedLibraries[editor] = true;
    }
    // Update libraries, in case new plugins etc have not been initialized yet.
    else if (typeof Drupal.wysiwyg.editor.update[editor] === 'function') {
      Drupal.wysiwyg.editor.update[editor](jQuery.extend(true, {}, settings.configs[editor]), getPluginInfo('global:' + editor));
    }
  }
  //settings.configs = {};
  for (var triggerId in settings.triggers) {
    var trigger = settings.triggers[triggerId];
    var fieldId = trigger.field;
    var baseFieldId = (fieldId.indexOf('--') === -1 ? fieldId : fieldId.substr(0, fieldId.indexOf('--')));
    var fieldInfo = null;
    if (!(fieldInfo = _fieldInfoStorage[baseFieldId])) {
      fieldInfo = _fieldInfoStorage[baseFieldId] = {
        formats: {},
        select: trigger.select,
        resizable: trigger.resizable,
        summary: trigger.summary,
        getFormatInfo: function () {
          if (this.select) {
            this.activeFormat = 'format' + $('#' + this.select + ':input').val();
          }
          return getFormatInfo(this.activeFormat);
        }
        // 'activeFormat' and 'enabled' added below.
      }
    };
    for (var format in trigger) {
      if (format.indexOf('format') != 0 || fieldInfo.formats[format]) {
        continue;
      }
      fieldInfo.formats[format] = {
        'enabled': trigger[format].status
      }
      if (!_formatInfoStorage[format]) {
        _formatInfoStorage[format] = {
          editor: trigger[format].editor,
          editorSettings: {},
          getPluginInfo: function () {
            return getPluginInfo(formatId);
          }
        };
      }
      // Always update these since they are stored as state.
      _formatInfoStorage[format].toggle = trigger[format].toggle;
      if (trigger[format].skip_summary) {
        fieldInfo.formats[format].skip_summary = true;
      }
    }
    var $selectbox = null;
    // Always update these since Drupal generates new ids on AJAX calls.
    fieldInfo.summary = trigger.summary;
    if (trigger.select) {
      _selectToField[trigger.select.replace(/--\d+$/,'')] = trigger.field;
      fieldInfo.select = trigger.select;
      // Specifically target input elements in case selectbox wrappers have
      // hidden the real element and cloned its attributes.
      $selectbox = $('#' + trigger.select + ':input', context).filter('select');
      // Attach onChange handlers to input format selector elements.
      $selectbox.unbind('change.wysiwyg').bind('change.wysiwyg', formatChanged);
    }
    // Always update the active format to ensure the righ profile is used if a
    // field was removed and gets re-added and the instance was left behind.
    fieldInfo.activeFormat = 'format' + ($selectbox ? $selectbox.val() : trigger.activeFormat);
    fieldInfo.enabled = fieldInfo.formats[fieldInfo.activeFormat] && fieldInfo.formats[fieldInfo.activeFormat].enabled;
  }
  //settings.triggers = {};
}

/**
 * Helper to prepare and attach an editor for a single field.
 *
 * Creates the 'instance' object under Drupal.wysiwyg.instances[fieldId].
 *
 * @param context
 *   A DOM element, supplied by Drupal.attachBehaviors().
 * @param params
 *   An object containing state information for the editor with the following
 *   properties:
 *   - 'status': A boolean stating whether the editor is currently active. If
 *     false, the default textarea behaviors will be attached instead (aka the
 *     'none' editor implementation).
 *   - 'editor': The internal name of the editor to attach when active.
 *   - 'field': The field id to use as an output target for the editor.
 *   - 'format': The name of the active text format (prefixed 'format').
 *   - 'resizable': A boolean indicating whether the original textarea was
 *      resizable.
 *   Note: This parameter is passed directly to the editor implementation and
 *   needs to have been reconstructed or cloned before attaching.
 * @param editorSettings
 *   An object containing all the settings the editor needs for this field.
 *   Settings are automatically cloned to prevent editors from modifying them.
 */
function attachToField(context, params, editorSettings) {
  // If the editor isn't active, attach default behaviors instead.
  var editor = (params.status ? params.editor : 'none');
  // Settings are deep merged (cloned) to prevent editor implementations from
  // permanently modifying them while attaching.
  var clonedSettings = jQuery.extend(true, {}, editorSettings);
  // (Re-)initialize field instance.
  var internalInstance = new WysiwygInternalInstance(params);
  _internalInstances[params.field] = internalInstance;
  Drupal.wysiwyg.instances[params.field] = internalInstance.publicInstance;
  if ($.isFunction(Drupal.wysiwyg.editor.attach[editor])) {
    Drupal.wysiwyg.editor.attach[editor].call(internalInstance, context, params, params.status ? clonedSettings : {});
  }
}

/**
 * Detach all editors from a target element.
 *
 * Ensures Drupal's original textfield resize functionality is restored if
 * enabled and the triggering reason is 'unload'.
 *
 * Also detaches editors from the summary field, if available.
 *
 * @param context
 *   A DOM element, supplied by Drupal.detachBehaviors().
 * @param fieldId
 *   The id of an element to attach an editor to.
 * @param trigger
 *   A string describing what is causing the editor to be detached.
 *   - 'serialize': The editor normally just syncs its contents to the original
 *     textarea for value serialization before an AJAX request.
 *   - 'unload': The editor is to be removed completely and the original
 *     textarea restored.
 *
 * @see Drupal.detachBehaviors()
 */
Drupal.wysiwygDetach = function (context, fieldId, trigger) {
  var fieldInfo = getFieldInfo(fieldId),
      editor = fieldInfo.getFormatInfo().editor,
      trigger = trigger || 'unload',
      previousStatus = (_internalInstances[fieldId] && _internalInstances[fieldId]['status']);
  // Detach from main field.
  detachFromField(context, {'editor': editor, 'status': previousStatus, 'field': fieldId, 'resizable': fieldInfo.resizable}, trigger);
  if (trigger == 'unload') {
    // Attach the resize behavior by forcing status to false. Other values are
    // intentionally kept the same to show which editor is normally attached.
    attachToField(context, {'editor': editor, 'status': false, 'format': fieldInfo.activeFormat, 'field': fieldId, 'resizable': fieldInfo.resizable});
    Drupal.wysiwygAttachToggleLink(context, fieldId);
  }
  // Detach from summary field.
  if (fieldInfo.summary && _internalInstances[fieldInfo.summary]) {
    // The "Edit summary" click handler could re-enable the editor by mistake.
    $('#' + fieldId).parents('.text-format-wrapper').find('.link-edit-summary').unbind('click.wysiwyg');
    // Summary instances may have a different status if no real editor was
    // attached yet because the field was hidden.
    if (_internalInstances[fieldInfo.summary]) {
      previousStatus = _internalInstances[fieldInfo.summary]['status'];
    }
    detachFromField(context, {'editor': editor, 'status': previousStatus, 'field': fieldInfo.summary, 'resizable': fieldInfo.resizable}, trigger);
    if (trigger == 'unload') {
      attachToField(context, {'editor': editor, 'status': false, 'format': fieldInfo.activeFormat, 'field': fieldInfo.summary, 'resizable': fieldInfo.resizable});
    }
  }
};

/**
 * Helper to detach and clean up after an editor for a single field.
 *
 * Removes the 'instance' object under Drupal.wysiwyg.instances[fieldId].
 *
 * @param context
 *   A DOM element, supplied by Drupal.detachBehaviors().
 * @param params
 *   An object containing state information for the editor with the following
 *   properties:
 *   - 'status': A boolean stating whether the editor is currently active. If
 *     false, the default textarea behaviors will be attached instead (aka the
 *     'none' editor implementation).
 *   - 'editor': The internal name of the editor to attach when active.
 *   - 'field': The field id to use as an output target for the editor.
 *   - 'format': The name of the active text format (prefixed 'format').
 *   - 'resizable': A boolean indicating whether the original textarea was
 *      resizable.
 *   Note: This parameter is passed directly to the editor implementation and
 *   needs to have been reconstructed or cloned before detaching.
 * @param trigger
 *   A string describing what is causing the editor to be detached.
 *   - 'serialize': The editor normally just syncs its contents to the original
 *     textarea for value serialization before an AJAX request.
 *   - 'unload': The editor is to be removed completely and the original
 *     textarea restored.
 *
 * @see Drupal.wysiwygDetach()
 */
function detachFromField(context, params, trigger) {
  var editor = (params.status ? params.editor : 'none');
  if (jQuery.isFunction(Drupal.wysiwyg.editor.detach[editor])) {
    Drupal.wysiwyg.editor.detach[editor].call(_internalInstances[params.field], context, params, trigger);
  }
  if (trigger == 'unload') {
    delete Drupal.wysiwyg.instances[params.field];
    delete _internalInstances[params.field];
  }
}

/**
 * Append or update an editor toggle link to a target element.
 *
 * @param context
 *   A DOM element, supplied by Drupal.attachBehaviors().
 * @param fieldId
 *   The id of an element to attach an editor to.
 */
Drupal.wysiwygAttachToggleLink = function(context, fieldId) {
  var fieldInfo = getFieldInfo(fieldId),
      editor = fieldInfo.getFormatInfo().editor;
  if (!fieldInfo.getFormatInfo().toggle) {
    // Otherwise, ensure that toggle link is hidden.
    $('#wysiwyg-toggle-' + fieldId).hide();
    return;
  }
  if (!$('#wysiwyg-toggle-' + fieldId, context).length) {
    var text = document.createTextNode(fieldInfo.enabled ? Drupal.settings.wysiwyg.disable : Drupal.settings.wysiwyg.enable),
      a = document.createElement('a'),
      div = document.createElement('div');
    $(a).attr({ id: 'wysiwyg-toggle-' + fieldId, href: 'javascript:void(0);' }).append(text);
    $(div).addClass('wysiwyg-toggle-wrapper').append(a);
    if ($('#' + fieldInfo.select).closest('.fieldset-wrapper').prepend(div).length == 0) {
      // Fall back to inserting the link right after the field.
      $('#' + fieldId).after(div);
    };
  }
  $('#wysiwyg-toggle-' + fieldId, context)
    .html(fieldInfo.enabled ? Drupal.settings.wysiwyg.disable : Drupal.settings.wysiwyg.enable).show()
    .unbind('click.wysiwyg')
    .bind('click.wysiwyg', { 'fieldId': fieldId, 'context': context }, Drupal.wysiwyg.toggleWysiwyg);

  // Hide toggle link in case no editor is attached.
  if (editor == 'none') {
    $('#wysiwyg-toggle-' + fieldId).hide();
  }
};

/**
 * Callback for the Enable/Disable rich editor link.
 */
Drupal.wysiwyg.toggleWysiwyg = function (event) {
  var context = event.data.context,
      fieldId = event.data.fieldId,
      fieldInfo = getFieldInfo(fieldId);
  // Toggling the enabled state indirectly toggles use of the 'none' editor.
  if (fieldInfo.enabled) {
    fieldInfo.enabled = false;
    Drupal.wysiwygDetach(context, fieldId, 'unload');
  }
  else {
    fieldInfo.enabled = true;
    Drupal.wysiwygAttach(context, fieldId);
  }
  fieldInfo.formats[fieldInfo.activeFormat].enabled = fieldInfo.enabled;
}


/**
 * Event handler for when the selected format is changed.
 */
function formatChanged(event) {
  var fieldId = _selectToField[this.id.replace(/--\d+$/,'')];
  var context = $(this).closest('form');
  // Field state is fetched by reference.
  var currentField = getFieldInfo(fieldId);
  // Save the state of the current format.
  if (currentField.formats[currentField.activeFormat]) {
    currentField.formats[currentField.activeFormat].enabled = currentField.enabled;
  }
  // Switch format/profile.
  currentField.activeFormat = 'format' + this.value;
  // Load the state from the new format.
  if (currentField.formats[currentField.activeFormat]) {
    currentField.enabled = currentField.formats[currentField.activeFormat].enabled;
  }
  else {
    currentField.enabled = false;
  }
  // Attaching again will use the changed field state.
  Drupal.wysiwygAttach(context, fieldId);
}

/**
 * Convert JSON type placeholders into the actual types.
 *
 * Recognizes function references (callbacks) and Regular Expressions.
 *
 * To create a callback, pass in an object with the following properties:
 * - 'drupalWysiwygType': Must be set to 'callback'.
 * - 'name': A string with the name of the callback, use
 *   'object.subobject.method' syntax for methods in nested objects.
 * - 'context': An optional string with the name of an object for overriding
 *   'this' inside the function. Use 'object.subobject' syntax for nested
 *   objects. Defaults to the window object.
 *
 * To create a RegExp, pass in an object with the following properties:
 * - 'drupalWysiwygType: Must be set to 'regexp'.
 * - 'regexp': The Regular Expression as a string, without / wrappers.
 * - 'modifiers': An optional string with modifiers to set on the RegExp object.
 *
 * @param json
 *  The json argument with all recognized type placeholders replaced by the real
 *  types.
 *
 * @return The JSON object with placeholder types replaced.
 */
function processObjectTypes(json) {
  var out = null;
  if (typeof json != 'object') {
    return json;
  }
  out = new json.constructor();
  if (json.drupalWysiwygType) {
    switch (json.drupalWysiwygType) {
      case 'callback':
        out = callbackWrapper(json.name, json.context);
        break;
      case 'regexp':
        out = new RegExp(json.regexp, json.modifiers ? json.modifiers : undefined);
        break;
      default:
        out.drupalWysiwygType = json.drupalWysiwygType;
    }
  }
  else {
    for (var i in json) {
      if (json.hasOwnProperty(i) && json[i] && typeof json[i] == 'object') {
        out[i] = processObjectTypes(json[i]);
      }
      else {
        out[i] = json[i];
      }
    }
  }
  return out;
}

/**
 * Convert function names into function references.
 *
 * @param name
 *  The name of a function to use as callback. Use the 'object.subobject.method'
 *  syntax for methods in nested objects.
 * @param context
 *  An optional string with the name of an object for overriding 'this' inside
 *  the function. Use 'object.subobject' syntax for nested objects. Defaults to
 *  the window object.
 *
 * @return
 *  A function which will call the named function or method in the proper
 *  context, passing through arguments and return values.
 */
function callbackWrapper(name, context) {
  var namespaces = name.split('.'), func = namespaces.pop(), obj = window;
  for (var i = 0; obj && i < namespaces.length; i++) {
    obj = obj[namespaces[i]];
  }
  if (!obj) {
    throw "Wysiwyg: Unable to locate callback " + namespaces.join('.') + "." + func + "()";
  }
  if (!context) {
    context = obj;
  }
  else if (typeof context == 'string'){
    namespaces = context.split('.');
    context = window;
    for (i = 0; context && i < namespaces.length; i++) {
      context = context[namespaces[i]];
    }
    if (!context) {
      throw "Wysiwyg: Unable to locate context object " + namespaces.join('.');
    }
  }
  if (typeof obj[func] != 'function') {
    throw "Wysiwyg: " + func + " is not a callback function";
  }
  return function () {
    return obj[func].apply(context, arguments);
  }
}

var oldBeforeSerialize = (Drupal.ajax ? Drupal.ajax.prototype.beforeSerialize : false);
if (oldBeforeSerialize) {
  /**
   * Filter the ajax_html_ids list sent in AJAX requests.
   *
   * This overrides part of the form serializer to not include ids we know will
   * not collide because editors are removed before those ids are reused.
   *
   * This avoids hitting like max_input_vars, which defaults to 1000,
   * even with just a few active editor instances.
   */
  Drupal.ajax.prototype.beforeSerialize = function (element, options) {
    var ret = oldBeforeSerialize.call(this, element, options);
    var excludeSelectors = [];
    $.each(Drupal.wysiwyg.excludeIdSelectors, function () {
      if ($.isArray(this)) {
        excludeSelectors = excludeSelectors.concat(this);
      }
    });
    if (excludeSelectors.length > 0) {
      options.data['ajax_html_ids[]'] = [];
      $('[id]:not(' + excludeSelectors.join(',') + ')').each(function () {
      options.data['ajax_html_ids[]'].push(this.id);
      });
    }
    return ret;
  }
}

// Respond to CTools detach behaviors event.
$(document).unbind('CToolsDetachBehaviors.wysiwyg').bind('CToolsDetachBehaviors.wysiwyg', function(event, context) {
  $('.wysiwyg:input', context).removeOnce('wysiwyg').each(function () {
    Drupal.wysiwygDetach(context, this.id, 'unload');
    // The 'none' instances are destroyed with the dialog.
    delete Drupal.wysiwyg.instances[this.id];
    delete _internalInstances[this.id];
    var baseFieldId = (this.id.indexOf('--') === -1 ? this.id : this.id.substr(0, this.id.indexOf('--')));
    delete _fieldInfoStorage[baseFieldId];
  });
});

})(jQuery);
