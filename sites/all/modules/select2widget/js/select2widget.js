/**
 * Created with IntelliJ IDEA.
 * User: Sergey Grigorenko
 * Date: 17.07.13
 * Time: 20:14
 */
(function ($) {
  Drupal.behaviors.select2widget = {
    attach: function (context, settings) {
      this.overwriteSelect2Defaults();

      if (checkjQueryRequirements()) {
        var config = settings.select2widget2;
        if (typeof config != 'undefined' && typeof config.elements != 'undefined') {
            for (var el in config.elements) {
                var e = $('#' + config.elements[el].id, context);
                e.select2({
                  width: 'element'
                });
            }
        }

        var config = settings.select2widgetajax;

        if (typeof config != 'undefined' && typeof config.elements != 'undefined') {
          var settings = config.elements;

          for (var el in config.elements) {
            var e = $('#' + config.elements[el].id + ':not(.select2widget-processed)', context);

            var url = Drupal.settings.basePath + settings[el].url;

            if(e.length == 0) {
              continue;
            }

            e.addClass('select2widget-processed');

            e.select2({
              width: (settings[el].width) ? settings[el].width : 'element',
              separator: settings[el].separator,
              placeholder: Drupal.t(settings[el].placeholder),
              allowClear: true,
              minimumInputLength: settings[el].min_char,
              maximumSelectionSize: settings[el].cardinality,
              multiple: settings[el].cardinality == 1 ? false : true,
              tokenSeparators : [','],
              createSearchChoice: function(term, data) {
                var el = this.containerId.replace('s2id_', '');

                if(!settings[el].allow_new || settings[el].allow_new == 0) {
                  return;
                }

                if ($(data).filter(function() {
                  return this.title.localeCompare(term) === 0;
                }).length === 0) {
                  return {
                    id:  term,
                    title: term,
                    data: term
                  };
                }
              },
              ajax: { // instead of writing the function to execute the request we use Select2's convenient helper
                  url: url,
                  data: function (term, page) {
                    return {
                      search_string: term // search term
                    };
                  },
                  quietMillis: (settings[el].delay) ? settings[el].delay : 100,
                  dataType: 'json',
                  results: function (data, page) {
                    // parse the results into the format expected by Select2.
                    // since we are using custom formatting functions we do not need to alter remote JSON data
                    return {results: data};
                  }
              },
              initSelection: function (element, callback) {
                // the input tag has a value attribute preloaded that points to a preselected movie's id
                // this function resolves that id attribute to an object that select2 can render
                // using its formatResult renderer - that way the movie name is shown preselected

                var data = [];

                if(settings[el].cardinality == 1) {
                  for (id in settings[el].init) {
                    data = {id: id, title: settings[el].init[id]};
                  }
                }
                else {
                  for (id in settings[el].init) {
                    data.push({id: id, title: settings[el].init[id]});
                  }
                  element.val('');
                }

                // Drupal settings get merged on ajax, so empty out init so
                // has new value when updated via ajax.
                if (typeof Drupal.settings.select2widgetajax != 'undefined' && typeof Drupal.settings.select2widgetajax.elements[el] != 'undefined') {
                  Drupal.settings.select2widgetajax.elements[el].init = {};
                }

                callback(data);

              },

              formatResult: select2widgetFormatResult, // omitted for brevity, see the source of this page
              formatSelection: select2widgetFormatSelection,  // omitted for brevity, see the source of this page
              dropdownCssClass: "bigdrop", // apply css that makes the dropdown taller
              escapeMarkup: function (m) {
                  return m;
              } // we do not want to escape markup since we are displaying html in results
            });

            //  }
          }
        }
      }
      else {
        alert('Select2 widget required jQuery version >=1.7.1 \nGo to "Configuration -> Development -> jQuery update" and choose "jQuery Version" 1.7 or 1.8.')
      }
    },
    completedCallback: function () {
        // Do nothing. But it's here in case other modules/themes want to override it.
    },

    overwriteSelect2Defaults: function() {
      $.fn.select2.defaults.formatInputTooShort = function(input, min) {
         return Drupal.t('Please enter one more character');
      }

      $.fn.select2.defaults.formatNoMatches = function () {
        return Drupal.t("No matches found");
      }

      $.fn.select2.defaults.formatLoadMore = function (pageNumber) {
        return Drupal.t("Loading more results...");
      }

      $.fn.select2.defaults.formatSearching = function () {
        return Drupal.t("Searching...");
      }
    }
  };

  function checkjQueryRequirements(version) {
    var version = $.fn.jquery.split('.');

    if (version[0] < 1 || version[1] < 7) {
      return false;
    }

    return true;
  }

  function select2widgetFormatResult(data) {
    return data.data;
  }

  function select2widgetFormatSelection(data) {
   return data.title;
  }
})(jQuery);
