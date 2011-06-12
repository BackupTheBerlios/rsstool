#!/usr/bin/php -q
<?php
/*
rsstool2sql.php - script to convert rsstool proprietary XML into SQL

Copyright (c) 2011 NoisyB


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
//phpinfo ();
//error_reporting(E_ALL | E_STRICT);
// rsstool path and options
$rsstool_path = '/usr/local/bin/rsstool';
//$rsstool_path = '../../src/rsstool';
$rsstool_opts = '--shtml --sbin --hack-google';


function
misc_exec ($cmdline, $debug = 0)
{
  if ($debug)
    echo 'cmdline: '.$cmdline."\n"
        .'escaped: '.escapeshellcmd ($cmdline).' (not used)'."\n"
;
  if ($debug == 2)
    return '';
 
  $a = array();

  exec ($cmdline, $a, $res);

  $p = '';
  if ($debug)
    $p = $res."\n";
 
  $i_max = count ($a);
  for ($i = 0; $i < $i_max; $i++)
    $p .= $a[$i]."\n";

  return $p;
}


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


function
misc_crc24 ($s, $crc = 0xb704ce)
{
  for ($n = 0; $n < strlen ($s); $n++)
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


function
misc_sql_stresc ($s, $db_conn = NULL)
{
  if ($db_conn)
    return mysql_real_escape_string ($s, $db_conn);
  if (function_exists ('mysql_escape_string'))
    return mysql_escape_string ($s); // deprecated
  echo 'WARNING: neither mysql_real_escape_string() or mysql_escape_string() could be found/used'."\n"
      .'         making this script vulnerable to SQL injection attacks'."\n";
  return $s;
}


function
rsstool_write_ansisql ($xml, $tv2_category, $db_conn = NULL)
{
  $sql_update = 0;
  $tv2_engine = 0;
  $p = '';

  $p .= '-- -----------------------------------------------------------'."\n"
       .'-- RSStool - read, parse, merge and write RSS and Atom feeds'."\n"
       .'-- -----------------------------------------------------------'."\n"
       ."\n"
       .'-- DROP TABLE IF EXISTS rsstool_table;'."\n"
       .'-- CREATE TABLE rsstool_table ('."\n"
//       .'--   rsstool_url_md5 varchar(32) NOT NULL default \'\','."\n"
       .'--   rsstool_url_crc32 int(10) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_site text NOT NULL,'."\n"
       .'--   rsstool_dl_url text NOT NULL,'."\n"
//       .'--   rsstool_dl_url_md5 varchar(32) NOT NULL default \'\','."\n"
       .'--   rsstool_dl_url_crc32 int(10) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_title text NOT NULL,'."\n"
//       .'--   rsstool_title_md5 varchar(32) NOT NULL default \'\','."\n"
       .'--   rsstool_title_crc32 int(10) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_desc text NOT NULL,'."\n"
       .'--   rsstool_date bigint(20) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_dl_date bigint(20) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_keywords text NOT NULL,'."\n"
       .'--   rsstool_media_duration bigint(20) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_image text NOT NULL,'."\n"
       .'--   rsstool_event_start bigint(20) unsigned NOT NULL default \'0\','."\n"
       .'--   rsstool_event_end bigint(20) unsigned NOT NULL default \'0\','."\n"
       .'--   UNIQUE KEY rsstool_url_crc32 (rsstool_url_crc32),'."\n"
//       .'--   UNIQUE KEY rsstool_url_md5 (rsstool_url_md5),'."\n"
       .'--   UNIQUE KEY rsstool_title_crc32 (rsstool_title_crc32),'."\n"
//       .'--   UNIQUE KEY rsstool_title_md5 (rsstool_title_md5),'."\n"
       .'--   FULLTEXT KEY rsstool_title (rsstool_title),'."\n"
       .'--   FULLTEXT KEY rsstool_desc (rsstool_desc)'."\n"
       .'-- ) TYPE=MyISAM;'."\n"
       ."\n";

  $p .= ''
       .'-- DROP TABLE IF EXISTS rsstool_table;'."\n"
       .'-- CREATE TABLE IF NOT EXISTS keyword_table ('."\n"
//       .'--   rsstool_url_md5 varchar(32) NOT NULL,'."\n"
       .'--   rsstool_url_crc32 int(10) unsigned NOT NULL,'."\n"
       .'--   rsstool_keyword_crc32 int(10) unsigned NOT NULL,'."\n"
       .'--   rsstool_keyword_crc24 int(10) unsigned NOT NULL,'."\n"
       .'--   rsstool_keyword_crc16 smallint(5) unsigned NOT NULL,'."\n"
       .'--   PRIMARY KEY (rsstool_url_crc32,rsstool_keyword_crc16),'."\n"
       .'--   KEY rsstool_keyword_16bit (rsstool_keyword_crc16)'."\n"
       .'-- ) ENGINE=MyISAM DEFAULT CHARSET=utf8;'."\n"
       ."\n";

  $items = count ($xml->item);
  for ($i = 0; $i < $items; $i++)
    {
      // rsstool_table
      $p .= 'INSERT IGNORE INTO rsstool_table ('
           .' rsstool_dl_url,'
//           .' rsstool_dl_url_md5,'
           .' rsstool_dl_url_crc32,'
           .' rsstool_dl_date,'
           .' rsstool_site,'
           .' rsstool_url,'
//           .' rsstool_url_md5,'
           .' rsstool_url_crc32,'
           .' rsstool_date,'
           .' rsstool_title,'
//           .' rsstool_title_md5,'
           .' rsstool_title_crc32,'
           .' rsstool_desc,'
//           .' rsstool_media_keywords,'
           .' rsstool_keywords,'
           .' rsstool_media_duration,'
           .' rsstool_image,'
           .' rsstool_event_start,'
           .' rsstool_event_end';

      // HACK: tv2 category
      if ($tv2_engine == 1)
        $p .= ', tv2_category, tv2_moved';

      $p .= ' ) VALUES ('
           .' \''.misc_sql_stresc ($xml->item[$i]->dl_url, $db_conn).'\','
//           .' \''.$xml->item[$i]->dl_url_md5.'\','
           .' \''.$xml->item[$i]->dl_url_crc32.'\','
           .' \''.$xml->item[$i]->dl_date.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->site, $db_conn).'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->url, $db_conn).'\','
//           .' \''.$xml->item[$i]->url_md5.'\','
           .' \''.$xml->item[$i]->url_crc32.'\','
           .' \''.$xml->item[$i]->date.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->title, $db_conn).'\','
//           .' \''.$xml->item[$i]->title_md5.'\','
           .' \''.$xml->item[$i]->title_crc32.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->desc, $db_conn).'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->media_keywords, $db_conn).'\','
           .' \''.($xml->item[$i]->media_duration * 1).'\','
           .' \''.$xml->item[$i]->image.'\','  
           .' \''.($xml->item[$i]->event_start * 1).'\','
           .' \''.($xml->item[$i]->event_end * 1).'\'';

      // HACK: tv2 category
      if ($tv2_engine == 1)
        $p .= ', \''.$tv2_category.'\', \''.$tv2_category.'\'';

      $p .= ' );'."\n";

      // UPDATE rsstool_table
      $p .= '-- just update if row exists'."\n";
      if ($sql_update == 0)
        $p .= '-- ';
      $p .= 'UPDATE rsstool_table SET '
           .' rsstool_title = \''.misc_sql_stresc ($xml->item[$i]->title, $db_conn).'\','
//           .' rsstool_title_md5 = \''.$xml->item[$i]->title_md5.'\','
           .' rsstool_title_crc32 = \''.$xml->item[$i]->title_crc32.'\','
           .' rsstool_desc = \''.misc_sql_stresc ($xml->item[$i]->desc, $db_conn).'\''
           .' WHERE rsstool_url_crc32 = '.$xml->item[$i]->url_crc32
           .';'
           ."\n";

      // keyword_table
      $a = explode (' ', $xml->item[$i]->media_keywords);
      for ($j = 0; isset ($a[$j]); $j++)
        if (trim ($a[$j]) != '')
          $p .= 'INSERT IGNORE INTO keyword_table ('
//               .' rsstool_url_md5,'
               .' rsstool_url_crc32,'
               .' rsstool_keyword_crc32,'
               .' rsstool_keyword_crc24,'
               .' rsstool_keyword_crc16'
               .' ) VALUES ('
//               .' \''.$xml->item[$i]->url_md5.'\','
               .' '.$xml->item[$i]->url_crc32.','
               .' '.sprintf ("%u", crc32 ($a[$j])).','
               .' '.sprintf ("%u", misc_crc24 ($a[$j])).','
               .' '.misc_crc16 ($a[$j])
               .' );'
               ."\n";
    }

  return $p;
}


function
rsstool_write_csv ($xml)
{
  $p = '';

  for ($i = 0; isset ($xml->item[$i]); $i++)
    $p .= ''
         .$xml->item[$i]->dl_url.', '
         .$xml->item[$i]->dl_date.', '
         .$xml->item[$i]->site.', '
         .$xml->item[$i]->url.', '
         .$xml->item[$i]->date.', '
         .$xml->item[$i]->title.', '
         .strip_tags ($xml->item[$i]->desc) // .', '
//         .$xml->item[$i]->media_duration
         ."\n";

  $p = str_replace ("\n", '', $p);

  return $p;
}



// main ()
if ($argc < 2)
  {
    echo 'USAGE: rsstool2sql.php URL|FILE'."\n\n";            
    exit;
  }

$debug = 0;


$tmp = tempnam (sys_get_temp_dir (), 'rsstool2sql_');

// DEBUG
//  echo $tmp."\n";

$p = $rsstool_path.' '.$rsstool_opts.' --xml "'.$argv[1].'" -o "'.$tmp.'"';
// DEBUG
echo $p."\n";
echo misc_exec ($p, $debug);

$xml = simplexml_load_file ($tmp);
unlink ($tmp);

// DEBUG
//  print_r ($xml);
//  exit;

// output
$p = rsstool_write_ansisql ($xml, NULL);
$f = 'rsstool2sql.sql';
//$p = rsstool_write_csv ($xml);
//$f = 'rsstool2csv.txt';
$fh = fopen ($f, 'w');
if ($fh)
  {
    fwrite ($fh, $p);
    fclose ($fh);
    echo $f.' written'."\n";
  }
else
  echo 'ERROR: could not write '.$f."\n";


exit;



?>?>