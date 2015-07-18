<?php

/**
 * @file
 * Contains FeedsSimplePieParser and related classes.
 */

/**
 * Adapter to present SimplePie_Enclosure as FeedsEnclosure object.
 */
class FeedsSimplePieEnclosure extends FeedsEnclosure {
  protected $simplepie_enclosure;
  private $_serialized_simplepie_enclosure;

  /**
   * Constructor requires SimplePie enclosure object.
   */
  function __construct(SimplePie_Enclosure $enclosure) {
    $this->simplepie_enclosure = $enclosure;
  }

  /**
   * Serialization helper.
   *
   * Handle the simplepie enclosure class seperately ourselves.
   */
  public function __sleep() {
    $this->_serialized_simplepie_enclosure = serialize($this->simplepie_enclosure);
    return array('_serialized_simplepie_enclosure');
  }

  /**
   * Unserialization helper.
   *
   * Ensure that the simplepie class definitions are loaded for the enclosure when unserializing.
   */
   public function __wakeup() {
     feeds_include_simplepie();
     $this->simplepie_enclosure = unserialize($this->_serialized_simplepie_enclosure);
  }

  /**
   * Override parent::getValue().
   */
  public function getValue() {
    return $this->simplepie_enclosure->get_link();
  }

  /**
   * Override parent::getMIMEType().
   */
  public function getMIMEType() {
    return $this->simplepie_enclosure->get_real_type();
  }

}

/**
 * Class definition for Common Syndication Parser.
 *
 * Parses RSS and Atom feeds.
 */
class FeedsSimplePieParser extends FeedsParser {

  /**
   * Implements FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    feeds_include_simplepie();

    // Initialize SimplePie.
    $parser = new SimplePie();
    $parser->set_raw_data($fetcher_result->getRaw());
    $parser->set_stupidly_fast(TRUE);
    $parser->encode_instead_of_strip(FALSE);
    // @todo Is caching effective when we pass in raw data?
    $parser->enable_cache(TRUE);
    $parser->set_cache_location($this->cacheDirectory());
    $parser->init();

    // Construct the standard form of the parsed feed
    $result = new FeedsParserResult();
    $result->title = html_entity_decode(($title = $parser->get_title()) ? $title : $this->createTitle($parser->get_description()));
    $result->description = $parser->get_description();
    $result->link = html_entity_decode($parser->get_link());

    $items_num = $parser->get_item_quantity();
    for ($i = 0; $i < $items_num; $i++) {
      $item = array();
      $simplepie_item = $parser->get_item($i);
      $item['title'] = html_entity_decode(($title = $simplepie_item->get_title()) ? $title : $this->createTitle($simplepie_item->get_content()));
      $item['description'] = $simplepie_item->get_content();
      $item['url'] = html_entity_decode($simplepie_item->get_link());
      // Use UNIX time. If no date is defined, fall back to REQUEST_TIME.
      $item['timestamp'] = $simplepie_item->get_date("U");
      if (empty($item['timestamp'])) {
        $item['timestamp'] = REQUEST_TIME;
      }
      $item['guid'] = $simplepie_item->get_id();
      // Use URL as GUID if there is no GUID.
      if (empty($item['guid'])) {
        $item['guid'] = $item['url'];
      }
      $author = $simplepie_item->get_author();
      $item['author_name'] = isset($author->name) ? html_entity_decode($author->name) : '';
      $item['author_link'] = isset($author->link) ? $author->link : '';
      $item['author_email'] = isset($author->email) ? $author->email : '';
      // Enclosures
      $enclosures = $simplepie_item->get_enclosures();
      if (is_array($enclosures)) {
        foreach ($enclosures as $enclosure) {
          $item['enclosures'][] = new FeedsSimplePieEnclosure($enclosure);
        }
      }
      // Location
      $latitude = $simplepie_item->get_latitude();
      $longitude = $simplepie_item->get_longitude();
      if (!is_null($latitude) && !is_null($longitude)) {
        $item['location_latitude'][] = $latitude;
        $item['location_longitude'][] = $longitude;
      }
      // Extract tags related to the item
      $simplepie_tags = $simplepie_item->get_categories();
      $tags = array();
      $domains = array();
      if (count($simplepie_tags) > 0) {
        foreach ($simplepie_tags as $tag) {
          $tags[] = (string) $tag->term;
          $domain = (string) $tag->get_scheme();
          if (!empty($domain)) {
            if (!isset($domains[$domain])) {
              $domains[$domain] = array();
            }
            $domains[$domain][] = count($tags) - 1;
          }
        }
      }
      $item['domains'] = $domains;
      $item['tags'] = $tags;

      // Allow parsing to be extended.
      $this->parseExtensions($item, $simplepie_item);
      $item['raw'] = $simplepie_item->data;

      $result->items[] = $item;
    }
    // Release parser.
    unset($parser);

    return $result;
  }

  /**
   * Allow extension of FeedsSimplePie item parsing.
   */
  protected function parseExtensions(&$item, $simplepie_item) {}

  /**
   * Return mapping sources.
   */
  public function getMappingSources() {
    return array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('Title of the feed item.'),
      ),
      'description' => array(
        'name' => t('Description'),
        'description' => t('Description of the feed item.'),
      ),
      'author_name' => array(
        'name' => t('Author name'),
        'description' => t('Name of the feed item\'s author.'),
      ),
      'author_link' => array(
        'name' => t('Author link'),
        'description' => t('Link to the feed item\'s author.'),
      ),
      'author_email' => array(
        'name' => t('Author email'),
        'description' => t('Email address of the feed item\'s author.'),
      ),
      'timestamp' => array(
        'name' => t('Published date'),
        'description' => t('Published date as UNIX time GMT of the feed item.'),
      ),
      'url' => array(
        'name' => t('Item URL (link)'),
        'description' => t('URL of the feed item.'),
      ),
      'guid' => array(
        'name' => t('Item GUID'),
        'description' => t('Global Unique Identifier of the feed item.'),
      ),
      'tags' => array(
        'name' => t('Categories'),
        'description' => t('An array of categories that have been assigned to the feed item.'),
      ),
      'domains' => array(
        'name' => t('Category domains'),
        'description' => t('Domains of the categories.'),
      ),
      'location_latitude' => array(
        'name' => t('Latitudes'),
        'description' => t('An array of latitudes assigned to the feed item.'),
      ),
      'location_longitude' => array(
        'name' => t('Longitudes'),
        'description' => t('An array of longitudes assigned to the feed item.'),
      ),
      'enclosures' => array(
        'name' => t('Enclosures'),
        'description' => t('An array of enclosures attached to the feed item.'),
      ),
    ) + parent::getMappingSources();
  }

  /**
   * Returns cache directory. Creates it if it doesn't exist.
   */
  protected function cacheDirectory() {
    $directory = 'public://simplepie';
    file_prepare_directory($dir, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
    return $directory;
  }

  /**
   * Generate a title from a random text.
   */
  protected function createTitle($text = FALSE) {
    // Explode to words and use the first 3 words.
    $words = preg_split("/[\s,]+/", $text);
    $words = array_slice($words, 0, 3);
    return implode(' ', $words);
  }

}
