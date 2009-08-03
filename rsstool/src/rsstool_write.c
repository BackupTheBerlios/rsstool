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
#include "misc/defines.h"
#include "misc/net.h"
#include "misc/xml.h"
#include "misc/string.h"
#include "misc/rss.h"
#include "misc/sql.h"
#include "misc/hash.h"
#include "rsstool_defines.h"
#include "rsstool.h"
#include "rsstool_misc.h"
#include "rsstool_write.h"


static int current_item = 0; // used by rsstool_write_template2


static void
rsstool_pipe_reopen (st_rsstool_t *rt)
{
#ifndef _WIN32
  (void) rt;
#endif

  if (rsstool.output_file && rsstool.output_file != stdout)
    pclose (rsstool.output_file);

  rsstool.output_file = popen (rsstool.pipe_command, "w");

  if (!rsstool.output_file)
    {
      fprintf (stderr, "ERROR: could not open pipe to command '%s' (using stdout)\n", rsstool.pipe_command);
      rsstool.output_file = stdout;
      *(rsstool.pipe_command) = 0;
    }
}


int
rsstool_write_property (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("#\n"
         "# RSStool - RSS feed as property file\n"
         "#\n", rsstool.output_file);

  for (; i < items; i++)
    {
//      fprintf (rsstool.output_file, "version_%d=%s\n", i, rss_get_version_s_by_id (rt->item[i]->version));
      fprintf (rsstool.output_file, "date_%d=%ld\n", i, rt->item[i]->date);
      fprintf (rsstool.output_file, "url_%d=%s\n", i, rt->item[i]->url);
      fprintf (rsstool.output_file, "title_%d=%s\n", i, rt->item[i]->title);
      fprintf (rsstool.output_file, "desc_%d=%s\n", i, rt->item[i]->desc);
    }

  return 0;
}


static int
get_item_value (const char *tag)
{
  const char *p = xml_tag_get_value (tag, "item");

  if (p)
    {
      int value = strtol (p, NULL, 10);

      return MAX (value, 0);
    }

  // HACK: for rsstool_write_template2() so the filters of
  //         rsstool_write_template() can be used
  return current_item;
}


static const char *
template_url_filter (const char *s)
{
  return rsstool.item[get_item_value (s)]->url;
}


static const char *
template_site_filter (const char *s)
{
  return rsstool.item[get_item_value (s)]->site;
}


static const char *
template_feed_url_filter (const char *s)
{
  return rsstool.item[get_item_value (s)]->feed_url;
}


static const char *
template_date_filter (const char *s)
{
  static char buf[MAXBUFSIZE];
  const char *format = xml_tag_get_value (s, "format");

  if (!format)
    format = "%Y%m%d %H:%M";
//    format = "%a, %d %b %Y %H:%M:%S %Z";

  strftime (buf, MAXBUFSIZE, format, localtime (&rsstool.item[get_item_value (s)]->date));

  return buf;
}


static const char *
template_title_filter (const char *s)
{
  return rsstool.item[get_item_value (s)]->title;
}


static const char *
template_desc_filter (const char *s)
{
  return rsstool.item[get_item_value (s)]->desc;
}


static const char *
template_start_filter (const char *s)
{
// TODO: make comment strings configurable in the parse tags
  if (get_item_value (s) >= rsstool.item_count)
    return "<!--";
  else
    return "";
}


static const char *
template_end_filter (const char *s)
{
  if (get_item_value (s) >= rsstool.item_count)
    return "-->";
  else
    return "";
}


static const char *
template_updated_filter (const char *s)
{
#ifndef  _WIN32
  (void) s;
#endif
  static char buf[MAXBUFSIZE];
  time_t t = time (0);

  strftime (buf, MAXBUFSIZE, "powered by RSStool " RSSTOOL_VERSION_S " %Y%m%d %H:%M:%S %Z", localtime (&t));
  return buf;
}


static const char *
template_rsstool_filter (const char *s)
{
#ifndef _WIN32
  (void) s;
#endif
  return "RSStool " RSSTOOL_VERSION_S;
}


static st_tag_filter_t f[] = {
  {
    "rsstool:url",
    template_url_filter
  },
  {
    "rsstool:title",
    template_title_filter
  },
  {
    "rsstool:desc",
    template_desc_filter
  },
  {
    "rsstool:date",
    template_date_filter
  },
  {
    "rsstool:site",
    template_site_filter
  },
  {
    "rsstool:feed_url",
    template_feed_url_filter
  },
  {
    "rsstool:start",
    template_start_filter
  },
  {
    "rsstool:end",
    template_end_filter
  },
  {
    "rsstool:updated",
    template_updated_filter
  },
  {
    "rsstool:rsstool",
    template_rsstool_filter
  },
  {NULL, NULL}
};


