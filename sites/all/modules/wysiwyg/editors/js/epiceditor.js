(function($) {

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.epiceditor = function (context, params, settings) {
  var $target = $('#' + params.field);
  var containerId = params.field + '-epiceditor';
  var defaultContent = $target.val();
  $target.hide().after('<div id="' + containerId + '" />');

  settings.container = containerId;
  settings.file = {
    defaultContent: defaultContent
  };
  settings.theme = {
    preview: '/themes/preview/preview-dark.css',
    editor: '/themes/editor/' + settings.theme + '.css'
  }
  var editor = new EpicEditor(settings).load();
  $target.data('epiceditor', editor);
};

/**
 * Detach a single or all editors.
 */
Drupal.wysiwyg.editor.detach.epiceditor = function (context, params, trigger) {
  var $target = $('#' + params.field);
  var editor = $target.data('epiceditor');

  $target.val(editor.exportFile());

  editor.unload(function () {
    $target.show();
  });
};

})(jQuery);
