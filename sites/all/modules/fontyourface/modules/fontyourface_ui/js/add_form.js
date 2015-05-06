var fontyourfaceSampleText = '';
var fontyourfaceSampleMarkup = '';

(function ($) {

  Drupal.behaviors.fontyourfaceAddForm = {

    attach:function(context, settings) {

      var input = $('#edit-css');
      var selector = input.val();

      fontyourfaceSampleText = $('#edit-sample-text').val();
      fontyourfaceSampleMarkup = $('.fontyourface-view').html();

      $('#edit-sample-text').keyup(function() {

        var markup = fontyourfaceSampleMarkup;
        var updatedText = $('#edit-sample-text').val();

        if (updatedText != fontyourfaceSampleText) {

          markup = markup.split(fontyourfaceSampleText).join(updatedText);
          markup = markup.split(fontyourfaceEncodeURI(fontyourfaceSampleText)).join(fontyourfaceEncodeURI(updatedText));

        } // if

        $('.fontyourface-view').html(markup);

      });

      if (selector && selector.length > 0) {

        var select = $('<select id="edit-css-select"><option value="">-- none --</option><option value="h1, h2, h3, h4, h5, h6">all headers (h1, h2, h3, h4, h5, h6)</option><option value="h1">h1</option><option value="h2">h2</option><option value="h3">h3</option><option value="p, div">standard text (p, div)</option><option value="body">everything (body)</option><option value="&lt;none&gt;">-- add selector in theme CSS --</option><option value="-- other --">other</option>')
          .change(fontyourfaceCssSelectChange)
          .insertBefore(input);

        if (select.find('option[value="' + selector + '"]').length > 0) {

          select.find('option[value=' + selector + ']').attr('selected', true);
          input.hide();

        } // if
        else {

          select.find('option[value="-- other --"]').attr('selected', true);
          input.show();

        } // else
        
      } // if

    } // attach

  } // Drupal.behaviors.fontyourfaceAddForm

  function fontyourfaceCssSelectChange() {

    var selector = $(this).val();

    if (selector == '') {
      $('#edit-css').show();
    } // if
    else {
      $('#edit-css').val(selector);
      $('#edit-css').hide();
    } // else

  } // fontyourfaceCssSelectChange

  function fontyourfaceEncodeURI(text) {
  
    return encodeURIComponent(text)
      .replace(/!/g, '%21').replace(/'/g, '%27').replace(/\(/g, '%28')
      .replace(/\)/g, '%29').replace(/\*/g, '%2A').replace(/%20/g, '+');

  } // fontyourfaceEncodeURI

})(jQuery);
