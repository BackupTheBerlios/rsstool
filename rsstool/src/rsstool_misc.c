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
//#include "misc/win32.h"
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <time.h>
#include "misc/getopt2.h"
#include "misc/xml.h"
#include "misc/string.h"
#include "misc/rss.h"
#include "misc/hash.h"
#include "misc/misc.h"
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


static char *
rsstool_strip_bin (char *s)
{
  unsigned char *p = (unsigned char *) s;

  while (*p)
    if (!isprint (*p))
      strmove ((char *) p, (char *) (p + 1));
    else
      p++;

  return s;
}


static char *
rsstool_hack_google (char *s)
{
  strrep (s, "<em>", "");
  return strrep (s, "</em>", "");
}


const char *
rsstool_normalize_feed (st_rsstool_t *rt, const char *file)
{
  char buf[MAXBUFSIZE];
  char tname[FILENAME_MAX];
  FILE *fh = NULL, *tmp = NULL;

  if (!file)
    return NULL;

  if (!(*file))
    return NULL;

  if (!(fh = fopen (file, "r")))
    {
      sprintf (buf, "rsstool_normalize_feed(): could not read %s\n", file);
      rsstool_log (rt, buf);

      return NULL;   
    }

  *tname = 0;
  tmpnam3 (tname, 0);
 
  if (!(tmp = fopen (tname, "wb")))
    {
      sprintf (buf, "rsstool_normalize_feed(): could not write %s\n", tname);
      rsstool_log (rt, buf);

      fclose (fh);

      return NULL;   
    } 

  while (fgets (buf, MAXBUFSIZE, fh))
    {
      // strip unprintable character
      if (rt->strip_bin)
        rsstool_strip_bin (buf);

      // HACK: fix for google video feeds that have un-escaped <em> tags
      if (rt->hack_google)
        {
//          if (!strstr (rt->item[0]->feed_url, "video.google.com"))
//            rsstool_log (rt, "rsstool_normalize_feed(): not google/video " OPTION_LONG_S "hack-google ignored");
//          else
            rsstool_hack_google (buf);
        }

      fputs (buf, tmp);
   }

  fclose (fh);
  fclose (tmp);

  remove (file);
  rename (tname, file);

  return file;
}


static time_t
rsstool_get_event_start (const char * desc)
{
// make sure that &tz=0
//  http://www.gamescast.tv/rss/rss-events.php?game=sc2&tz=0
//Begins: 12/05 7:00pm,
//Ends: 12/05 9:00pm,
//Show Type: Podcast,
//Game Featured: StarCraft 2
  char buf[32];
  char *s = (char *) desc;

  s = strstr (s, "Begins: ");
  if (!s)
    return 0;

  strncpy (buf, s + 7, 32)[31] = 0;
  s = strchr (buf, ',');
  if (s)
    *s = 0;

  return strptime2 (strtriml (strtrimr (buf)));
}


static time_t
rsstool_get_event_end (const char * desc)
{
  char buf[32];
  char *s = (char *) desc;

  s = strstr (s, "Ends: ");
  if (!s)
    return 0;

  strncpy (buf, s + 5, 32)[31] = 0;
  s = strchr (buf, ',');
  if (s)
    *s = 0;

  return strptime2 (strtriml (strtrimr (buf)));
}


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
  return " ";
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


