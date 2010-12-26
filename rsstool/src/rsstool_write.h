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
  write feed contents...

  rsstool_write_property()  as properties
  rsstool_write_template()  by replacing tags in a template file
  rsstool_write_template2() by replacing tags in a template file, once for each item
  rsstool_write_txt()       as txt
  rsstool_write_href()      as list of URLs
  rsstool_write_html()      as html
  rsstool_write_mediawiki() as XML for MediaWiki Import function
  rsstool_write_ansisql()   as ANSI SQL script
  rsstool_write_bookmarks() as bookmarks.html
  rsstool_write_csv()       as CSV
  rsstool_write_rss()       as RSS feed
*/
extern int rsstool_write_property (st_rsstool_t *rsstool);
extern int rsstool_write_template (st_rsstool_t *rsstool, const char *template_file);
extern int rsstool_write_template2 (st_rsstool_t *rt, const char *template_file);
extern int rsstool_write_txt (st_rsstool_t *rsstool);
extern int rsstool_write_href (st_rsstool_t *rsstool);
extern int rsstool_write_html (st_rsstool_t *rsstool);
extern int rsstool_write_mediawiki (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql (st_rsstool_t *rsstool);
extern int rsstool_write_bookmarks (st_rsstool_t *rsstool);
extern int rsstool_write_php (st_rsstool_t *rsstool);
extern int rsstool_write_csv (st_rsstool_t *rsstool, int separator);
extern int rsstool_write_rss (st_rsstool_t *rt, int version);


/*
  Deprecated

  rsstool_write_ansisql_joomla() write feed contents as ANSI SQL script
                                   for import into Joomla! CMS   
  rsstool_write_ansisql_dragonfly() write feed contents as ANSI SQL script
                                   for import into Dragonfly CMS
  rsstool_write_ansisql_*()  write feed contents as ANSI SQL script
                                (old structure)
*/
extern int rsstool_write_ansisql_joomla (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_dragonfly (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_092 (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_094 (st_rsstool_t *rsstool);
extern int rsstool_write_ansisql_095 (st_rsstool_t *rsstool);


#endif  // RSSTOOL_WRITE_H
