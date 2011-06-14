<?php

function
misc_crc24 ($s, $crc = 0xb704ce)
{
  $len = strlen ($s);
  for ($n = 0; $n < $len; $n++)
    {
      $crc ^= (ord($s[$n]) & 0xff) << 0x10;
      for ($i = 0; $i < 8; $i++)
        {
          $crc <<= 1;
          if ($crc & 0x1000000) $crc ^= 0x1864cfb;
        }
    }

  return ((($crc >> 0x10) & 0xff) << 16) | ((($crc >> 0x8) & 0xff) << 8) | ($crc & 0xff);
}



?>