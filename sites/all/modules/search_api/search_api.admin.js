/**
 * @file
 * Javascript enhancements for the Search API admin pages.
 */

(function ($) {

/**
 * Allows the re-ordering of enabled data alterations and processors.
 */
// Copied from filter.admin.js
Drupal.behaviors.searchApiStatus = {
  attach: function (context, settings) {
    $('.search-api-status-wrapper input.form-checkbox', context).once('search-api-status', function () {
      var $checkbox = $(this);
      // Retrieve the tabledrag row belonging to this processor.
      var $row = $('#' + $checkbox.attr('id').replace(/-status$/, '-weight'), context).closest('tr');
      // Retrieve the vertical tab belonging to this processor.
      var $tab = $('#' + $checkbox.attr('id').replace(/-status$/, '-settings'), context).data('verticalTab');

      // Bind click handler to this checkbox to conditionally show and hide the
      // filter's tableDrag row and vertical tab pane.
      $checkbox.bind('click.searchApiUpdate', function () {
        if ($checkbox.is(':checked')) {
          $row.show();
          if ($tab) {
            $tab.tabShow().updateSummary();
          }
        }
        else {
          $row.hide();
          if ($tab) {
            $tab.tabHide().updateSummary();
          }
        }
        // Restripe table after toggling visibility of table row.
        Drupal.tableDrag['search-api-' + $checkbox.attr('id').replace(/^edit-([^-]+)-.*$/, '$1') + '-order-table'].restripeTable();
      });

      // Attach summary for configurable items (only for screen-readers).
      if ($tab) {
        $tab.fieldset.drupalSetSummary(function (tabContext) {
          return $checkbox.is(':checked') ? Drupal.t('Enabled') : Drupal.t('Disabled');
        });
      }

      // Trigger our bound click handler to update elements to initial state.
      $checkbox.triggerHandler('click.searchApiUpdate');
    });
  }
};

/**
 * Processes elements with the .dropbutton class on page load.
 */
Drupal.behaviors.searchApiDropButton = {
  attach: function (context, settings) {
    var $dropbuttons = $(context).find('.dropbutton-wrapper').once('dropbutton');
    if ($dropbuttons.length) {
      //$('.dropbutton-toggle', $dropbuttons).click(dropbuttonClickHandler);
      // Initialize all buttons.
      for (var i = 0, il = $dropbuttons.length; i < il; i++) {
        DropButton.dropbuttons.push(new DropButton($dropbuttons[i], settings.dropbutton));
      }
      // Adds the delegated handler that will toggle dropdowns on click.
      $('.dropbutton-toggle', $dropbuttons).click(dropbuttonClickHandler);
    }
  }
};

/**
 * Delegated callback for opening and closing dropbutton secondary actions.
 */
function dropbuttonClickHandler(e) {
  e.preventDefault();
  $(e.target).closest('.dropbutton-wrapper').toggleClass('open');
}

/**
 * A DropButton presents an HTML list as a button with a primary action.
 *
 * All secondary actions beyond the first in the list are presented in a
 * dropdown list accessible through a toggle arrow associated with the button.
 *
 * @param {jQuery} dropbutton
 *   A jQuery element.
 *
 * @param {Object} settings
 *   A list of options including:
 *    - {String} title: The text inside the toggle link element. This text is
 *      hidden from visual UAs.
 */
function DropButton(dropbutton, settings) {
  // Merge defaults with settings.
  var options = $.extend({'title': Drupal.t('List additional actions')}, settings);
  var $dropbutton = $(dropbutton);
  this.$dropbutton = $dropbutton;
  this.$list = $dropbutton.find('.dropbutton');
  // Find actions and mark them.
  this.$actions = this.$list.find('li').addClass('dropbutton-action');

  // Add the special dropdown only if there are hidden actions.
  if (this.$actions.length > 1) {
    // Identify the first element of the collection.
    var $primary = this.$actions.slice(0, 1);
    // Identify the secondary actions.
    var $secondary = this.$actions.slice(1);
    $secondary.addClass('secondary-action');
    // Add toggle link.
    $primary.after(Drupal.theme('dropbuttonToggle', options));
    // Bind mouse events.
    this.$dropbutton
      .addClass('dropbutton-multiple')
      /**
       * Adds a timeout to close the dropdown on mouseleave.
       */
      .bind('mouseleave.dropbutton', $.proxy(this.hoverOut, this))
      /**
       * Clears timeout when mouseout of the dropdown.
       */
      .bind('mouseenter.dropbutton', $.proxy(this.hoverIn, this))
      /**
       * Similar to mouseleave/mouseenter, but for keyboard navigation.
       */
      .bind('focusout.dropbutton', $.proxy(this.focusOut, this))
      .bind('focusin.dropbutton', $.proxy(this.focusIn, this));
  }
}

/**
 * Extend the DropButton constructor.
 */
$.extend(DropButton, {
  /**
   * Store all processed DropButtons.
   *
   * @type {Array}
   */
  dropbuttons: []
});

/**
 * Extend the DropButton prototype.
 */
$.extend(DropButton.prototype, {
  /**
   * Toggle the dropbutton open and closed.
   *
   * @param {Boolean} show
   *   (optional) Force the dropbutton to open by passing true or to close by
   *   passing false.
   */
  toggle: function (show) {
    var isBool = typeof show === 'boolean';
    show = isBool ? show : !this.$dropbutton.hasClass('open');
    this.$dropbutton.toggleClass('open', show);
  },

  hoverIn: function () {
    // Clear any previous timer we were using.
    if (this.timerID) {
      window.clearTimeout(this.timerID);
    }
  },

  hoverOut: function () {
    // Wait half a second before closing.
    this.timerID = window.setTimeout($.proxy(this, 'close'), 500);
  },

  open: function () {
    this.toggle(true);
  },

  close: function () {
    this.toggle(false);
  },

  focusOut: function (e) {
    this.hoverOut.call(this, e);
  },

  focusIn: function (e) {
    this.hoverIn.call(this, e);
  }
});

$.extend(Drupal.theme, {
  /**
   * A toggle is an interactive element often bound to a click handler.
   *
   * @param {Object} options
   *   - {String} title: (optional) The HTML anchor title attribute and
   *     text for the inner span element.
   *
   * @return {String}
   *   A string representing a DOM fragment.
   */
  dropbuttonToggle: function (options) {
    return '<li class="dropbutton-toggle"><button type="button" role="button"><span class="dropbutton-arrow"><span class="visually-hidden">' + options.title + '</span></span></button></li>';
  }
});

// Expose constructor in the public space.
Drupal.DropButton = DropButton;

})(jQuery);
