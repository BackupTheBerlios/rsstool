<?php


function
misc_crc16 ($s, $crc = 0)
{
  $len = strlen ($s);
  for ($i = 0; $i < $len; $i++)
    {
      $crc ^= ord ($s[$i]);
      for ($j = 0; $j < 8; $j++)
        if (($crc & 1) == 1)
          $crc = ($crc >> 1) ^ 0xa001;
        else
          $crc >>= 1;
    }

  return $crc;
}



?>