(function ($) {

  Drupal.behaviors.fontsComProject = {

    attach:function(context, settings) {

      $('#edit-edit-project').hide();
      
      if ($('#edit-project').val() != '') {
        $('#edit-project').change();
      } // if

    } // attach

  } // Drupal.behaviors.fontsComProject

})(jQuery);