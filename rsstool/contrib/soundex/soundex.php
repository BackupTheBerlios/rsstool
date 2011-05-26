#!/usr/local/bin/php -q
<?php


function
soundex2 ($s)
{
  $k = ' 123 12  22455 12623 1 2 2';
  $u = strtoupper ($s);
  $l = strlen ($s);
  $d = $u[0];

  $p = NULL;
  for ($i = 0; $i < $l; $i++)
    {
      $s = trim ($k[ord ($u[$i]) - 65]);
      if ($s != $p)
        {
          if ($i > 0) $d .= $s;
          $p = $s;
        }
    }

  return substr ($d.'000', 0, 4);
}


function
strrand2 ($min = 3, $max = 10)
{
  $s = 'abcdefghijklmnopqrstuvwxyz';
  $s_len = strlen ($s);
  $l = mt_rand ($min, $max);

  $p = '';
  for ($i = 0; $i < $l; $i++)
    $p .= $s[mt_rand (0, $s_len)];

  return $p;
}


$p = strrand2 ();
echo $p."\n";
echo soundex ($p)."\n";
echo soundex2 ($p)."\n";


?>