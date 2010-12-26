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


class misc_sql
{
protected $host = NULL;
protected $user = NULL;
protected $password = NULL;
protected $database = NULL;
protected $conn = NULL; // connection
//protected $assoc = 0; // last fetch was assoc
public $res = NULL; // resource
protected $unbuffered = 0; // last query was unbuffered
protected $memcache_expire = 0; // 0 == off
protected $memcache = NULL;


function
sql_stresc ($s)
{
//  return mysql_escape_string ($s); // deprecated
  return mysql_real_escape_string ($s, $this->conn);
}


function
stresc ($s)
{
  return $this->stresc ($s);
}


function
sql_open ($host /* = 'localhost' */ , $user, $password, $database, $memcache_expire = 0)
{
  // DEBUG
//  print_r ($host.$database);
//  exit;

  if ($this->conn)
    {
      mysql_close ($this->conn);
//      $this->conn = NULL;
    }

  $this->host = $host;
  $this->user = $user;
  $this->password = $password;
  $this->database = $database;

  $this->conn = mysql_connect ($host, $user, $password);
  if ($this->conn == FALSE)
    {
      echo mysql_error ();
      return;
    }

  if (mysql_select_db ($database, $this->conn) == FALSE)
    {
      echo mysql_error ();
      return;
    }

  // open memcache too
  if ($memcache_expire > 0)
    {
      $this->memcache = new Memcache;
      if ($this->memcache->connect ('localhost', 11211) != TRUE)
        {
          echo 'memcache: could not connect';
          $this->memcache_expire = 0;
          return;
        }

      $this->memcache_expire = $memcache_expire;
    }
}


function
sql_read ($assoc = 0, $debug = 0)
{
  if ($debug == 1)
    if ($this->res == TRUE)
      {
        echo 'result is TRUE';
        if (!is_resource ($this->res))
          echo 'but no resource';
      }

  if (!is_resource ($this->res)) // either FALSE or just TRUE
    return NULL;  

  $a = array ();
  if ($assoc)
    {
      while ($row = mysql_fetch_array ($this->res, MYSQL_ASSOC)) // MYSQL_ASSOC, MYSQL_NUM, MYSQL_BOTH
        $a[] = $row;
//      $this->assoc = 1;
    }
  else
    {
      while ($row = mysql_fetch_array ($this->res)) // MYSQL_BOTH
        $a[] = $row;
//      $this->assoc = 0;
    }

  if ($debug == 1)
    {
      $p = '<tt>';
      $i_max = sizeof ($a);
      for ($i = 0; $i < $i_max; $i++)
        {
          $p .= implode (' ', $a[$i]);
          $p .= '</tt><br>';
        }

      echo $p;
    }

  return $a;
}


function
sql_getrow ($row, $assoc = 0, $debug = 0)
{
  if ($debug == 1)
    if ($this->res == TRUE)
      echo 'result is TRUE but no resource';

  if (!is_resource ($this->res)) // either FALSE or just TRUE
    return NULL;

  if ($this->unbuffered)
    {
      // DEBUG
      echo '<tt>ERROR: mysql_num_rows() and mysql_data_seek() after mysql_unbuffered_query()<br>';
    }

  $num_rows = mysql_num_rows ($this->res);
  if ($row >= $num_rows || $num_rows == 0)
    return NULL;

  if (mysql_data_seek ($this->res, $row) == FALSE)
    return NULL;

  if ($assoc)
    {
      $a = mysql_fetch_array ($this->res, MYSQL_ASSOC); // MYSQL_ASSOC, MYSQL_NUM, MYSQL_BOTH
//      $this->assoc = 1;
    }
  else
    {
      $a = mysql_fetch_array ($this->res); // MYSQL_BOTH
//      $this->assoc = 0;
    }

  if ($debug == 1)
    {
      $p = '<tt>';
      $p .= implode (' ', $a);
      $p .= '</tt><br>';

      echo $p;
    }

  return $a;
}


function
sql_write ($sql_query_s, $unbuffered = 0, $debug = 0)
{
  if ($debug == 1)
    {
//      $sql_query_s = 'explain '.$sql_query_s;
      echo ''
          .'<br><br><tt>'
          .$sql_query_s
          .'</tt><br><br>'
;
    }

  if (is_resource ($this->res))
    {
      mysql_free_result ($this->res);
//      $this->res = NULL;
    }

  if ($this->memcache_expire > 0)
    {
      // data from the cache
      $p = $this->memcache->get (md5 ($sql_query_s));
      if ($p)
        $this->res = unserialize ($p);
      return 1;
    }

  if ($unbuffered)
    {
      $this->res = mysql_unbuffered_query ($sql_query_s, $this->conn);
      $this->unbuffered = 1;
    }
  else
    {
      $this->res = mysql_query ($sql_query_s, $this->conn);
      $this->unbuffered = 0;
    }

  if (is_resource ($this->res)) // cache resources only, not TRUE's
    if ($this->memcache_expire > 0)
      {
        // store data in the cache
        $this->memcache->set (md5 ($sql_query_s), serialize ($this->res), false, $this->memcache_expire);
      }

  if ($this->res != FALSE) // TRUE or resource (depending on query)
    return 1;
  return 0;
}


function
sql_close ()
{
  if (is_resource ($this->res))          
    {
      mysql_free_result ($this->res);    
//      $this->res = NULL;
    }

  if ($this->conn)
    {
      mysql_close ($this->conn);
//      $this->conn = NULL;
    }
}


function
sql_seek ($row)
{
  if ($this->unbuffered)
    {
      // DEBUG
      echo '<tt>ERROR: mysql_data_seek() after mysql_unbuffered_query()<br>';
    }
  return mysql_data_seek ($this->res, $row);
}


function
sql_get_result ()
{
  // returns FALSE or TRUE or resource
  return $this->res;
}


function
sql_get_rows ()
{
  if ($this->unbuffered)
    {
      // DEBUG
      echo '<tt>ERROR: mysql_num_rows() after mysql_unbuffered_query()<br>';
    }
  return mysql_num_rows ($this->res);
}


function
sql_get_table_rows ($table)
{
  $sql_query_s = 'SELECT COUNT(*) FROM '.$table.' WHERE 1';
  $this->sql_write ($sql_query_s);
  $a = $this->sql_read ();
  return $a[0][0];
}

}


}

?>