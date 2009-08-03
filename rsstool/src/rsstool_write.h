/*
rsstool_write.h - definitions for RSStool

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
#ifndef RSSTOOL_WRITE_H
#define RSSTOOL_WRITE_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif


/*
  rsstool_write_property()  write feed contents as properties
  rsstool_write_template()  write feed contents by replacing tags
                              in a template file
  rsstool_write_template2() write feed contents by replacing tags
                              in a template file, once for each item
  rsstool_write_txt()       write feed contents as txt
  rsstool_write_href()      write feed contents as a list of URLs
  rsstool_write_html()      write feed contents as html
  rsstool_write_ansisql()   write feed contents as ANSI SQL script
  rsstool_write_ansisqlold()  write feed contents as ANSI SQL script
                                (old structure)
  rsstool_write_ansisql_joomla() write feed contents as ANSI SQL script
                                   for import into Joomla! CMS
  rsstool_write_ansisql_dragonfly() write feed contents as ANSI SQL script
                                   for import into Dragonfly CMS
  rsstool_write_bookmarks() write feed contents as bookmarks.html
  rsstool_write_csv()       write feed contents as CSV
  rsstool_write_rss()       write feed contents as RSS feed

  rsstool_write_mysql()     write feed contents to MySQL db
  rsstool_write_odbc()      write feed contents to ODBC compat db
*/
extern int rsstool_write_property (st_rsstool_t *rsstool);
extern int rsstool_write_template (st_rsstool_t *rsstool, const char *template_file);
extern int rsstool_write_template2 (st_rsstool_t *rt, const char *template_file);
extern int rsstool_write_txt (st_rsstool_t *rsstool);
extern int rsstool_write_href (st_rsstool_t *rsstool);
extern int rsstool_write_html (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_092 (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_094 (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_095 (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_joomla (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_dragonfly (st_rsstool_t *rsstool);
extern int rsstool_write_bookmarks (st_rsstool_t *rsstool);
extern int rsstool_write_php (st_rsstool_t *rsstool);
extern int rsstool_write_csv (st_rsstool_t *rsstool, int separator);
extern int rsstool_write_rss (st_rsstool_t *rt, int version);
#ifdef  USE_MYSQL
extern int rsstool_write_mysql (st_rsstool_t *rt);
#endif
#ifdef  USE_ODBC
extern int rsstool_write_odbc (st_rsstool_t *rt);
#endif

#endif  // RSSTOOL_WRITE_H
