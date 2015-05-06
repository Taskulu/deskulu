(function ($) {

// Explain link in query log
Drupal.behaviors.devel_explain = {
  attach: function(context, settings) {
    $('a.dev-explain').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-explain', cell).load(settings.basePath + '?q=devel/explain/' + settings.devel.request_id + '/' + qid).show();
      $('.dev-placeholders', cell).hide();
      $('.dev-arguments', cell).hide();
      return false;
    });
  }
}

// Arguments link in query log
Drupal.behaviors.devel_arguments = {
  attach: function(context, settings) {
    $('a.dev-arguments').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-arguments', cell).load(settings.basePath + '?q=devel/arguments/' + settings.devel.request_id + '/' + qid).show();
      $('.dev-placeholders', cell).hide();
      $('.dev-explain', cell).hide();
      return false;
    });
  }
}

// Placeholders link in query log
Drupal.behaviors.devel_placeholders = {
  attach: function(context, settings) {
    $('a.dev-placeholders').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-explain', cell).hide();
      $('.dev-arguments', cell).hide();
      $('.dev-placeholders', cell).show();
      return false;
    });
  }
}

})(jQuery);
