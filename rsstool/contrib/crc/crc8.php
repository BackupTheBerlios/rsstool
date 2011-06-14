<?php

function
misc_crc8 ($s, $crc = 0)    
{
  $polynomial = (0x1070 << 3);

  $len = strlen ($s);
  for ($i = 0; $i < $len; $i++)
    {
      $crc ^= ord ($s[$i]);
      $crc <<= 8; 
      for ($j = 0; $j < 8; $j++)  
        {
          if (($crc & 0x8000) != 0)    
            $crc ^= $polynomial; 
          $crc <<= 1; 
        }
      $crc = ($crc >> 8) & 0xff;       
    }
  return $crc;
}

echo sprintf ("%x", misc_crc8 ("test"));

?>
