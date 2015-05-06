(function ($) {

  Drupal.behaviors.fontyourfaceBrowse = {

    attach:function(context, settings) {

      $('.fontyourface-footer a.enable-link').click(fontyourfaceEnableClick);
      $('.fontyourface-footer a.disable-link').click(fontyourfaceDisableClick);

    } // attach

  } // Drupal.behaviors.fontyourfaceAddForm

  function fontyourfaceEnableClick() {
  
    var link = $(this);
    var fid = link.parent().attr('data-fid');
    var enableUrl = Drupal.settings.basePath + '?q=admin/appearance/fontyourface/ajax/enable/' + fid;

    $('<span class="fontyourface-indicator"></span>').appendTo(this);

    $.post(enableUrl, {fid: fid}, function(json) {

      var font = link.parents('.font');
            
      if (json.complete == '1') {

        font.find('.enabled-no')
          .addClass('enabled-yes')
          .removeClass('enabled-no');
        link
          .text(Drupal.t('Disable'))
          .addClass('disable-link')
          .removeClass('enable-link')
          .unbind('click')
          .click(fontyourfaceDisableClick);

      } // if

      font.find('.fontyourface-indicator').remove();
      $('.view-header div').html(json.status);

    }, 'json');

    return false;
  
  } // fontyourfaceEnableClick
  
  function fontyourfaceDisableClick() {
  
    var link = $(this);
    var fid = link.parent().attr('data-fid');
    var disableUrl = Drupal.settings.basePath + '?q=admin/appearance/fontyourface/ajax/disable/';

    $('<span class="fontyourface-indicator"></span>').appendTo(this);
    
    $.post(disableUrl, {fid: fid}, function(json) {

      var font = link.parents('.font');

      if (json.complete == '1') {

        font.find('.enabled-yes')
          .addClass('enabled-no')
          .removeClass('enabled-yes');
        link
          .text(Drupal.t('Enable'))
          .addClass('enable-link')
          .removeClass('disable-link')
          .unbind('click')
          .click(fontyourfaceEnableClick);

      } // if
      
      font.find('.fontyourface-indicator').remove();
      $('.view-header div').html(json.status);

    }, 'json');

    return false;
  
  } // fontyourfaceDisableClick

})(jQuery);
