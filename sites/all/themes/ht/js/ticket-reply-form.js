(function ($) {
    Drupal.behaviors.ticket_form_reply = {
        attach: function(context, settings) {
            $('.comment-form .title-wrapper ul li.cc a').on('click', function(e) {
                $(this).parent().hide();
                $('#field_email_cc_wrapper').removeClass('hidden');
                e.preventDefault();
                return false;
            });
            $('.comment-form .title-wrapper ul li.bcc a').on('click', function(e) {
                $(this).parent().hide();
                $('#field_email_bcc_wrapper').removeClass('hidden');
                e.preventDefault();
                return false;
            })
        }
    };
})(jQuery);