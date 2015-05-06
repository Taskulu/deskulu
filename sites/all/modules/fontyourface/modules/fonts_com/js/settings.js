(function ($) {

  Drupal.behaviors.fontsComSettings = {

    attach:function(context, settings) {

      $('#edit-auth a.fieldset-title').click(function() {

        fontsComSettingsCollapse('#edit-pass');
        fontsComSettingsCollapse('#edit-create');

      });

      $('#edit-pass a.fieldset-title').click(function() {

        fontsComSettingsCollapse('#edit-auth');
        fontsComSettingsCollapse('#edit-create');

      });

      $('#edit-create a.fieldset-title').click(function() {

        fontsComSettingsCollapse('#edit-auth');
        fontsComSettingsCollapse('#edit-pass');

      });

    } // attach

  } // Drupal.behaviors.fontsComSettings

  function fontsComSettingsCollapse(id) {

    if (!$(id).is('.collapsed')) {
      Drupal.toggleFieldset(id);
    } // if

  } // fontsComSettingsCollapse

})(jQuery);
