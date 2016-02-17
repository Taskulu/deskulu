(function($) {

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.epiceditor = function (context, params, settings) {
  var $target = $('#' + params.field),
    containerId = params.field + '-epiceditor',
    defaultContent = $target.val(),
    $container = $('<div id="' + containerId + '" />');
  $target.hide().after($container);
  if (!settings.height) {
    settings.height = $('#' + params.field).height();
  }
  $container.height(settings.height);

  settings.container = containerId;
  settings.file = {
    defaultContent: defaultContent
  };
  settings.theme = {
    preview: '/themes/preview/preview-dark.css',
    editor: '/themes/editor/' + settings.theme + '.css'
  };
  var editor = new EpicEditor(settings).load();
  $target.data('epiceditor', editor);
};

/**
 * Detach a single or all editors.
 */
Drupal.wysiwyg.editor.detach.epiceditor = function (context, params, trigger) {
  var $targets = $();
  if (typeof params != 'undefined') {
    $targets.push($('#' + params.field));
  }
  else {
    // Check all fields in context for active EpicEditor instances.
    var $wysiwygs = $('.wysiwyg-processed', context);
    $wysiwygs.each(function () {
      var $this = $(this);
      if (!$this.data('epiceditor')) {
        $targets.push($this);
      }
    });
  }

  $targets.each(function () {
    var $target = $(this);
    var editor = $target.data('epiceditor');
    // Save contents of the editor back into the textarea.
    $target.val(editor.exportFile());
    if (trigger !== 'serialize') {
      // Remove editor instance and container.
      editor.unload(function () {
        $target.show();
        $('#' + $target.attr('id') + '-epiceditor').remove();
      });
      $target.removeData('epiceditor');
    }
  });
};

/**
 * Check if a DOM node is inside another or if they are the same.
 */
function isInside (innerNode, outerNode) {
  var found = false;
  if (innerNode === outerNode) {
    return true;
  }
  $(innerNode).parents().each(function (index, parent) {
    if (parent === outerNode) {
      found = true;
      return false;
    }
  });
  return found;
}

/**
 * Converts HTML markup to plain text.
 *
 * EpicEditor isn't WYSIWYG and is meant to handle plain text though it does so
 * in a contentEditable element. This is taken from EpicEditor's internal
 * _setText() function in version 0.2.0.
 */
function toPlainText (content) {
  content = content.replace(/</g, '&lt;');
  content = content.replace(/>/g, '&gt;');
  content = content.replace(/\n/g, '<br>');
  content = content.replace(/\s\s/g, ' &nbsp;')
  return content;
}

Drupal.wysiwyg.editor.instance.epiceditor = {
  insert: function (content) {
    var instance = this.getInstance();
    var editingArea = instance.getElement('editor').body;
    // IE.
    // @todo Can't test this, EpicEditor breaks in IE.
    if (document.selection) {
      var sel = editingArea.selection;
      range = sel.createRange();
      // If the caret is not in the editing area, just append the content.
      if (!isInside(range.parentElement(), editingArea)) {
        editingArea.innerHTML += toPlainText(content);
      }
      else {
        // Insert content and set the caret after it.
        range.pasteHTML(content);
        range.select();
        range.collapse(false);
      }
    }
    else {
      // The code below doesn't work in IE, but it never gets here.
      var sel = editingArea.ownerDocument.getSelection();

      // Convert selection to a range.
        // W3C compatible.
        if (sel.getRangeAt) {
          if (sel.rangeCount > 0) {
            range = sel.getRangeAt(0);
          }
        }
        // Safari.
        else {
          range = editingArea.ownerDocument.createRange();
          range.setStart(sel.anchorNode, sel.anchorOffset);
          range.setEnd(sel.focusNode, userSeletion.focusOffset);
        }
      // If the caret is not in the editing area, just append the content.
      if (sel.rangeCount == 0 || !isInside(range.commonAncestorContainer, editingArea)) {
        editingArea.innerHTML += toPlainText(content);
        return;
      }

      var fragment = editingArea.ownerDocument.createDocumentFragment();
      // Fragments don't support innerHTML.
      var wrapper = editingArea.ownerDocument.createElement('div');
      wrapper.innerHTML = toPlainText(content);
      while (wrapper.firstChild) {
        fragment.appendChild(wrapper.firstChild);
      }
      // Append a temporary node to control caret position.
      var tn = editingArea.ownerDocument.createElement('span');
      fragment.appendChild(tn);
      range.deleteContents();
      // Only fragment children are inserted.
      range.insertNode(fragment);
      // Move caret to temp node and remove it.
      range.setStartBefore(tn);
      range.setEndBefore(tn);
      sel.removeAllRanges();
      sel.addRange(range);
      tn.parentNode.removeChild(tn);
    }
  },

  setContent: function (content) {
    this.getInstance().importFile(null, content);
  },

  getContent: function () {
    return this.getInstance().exportFile();
  },

  isFullscreen: function () {
    return this.getInstance().is('fullscreen');
  },

  getInstance: function () {
    if (!this.editorInstance) {
      this.editorInstance = $('#' + this.field).data('epiceditor');
    }
    return this.editorInstance;
  }
}

})(jQuery);