int
rsstool_write_template (st_rsstool_t *rt, const char *template_file)
{
#ifndef _WIN32
  (void) rt;
#endif
//  char buf[MAXBUFSIZE + 2 * sizeof (st_rsstool_item_t)];
  char buf[MAXBUFSIZE * 10];
  FILE *fh = NULL;
  int cf = 0;

  if (!(fh = fopen (template_file, "r")))
    return -1;

  while (fgets (buf, MAXBUFSIZE, fh))
    {
      cf = xml_tag_filter (buf, f, cf);
      fputs (buf, rsstool.output_file);
    }

  fclose (fh);

  return 0;
}


int
rsstool_write_template_s (st_rsstool_t *rt, const char *template_s)
{
#ifndef _WIN32
  (void) rt;
#endif
  char *p = (char *) malloc (strlen (template_s) * 2);

  if (p)
    {
      strcpy (p, template_s);
      xml_tag_filter (p, f, 0);
      fputs (p, rsstool.output_file);
      free (p);

      return 0;
    }

  return -1;
}


int
rsstool_write_template2 (st_rsstool_t *rt, const char *template_file)
{
#ifndef _WIN32
  (void) rt;
#endif
  char buf[MAXBUFSIZE * 10];
  FILE *fh = NULL;
  int cf, i = 0;
  int items = rsstool_get_item_count (rt);

  if (!(fh = fopen (template_file, "r")))
    return -1;

  for (; i < items; i++)
    {
      current_item = i;

      if (*(rt->pipe_command))
        rsstool_pipe_reopen (rt); // send to pipe instead

      cf = 0;
      while (fgets (buf, MAXBUFSIZE, fh))
        {
          cf = xml_tag_filter (buf, f, cf);
          fputs (buf, rsstool.output_file);
        }
      fseek (fh, 0, SEEK_SET);
    }

  fclose (fh);

  return 0;
}


