diff --git a/replace/misc/1.9/overlay-parent.js b/replace/misc/1.9/overlay-parent.js
index 480c007..9929f84 100644
--- a/replace/misc/1.9/overlay-parent.js
+++ b/replace/misc/1.9/overlay-parent.js
@@ -903,17 +903,6 @@ Drupal.overlay.getDisplacement = function (region) {
  *   the entire page.
  */
 Drupal.overlay.makeDocumentUntabbable = function (context) {
-  // Manipulating tabindexes for the entire document is unacceptably slow in IE6
-  // and IE7, so in those browsers, the underlying page will still be reachable
-  // via the tab key. However, we still make the links within the Disable
-  // message unreachable, because the same message also exists within the
-  // child document. The duplicate copy in the underlying document is only for
-  // assisting screen-reader users navigating the document with reading commands
-  // that follow markup order rather than tab order.
-  if (jQuery.browser.msie && parseInt(jQuery.browser.version, 10) < 8) {
-    $('#overlay-disable-message a', context).attr('tabindex', -1);
-    return;
-  }
 
   context = context || document.body;
   var $overlay, $tabbable, $hasTabindex;
@@ -950,12 +939,6 @@ Drupal.overlay.makeDocumentUntabbable = function (context) {
  *   the entire page.
  */
 Drupal.overlay.makeDocumentTabbable = function (context) {
-  // Manipulating tabindexes is unacceptably slow in IE6 and IE7. In those
-  // browsers, the underlying page was never made unreachable via tab, so
-  // there is no work to be done here.
-  if (jQuery.browser.msie && parseInt(jQuery.browser.version, 10) < 8) {
-    return;
-  }
 
   var $needsTabindex;
   context = context || document.body;
@@ -963,18 +946,7 @@ Drupal.overlay.makeDocumentTabbable = function (context) {
   // Make the underlying document tabbable again by removing all existing
   // tabindex attributes.
   var $tabindex = $('[tabindex]', context);
-  if (jQuery.browser.msie && parseInt(jQuery.browser.version, 10) < 8) {
-    // removeAttr('tabindex') is broken in IE6-7, but the DOM function
-    // removeAttribute works.
-    var i;
-    var length = $tabindex.length;
-    for (i = 0; i < length; i++) {
-      $tabindex[i].removeAttribute('tabIndex');
-    }
-  }
-  else {
-    $tabindex.removeAttr('tabindex');
-  }
+  $tabindex.removeAttr('tabindex');
 
   // Restore the tabindex attributes that existed before the overlay was opened.
   $needsTabindex = $(Drupal.overlay._hasTabindex, context);
