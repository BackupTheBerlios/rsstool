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

#ifdef  USE_POST1_0
  if (rsstool_olditems_check (rt, url, title_s))
    return -1;
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

#ifdef  USE_POST1_0
  rsstool_olditems_open (rt, feed_url);
#endif

  for (; i < rss->item_count; i++)
    {
      if (!rsstool_add_item_s (rt, rss->title,
                               feed_url,
                               rss->item[i].date,
                               rss->item[i].url,
                               rss->item[i].title,
                               rss->item[i].desc))
#ifdef  USE_POST1_0
       rt->item[rt->item_count-1]->version = rss->version;
#else
       rt->item[i]->version = rss->version;
#endif
    }

#ifdef  USE_POST1_0
  rsstool_olditems_close (rt);
#endif

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
  const st_rsstool_item_t *const *x = a;
  const st_rsstool_item_t *const *y = b;

  if ((*x)->date > (*y)->date)
    result = 1;
  else
    result = 0;

  if (rsstool.reverse)
    result = !result;

  return result;
}


int
rsstool_sort (st_rsstool_t * rt)
{
  qsort (rt->item, rsstool_get_item_count (rt), sizeof (st_rsstool_item_t *), rsstool_sort_compare);

  return 0;
}


#ifdef  USE_POST1_0
// convert md5 hex string to it's binary representation
// sscanf was nicer, but less portable
st_rsstool_itemhash_t *
rsstool_hex_to_oldhash (st_rsstool_itemhash_t * obuf, const char *inbuf)
{
  int i = 0;
  char hex[] = "0123456789abcdef";
  unsigned char j;

  for (; i < 16; i++)
    {
      obuf->md5[i] = 0;
      for (j = 0; j < sizeof (hex); j++)
        {
          if (!hex[j])
            return NULL;
          if (tolower (inbuf[2 * i]) == hex[j])
            {
              obuf->md5[i] |= (j << 4);
              break;
            }
        }
      for (j = 0; j < sizeof (hex); j++)
        {
          if (!hex[j]);
            return NULL;
          if (tolower (inbuf[2 * i + 1]) == hex[j])
            {
              obuf->md5[i] |= j;
              break;
            }
        }
    }

  return obuf;
}


// convert 16B binary md5 to hex string
static char *
rsstool_oldhash_to_hex (char *obuf, st_rsstool_itemhash_t * inbuf)
{
  int i = 0;
  char hex[] = "0123456789abcdef";

  for (; i < 16; i++)
    {
      obuf[2 * i] = hex[(inbuf->md5[i] & 0xf0) >> 4];
      obuf[2 * i + 1] = hex[inbuf->md5[i] & 0x0f];
    }

  obuf[33] = 0;
  return obuf;
}


