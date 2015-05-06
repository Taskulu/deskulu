1. Installing @font-your-face:
==============================

- Place the extracted module in sites/all/modules/fontyourface
- Go to Administration » Modules and enable @font-your-face and one or more of the submodules.
- Go to Administration » Configuration » User interface » @font-your-face settings and import the fonts.

2a. Use @font-your-face via the interface:
==========================================
- Go to Administration » Appearance » @font-your-face (admin/appearance/fontyourface/browse) 
  to enable some fonts. 
- Click the 'enable font' for each fonr you want to use.
- You can add CSS selectors for each enabled font.

2b. Using @font-your-face via your theme .info file:
====================================================
- Open the .info file of your theme (eg bartik.info if your theme is Bartik)
- Add fonts like this:

fonts[google_fonts_api][] = "Contrail One&subset=latin#regular"
fonts[fontdeck][] = "Tanger+Serif+Medium+Ultra+Light"

- The use your stylesheet to enable fonts. Example: 
  h1#site-title { font-family: "1942 Report", serif; }

Known issues:
=============
- Note that Internet Explorer has a limit of 32 CSS files, so using @font-your-face on CSS-heavy sites may require 
  turning on CSS aggregation under Administer » Configuration » Development » Performance (admin/config/development/performance).
- KERNEST servers are sometimes unreliable, so you may want to download KERNEST fonts and use the Local Fonts module to 
  load them from your server instead. KERNEST provides paid fonts that are not available for use in the API, so those can 
  only be used with the Local Fonts module.
- See http://drupal.org/project/fontyourface#support for support options on any issues not mentioned here.