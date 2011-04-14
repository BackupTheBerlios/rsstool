/*
rsstool_misc.h - miscellaneous functions for RSStool

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
#ifndef RSSTOOL_MISC_H
#define RSSTOOL_MISC_H


#ifdef  DEBUG
extern void rsstool_st_rsstool_t_sanity_check (st_rsstool_t *rsstool);
#endif


// parse start and length of an event from string
//extern unsigned long int rsstool_get_event_start (const char *s);
//extern unsigned long int rsstool_get_event_len (const char *s);


/*
  Miscellaneous functions

  rsstool_strip_html()      strip html tags from a string

  rsstool_parse_rss()       (down)load an RSS feed and parse it into st_rsstool_t

  rsstool_add_item()        add RSS or Atom items to st_rsstool_t
  rsstool_add_item_s()      add a single item to st_rsstool_t
  rsstool_get_item_count()  get current number of items in st_rsstool_t

  rsstool_sort()            sort all items

  rsstool_get_links()       get links from a HTML document

  rsstool_log()             write string to logfile
*/
extern char *rsstool_strip_html (char *html);
extern int rsstool_parse_rss (st_rsstool_t *rt, const char *feed_url, const char *file);
extern int rsstool_add_item (st_rsstool_t *rt, st_rss_t *rss, const char *feed_url);
extern int rsstool_add_item_s (st_rsstool_t *rt,
                               const char *site,
                               const char *feed_url,
                               time_t date,
                               const char *url,
                               const char *title,
                               const char *desc,
                               const char *keywords,
                               int media_duration);
extern int rsstool_get_item_count (st_rsstool_t *rt);
extern int rsstool_sort (st_rsstool_t * rt);
extern int rsstool_get_links (const char *file);
extern int rsstool_log (st_rsstool_t * rt, const char *s);


#endif  // RSSTOOL_MISC_H