int
rsstool_write_txt (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;
  char site_s[RSSTOOL_MAXBUFSIZE], title_s[RSSTOOL_MAXBUFSIZE],
    desc_s[RSSTOOL_MAXBUFSIZE];

  for (; i < items; i++)
    {
      char buf[MAXBUFSIZE];
      strftime (buf, MAXBUFSIZE, "%Y%m%d %H:%M", localtime (&rt->item[i]->date));

      strncpy (site_s, rt->item[i]->site, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (title_s, rt->item[i]->title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (desc_s, rt->item[i]->desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;

//      rsstool_strip_html (site_s);
//      rsstool_strip_html (title_s);
//      rsstool_strip_html (desc_s);

      if (*(rt->pipe_command))
        rsstool_pipe_reopen (rt); // send to pipe instead

      if (!(*desc_s))
        {
          fputs (site_s, rsstool.output_file);
          fputc (' ', rsstool.output_file); 
          fputs (buf, rsstool.output_file);
          fputc (' ', rsstool.output_file); 
          fputs (rt->item[i]->url, rsstool.output_file);
          fputc ('\n', rsstool.output_file);
          fputs (title_s, rsstool.output_file);
          fputc ('\n', rsstool.output_file);
        }
      else
        {
          fputs (site_s, rsstool.output_file);
          fputc (' ', rsstool.output_file);
          fputs (title_s, rsstool.output_file);   
          fputs ("\n(", rsstool.output_file);
          fputs (rt->item[i]->url, rsstool.output_file);
          fputs (", ", rsstool.output_file);
          fputs (buf, rsstool.output_file);
          fputs (")\n", rsstool.output_file);
          fputs (desc_s, rsstool.output_file);
          fputc ('\n', rsstool.output_file);
        }
    }

  return 0;
}


int
rsstool_write_href (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;

  for (; i < items; i++)
    {
      if (*(rt->pipe_command))
        rsstool_pipe_reopen (rt); // send to pipe instead

      fputs (rt->item[i]->url, rsstool.output_file);
      fputc ('\n', rsstool.output_file);          
    }

  return 0;
}


int
rsstool_write_html (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;

  for (; i < items; i++)
    {
      char buf[MAXBUFSIZE];
      strftime (buf, MAXBUFSIZE, "%Y%m%d %H:%M", localtime (&rt->item[i]->date));

      if (*(rt->pipe_command))
        rsstool_pipe_reopen (rt); // send to pipe instead

      if (!(*rt->item[i]->desc))
        {
          fputs (rt->item[i]->site, rsstool.output_file);
          fputc (' ', rsstool.output_file);
          fputs (buf, rsstool.output_file);
          fputs (" <a href=\"", rsstool.output_file);
          fputs (rt->item[i]->url, rsstool.output_file);              
          fputs ("\">", rsstool.output_file);              
          fputs (rt->item[i]->title, rsstool.output_file);             
          fputs ("</a><br>\n", rsstool.output_file);             
        }
      else
        {
          fputs ("<a href=\"", rsstool.output_file);   
          fputs (rt->item[i]->url, rsstool.output_file);  
          fputs ("\">", rsstool.output_file);
          fputs (rt->item[i]->title, rsstool.output_file);
          fputs ("</a><br>\n", rsstool.output_file);
          fputs (rt->item[i]->site, rsstool.output_file);
          fputs (" (", rsstool.output_file);
          fputs (buf, rsstool.output_file);
          fputs (")<br>\n", rsstool.output_file);
          fputs (rt->item[i]->desc, rsstool.output_file);
          fputs ("<br>\n<hr>", rsstool.output_file);            
        }
    }

  return 0;
}


int
rsstool_write_php (st_rsstool_t *rt)
{
  char buf[MAXBUFSIZE];
  int items = rsstool_get_item_count (rt);
  int i = 0;
  time_t t = time (0);

  strftime (buf, MAXBUFSIZE, "generated by RSStool " RSSTOOL_VERSION_S " %Y%m%d %H:%M:%S %Z", localtime (&t));
  fprintf (rsstool.output_file,
           "<?php\n"
           "/*\n"
           "  %s\n"
           "*/\n"
           "//phpinfo ();\n"
           "\n"
           "\n"
           "$rsstool_array = array (\n", buf);

  for (; i < items; i++)
    {
      fprintf (rsstool.output_file, "%s\n  array (\"", i ? "," : "");

      sprintf (buf, "%s", rt->item[i]->url);
//      fprintf (rsstool.output_file, "%s\",\n         \"", str_escape_code (buf));
      fprintf (rsstool.output_file, "%s\",\n         \"", buf);

      sprintf (buf, "%s", rt->item[i]->title);
      fprintf (rsstool.output_file, "%s\",\n         \"", str_escape_code (buf));

      sprintf (buf, "%s", rt->item[i]->site);
      fprintf (rsstool.output_file, "%s\",\n         \"", str_escape_code (buf));

      sprintf (buf, "%ld", rt->item[i]->date);
      fprintf (rsstool.output_file, "%s\",\n         \"", str_escape_code (buf));

      sprintf (buf, "%s", rt->item[i]->desc);
      fprintf (rsstool.output_file, "%s\"\n        )", str_escape_code (buf));
    }

  fputs ("  );\n"
         "\n"
         "//echo \"<pre><tt>\";\n"
         "//print_r ($rsstool_array);\n"
         "\n", rsstool.output_file);

  fputs ("$rsstool_updated = \"", rsstool.output_file);
  strftime (buf, MAXBUFSIZE, "updated with <a href=\\\"http://rsstool.y7.ath.cx\\\">rsstool</a> " RSSTOOL_VERSION_S " %Y%m%d %H:%M:%S %Z\"", localtime (&t));
  fputs (buf, rsstool.output_file);

  fputs (";\n"
         "\n"
         "//echo \"<pre><tt>\";\n"
         "//echo $rsstool_updated;\n"  
         "\n?>", rsstool.output_file);
  return 0;
}


int
rsstool_write_bookmarks (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("<!DOCTYPE NETSCAPE-Bookmark-file-1>\n"
         "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n"
         "<TITLE>Bookmarks</TITLE>\n"
         "<H1>Bookmarks</H1>\n"
         "\n"
         "<DL><p>\n", rsstool.output_file);

  for (; i < items; i++)
    fprintf (rsstool.output_file, "    <DT><A HREF=\"%s\" ADD_DATE=\"%ld\">%s</A>\n", rt->item[i]->url, (unsigned long) rt->item[i]->date, rt->item[i]->title);

  fputs ("</DL><p>\n", rsstool.output_file);

  return 0;
}


int
rsstool_write_csv (st_rsstool_t *rt, int separator)
{
  char buf[MAXBUFSIZE];
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fprintf (rsstool.output_file, "SITE%cDATE%cURL%cTITLE%cDESC\n",
    separator, separator, separator, separator);

  for (; i < items; i++)
    {
      sprintf (buf, "%s", rt->item[i]->site);
      fprintf (rsstool.output_file, "\"%s\"%c", str_escape_code (buf), separator);
      sprintf (buf, "%ld", (unsigned long) rt->item[i]->date);
      fprintf (rsstool.output_file, "\"%s\"%c", str_escape_code (buf), separator);
      sprintf (buf, "%s", rt->item[i]->url);
      fprintf (rsstool.output_file, "\"%s\"%c", str_escape_code (buf), separator);
      sprintf (buf, "%s", rt->item[i]->title);
      fprintf (rsstool.output_file, "\"%s\"%c", str_escape_code (buf), separator);
      sprintf (buf, "%s", rt->item[i]->desc);
      fprintf (rsstool.output_file, "\"%s\"\n", str_escape_code (buf));
    }

  return 0;
}


int
rsstool_write_rss (st_rsstool_t *rt, int version)
{
  st_rss_t rss;
  int i = 0;

  memset (&rss, 0, sizeof (st_rss_t));

  rss.version = version;
  strcpy (rss.title, "RSStool");
  strcpy (rss.url, "http://rsstool.berlios.de");
  strcpy (rss.desc, "read, parse, merge and write RSS (and Atom) feeds");

  for (; i < rsstool_get_item_count (rt) && i < RSSMAXITEM; i++)
    {
      strncpy (rss.item[i].title, rt->item[i]->title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].url, rt->item[i]->url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].desc, rt->item[i]->desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      rss.item[i].date = rt->item[i]->date;
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
rsstool_write_ansisql (st_rsstool_t *rt)
{
  st_hash_t *dl_url_h = NULL;
  st_hash_t *url_h = NULL;
  st_hash_t *title_h = NULL;
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("-- -----------------------------------------------------------\n"
         "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
         "-- -----------------------------------------------------------\n"
         "\n"
         "-- DROP TABLE IF EXISTS `rsstool_table`;\n"
         "-- CREATE TABLE `rsstool_table` (\n"
         "--   `rsstool_url_md5` varchar(32) NOT NULL default '',\n"
         "--   `rsstool_url_crc32` int(10) unsigned NOT NULL default '0',\n"
         "--   `rsstool_site` text NOT NULL,\n"
         "--   `rsstool_dl_url` text NOT NULL,\n"
         "--   `rsstool_dl_url_md5` varchar(32) NOT NULL default '',\n"
         "--   `rsstool_dl_url_crc32` int(10) unsigned NOT NULL default '0',\n"
         "--   `rsstool_title` text NOT NULL,\n"
         "--   `rsstool_title_md5` varchar(32) NOT NULL default '',\n"
         "--   `rsstool_title_crc32` int(10) unsigned NOT NULL default '0',\n"
         "--   `rsstool_url` text NOT NULL,\n"
         "--   `rsstool_desc` text NOT NULL,\n"
         "--   `rsstool_date` bigint(20) unsigned NOT NULL default '0',\n"
         "--   `rsstool_dl_date` bigint(20) unsigned NOT NULL default '0',\n"
         "--   UNIQUE KEY `rsstool_url_crc32` (`rsstool_url_crc32`),\n"
         "--   UNIQUE KEY `rsstool_url_md5` (`rsstool_url_md5`),\n"
         "--   UNIQUE KEY `rsstool_title_crc32` (`rsstool_title_crc32`),\n"
         "--   UNIQUE KEY `rsstool_title_md5` (`rsstool_title_md5`),\n"
         "--   FULLTEXT KEY `rsstool_title` (`rsstool_title`),\n"
         "--   FULLTEXT KEY `rsstool_desc` (`rsstool_desc`)\n"
         "-- ) TYPE=MyISAM;\n"
         "\n", rsstool.output_file);

  for (; i < items; i++)
    {
      dl_url_h = hash_open (HASH_MD5|HASH_CRC32);
      url_h = hash_open (HASH_MD5|HASH_CRC32);
      title_h = hash_open (HASH_MD5|HASH_CRC32);

      dl_url_h = hash_update (dl_url_h, (const unsigned char *) rt->item[i]->feed_url, strlen (rt->item[i]->feed_url));
      url_h = hash_update (url_h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
      title_h = hash_update (title_h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      fprintf (rsstool.output_file,
               "INSERT IGNORE INTO `rsstool_table` ("
               " `rsstool_dl_url`, `rsstool_dl_url_md5`, `rsstool_dl_url_crc32`,"
               " `rsstool_dl_date`, `rsstool_site`,"
               " `rsstool_url`, `rsstool_url_md5`, `rsstool_url_crc32`,"
               " `rsstool_date`,"
               " `rsstool_title`, `rsstool_title_md5`, `rsstool_title_crc32`,"
               " `rsstool_desc`"
               ") VALUES ("
               " '%s', '%s', '%u',"
               " '%ld', '%s',"
               " '%s', '%s', '%u',"
               " '%ld',"
               " '%s', '%s', '%u',"
               " '%s'"
               ");\n",
        sql_stresc (rt->item[i]->feed_url),
        hash_get_s (dl_url_h, HASH_MD5),
        hash_get_crc32 (dl_url_h),
        time (0),
        sql_stresc (rt->item[i]->site),
        sql_stresc (rt->item[i]->url),
        hash_get_s (url_h, HASH_MD5),
        hash_get_crc32 (url_h),
        rt->item[i]->date,
        sql_stresc (rt->item[i]->title),
        hash_get_s (title_h, HASH_MD5),
        hash_get_crc32 (title_h),
        sql_stresc (rt->item[i]->desc));

      hash_close (dl_url_h);
      hash_close (url_h);
      hash_close (title_h);
    }

  return 0;
}


int
rsstool_write_ansisql_092 (st_rsstool_t *rt)
{
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ( "-- -----------------------------------------------------------\n"
          "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
          "-- -----------------------------------------------------------\n"
          "\n"
          "-- DROP TABLE rsstool_table;\n"
          "-- CREATE TABLE rsstool_table\n"
          "-- (\n"
          "--   rsstool_site      blob,\n"
          "--   rsstool_title     blob,\n"
          "--   rsstool_url       blob,\n"
          "--   rsstool_date      int(11),\n"
          "--   rsstool_desc      blob\n"
          "-- );\n"
          "\n", rsstool.output_file);

  for (; i < items; i++)
    {
      fprintf (rsstool.output_file, "INSERT IGNORE INTO rsstool_table (rsstool_site, rsstool_title, rsstool_url, rsstool_date, rsstool_desc) VALUES (\"%s\", \"%s\", \"%s\", %ld, \"%s\");\n",
        sql_stresc (rt->item[i]->site),
        sql_stresc (rt->item[i]->title),
        sql_stresc (rt->item[i]->url),
        rt->item[i]->date,
        sql_stresc (rt->item[i]->desc));
    }

  return 0;
}


int
rsstool_write_ansisql_094 (st_rsstool_t *rt)
{
  st_hash_t *h = NULL;
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("-- -----------------------------------------------------------\n"
         "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
         "-- -----------------------------------------------------------\n"
         "\n"
         "-- DROP TABLE IF EXISTS `rsstool_table`;\n"
         "-- CREATE TABLE IF NOT EXISTS `rsstool_table`\n"
         "-- (\n"
         "--   `rsstool_dl_url`     text,\n"
         "--   `rsstool_dl_date`    int(11),\n"
         "--   `rsstool_site`       text,\n"
         "--   `rsstool_url`        text,\n"
         "--   `rsstool_url_md5`    varchar(32),\n"
         "--   `rsstool_url_crc32`  varchar(8),\n"
         "--   `rsstool_date`       int(11),\n"
         "--   `rsstool_title`      text,\n"
         "--   `rsstool_desc`       text,\n"
         "--   UNIQUE KEY           `rsstool_url_md5` (`rsstool_url_md5`)\n"
         "-- );\n"
         "\n", rsstool.output_file);

  for (; i < items; i++)
    {
      h = hash_open (HASH_MD5|HASH_CRC32);
      h = hash_update (h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
//      h = hash_update (h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      fprintf (rsstool.output_file, "INSERT IGNORE INTO `rsstool_table` (`rsstool_dl_url`, `rsstool_dl_date`, `rsstool_site`, `rsstool_url`,"
              " `rsstool_url_md5`, `rsstool_url_crc32`, `rsstool_date`, `rsstool_title`, `rsstool_desc`)"
              " VALUES ('%s', '%ld', '%s', '%s', '%s', '%x', '%ld', '%s', '%s')"
              ";\n",
        sql_stresc (rt->item[i]->feed_url),
        time (0),
        sql_stresc (rt->item[i]->site),
        sql_stresc (rt->item[i]->url),
        hash_get_s (h, HASH_MD5),
        hash_get_crc32 (h),
        rt->item[i]->date,
        sql_stresc (rt->item[i]->title),
        sql_stresc (rt->item[i]->desc));

      hash_close (h);
    }

  return 0;
}


int
rsstool_write_ansisql_095 (st_rsstool_t *rt)
{
  st_hash_t *url_h = NULL;
  st_hash_t *title_h = NULL;
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("-- -----------------------------------------------------------\n"
         "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
         "-- -----------------------------------------------------------\n"
         "\n"
         "-- DROP TABLE IF EXISTS `rsstool_table`;\n"
         "-- CREATE TABLE IF NOT EXISTS `rsstool_table`\n"
         "-- (\n"
         "--   `rsstool_dl_url`     text,\n"
         "--   `rsstool_dl_date`    int(11),\n"
         "--   `rsstool_site`       text,\n"
         "--   `rsstool_url`        text,\n"
         "--   `rsstool_url_md5`    varchar(32),\n"
         "--   `rsstool_url_crc32`  varchar(8),\n"
         "--   `rsstool_date`       int(11),\n"
         "--   `rsstool_title`      text,\n"
         "--   `rsstool_title_md5`  varchar(32),\n"
         "--   `rsstool_title_crc32`varchar(8),\n"
         "--   `rsstool_desc`       text,\n"
         "--   UNIQUE KEY           `rsstool_url_md5` (`rsstool_url_md5`),\n"
         "--   UNIQUE KEY           `rsstool_url_crc32` (`rsstool_url_crc32`),\n"
         "--   UNIQUE KEY           `rsstool_title_md5` (`rsstool_title_md5`),\n"
         "--   UNIQUE KEY           `rsstool_title_crc32` (`rsstool_title_crc32`)\n"
         "-- );\n"
         "\n", rsstool.output_file);

  for (; i < items; i++)
    {
      url_h = hash_open (HASH_MD5|HASH_CRC32);
      title_h = hash_open (HASH_MD5|HASH_CRC32);

      url_h = hash_update (url_h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
      title_h = hash_update (title_h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      fprintf (rsstool.output_file,
               "INSERT IGNORE INTO `rsstool_table` (`rsstool_dl_url`, `rsstool_dl_date`, `rsstool_site`, `rsstool_url`,"
               " `rsstool_url_md5`, `rsstool_url_crc32`, `rsstool_date`, `rsstool_title`, `rsstool_title_md5`,"
               " `rsstool_title_crc32`, `rsstool_desc`)"
               " VALUES ('%s', '%ld', '%s', '%s', '%s', '%x', '%ld', '%s', '%s', '%x', '%s')"
               ";\n",
        sql_stresc (rt->item[i]->feed_url),
        time (0),
        sql_stresc (rt->item[i]->site),
        sql_stresc (rt->item[i]->url),
        hash_get_s (url_h, HASH_MD5),
        hash_get_crc32 (url_h),
        rt->item[i]->date,
        sql_stresc (rt->item[i]->title),
        hash_get_s (title_h, HASH_MD5),
        hash_get_crc32 (title_h),
        sql_stresc (rt->item[i]->desc));

      hash_close (url_h);
      hash_close (title_h);
    }

  return 0;
}


int
rsstool_write_ansisql_joomla (st_rsstool_t *rt)
{
  st_hash_t *url_h = NULL;
  st_hash_t *title_h = NULL;
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("-- -----------------------------------------------------------\n"
         "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
         "-- -----------------------------------------------------------\n"
         "\n"
         "-- DROP TABLE IF EXISTS `jos_content`;\n"
         "-- CREATE TABLE IF NOT EXISTS `jos_content`\n"
         "-- (\n"
         "--   `id` int(11) unsigned NOT NULL default '0',\n"
         "--   `title` varchar(100) NOT NULL default '',\n"
         "--   `title_alias` varchar(100) NOT NULL default '',\n"
         "--   `introtext` mediumtext NOT NULL,\n"
         "--   `fulltext` mediumtext NOT NULL,\n"
         "--   `state` tinyint(3) NOT NULL default '0',\n"
         "--   `sectionid` int(11) unsigned NOT NULL default '0',\n"
         "--   `mask` int(11) unsigned NOT NULL default '0',\n"
         "--   `catid` int(11) unsigned NOT NULL default '0',\n"
         "--   `created` datetime NOT NULL default '0000-00-00 00:00:00',\n"
         "--   `created_by` int(11) unsigned NOT NULL default '0',\n"
         "--   `created_by_alias` varchar(100) NOT NULL default '',\n"
         "--   `modified` datetime NOT NULL default '0000-00-00 00:00:00',\n"
         "--   `modified_by` int(11) unsigned NOT NULL default '0',\n"
         "--   `checked_out` int(11) unsigned NOT NULL default '0',\n"
         "--   `checked_out_time` datetime NOT NULL default '0000-00-00 00:00:00',\n"
         "--   `publish_up` datetime NOT NULL default '0000-00-00 00:00:00',\n"
         "--   `publish_down` datetime NOT NULL default '0000-00-00 00:00:00',\n"
         "--   `images` text NOT NULL,\n"
         "--   `urls` text NOT NULL,\n"
         "--   `attribs` text NOT NULL,\n"
         "--   `version` int(11) unsigned NOT NULL default '1',\n"
         "--   `parentid` int(11) unsigned NOT NULL default '0',\n"
         "--   `ordering` int(11) NOT NULL default '0',\n"
         "--   `metakey` text NOT NULL,\n"
         "--   `metadesc` text NOT NULL,\n"
         "--   `access` int(11) unsigned NOT NULL default '0',\n"
         "--   `hits` int(11) unsigned NOT NULL default '0',\n"
         "--   PRIMARY KEY  (`id`),\n"
         "--   KEY `idx_section` (`sectionid`),\n"
         "--   KEY `idx_access` (`access`),\n"
         "--   KEY `idx_checkout` (`checked_out`),\n"
         "--   KEY `idx_state` (`state`),\n"
         "--   KEY `idx_catid` (`catid`),\n"
         "--   KEY `idx_mask` (`mask`)\n"
         "-- );\n", rsstool.output_file);

  fputs ("\n"
         "-- This will add the necessary AUTO_INCREMENT to the `id` col in the `jos_content` table\n"
         "-- Uncomment this SQL command if you use RSStool for the first time to import feeds into the\n"
         "-- Joomla! CMS database\n"
         "-- ALTER TABLE `jos_content` CHANGE `id` `id` INT(11) UNSIGNED NULL AUTO_INCREMENT;\n", rsstool.output_file);

  for (; i < items; i++)
    {
      char buf[MAXBUFSIZE];
      char date_s[64];

      url_h = hash_open (HASH_MD5|HASH_CRC32);
      title_h = hash_open (HASH_MD5|HASH_CRC32);

      url_h = hash_update (url_h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
      title_h = hash_update (title_h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      // prepare url
      strncpy (buf, rt->item[i]->url, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
      strrep (buf, "http://", "");

      // prepare date YYYY-MM-DD HH:MM:SS  	
      strftime (date_s, 64, "%04Y-%02m-%02d %02H:%02M:%02S", localtime (&rt->item[i]->date));

      fprintf (rsstool.output_file,
               "INSERT IGNORE INTO `jos_content` ("
//               " `id`,"
               " `title`,"
               " `introtext`,"
               " `urls`,"
               " `state`,"
               " `sectionid`,"
               " `catid`,"
               " `created`,"
               " `publish_up`"
               ")"
//               " VALUES ('%u', '%s', '%s', '%s')"
               " VALUES ('%s', '%s', '%s', 1, 1, 1, '%s', '%s')"
               ";\n",
//        hash_get_crc32 (url_h),
        sql_stresc (rt->item[i]->title),
        sql_stresc (rt->item[i]->desc),
        buf,
        date_s,
        date_s);

      hash_close (url_h);
      hash_close (title_h);
    }

  return 0;
}


int
rsstool_write_ansisql_dragonfly (st_rsstool_t *rt)
{
  st_hash_t *url_h = NULL;
  st_hash_t *title_h = NULL;
  int items = rsstool_get_item_count (rt);
  int i = 0;

  fputs ("-- -----------------------------------------------------------\n"
         "-- RSStool - read, parse, merge and write RSS (and Atom) feeds\n"
         "-- -----------------------------------------------------------\n"
         "\n"
         "-- DROP TABLE IF EXISTS `jos_content`;\n"
         "-- CREATE TABLE IF NOT EXISTS `cms_stories`\n"
         "-- (\n"
         "--   `sid` int(10) unsigned NOT NULL auto_increment,\n"
         "--   `catid` int(11) NOT NULL default '0',\n"
         "--   `aid` varchar(30) NOT NULL default '',\n"
         "--   `title` varchar(80) default '',\n"
         "--   `time` int(11) NOT NULL default '1133334197',\n"
         "--   `hometext` text,\n"
         "--   `bodytext` text NOT NULL,\n"
         "--   `comments` int(11) default '0',\n"
         "--   `counter` mediumint(9) default '0',\n"
         "--   `topic` tinyint(4) NOT NULL default '1',\n"
         "--   `informant` varchar(40) NOT NULL default '',\n"
         "--   `notes` text NOT NULL,\n"
         "--   `ihome` tinyint(4) NOT NULL default '0',\n"
         "--   `alanguage` varchar(30) NOT NULL default '',\n"
         "--   `acomm` tinyint(4) NOT NULL default '0',\n"
         "--   `haspoll` tinyint(4) NOT NULL default '0',\n"
         "--   `poll_id` int(11) NOT NULL default '0',\n"
         "--   `score` int(11) NOT NULL default '0',\n"
         "--   `ratings` int(11) NOT NULL default '0',\n"
         "--   `associated` text NOT NULL,\n"
         "--   `display_order` tinyint(4) NOT NULL default '0',\n"
         "--   PRIMARY KEY  (`sid`),\n"
         "--   UNIQUE KEY `sid` (`sid`),\n"
         "--   KEY `catid` (`catid`),\n"
         "--   KEY `counter` (`counter`),\n"
         "--   KEY `topic` (`topic`)\n"
         "-- );\n", rsstool.output_file);

  for (; i < items; i++)
    {
      char buf[RSSTOOL_MAXBUFSIZE * 3];

      url_h = hash_open (HASH_MD5|HASH_CRC32);
      title_h = hash_open (HASH_MD5|HASH_CRC32);

      url_h = hash_update (url_h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
      title_h = hash_update (title_h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      // insert url into desc
      sprintf (buf, "<a href=\"%s\">", rt->item[i]->url);
      strcat (buf, sql_stresc (rt->item[i]->title));
      strcat (buf, "</a><br>");
      strcat (buf, sql_stresc (rt->item[i]->desc));

      fprintf (rsstool.output_file,
               "INSERT IGNORE INTO `cms_stories` ("
               " `title`,"
               " `time`,"
               " `hometext`,"
               " `ihome`"
               ")"
               " VALUES ('%s', '%ld', '%s', 1)"
               ";\n",
//        hash_get_crc32 (url_h),
        sql_stresc (rt->item[i]->title),
        rt->item[i]->date,
        buf);

      hash_close (url_h);
      hash_close (title_h);
    }

  return 0;
}


#if     (defined USE_MYSQL || defined USE_ODBC)
static int
rsstool_db_url_validate (st_strurl_t *url)
{
  if (!(*url->request))
    {
      fputs ("You have to specify a database (URL syntax: user:passwd@host:port/database)\n", stderr);
      return -1;
    }

  fputs ("Connecting to ", stderr);

  if (!(*url->user))
    strcpy (url->user, "admin");
  fputs (url->user, stderr);

  if (*url->pass)
    fprintf (stderr, ":%s", url->pass);

  if (!(*url->host))
    strcpy (url->host, "localhost");
  fprintf (stderr, "@%s", url->host);

  if (url->port < 1)
    url->port = 3306; // default
  fprintf (stderr, ":%d%s", url->port, url->request);
 
  fputs (" ... ", stderr);

  return 0;
}
#endif


#ifdef  USE_MYSQL
int
rsstool_write_mysql (st_rsstool_t *rt)
{
  st_sql_t *sql = NULL;
  st_strurl_t url;
  char buf[MAXBUFSIZE];

  strurl (&url, rt->dburl);

  if (rsstool_db_url_validate (&url) == -1)
    return -1;

  strtrim_s (url.request, "/", NULL);

  if (!(sql = sql_open (url.host, url.port, url.request, url.user, url.pass, SQL_MYSQL)))
    {
      fputs ("FAILED\n", stderr);
      return -1;
    }

  fputs ("OK\n", stderr);

  sql_query (sql, "DROP TABLE IF EXISTS rsstool_table");

  while (sql_gets (sql, buf, MAXBUFSIZE))
    {
      fputs (buf, stdout);
      fputc ('\n', stdout);
    }

  sql_close (sql);

  return 0;
}
#endif


#ifdef  USE_ODBC
int
rsstool_write_odbc (st_rsstool_t *rt)
{
  st_sql_t *sql = NULL;
  st_strurl_t url;

  strurl (&url, rt->dburl);

  if (rsstool_db_url_validate (&url) == -1)
    return -1;

  strtrim_s (url.request, "/", NULL);

  if (!(sql = sql_open (url.host, url.port, url.request, url.user, url.pass, SQL_ODBC)))
    {
      fputs ("FAILED\n", stderr);
      return -1;
    }

  fputs ("OK\n", stderr);

  sql_query (sql, "SELECT * FROM user");

  sql_close (sql);

  return 0;
}
#endif
