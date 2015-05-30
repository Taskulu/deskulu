#CKEditor plugin: Dragable image resizing

This plugin implements draggable image resizing, currently just for Webkit-based browsers (Chrome/Safari/Opera). This feature already exists in Firefox and Internet Explorer as a built-in browser capability but not in the other browsers. So if you or your users are used to seeing the drag-to-resize handles at the corner of images, but use Chrome or Safari, install this plugin to get it back (with a few bonus features).

###Demo:
[Online Demo Here] (http://sstur.github.io/ck-dragresize/)

###Features:
 * Shows semi-transparent overlay while resizing
 * Enforces Aspect Ratio (unless holding shift)
 * Snap to size of other images in editor (optional)
 * Escape while dragging cancels resize
 * Undo and Redo support
 * Image dragging and Right-click still work

I have implemented this feature in pure JavaScript with no external dependencies. It only activates if a supported browser is detected. It has been tested in most recent versions of Chrome and Safari on PC and Mac.

###Browser Support
 * Chrome and Safari are currently supported
 * Opera support is reportedly working but not fully tested
 * Firefox and IE have this feature built-in, so this plugin does not activate in those browsers

###Todo / Planned Features
 * Somehow account for images that have border/padding so sizing is more accurate
 * Allow use in Firefox and modern IE (disabling the built-in feature)

###Contributers:
  * [Simon Sturmer] (https://github.com/sstur)
  * [Nathan Haug] (https://github.com/quicksketch)
  * [ruscoder] (https://github.com/ruscoder)
  * [Brant Wynn] (https://github.com/brantwynn)

Please, if you notice any bugs, open an issue in the [issue tracker](ck-dragresize/issues).

This plugin is licensed under the MIT license. See [LICENSE](ck-dragresize/blob/master/LICENSE) for further details.
