<?php
/*
sql.php - simplified wrappers for SQL access

Copyright (c) 2006 NoisyB


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
if (!defined ('MISC_SQL_PHP'))
{
define ('MISC_SQL_PHP', 1);
//require_once ('misc/class.stemmer.inc');


class misc_sql
{
var $host = NULL;
var $user = NULL;
var $password = NULL;
var $database = NULL;
var $conn = NULL;
var $res = FALSE;

var $use_memcache = 0;
var $memcache = NULL;
//var $row_pos = -1;


function
sql_stresc ($s)
{
//  return mysql_escape_string ($s); // deprecated
  return mysql_real_escape_string ($s, $this->conn);
}


function
sql_open ($host, $user, $password, $database, $use_memcache = 0)
{
  if (!is_null ($this->conn))
    mysql_close ($this->conn);

  $this->host = $host;
  $this->user = $user;
  $this->password = $password;
  $this->database = $database;

  $this->conn = mysql_connect ($host, $user, $password) or die (mysql_error ());
  mysql_select_db ($database, $this->conn);

  if ($use_memcache == 1)
    {
      $this->memcache = new Memcache;
      $this->memcache->connect ('localhost', 11211) or die ("memcache: could not connect");
      $this->use_memcache = 1;
    }
}


function
sql_read ($debug)
{
  $a = Array ();

  if (!isset ($this->res))
    return NULL;

  if (is_null ($this->res))
    return NULL;

  if ($this->res == FALSE)
    return NULL;

//  while ($row = mysql_fetch_array ($this->res, MYSQL_BOTH))
  while ($row = mysql_fetch_array ($this->res))
    $a[] = $row;

  if ($debug == 1)
    {
      $p = '<tt>';
      $i_max = sizeof ($a);
      for ($i = 0; $i < $i_max; $i++)
        {
          $j_max = sizeof ($a[$i]);
          for ($j = 0; $j < $j_max; $j++)
            $p .= $a[$i][$j]
                 .' ';

          $p .= '</tt><br>';
        }

      echo $p;
    }

  return $a;
}


function
sql_getrow ($row, $debug)
{
  if (is_null ($this->res))
    return NULL;

  if ($this->res == FALSE)
    return NULL;

  if ($row >= mysql_num_rows ($this->res))
    return NULL;

  if (mysql_data_seek ($this->res, $row) == false)
    return NULL;

//  $this->row_pos = $row;

  $a = mysql_fetch_row ($this->res);

  if ($debug == 1)
    {
      $p = '<tt>';
      $i_max = sizeof ($a);
      for ($i = 0; $i < $i_max; $i++)
        $p .= $a[$i]
           .' ';

      $p .= '</tt><br>';

      echo $p;
    }

  return $a;
}


function
sql_write ($sql_query_s, $debug)
{
  if ($debug == 1)
    echo '<br><br><tt>'
        .$sql_query_s
        .'</tt><br><br>';

  if ($this->res != FALSE)
    {
      mysql_free_result ($this->res);
      $this->res = NULL;
    }

  if ($this->use_memcache == 1)
    {
      // data from the cache
      $this->res = unserialize ($this->memcache->get (md5 ($sql_query_s)));
    }

  if ($this->res == NULL || $this->res == FALSE)
    {
      $this->res = mysql_query ($sql_query_s);

      if ($this->use_memcache == 1)
        {
          // store data in the cache (data will expire in 60 seconds)
          $this->memcache->set (md5 ($sql_query_s), serialize ($this->res), false, 60) 
            or die ("memcache: failed to save data at the server");
        }
    }

  if ($this->res != FALSE)
    return 1;
  return 0;
}


function
sql_close ()
{
//  if (!is_null ($this->res))
  if ($this->res != FALSE)
    { 
      mysql_free_result ($this->res);
      $this->res = FALSE;
    }

  if (!is_null ($this->conn))
    {
      mysql_close ($this->conn);
      $this->conn = NULL;
    }

//  $this->row_pos = -1;
}


function
sql_seek ($row)
{
  return mysql_data_seek ($this->res, $row);
}


function
sql_get_result ()
{
  return $this->res;
}


function
sql_get_rows ()
{
  return mysql_num_rows ($this->res);
}


function
sql_prep_query ($search_s, $word_stems)
{
  if ($word_stems)
    $stemmer = new stemmer;

  $s = trim ($search_s);
  $s = str_replace ('  ', ' ', $s); // remove irrelevant white spaces
  $s = str_replace ('  ', ' ', $s);
  $s = str_replace ('  ', ' ', $s);
  $s = str_replace ('  ', ' ', $s);
  $a = explode (' ', $s);
//  $a = Array ();

/*
  $quote = 0;
//  $space = 0;
  $pos = 0;
  $j = 0;
  $i_max = strlen ($s);
  for ($i = 0; $i < $i_max; $i++)
    switch ($s[$i])
      {
         case "\"":
           $quote != $quote;
           if (!$quote)
             $pos++;
           break;

         case ' ':
           if (!$quote)
             {
               $pos++;
               $j = 0;
             }
           else
             $a[$pos][$j++] = ' ';
           break;

         default:
           $a[$pos][$j++] = $s[$i];
      }

print_r ($a);
*/

  $s = '';
  $i_max = sizeof ($a);
  for ($i = 0; $i < $i_max; $i++)
    {
      if ($word_stems)
        $a[$i] = $this->sql_stresc ($stemmer->stem (strtolower ($a[$i])));
      else
        $a[$i] = $this->sql_stresc ($a[$i]);

      $s .= (!$i ? '' : ' ')
           .($a[$i][0] != '-' ? '+' : '')
           .$a[$i];
    }

  return $s;
}


}
}

?>