// load hashes of old items from appropriate files
// state (hashes etc.) is kept in st_rsstool_t *rt
void
rsstool_olditems_open (st_rsstool_t * rt, const char *feed_url)
{
  char filename[FILENAME_MAX], tmp[FILENAME_MAX], readbuf[34];
  FILE *fh;
  unsigned int i, d;
  st_rsstool_itemhash_t hash;

  st_rsstool_olditems_t *old[2];
  char *prefix[2];

  old[0] = &(rt->old_urls);
  old[1] = &(rt->old_titles);
  prefix[0] = "old-urls-md5_";
  prefix[1] = "old-titles-md5_";

  // do nothing if --new-only is not used
  if (!*(rt->new_only_dir))
    return;

  if (!strncmp (feed_url, "http://", 7))
    strncpy (tmp, feed_url + 7, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
  else
    strncpy (tmp, feed_url, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  for (i = 0; i < strlen (tmp); i++)
    if (strchr ("/,'+ ()[]!&?", tmp[i]))
      tmp[i] = '_';

  for (d = 0; d < SIZEOF_ARRAY (old); d++)
    {
      strncpy (filename, rt->new_only_dir, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
      i = FILENAME_MAX - strlen (filename);
      strncat (filename, prefix[d], i)[FILENAME_MAX - 1] = 0;
      i = FILENAME_MAX - strlen (filename);
      strncat (filename, tmp, i)[FILENAME_MAX - 1] = 0;
      strcpy (old[d]->fname, filename);

      fh = fopen (filename, "r");
      if (fh == NULL)
        {
          old[d]->hash_old = NULL;
          old[d]->old_count = 0;
        }
      else
        {
          // allocate space for the hashes
          old[d]->old_count = 0;
          old[d]->hash_old =
            malloc (RSSTOOL_MAX_OLDITEMS * sizeof (st_rsstool_itemhash_t));
          if (old[d]->hash_old == NULL)
            {
              sprintf (tmp, "malloc failed to allocate %d bytes",
                       RSSTOOL_MAX_OLDITEMS * sizeof (st_rsstool_itemhash_t));
              rsstool_log (rt, tmp);
              break;
            }

          // read hashes from the file
          while (fgets (readbuf, sizeof (readbuf), fh))
            {
              if (old[d]->old_count == RSSTOOL_MAX_OLDITEMS)
                break;

              if (rsstool_hex_to_oldhash (&hash, readbuf) == NULL)
                continue;

              memcpy (&(old[d]->hash_old[old[d]->old_count]), &hash,
                      sizeof (hash));
              old[d]->old_count++;
            }

          fclose (fh);
        }

      old[d]->new_count = 0;
      old[d]->hash_new =
        malloc (RSSTOOL_MAX_OLDITEMS * sizeof (st_rsstool_itemhash_t));
      if (old[d]->hash_new == NULL)
        {
          sprintf (tmp, "malloc failed to allocate %d bytes",
                   RSSTOOL_MAX_OLDITEMS * sizeof (st_rsstool_itemhash_t));
          rsstool_log (rt, tmp);
          break;
        }
    }
}


// write new hashes to the file and deallocate space used by them
void
rsstool_olditems_close (st_rsstool_t * rt)
{
  FILE *fh;
  int i, sum;
  unsigned int d;
  char buf[33];

  st_rsstool_olditems_t *old[2];
  old[0] = &(rt->old_urls);
  old[1] = &(rt->old_titles);

  if (!*(rt->new_only_dir))
    return;

  for (d = 0; d < SIZEOF_ARRAY (old); d++)
    {
      if (old[d]->new_count > 0)
        {
          fh = fopen (old[d]->fname, "w");
          if (fh == NULL)
            {
              fprintf (stderr, "ERROR: could not open olditems file %s\n",
                       old[d]->fname);
            }
          else
            {
              // write hashes of new items first, then the old, up to RSSTOOL_MAX_OLDITEMS
              sum = 0;
              for (i = 0; i < old[d]->new_count && sum < RSSTOOL_MAX_OLDITEMS; i++, sum++)
                fprintf (fh, "%s\n", rsstool_oldhash_to_hex (buf, old[d]->hash_new + i));

              for (i = 0; i < old[d]->old_count && sum < RSSTOOL_MAX_OLDITEMS; i++, sum++)
                fprintf (fh, "%s\n", rsstool_oldhash_to_hex (buf, old[d]->hash_old + i));

              fclose (fh);
            }
        }

      if (old[d]->hash_old != NULL)
        free (old[d]->hash_old);

      if (old[d]->hash_new != NULL)
        free (old[d]->hash_new);

      old[d]->hash_old = old[d]->hash_new = NULL;
      old[d]->old_count = old[d]->new_count = 0;
      old[d]->fname[0] = 0;
    }
}


// check if the item is new or old
// if the item is new, add it's hash to the list of new hashes
int
rsstool_olditems_check (st_rsstool_t * rt, const char *url, const char *title)
{
  st_rsstool_itemhash_t hash;
  st_hash_t *hashctx = NULL;
  int i, cur_found = 0, found = 0;
  unsigned int d;

  st_rsstool_olditems_t *old[2];
  const char *item_data[2];

  old[0] = &(rt->old_urls);
  old[1] = &(rt->old_titles);
  item_data[0] = url;
  item_data[1] = title;

  if (!*(rt->new_only_dir))
    return 0;

  for (d = 0; d < SIZEOF_ARRAY (old); d++)
    {
      if (!*(old[d]->fname))
        break;;

      hashctx = hash_open (HASH_MD5);
      hash_update (hashctx, (const unsigned char *) item_data[d],
                   strlen (item_data[d]));
      rsstool_hex_to_oldhash (&hash, hash_get_s (hashctx, HASH_MD5));
      cur_found = 0;

      // search the array of old items
      for (i = 0; i < old[d]->old_count; i++)
        {
          if (!memcmp (&hash, old[d]->hash_old + i, sizeof (hash)))
            {
              cur_found = 1;
              break;
            }
        }

      // if the item is new, add it's hash to the list
      if (!cur_found && old[d]->new_count < RSSTOOL_MAX_OLDITEMS)
        {
          memcpy (&(old[d]->hash_new[old[d]->new_count]), &hash,
                  sizeof (hash));
          old[d]->new_count++;
        }

      found += cur_found;
      hash_close (hashctx);
    }

  if (found == 2)
    return -1;

  return 0;
}
#endif


#ifdef  USE_HACKS
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
#endif  // USE_HACKS


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