char *
rsstool_strip_html (char *html, const char *strip_html_allow)
{
  int i = 0;
  char buf[MAXBUFSIZE];
  int argc = 0;
#define MAX_TAGS 1024
  char *arg[MAX_TAGS];
  st_tag_filter_t filter[MAX_TAGS];

  if (!strip_html_allow)
    {
      xml_tag_filter (html, strip_html_filter, 0); // strip all
      return html;
    }

  strncpy (buf, strip_html_allow, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  argc = explode (arg, buf, ",", MAX_TAGS);
  // DEBUG
//  for (i = 0; i < argc && i < MAX_TAGS - 2; i++)
//    printf ("arg[%d]==%s\n", i, arg[i]);

  for (i = 0; i < argc && i < MAX_TAGS - 2; i++)
    {
      filter[i].start_tag = arg[i];
      filter[i].filter = strip_html_pass;
    }
  filter[i].start_tag = "";
  filter[i].filter = strip_html_nopass;

  xml_tag_filter (html, filter, 0);

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
  int i = 0;

  strrep (html, "\t", " ");
  for (; i < 2; i++)
    strrep (html, "  ", " ");

  return html;
}


int
rsstool_parse_rss (st_rsstool_t *rt, const char *feed_url, const char *file)
{
  int i = 0;
  st_rss_t *rss = NULL;

  if (!file)
    return -1;

  rss = rss_open (file, (rt->encoding[0] ? rt->encoding : NULL));

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
                    const char *desc,
                    const char *user,
                    const char *media_keywords,
                    const char *media_image,
                    int media_duration)
{
  int i = 0;
  char buf[MAXBUFSIZE];
  char url_s[RSSTOOL_MAXBUFSIZE],
       site_s[RSSTOOL_MAXBUFSIZE],
       title_s[RSSTOOL_MAXBUFSIZE],
       user_s[RSSTOOL_MAXBUFSIZE],
       media_keywords_s[RSSTOOL_MAXBUFSIZE],
       media_image_s[RSSTOOL_MAXBUFSIZE],
       desc_s[RSSTOOL_MAXBUFSIZE];
  char *p = NULL;
  time_t event_start = date,
         event_end = 0;

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
  strncpy (url_s, url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  if (user)
    strncpy (user_s, user, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  if (media_image)
    strncpy (media_image_s, media_image, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;

  *buf = 0;
  if (rt->strip_keywords == 2)
    strncpy (buf, desc ? desc : "", MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  else if (rt->strip_keywords == 1)
    strncpy (buf, title ? title : "", MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  else
    snprintf (buf, MAXBUFSIZE, "%s %s", title ? title : "", desc ? desc : "");
  rsstool_strip_html (buf, NULL); // remove links too
  strncpy (media_keywords_s, misc_get_keywords (buf, 0), RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  // IF media:keywords are present use them instead
  if (media_keywords)
    if (*media_keywords)
      strncpy (media_keywords_s, media_keywords, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;

  // HACK
  if (rt->hack_event)
    {
      if (!strstr (feed_url, "www.gamescast.tv"))
        rsstool_log (rt, "rsstool_add_item_s(): not gamescast " OPTION_LONG_S "hack-event ignored");
      else
        {
          event_start = rsstool_get_event_start (desc_s);
          event_end = rsstool_get_event_end (desc_s);
        }
    }

  // HACK: remove eventual google redirect
  if (rt->hack_google)
    {
      if (strstr (url_s, "www.google.com"))
        {
          if ((p = strstr (url_s, "?q=")))
            {
              strmove (url_s, p + 3);
              if ((p = strstr (url_s, "&source=")))
                *p = 0;
            }

          // desc
          if ((p = strstr (desc_s, "<div")))
            *p = 0;
        }
      else if (strstr (url_s, "news.google.com"))
        {
          if ((p = strstr (url_s, "&url=")))
            {
              strmove (url_s, p + 5);
              if ((p = strstr (url_s, "&usg=")))
                *p = 0;
            }
        }
    }

  // HACK
//  if (rt->hack_youtube)
    {
      if (strstr (url_s, "www.youtube.com"))
        strrep (url_s, "&feature=youtube_gdata", "");
    }

  if (rsstool.strip_filter)
    {
      if (strfilter (title_s, rsstool.strip_filter) == 1)
        return 0;
      if (strfilter (desc_s, rsstool.strip_filter) == 1)
        return 0;
    }

  if (rsstool.strip_desc)
    *desc_s = 0;

  if (rsstool.strip_html)
    {
      rsstool_strip_html (desc_s, rsstool.strip_html_allow);
    }

  if (rsstool.strip_lf)
    {
      rsstool_strip_lf (desc_s);
    }

  if (rsstool.strip_whitespace)
    {
      strtriml (strtrimr (title_s));
      rsstool_strip_whitespace (desc_s);
    }

  if (date > 0 && date < rt->since)
    return -1;

  for (i = 0; i < rt->item_count && rt->item[i]; i++)
    {
      if (rt->item[i]->url && url)
        if (!strncmp (rt->item[i]->url, url, RSSTOOL_MAXBUFSIZE))
          return 0; // dupe
      
      if (rt->item[i]->title)
        if (!strncmp (rt->item[i]->title, title_s, RSSTOOL_MAXBUFSIZE))
          return 0; // dupe

// because there are feeds w/ items that have no description
#if 0
      if (rt->item[i]->desc)
        if (!strcmp (rt->item[i]->desc, desc_s, RSSTOOL_MAXBUFSIZE))
          return 0; // dupe
#endif
    }

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
  strncpy (rt->item[i]->url, url_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->title, title_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->desc, desc_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->user, user_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->media_keywords, media_keywords_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  strncpy (rt->item[i]->media_image, media_image_s, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
  rt->item[i]->media_duration = media_duration;
  rt->item[i]->event_start = event_start;
  rt->item[i]->event_end = event_end;

  rt->item_count++;

  return 0;
}


int
rsstool_add_item (st_rsstool_t *rt, st_rss_t *rss, const char *feed_url)
{
  int i = 0;

  for (; i < rss->item_count; i++)
    rsstool_add_item_s (rt, rss->title,
                            feed_url,
                            rss->item[i].date,
                            rss->item[i].url,
                            rss->item[i].title,
                            rss->item[i].desc,
                            rss->item[i].user,
                            rss->item[i].media.keywords,
                            rss->item[i].media.thumbnail,
                            rss->item[i].media.duration);

  for (i = 0; i < rt->item_count; i++)
    rt->item[i]->version = rss->version;

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
                      rsstool.start_time,
                      p ? p : "",
                      p ? p : "",
                      "",
                      "",
                      "",
                      "",
                      0
);

  return "";
}


int
rsstool_parse (const char *file)
{
  // parse and structure non-RSS content (e.g. HTML document)
#warning TODO: html between links is the description
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
    cf = xml_tag_filter (buf, f, cf);

  fclose (fh);

  return 0;
}
