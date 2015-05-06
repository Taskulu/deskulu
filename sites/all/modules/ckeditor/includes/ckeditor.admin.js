/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
(function ($) {
  Drupal.ckeditor_ver = 4;

  $(document).ready(function() {
    if (typeof(CKEDITOR) == "undefined")
      return;

    // $('#edit-uicolor-textarea').show();

    if (Drupal.settings.ckeditor_version) {
      Drupal.ckeditor_ver = Drupal.settings.ckeditor_version.split('.')[0];
    }

    Drupal.editSkinEditorInit = function() {
      var skinframe_src = $('#skinframe').attr('src');
      //skinframe_src = skinframe_src.replace(/skin=[^&]+/, 'skin='+$("#edit-skin").val());
      var skin = skinframe_src.match(/skin=([^&]+)/)[1];
      if ($('#edit-uicolor').val() == 'custom') {
        skinframe_src = skinframe_src.replace(/uicolor=[^&]+/, 'uicolor='+$('input[name$="uicolor_user"]').val().replace('#', '') || 'D3D3D3');
      }
      else {
        skinframe_src = skinframe_src.replace(/uicolor=[^&]+/, 'uicolor=D3D3D3');
      }
      $('#skinframe').attr('src', skinframe_src);

      if (Drupal.ckeditor_ver == 3) {
        if (skin == "kama") {
          $("#edit-uicolor").removeAttr('disabled');
          $("#edit-uicolor").parent().removeClass('form-disabled');
        }
        else {
          $("#edit-uicolor").attr('disabled', 'disabled');
          $("#edit-uicolor").parent().addClass('form-disabled');
        }
      }
      else {
        $("#edit-uicolor").removeAttr('disabled');
        $("#edit-uicolor").parent().removeClass('form-disabled');
      }
    };

    Drupal.editSkinEditorInit();

    $("#edit-uicolor").bind("change", function() {
      Drupal.editSkinEditorInit();
    });

    $("#input-formats :checkbox").change(function() {
      $('#security-filters .filter-warning').hide();
      $('#security-filters div.filter-text-formats[filter]').html('');
      $('#security-filters ul.text-formats-config').html('');
      $('#input-formats :checked').each(function() {
        var format_name = $(this).val();
        var format_label = $('label[for="' + $(this).attr('id') + '"]').html();

        if (typeof(Drupal.settings.text_formats_config_links[format_name]) != 'undefined') {
          var text = "<li>" + format_label + " - <a href=\"" + Drupal.settings.text_formats_config_links[format_name].config_url + "\">configure</a></li>";
          var dataSel = $('#security-filters ul.text-formats-config');
          var html = dataSel.html();
          if (html == null || html.length == 0) {
            dataSel.html(text);
          }
          else {
            html += text;
            dataSel.html(html);
          }
        }

        $('#security-filters div.filter-text-formats[filter]').each(function() {
          var filter_name = $(this).attr('filter');
          var dataSel = $(this);
          var html = dataSel.html();
          var status = "enabled";
          if (typeof Drupal.settings.text_format_filters[format_name][filter_name] == 'undefined') {
            status = "disabled";
          }
          var text = "<span class=\"filter-text-format-status " + status + "\">" + format_label + ': </span><br/>';

          if (html == null || html.length == 0) {
            dataSel.html(text);
          }
          else {
            html += text;
            dataSel.html(html);
          }
        });
      });
    });
    $("#input-formats :checkbox:eq(0)").trigger('change');

    $(".cke_load_toolbar").click(function() {
      var buttons = eval('Drupal.settings.'+$(this).attr("id"));
      var text = "[\n";
      for(i in buttons) {
        if (typeof buttons[i] == 'string'){
          text = text + "    '/',\n";
        }
        else {
          text = text + "    [";
          max = buttons[i].length - 1;
          rows = buttons.length - 1;
          for (j in buttons[i]) {
            if (j < max){
              text = text + "'" + buttons[i][j] + "',";
            } else {
              text = text + "'" + buttons[i][j] + "'";
            }
          }
          if (i < rows){
            text = text + "],\n";
          } else {
            text = text + "]\n";
          }
        }
      }

      text = text + "]";
      text = text.replace(/\['\/'\]/g,"'/'");
      $("#edit-toolbar").attr('value',text);
      if (Drupal.settings.ckeditor_toolbar_wizard == 't'){
        Drupal.ckeditorToolbarReload();
      }
      return false;
    });

    if (Drupal.settings.ckeditor_toolbar_wizard == 'f'){
      $("form#ckeditor-admin-profile-form textarea#edit-toolbar, form#ckeditor-admin-profile-form #edit-toolbar + .grippie").show();
    }
  });
})(jQuery);
