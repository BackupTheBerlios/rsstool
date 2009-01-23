/*
rsstool_defines.h - definitions for RSStool

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
#ifndef RSSTOOL_DEFINES_H
#define RSSTOOL_DEFINES_H


#define ARGS_MAX 32768
#define MAXBUFSIZE 32768


#define RSSTOOL_MAXITEM 4096
//#define RSSTOOL_MAXITEM_S "4096"
#define RSSTOOL_MAXBUFSIZE 4096
//#define RSSTOOL_MAXBUFSIZE_S "4096"

#define RSSTOOL_MAX_OLDITEMS 512

//#define RSSTOOL_STRNCPY(a,b) strncpy(a,b,RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE-1]=0
#define RSSTOOL_USER_AGENT_S "Mozilla/5.0 (X11; U; Linux i586; en-US; rv:1.7.12) Gecko/20060205 Debian/1.7.12-1.1"
#define RSSTOOL_VERSION_S "1.0.1"


// item parts
//#define RSSTOOL_LINK  (1<<0)
//#define RSSTOOL_DESC  (1<<1)
//#define RSSTOOL_DATE  (1<<2)
//#define RSSTOOL_TITLE (1<<3)


enum {
  RSSTOOL_VER = 1,
  RSSTOOL_HELP,
  RSSTOOL_HREF,
  RSSTOOL_REVERSE,
  RSSTOOL_INPUT_FILE,
  RSSTOOL_HTML,
  RSSTOOL_TXT,
  RSSTOOL_LOG,
  RSSTOOL_PROP,
  RSSTOOL_PROPERTY,
  RSSTOOL_O,
  RSSTOOL_TEMPLATE,
  RSSTOOL_TEMPLATE2,
  RSSTOOL_QUIET,
  RSSTOOL_MAX,
  RSSTOOL_SQL,
  RSSTOOL_MYSQL,
  RSSTOOL_ODBC,
  RSSTOOL_BOOKMARKS,
  RSSTOOL_CSV,
  RSSTOOL_RSS,
  RSSTOOL_U,
  RSSTOOL_GZIP,
  RSSTOOL_SHTML,
  RSSTOOL_SHTML2,
  RSSTOOL_SWHITE,
  RSSTOOL_SDESC,
  RSSTOOL_SLF,
  RSSTOOL_DIGG,
  RSSTOOL_SQLOLD,
  RSSTOOL_PARSE,
  RSSTOOL_STITLE,
  RSSTOOL_WGET,
  RSSTOOL_CURL,
//  RSSTOOL_NEW_ONLY,
  RSSTOOL_SINCE,
  RSSTOOL_FIXDATE,
  RSSTOOL_JOOMLA,
  RSSTOOL_DRAGONFLY,
  RSSTOOL_PHP,
  RSSTOOL_NOSORT,
  RSSTOOL_PIPE
};


enum {
  RSSTOOL_OUTPUT_HTML = 1,
  RSSTOOL_OUTPUT_TXT,
  RSSTOOL_OUTPUT_SQL,
  RSSTOOL_OUTPUT_SQLOLD,
  RSSTOOL_OUTPUT_BOOKMARKS,
  RSSTOOL_OUTPUT_CSV,
  RSSTOOL_OUTPUT_RSS,
  RSSTOOL_OUTPUT_TEMPLATE,
  RSSTOOL_OUTPUT_TEMPLATE2,
  RSSTOOL_OUTPUT_PROPERTY,
  RSSTOOL_OUTPUT_ODBC,
  RSSTOOL_OUTPUT_MYSQL,
  RSSTOOL_OUTPUT_HREF,
  RSSTOOL_OUTPUT_JOOMLA,
  RSSTOOL_OUTPUT_DRAGONFLY,
  RSSTOOL_OUTPUT_PHP
};

#endif  // RSSTOOL_DEFINES_H
