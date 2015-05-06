(function ($) {
  Drupal.behaviors.RateSlider = {
    attach: function (context) {
      $('.rate-widget-slider:not(.rate-slider-processed)',context).addClass('rate-slider-processed').each(function() {
        var widget = $(this);
        var ids = widget.attr('id').match(/^rate\-([a-z]+)\-([0-9]+)\-([0-9]+)\-([0-9])$/);
        var data = {
          content_type: ids[1],
          content_id: ids[2],
          widget_id: ids[3],
          widget_mode: ids[4]
        };

        var s = $(".rate-slider", widget);
        var v = $(s).attr("class").match(/rate\-value\-([0-9]+)/)[1];

        widget.prepend(s);
        
        // Check if this widget is active (disabled widgets have <span>'s instead of <a>'s).'
        if ($("ul a", widget).length > 0) {
          // Add the slider.
          s.slider({
            min: 10,
            max: 100,
            steps: 90,
            startValue: v, // jQuery UI 1.2
            value: v, // jQuery UI 1.3
            slide: function(event,ui) {
              //ui.value;
              var vote = ui.value / 10;
              $(".rate-slider-value", s).width(((ui.value - 10) * (10 / 9)) + '%');
              $(".rate-info", widget).text(Drupal.t("Your vote: !vote", {"!vote": vote.toFixed(1)}));
            },
            stop: function(event,ui) {
              data.value = ui.value;
              var itemid = "#rate-button-" + Math.round(data.value / 10);
              var token = $(itemid).attr('href').match(/rate\=([a-zA-Z0-9\-_]{32,64})/)[1];
              return Drupal.rateVote(widget, data, token);
            }
          });
        }
        else {
          // Widget is disabled. Only add the slider styling.
          $(s).width('200px');
          $(s).addClass('ui-slider');
        }

        // Add the rating bar.
        s.prepend('<div class="rate-slider-value" style="width: ' + ((v - 10) * (10 / 9)) + '%" />');

        // Hide the links for the non-js variant.
        $("ul", widget).hide();
      });
    }
  }
})(jQuery);
