/*
rsstool_write.c - write functions for RSStool

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "misc/net.h"
#include "misc/xml.h"
#include "misc/string.h"
#include "misc/rss.h"
//#include "misc/sql.h"
#include "misc/hash.h"
#include "rsstool_defines.h"
#include "rsstool.h"
#include "rsstool_misc.h"
#include "rsstool_write.h"


int
rsstool_write_rss (st_rsstool_t *rt, int version)
{
  st_rss_t rss;
  int i = 0;

  memset (&rss, 0, sizeof (st_rss_t));

  rss.version = version;
  strcpy (rss.title, "RSStool");
  strcpy (rss.url, "http://rsstool.berlios.de");
  strcpy (rss.desc, "read, parse, merge and write RSS and Atom feeds");

  for (; i < rsstool_get_item_count (rt) && i < RSSMAXITEM; i++)
    {
      strncpy (rss.item[i].title, rt->item[i]->title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].url, rt->item[i]->url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].desc, rt->item[i]->desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      rss.item[i].date = rt->item[i]->date;
      rss.item[i].media.duration = rt->item[i]->media_duration;
      rss.item_count++;
    }

  if (i == RSSMAXITEM)
    {
      char buf[MAXBUFSIZE];

      sprintf (buf, "can write only RSS feeds with up to %d items (was %d items)\n",
        RSSMAXITEM, rsstool_get_item_count (rt));
      rsstool_log (rt, buf);
    }

  return rss_write (rsstool.output_file, &rss, version);
}


int
rsstool_write_xml (st_rsstool_t *rt)
{
  st_rss_t rss;
  int i = 0;
  int version = 2;

  memset (&rss, 0, sizeof (st_rss_t));

  rss.version = version;
  strcpy (rss.title, "RSStool");
  strcpy (rss.url, "http://rsstool.berlios.de");
  strcpy (rss.desc, "read, parse, merge and write RSS and Atom feeds");

  for (; i < rsstool_get_item_count (rt) && i < RSSMAXITEM; i++)
    {
      strncpy (rss.item[i].title, rt->item[i]->title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].url, rt->item[i]->url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].desc, rt->item[i]->desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      rss.item[i].date = rt->item[i]->date;
      rss.item[i].media.duration = rt->item[i]->media_duration;
      rss.item_count++;
    }

  if (i == RSSMAXITEM)
    {
      char buf[MAXBUFSIZE];

      sprintf (buf, "can write only RSS feeds with up to %d items (was %d items)\n",
        RSSMAXITEM, rsstool_get_item_count (rt));
      rsstool_log (rt, buf);
    }

  return rss_write (rsstool.output_file, &rss, version);
}


