(function ($) {

/**
 * Override Views prototype function so it can recognize Bootstrap AJAX pagers.
 * Attach the ajax behavior to each link.
 */
Drupal.views.ajaxView.prototype.attachPagerAjax = function() {
  this.$view.find('ul.pager > li > a, th.views-field a, .attachment .views-summary a, ul.pagination li a')
  .each(jQuery.proxy(this.attachPagerLinkAjax, this));
};

})(jQuery);
