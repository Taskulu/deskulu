<?php

/**
 * Check if the given IP is a local IP-address.
 * 
 * @param string $ip
 * @return bool
 */
function rate_bots_is_local($ip) {
  if (preg_match('/^([012]?[0-9]{2})\\./', $ip, $match)) {
    switch ($match[1]) {
      case 10:
      case 127:
      case 172:
      case 192:
        return TRUE;
    }
  }
  return FALSE;
}

/**
 * Check if the current user is blocked.
 * 
 * This function will first check if the user is already known to be a bot.
 * If not, it will check if we have valid reasons to assume the user is a bot.
 * 
 * @return bool
 */
function rate_bots_is_blocked() {
  $ip = ip_address();
  $agent = $_SERVER['HTTP_USER_AGENT'];
  
  if (rate_bots_is_local($ip)) {
    // The IP-address is a local IP-address. This is probably because of
    // misconfigured proxy servers. Do only the user agent check.
    return rate_bots_check_agent($agent);
  }
  
  if (rate_bots_check_ip($ip)) {
    return TRUE;
  }
  
  if (rate_bots_check_agent($agent)) {
    // Identified as a bot by its user agent. Register this bot by IP-address
    // as well, in case this bots uses multiple agent strings.
    rate_bots_register_bot($ip);
    return TRUE;
  }
  
  $threshold = variable_get(RATE_VAR_BOT_MINUTE_THRESHOLD, 25);
  if ($threshold && (rate_bots_check_threshold($ip, 60) > $threshold)) {
    rate_bots_register_bot($ip);
    return TRUE;
  }
  
  $threshold = variable_get(RATE_VAR_BOT_HOUR_THRESHOLD, 250);
  // Always count, even if threshold is disabled. This is to determine if we
  // can skip the BotScout check.
  $count = rate_bots_check_threshold($ip, 3600);
  if ($threshold && ($count > $threshold)) {
    rate_bots_register_bot($ip);
    return TRUE;
  }
  
  if (!$count && rate_bots_check_botscout($ip)) {
    rate_bots_register_bot($ip);
    return TRUE;
  }
  
  return FALSE;
}

/**
 * Register new bot.
 * 
 * @param string $ip
 */
function rate_bots_register_bot($ip) {
  db_insert('rate_bot_ip')->fields(array('ip' => $ip))->execute();
}

/**
 * Check if the IP-address exists in the local bot database.
 * 
 * @param string $ip
 * @return bool
 */
function rate_bots_check_ip($ip) {
  return (bool) db_select('rate_bot_ip', 'rbi')
    ->fields('rbi', array('id'))
    ->condition('rbi.ip', $ip)
    ->range(0, 1)
    ->execute()
    ->fetchField();
}

/**
 * Check if the given user agent matches the local bot database.
 * 
 * @param string $agent
 * @return bool
 */
function rate_bots_check_agent($agent) {
  $sql = 'SELECT 1 FROM {rate_bot_agent} WHERE :agent LIKE pattern LIMIT 1';
  return (bool) db_query($sql, array(':agent' => $agent))->fetchField();
}

/**
 * Check the number of votes between now and $interval seconds ago.
 * 
 * @param string $ip
 * @param int $interval
 * @return int
 */
function rate_bots_check_threshold($ip, $interval) {
  $sql = 'SELECT COUNT(*) FROM {votingapi_vote} WHERE vote_source = :ip AND timestamp > :time';
  return db_query($sql, array(':ip' => $ip, ':time' => REQUEST_TIME - $interval))->fetchField();
}

/**
 * Check if the IP is in the BotScout database.
 * 
 * @param string $ip
 * @return bool
 */
function rate_bots_check_botscout($ip) {
  $key = variable_get(RATE_VAR_BOT_BOTSCOUT_KEY, '');
  if ($key) {
    $url = "http://botscout.com/test/?ip=$ip&key=$key";
    $data = drupal_http_request($url, array('timeout' => 2));
    if ($data->code == 200) {
      if ($data->data{0} == 'Y') {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Delete votes from bots.
 */
function rate_bots_delete_votes() {
  $limit = variable_get(RATE_VAR_CRON_DELETE_LIMIT, 25);
  $queue = DrupalQueue::get('rate_delete_votes');
  while ($item = $queue->claimItem()) {
    $ip = $item->data;
    $votes = db_select('votingapi_vote', 'v')
      ->fields('v', array('vote_id', 'entity_type', 'entity_id'))
      ->condition('v.vote_source', $ip)
      ->range(0, $limit)
      ->execute()
      ->fetchAll();
    foreach ($votes as $vote) {
      db_delete('votingapi_vote')
        ->condition('vote_id', $vote->vote_id)
        ->execute();
      votingapi_recalculate_results($vote->entity_type, $vote->entity_id, TRUE);
    }
    
    $queue->deleteItem($item);
    
    if (count($votes) == $limit) {
      // There may be more votes from this IP which needs to be deleted.
      $queue->createItem($ip);
    }
    
    $limit -= count($votes) + 1;
    if ($limit <= 0) {
      break;
    }
  }
}
