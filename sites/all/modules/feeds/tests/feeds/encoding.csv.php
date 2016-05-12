<?php
/**
 * @file
 * Result of encoding_{code}.csv file parsed by ParserCSV.inc
 */

// JSON is used here because it supports unicode literals. PHP does not.
$json = <<<EOT
[
  [
    "id",
    "text"
  ],
  [
    "1",
    "\u672c\u65e5\u306f\u3044\u3044\u5929\u6c17"
  ],
  [
    "2",
    "\uff71\uff72\uff73\uff74\uff75"
  ],
  [
    "3",
    "\u30c6\u30b9\u30c8"
  ],
  [
    "4",
    "\u2605"
  ]
]
EOT;

$control_result = json_decode($json);
