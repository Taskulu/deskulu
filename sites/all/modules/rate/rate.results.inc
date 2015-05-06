<?php

/**
 * @file
 * Function to generate rate results page.
 */

/**
 * Generate the rate results page.
 *
 * @param object $node
 * @return string
 */
function rate_results_page($node) {
  $output = '';

  $widgets = rate_get_active_widgets('node', $node->type);
  foreach ($widgets as $widget) {
    $output .= '<h2>' . check_plain($widget->title) . '</h2>';
    $votingapi_results = votingapi_select_results(array(
      'entity_type' => 'node',
      'entity_id' => $node->nid,
      'tag' => $widget->tag,
      'value_type' => $widget->value_type,
    ));
    $count = 0;
    $average = 0;
    $sum = 0;
    $votingapi_functions = array('count', 'average', 'sum');

    foreach ($votingapi_results as $result) {
      if (!in_array($result['function'], $votingapi_functions)) { //custom options, need to tally these together to get a total count
        $count += $result['value'];
      }
      else {
        ${$result['function']} = $result['value'];
      }
    }

    $output .= '<p>';
    $output .= t('Vote count: @count', array('@count' => $count)) . ' ';
    if ($widget->value_type != 'option') {
      $output .= t('Average: @avg', array('@avg' => round($average, 2))) . ' ';
    }
    if ($widget->value_type == 'points') {
      $output .= t('Points: @points', array('@points' => $sum));
    }
    $output .= '</p>';

    // Display a table with vote counts per button.
    if (count($widget->options) > 1) {
      $rows = array();
      foreach ($widget->options as $option) {
        $query = db_select('votingapi_vote', 'vv')
          ->condition('vv.entity_type', 'node')
          ->condition('vv.entity_id', $node->nid)
          ->condition('vv.value_type', $widget->value_type)
          ->condition('vv.value', $option[0])
          ->condition('vv.tag', $widget->tag)
          ->condition('vv.timestamp', 0, '>');
        $field = $query->addExpression('COUNT(*)', 'vote_count');
        $total_count = $query->execute()->fetchField();
        
        $title = $widget->translate ? t($option[1]) : $option[1];
        $rows[] = array($title, $total_count);
      }
      $header = array(t('Button'), t('Votes'));
      $output .= '<h4>' . t('Total votes per button') . '</h4>';
      $output .= theme('table', array('header' => $header, 'rows' => $rows));
    }

    // Display a graph if the chart module is enabled.
    if (module_exists('chart')) {
      $chart = array(
        '#chart_id' => 'rate_node_' . $node->nid . '_' . $widget->name,
        '#type' => CHART_TYPE_BAR_V_GROUPED,
        '#size' => chart_size(500, 200),
        '#adjust_resolution' => FALSE,
        '#data' => array(),
        '#mixed_axis_labels' => array(
          CHART_AXIS_Y_LEFT => array(),
          CHART_AXIS_X_BOTTOM => array(),
        ),
      );
      $sql = 'SELECT FLOOR((:day - vv.timestamp) / 86400) as days_ago, vv.value, COUNT(*) as count
      FROM {votingapi_vote} vv
      WHERE vv.entity_type = :entitytype
      AND vv.entity_id = :nid
      AND vv.value_type = :valuetype
      AND vv.tag = :tag
      AND vv.timestamp > :mintimestamp
      GROUP BY 1, 2';
      $end_of_day = mktime(23, 59, 59, (int) date('m'), (int) date('d'), (int) date('Y'));
      $min_timestamp = $end_of_day - (86400 * 30);
      $args = array(
        ':day' => $end_of_day,
        ':entitytype' => 'node',
        ':nid' => $node->nid,
        ':valuetype' => $widget->value_type,
        ':tag' => $widget->tag,
        ':mintimestamp' => $min_timestamp
      );
      $res = db_query($sql, $args);
      $results = array();
      $oldest = 7; // Display at least 7 days.
      while ($rec = $res->fetchAssoc()) {
        $oldest = max($oldest, $rec['days_ago']);
        $results[$rec['days_ago'] . ':' . $rec['value']] = $rec['count'];
      }
      if ($results) {
        $max_count = 0;
        foreach ($widget->options as $option) {
          $value = $option[0];
          $title = $widget->translate ? t($option[1]) : $option[1];
          $chart['#data'][$title] = array();
          for ($i = $oldest; $i >= 0; --$i) {
            $count = isset($results["$i:$value"]) ? $results["$i:$value"] : 0;
            $count = max(0.1, $count);
            $chart['#data'][$title][] = (int) $count;
            $max_count = max($max_count, $count);
          }
        }
        // Workaround the Chart API behaviour that lines with 0's are hidden. Apply a minimum of 1.
        foreach ($chart['#data'] as $title => $counts) {
          for ($i = 0; $i < count($counts); ++$i) {
            $chart['#data'][$title][$i] = max(1, round(($counts[$i] / $max_count) * 100));
          }
        }
        for ($i = $oldest; $i >= 0; --$i) {
          $label = date('j', time() - ($i * 86400));
          $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label($label);
        }
        // Define colors for the bars. The first color is green, second is red. This is for
        // thumbs up / down voting (the first is up, which needs to be green).
        $colors = array(
          '01df01', 'ff0000', '0101df', 'ffbc00', '01dfd6',
          'b404ae', '8a4b08', '38610b', '610b38', '610b5e',
        );
        $x = 0;
        $chart['#bar_size'] = chart_bar_size(ceil(20 / count($chart['#data'])), 0);
        foreach ($chart['#data'] as $title => $data) {
          $chart['#legends'][] = $title;
          $chart['#data_colors'][] = $colors[$x % count($colors)];
          ++$x;
        }
        $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, $max_count);
        $chart['#title'] = t('Votes in the last @num days', array('@num' => $oldest));
        $output .= theme('chart', array('chart' => $chart));
        $output .= '<p>' . t('Showed results are votes per day per button. The horizontal axis shows the days of the month. The vertical axis shows the number of votes.') . '</p>';
      }
    }
  }

  return $output;
}