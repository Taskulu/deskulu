(function($) {

/**
 * Attach this editor to a target element.
 */
Drupal.wysiwyg.editor.attach.nicedit = function(context, params, settings) {
  // Intercept and ignore submit handlers or they will revert changes made
  // since the instance was removed. The handlers are anonymous and hidden out
  // of scope in a closure so we can't unbind them. The same operations are
  // performed when the instance is detached anyway.
  var oldAddEvent = bkLib.addEvent;
  bkLib.addEvent = function(obj, type, fn) {
    if (type != 'submit') {
      oldAddEvent(obj, type, fn);
    }
  }
  // Attach editor.
  var editor = new nicEditor(settings);
  editor.panelInstance(params.field);
  // The old addEvent() must be restored after creating a new instance, as
  // plugins with dialogs use it to bind submit handlers to their forms.
  bkLib.addEvent = oldAddEvent;
  editor.addEvent('focus', function () {
    Drupal.wysiwyg.activeId = params.field;
  });
};

/**
 * Detach a single or all editors.
 *
 * See Drupal.wysiwyg.editor.detach.none() for a full description of this hook.
 */
Drupal.wysiwyg.editor.detach.nicedit = function (context, params, trigger) {
  if (typeof params != 'undefined') {
    var instance = nicEditors.findEditor(params.field);
    if (instance) {
      if (trigger == 'serialize') {
        instance.saveContent();
      }
      else {
        instance.ne.removeInstance(params.field);
        instance.ne.removePanel();
      }
    }
  }
  else {
    for (var e in nicEditors.editors) {
      // Save contents of all editors back into textareas.
      var instances = nicEditors.editors[e].nicInstances;
      for (var i = 0; i < instances.length; i++) {
        if (trigger == 'serialize') {
          instances[i].saveContent();
        }
        else {
          instances[i].remove();
        }
      }
      // Remove all editor instances.
      if (trigger != 'serialize') {
        nicEditors.editors[e].nicInstances = [];
      }
    }
  }
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
 * Instance methods for nicEdit.
 */
Drupal.wysiwyg.editor.instance.nicedit = {
  insert: function (content) {
    var instance = nicEditors.findEditor(this.field),
    editingArea = instance.getElm(),
    sel = instance.getSel(), range;
    // IE.
    if (document.selection) {
      range = sel.createRange();
      // If the caret is not in the editing area, just append the content.
      if (!isInside(range.parentElement(), editingArea)) {
        editingArea.innerHTML += content;
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
        editingArea.innerHTML += content;
        return;
      }

      var fragment = editingArea.ownerDocument.createDocumentFragment();
      // Fragments don't support innerHTML.
      var wrapper = editingArea.ownerDocument.createElement('div');
      wrapper.innerHTML = content;
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
    nicEditors.findEditor(this.field).setContent(content);
  },

  getContent: function () {
    return nicEditors.findEditor(this.field).getContent();
  }
};

})(jQuery);
