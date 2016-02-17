
/**
 * Wysiwyg plugin button implementation for Awesome plugin.
 */
Drupal.wysiwyg.plugins.awesome = {
  /**
   * Return whether the passed node belongs to this plugin.
   *
   * @param node
   *   The currently focused DOM element in the editor content.
   */
  isNode: function(node) {
    return ($(node).is('img.mymodule-awesome'));
  },

  /**
   * Execute the button.
   *
   * @param data
   *   An object containing data about the current selection:
   *   - format: 'html' when the passed data is HTML content, 'text' when the
   *     passed data is plain-text content.
   *   - node: When 'format' is 'html', the focused DOM element in the editor.
   *   - content: The textual representation of the focused/selected editor
   *     content.
   * @param settings
   *   The plugin settings, as provided in the plugin's PHP include file.
   * @param instanceId
   *   The ID of the current editor instance.
   */
  invoke: function(data, settings, instanceId) {
    // Generate HTML markup.
    if (data.format == 'html') {
      // Prevent duplicating a teaser break.
      if ($(data.node).is('img.mymodule-awesome')) {
        return;
      }
      var content = this._getPlaceholder(settings);
    }
    // Generate plain text.
    else {
      var content = '<!--break-->';
    }
    // Insert new content into the editor.
    if (typeof content != 'undefined') {
      Drupal.wysiwyg.instances[instanceId].insert(content);
    }
  },

  /**
   * Prepare all plain-text contents of this plugin with HTML representations.
   *
   * Optional; only required for "inline macro tag-processing" plugins.
   *
   * @param content
   *   The plain-text contents of a textarea.
   * @param settings
   *   The plugin settings, as provided in the plugin's PHP include file.
   * @param instanceId
   *   The ID of the current editor instance.
   */
  attach: function(content, settings, instanceId) {
    content = content.replace(/<!--break-->/g, this._getPlaceholder(settings));
    return content;
  },

  /**
   * Process all HTML placeholders of this plugin with plain-text contents.
   *
   * Optional; only required for "inline macro tag-processing" plugins.
   *
   * @param content
   *   The HTML content string of the editor.
   * @param settings
   *   The plugin settings, as provided in the plugin's PHP include file.
   * @param instanceId
   *   The ID of the current editor instance.
   */
  detach: function(content, settings, instanceId) {
    var $content = $('<div>' + content + '</div>');
    $.each($('img.mymodule-awesome', $content), function (i, elem) {
      //...
    });
    return $content.html();
  },

  /**
   * Helper function to return a HTML placeholder.
   *
   * The 'drupal-content' CSS class is required for HTML elements in the editor
   * content that shall not trigger any editor's native buttons (such as the
   * image button for this example placeholder markup).
   */
  _getPlaceholder: function (settings) {
    return '<img src="' + settings.path + '/images/spacer.gif" alt="&lt;--break-&gt;" title="&lt;--break--&gt;" class="wysiwyg-break drupal-content" />';
  }
};

/**
 * Because some editors add a lot of new elements with the id attribute set,
 * Wysiwyg provides a way to exclude such ids from the ajax_html_ids[] parameter
 * sent in AJAX requests. Serverside POST limits such as PHP's max_input_vars
 * could otherwise cause the request to be rejected.
 *
 * The filter gathers a list of jQuery selectors from a global list in
 * Drupal.wysiwyg.excludeIdSelectors, joins them with a comma separator and
 * wraps them in "[id]:not[...]", which is then run on the document the same way
 * Drupal core gathers the ids on every AJAX request.
 *
 * To add to the filter, set a unique key to Drupal.wysiwyg.excludeIdSelectors
 * and set its value to an Array holding one or more selector strings which
 * would match the element(s) to exclude.
 *
 * Beware not to match elements which are not removed before the actual request
 * is performed, or Drupal may accidentally reuse the same id for new elements.
 *
 * Below is a sample from ckeditor.inc matching every id starting with 'cke_'.
 */
Drupal.wysiwyg.excludeIdSelectors.wysiwyg_ckeditor = ['[id^="cke_"]'];
