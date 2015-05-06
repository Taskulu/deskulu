(function ($) {

Drupal.wysiwyg.plugins.fontyourface = {

  /**
   * Returns whether the passed node belongs to this plugin.
   */
  isNode: function(node) {
    return true;
  }, // isNode

  /**
   * Execute the button.
   */
  invoke: function(data, settings, instanceId) {

    var button = $('#cke_' + instanceId + ' .cke_button_fontyourface');
    var position = button.offset();
    var fontSelect = $('<div id="' + instanceId + '_fontyourface-select" class="fontyourface-select"><ul><li data-fid="0">-- NONE --</li></ul></div>');

    fontSelect.data('instanceId', instanceId);
    fontSelect.data('data', data);

    for (i in settings.fonts) {

      var font = settings.fonts[i];

      $('<li data-fid="' + font.fid + '">' + font.name + '</li>').appendTo(fontSelect.find('ul'));

    } // for
    
    fontSelect.find('li').click(function() {

      var li = $(this);
      var div = li.parents('div.fontyourface-select');
      var data = div.data('data');
      var instanceId = div.data('instanceId');
      var fid = li.attr('data-fid');

      if (fid > 0) {

        if (data.format == 'html') {
          var content = '<span class="fontyourface-' + fid + '">' + data.content + '</span>';
        } // if
        else {
          var content = '<span class="fontyourface-' + fid + '">' + data.content + '</span>';
        } // else

        if (typeof content != 'undefined') {
          Drupal.wysiwyg.instances[instanceId].insert(content);
        } // if

      } // if

      div.remove();

    });
    
    fontSelect.css({
      'position': 'absolute', 
      'top': position.top + button.height(), 
      'left': position.left,
      'border': '1px #999 solid',
      'background': '#fff'
    });
    fontSelect.find('ul').css({
      'list-style': 'none',
      'margin': '0'
    });
    fontSelect.find('li')
      .css({
        'margin': '0',
        'padding': '5px'
      })
      .hover(
        function() {
          $(this).css('background', '#ccc');
        }, 
        function() {
          $(this).css('background', '#fff');
        }
      );

    fontSelect.appendTo('body');

  }, // invoke

  attach: function(content, settings, instanceId) {
    return content;
  }, // attach

  detach: function(content, settings, instanceId) {
    return content;
  } // detach

};

})(jQuery);
