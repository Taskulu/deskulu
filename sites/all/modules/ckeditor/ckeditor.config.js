/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

/*
 WARNING: clear browser's cache after you modify this file.
 If you don't do this, you may notice that browser is ignoring all your changes.
 */
CKEDITOR.editorConfig = function(config) {
  config.indentClasses = [ 'rteindent1', 'rteindent2', 'rteindent3', 'rteindent4' ];

  // [ Left, Center, Right, Justified ]
  config.justifyClasses = [ 'rteleft', 'rtecenter', 'rteright', 'rtejustify' ];

  // The minimum editor width, in pixels, when resizing it with the resize handle.
  config.resize_minWidth = 450;

  // Protect PHP code tags (<?...?>) so CKEditor will not break them when
  // switching from Source to WYSIWYG.
  // Uncommenting this line doesn't mean the user will not be able to type PHP
  // code in the source. This kind of prevention must be done in the server
  // side
  // (as does Drupal), so just leave this line as is.
  config.protectedSource.push(/<\?[\s\S]*?\?>/g); // PHP Code

  // [#1762328] Uncomment the line below to protect <code> tags in CKEditor (hide them in wysiwyg mode).
  // config.protectedSource.push(/<code>[\s\S]*?<\/code>/gi);
  config.extraPlugins = '';

  /*
    * Append here extra CSS rules that should be applied into the editing area.
    * Example:
    * config.extraCss = 'body {color:#FF0000;}';
    */
  config.extraCss = '';
  /**
    * Sample extraCss code for the "marinelli" theme.
    */
  if (Drupal.settings.ckeditor.theme == "marinelli") {
    config.extraCss += "body{background:#FFF;text-align:left;font-size:0.8em;}";
    config.extraCss += "#primary ol, #primary ul{margin:10px 0 10px 25px;}";
  }
  if (Drupal.settings.ckeditor.theme == "newsflash") {
    config.extraCss = "body{min-width:400px}";
  }

  /**
    * CKEditor's editing area body ID & class.
    * See http://drupal.ckeditor.com/tricks
    * This setting can be used if CKEditor does not work well with your theme by default.
    */
  config.bodyClass = '';
  config.bodyId = '';
  /**
    * Sample bodyClass and BodyId for the "marinelli" theme.
    */
  if (Drupal.settings.ckeditor.theme == "marinelli") {
    config.bodyClass = 'singlepage';
    config.bodyId = 'primary';
  }

  // Make CKEditor's edit area as high as the textarea would be.
  if (this.element.$.rows > 0) {
    config.height = this.element.$.rows * 20 + 'px';
  }
}

/*
 * Sample toolbars
 */

//Toolbar definition for basic buttons
Drupal.settings.cke_toolbar_DrupalBasic = [ [ 'Format', 'Bold', 'Italic', '-', 'NumberedList','BulletedList', '-', 'Link', 'Unlink', 'Image' ] ];

//Toolbar definition for Advanced buttons
Drupal.settings.cke_toolbar_DrupalAdvanced = [
  ['Source'],
  ['Cut','Copy','Paste','PasteText','PasteFromWord','-','SpellChecker', 'Scayt'],
  ['Undo','Redo','Find','Replace','-','SelectAll'],
  ['Image','Flash','Table','HorizontalRule','Smiley','SpecialChar'],
  ['Maximize', 'ShowBlocks'],
  '/',
  ['Format'],
  ['Bold','Italic','Underline','Strike','-','Subscript','Superscript','-','RemoveFormat'],
  ['NumberedList','BulletedList','-','Outdent','Indent','Blockquote'],
  ['JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock','-','BidiLtr','BidiRtl'],
  ['Link','Unlink','Anchor','Linkit','LinkToNode','LinkToMenu']
];

// Toolbar definition for all buttons
Drupal.settings.cke_toolbar_DrupalFull = [
  ['Source'],
  ['Cut','Copy','Paste','PasteText','PasteFromWord','-','SpellChecker', 'Scayt'],
  ['Undo','Redo','Find','Replace','-','SelectAll'],
  ['Image','Flash','Table','HorizontalRule','Smiley','SpecialChar','Iframe'],
  '/',
  ['Bold','Italic','Underline','Strike','-','Subscript','Superscript','-','RemoveFormat'],
  ['NumberedList','BulletedList','-','Outdent','Indent','Blockquote','CreateDiv'],
  ['JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock','-','BidiLtr','BidiRtl','-','Language'],
  ['Link','Unlink','Anchor','Linkit','LinkToNode', 'LinkToMenu'],
  '/',
  ['Format','Font','FontSize'],
  ['TextColor','BGColor'],
  ['Maximize', 'ShowBlocks'],
  ['DrupalBreak', 'DrupalPageBreak']
];