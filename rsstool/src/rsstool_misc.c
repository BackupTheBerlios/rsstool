/*
rsstool_misc.c - miscellaneous functions for RSStool

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
#include <ctype.h>
#include <signal.h>
#ifdef  _WIN32
#include "misc/win32.h"
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <time.h>
#include "misc/xml.h"
#include "misc/string.h"
#include "misc/rss.h"
#include "misc/hash.h"
#include "rsstool_defines.h"
#include "rsstool.h"
#include "rsstool_misc.h"


#ifdef  DEBUG
void
rsstool_st_rsstool_t_sanity_check (st_rsstool_t *rt)
{
  int i = 0;

  for (; i < rsstool_get_item_count (rt); i++)
    printf ("pos:%d\n"
            "title: %s\n"
            "url: %s\n"
            "date: %ld\n"
            "desc: %s\n\n",
      i,
      rt->item[i]->title,
      rt->item[i]->url,
      rt->item[i]->date,
      rt->item[i]->desc);

  printf ("rsstool_get_item_count(): %d\n\n", rsstool_get_item_count (rt));
}
#endif


static const char *
strip_html_pass (const char * s)
{
  return s;
}


static const char *
strip_html_br (const char * s)
{
  (void) s;
  return "\n";
}


static const char *
strip_html_nopass (const char * s)
{
  (void) s;
  return "";
}


static st_tag_filter_t strip_html_filter[] = {
  {
    "br",
    strip_html_br
  },
  {
    "",
    strip_html_nopass
  }
};


static st_tag_filter_t strip_html_filter2[] = {
  {
    "a",
    strip_html_pass
  },
  {
    "/a",
    strip_html_pass
  },
  {
    "br",
    strip_html_br
  },
  {
    "",
    strip_html_nopass
  }
};


char *
rsstool_strip_html (char *html)
{
  if (rsstool.strip_html == 2) // strip all except links
    xml_tag_filter (html, strip_html_filter2, 0);
  else
    xml_tag_filter (html, strip_html_filter, 0); // strip all

  return html;
}


static char *
rsstool_strip_lf (char *html)
{
  return strrep (html, "\n", " ");
}


static char *
rsstool_strip_whitespace (char *html)
{
  strrep (html, "\t", " ");
  strrep (html, "  ", " ");
  return strrep (html, "  ", " ");
}


int
rsstool_parse_rss (st_rsstool_t *rt, const char *feed_url, const char *file)
{
  int i = 0;
  st_rss_t *rss = NULL;

  if (!file)
    return -1;

  rss = rss_open (file);

  if (!rss)
    return -1;

  // fix unset dates?
  if (rsstool.fixdate)
    {
      if (!rss->date)
        rss->date = rt->start_time;
      for (; i < rss->item_count; i++)
        if (!rss->item[i].date) // if the parsed item date is 0
          rss->item[i].date = rt->start_time; // default is current time
    }

  rsstool_add_item (rt, rss, feed_url ? feed_url : file);

  rss_close (rss);

  return 0;
}


int
rsstool_add_item_s (st_rsstool_t *rt,
                    const char *site,
                    const char *feed_url,
                    time_t date,
                    const char *url,
                    const char *title,
                    const char *desc)
{
  int i = 0;
  char buf[MAXBUFSIZE];
  char site_s[RSSTOOL_MAXBUFSIZE],
       title_s[RSSTOOL_MAXBUFSIZE],
       desc_s[RSSTOOL_MAXBUFSIZE];

  if (rt->item_count == RSSTOOL_MAXITEM)
    {
      sprintf (buf, "RSSTOOL_MAXITEM count reached (%d)", RSSTOOL_MAXITEM);
      rsstool_log (rt, buf);
      return -1;
    }

  if (!url)
    return -1;

  if (!(*url))
    return -1;

  strncpy (site_s, site, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (title_s, title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (desc_s, desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;

  if (rsstool.strip_desc)
    *desc_s = 0;

  if (rsstool.strip_html)
    {
      rsstool_strip_html (site_s);
      rsstool_strip_html (title_s);
      rsstool_strip_html (desc_s);
    }

  if (rsstool.strip_lf)
    {
      rsstool_strip_lf (site_s);
      rsstool_strip_lf (title_s);
      rsstool_strip_lf (desc_s);
    }

  if (rsstool.strip_whitespace)
    {
      rsstool_strip_whitespace (site_s);
      rsstool_strip_whitespace (title_s);
      rsstool_strip_whitespace (desc_s);
    }

#if 0
  for (i = 0; i < rt->item_count && rt->item[i]; i++)
    {
      if (rt->item[i]->url && url)
        if (!strncmp (rt->item[i]->url, url, RSSTOOL_MAXBUFSIZE))
          return 0; // dupe
      
      if (rt->item[i]->title)
        if (!strncmp (rt->item[i]->title, title_s, RSSTOOL_MAXBUFSIZE))
          return 0; // dupe

//      there are feeds w/ items that have no description
//      if (rt->item[i]->desc)
//        if (!strcmp (rt->item[i]->desc, desc_s, RSSTOOL_MAXBUFSIZE))
//          return 0; // dupe
    }
#endif

  if (date <= rt->since)
    return -1;

  i = rt->item_count;

  rt->item[i] = (st_rsstool_item_t *) malloc (sizeof (st_rsstool_item_t));
  if (!rt->item[i])
    {
      sprintf (buf, "malloc failed to allocate %d bytes", sizeof (st_rsstool_item_t));
      rsstool_log (rt, buf);
      return -1;
    }

  strncpy (rt->item[i]->site, site_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->feed_url, feed_url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  rt->item[i]->date = date;
  strncpy (rt->item[i]->url, url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->title, title_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->desc, desc_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;

  rt->item_count++;

  return 0;
}


int
rsstool_add_item (st_rsstool_t *rt, st_rss_t *rss, const char *feed_url)
{
  int i = 0;

  for (; i < rss->item_count; i++)
    {
      if (!rsstool_add_item_s (rt, rss->title,
                               feed_url,
                               rss->item[i].date,
                               rss->item[i].url,
                               rss->item[i].title,
                               rss->item[i].desc))
       rt->item[i]->version = rss->version;
    }

#ifdef  DEBUG
  rsstool_st_rsstool_t_sanity_check (rt);
#endif

  return 0;
}


int
rsstool_get_item_count (st_rsstool_t *rt)
{
  return rt->item_count;
}


static int
rsstool_sort_compare (const void *a, const void *b)
{
  int result = 0;
  const st_rsstool_item_t *const *x = (const st_rsstool_item_t *const *) a;
  const st_rsstool_item_t *const *y = (const st_rsstool_item_t *const *) b;

  if ((*x)->date > (*y)->date)
    result = 1;
  else if ((*x)->date < (*y)->date)
    result = -1;

  if (rsstool.reverse)
    result *= (-1);

  return result;
}


int
rsstool_sort (st_rsstool_t * rt)
{
  qsort (rt->item, rsstool_get_item_count (rt), sizeof (st_rsstool_item_t *), rsstool_sort_compare);

  return 0;
}


int
rsstool_log (st_rsstool_t * rt, const char *s)
{
  char buf[32];
  time_t t = time (0);
  FILE *o = rt->log ? rt->log : stderr;

  strftime (buf, 32, "%b %d %H:%M:%S", localtime (&t));

#ifdef  __linux__
  fprintf (o, "%s rsstool(%d): ", buf, getpid());
#else
  fprintf (o, "%s rsstool: ", buf);
#endif
  fputs (s, o);
  fputc ('\n', o);
  fflush (o);

  return 0;
}


const char *
a_pass (const char *s)
{
  const char *p = xml_tag_get_value (s, "href");

  rsstool_add_item_s (&rsstool,
                      "rsstool",
                      "--parse",  
                      time (0),
                      p ? p : "",
                      p ? p : "",
                      "");

  return "";
}


int
rsstool_get_links (const char *file)
{
  FILE *fh = NULL;
  char buf[MAXBUFSIZE];
  int cf = 0;
  st_tag_filter_t f[] = {
    {
      "a",
      a_pass
    },
    {NULL, NULL}
  };

  if (!(fh = fopen (file, "rb")))
    return -1;

  while (fgets (buf, MAXBUFSIZE, fh))
{
printf (buf);
    cf = xml_tag_filter (buf, f, cf);
}
  fclose (fh);

  return 0;
}
