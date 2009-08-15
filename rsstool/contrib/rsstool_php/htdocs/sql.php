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


function
sql_open ($host, $user, $pass, $db_name)
{
  $conn = mysql_connect ($host, $user, $pass) or die (mysql_error ());
  mysql_select_db ($db_name, $conn);

  return $conn;
}


function
sql_stresc ($conn, $s)
{
//  return mysql_escape_string ($s); // deprecated
  return mysql_real_escape_string ($s, $conn);
}


function
sql_read ($conn, $sql_statement, $debug)
{
  if ($debug == 1)
    echo "<br><br>"
        .$sql_statement
        ."<br><br>";

  if ($res = mysql_query ($sql_statement))
//  if ($res = mysql_real_query ($conn, $sql_statement, strlen ($sql_statement)))
    while ($row = mysql_fetch_array ($res))
      $all[] = $row;

  if ($debug == 1)
    {
      for ($i = 0; $all[$i]; $i++)
        {
          for ($j = 0; $all[$i][$j]; $j++)
            $p .= $all[$i][$j]
                 ." ";

          $p .= "<br>";
        }

      echo $p;
    }

  return $all;
}


function
sql_write ($conn, $sql_statement, $debug)
{
  if ($debug == 1)
    echo "<br><br>"
        .$sql_statement
        ."<br><br>";

  $res = mysql_query ($sql_statement);

  if ($res != FALSE)
    return 1;
  return 0;
}

?>
