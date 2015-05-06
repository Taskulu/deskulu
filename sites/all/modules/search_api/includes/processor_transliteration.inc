<?php

/**
 * @file
 * Contains SearchApiTransliteration.
 */

/**
 * Processor for making searches insensitive to accents and other non-ASCII characters.
 */
class SearchApiTransliteration extends SearchApiAbstractProcessor {

  protected function process(&$value) {
    // We don't touch integers, NULL values or the like.
    if (is_string($value)) {
      $value = transliteration_get($value, '', language_default('language'));
    }
  }

}
