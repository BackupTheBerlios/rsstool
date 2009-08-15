<?php
/*
index.php - rsstool PHP example

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
include ("config.php");
include ("sql.php");

  $db = sql_open ($host, $user, $password, $database);

  $p = "SELECT `rsstool_url`, `rsstool_title`, `rsstool_desc`"
      ." FROM `rsstool_table`"
      ." ORDER BY `rsstool_dl_date` DESC"
      ." LIMIT 0,100";

  $rsstool_table = sql_read ($db, $p, 0);

  $p = "";

  for ($i = 0; $rsstool_table[$i]; $i++)
    $p .= "<a href=\""
         .$rsstool_table[$i][0]
         ."\">"
         .$rsstool_table[$i][1]
         ."</a><br><br>"
         .$rsstool_table[$i][2]
         ."<br><br><hr><br>";

  echo $p;
?>
