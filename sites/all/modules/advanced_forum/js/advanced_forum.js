(function ($) {
  Drupal.advanced_forum = Drupal.advanced_forum || {};

  Drupal.behaviors.advanced_forum = {
    attach: function(context) {
      // Retrieve the collapsed status from a stored cookie.
      // cookie format is: page1=1,2,3/page2=1,4,5/page3=5,6,1...
      var cookie = $.cookie('Drupal.advanced_forum.collapsed');
      var pages = cookie ? cookie.split('/') : [];
      // Create associative array where key=page path and value=comma-separated list of collapsed forum ids
      Drupal.advanced_forum.collapsed_page = [];
      if (pages) {
        for (var i = 0; i < pages.length; i++) {
          var tmp = pages[i].split('=');
          Drupal.advanced_forum.collapsed_page[tmp[0]] = tmp[1].split(',');
        }
      }

      // Get data for current page
      Drupal.advanced_forum.collapsed_current = Drupal.advanced_forum.collapsed_page[encodeURIComponent(window.location.pathname)];

      if (!Drupal.advanced_forum.collapsed_current) {
        Drupal.advanced_forum.collapsed_current = [];
        // For intial load default collapsed state settings needs to checked in init function.
        Drupal.advanced_forum.initial_load = 1;
      }
      else {
        Drupal.advanced_forum.initial_load = 0;
      }

      var handleElement = $('.forum-collapsible', context);

      // Set initial collapsed state
      handleElement.once('forum-collapsible', Drupal.advanced_forum.init);

      handleElement.addClass('clickable').click(function(event) {
        event.preventDefault();

        // Get forum id
        var id = $(this).attr('id').split('-')[2];
        if ( $(this).hasClass('container-collapsed')) {
          Drupal.advanced_forum.expand(id, Drupal.settings.advanced_forum.effect);
          // Reset collapsed status
          Drupal.advanced_forum.collapsed_current.splice($.inArray(id, Drupal.advanced_forum.collapsed_current),1);
        }
        else {
          Drupal.advanced_forum.collapse(id, Drupal.settings.advanced_forum.effect);
          // Set collapsed status
          Drupal.advanced_forum.collapsed_current.push(id);
        }

        // Put status back
        Drupal.advanced_forum.collapsed_page[encodeURIComponent(window.location.pathname)] = Drupal.advanced_forum.collapsed_current;

        // Build cookie string
        cookie = '';
        for (var x in Drupal.advanced_forum.collapsed_page) {
          cookie += '/' + x + '=' + Drupal.advanced_forum.collapsed_page[x];
        }
        // Save new cookie
        $.cookie(
          'Drupal.advanced_forum.collapsed',
          cookie.substr(1),
          {
            path: '/',
            // The cookie should "never" expire.
            expires: 36500
          }
          );
      });
    }
  };

  /**
   * Initialize and set collapsible status.
   * Initial collapsing/expanding effect is set to 'toggle' to avoid flickers.
   */
  Drupal.advanced_forum.init = function() {
    // get forum id
    var id = $(this).attr('id').split('-')[2];

    // On initial load, deal with default collapsed state of containers.
    if(Drupal.advanced_forum.initial_load) {
      var list = 0;
      for(list in Drupal.settings.advanced_forum.default_collapsed_list) {
        if (id == list) {
          Drupal.advanced_forum.collapse(id, 'toggle');
          break;
        }
      }
    }
    else {
      // Check if item is collapsed
      if ($.inArray(id, Drupal.advanced_forum.collapsed_current) > -1) {
        $(this)
        .addClass('container-collapsed')
        .parent().addClass('container-collapsed');
        Drupal.advanced_forum.collapse(id, 'toggle');
      }
      else {
        $(this)
        .removeClass('container-collapsed')
        .parent().removeClass('container-collapsed');
        Drupal.advanced_forum.expand(id, 'toggle');
      }
    }

  };

  Drupal.advanced_forum.collapse = function(id, effect) {
    switch(effect) {
      case 'fade':
        $('#forum-table-' + id).fadeOut('fast');
        break;
      case 'slide':
        $('#forum-table-' + id).slideUp('fast');
        break;
      default:
        $('#forum-table-' + id).hide();
    }
    $('#forum-collapsible-' + id)
    .addClass('container-collapsed')
    .parent().addClass('container-collapsed');
  };

  Drupal.advanced_forum.expand = function(id, effect) {
    switch(effect) {
      case 'fade':
        $('#forum-table-' + id).fadeIn('fast');
        break;
      case 'slide':
        $('#forum-table-' + id).slideDown('fast');
        break;
      default:
        $('#forum-table-' + id).show();
    }
    $('#forum-collapsible-' + id)
    .removeClass('container-collapsed')
    .parent().removeClass('container-collapsed');
  };

})(jQuery);
