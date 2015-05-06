/**
 * @file devel_node_access.js.
 */

(function ($) {

  /**
   * Perform the access by user ajax request.
   */
  function devel_node_access_user_ajax(context, settings) {
    // Get the cell ID for the first .dna-permission that isn't processed.
    var cell = $('td.dna-permission', context)
               .not('.ajax-processed', context)
               .attr('id');
    if (cell !== undefined) {
      // Generate the URI from the basePath, path, data type, cell ID, and a
      // random token to bypass caching.
      var url = settings.basePath
              + "?q="
              + 'devel/node_access/by_user/json/'
              + cell
              + '/'
              + Math.floor((1000000000 * Math.random())).toString(16);
      // Execute Ajax callback and handle the response.
      $.getJSON(url, function(data) {
        $('#' + cell, context).html(data).addClass('ajax-processed');
        // Call this function again.
        devel_node_access_user_ajax(context, settings);
      });
      // Ajax fails silently on error, mark bad requests with an error message.
      // If the request is just slow this will update when the request succeeds.
      setTimeout(
        function() {
          if ($('#' + cell, context).hasClass('ajax-processed') == false) {
            $('#' + cell, context)
              .html(
                '<span class="error">'
                + '<a href="' + url.replace('/json/', '/html/') + '">'
                + Drupal.t('Error: could not explain access')
                + '</a>'
                + '</span>'
              )
              .addClass('ajax-processed');
            // Call this function again.
            devel_node_access_user_ajax(context, settings);
          }
        },
        3000
      );

    }
  }

  /**
   * Attach the access by user behavior which initiates ajax.
   */
  Drupal.behaviors.develNodeAccessUserAjax = {
    attach: function(context, settings) {
      // Start the ajax.
      devel_node_access_user_ajax(context, settings);
    }
  };

})(jQuery);
