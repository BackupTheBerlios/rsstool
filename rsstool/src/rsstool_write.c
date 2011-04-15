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
#include "misc/base64.h"
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
  int items = rsstool_get_item_count (rt);

  memset (&rss, 0, sizeof (st_rss_t));

  rss.version = version;
  strcpy (rss.title, "RSStool");
  strcpy (rss.url, "http://rsstool.berlios.de");
  strcpy (rss.desc, "read, parse, merge and write RSS and Atom feeds");

  for (; i < items && i < RSSMAXITEM; i++)
    {
      strncpy (rss.item[i].title, rt->item[i]->title, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].url, rt->item[i]->url, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      strncpy (rss.item[i].desc, rt->item[i]->desc, RSSTOOL_MAXBUFSIZE)[RSSTOOL_MAXBUFSIZE - 1] = 0;
      rss.item[i].date = rt->item[i]->date;
      rss.item[i].media.duration = rt->item[i]->media_duration;
      rss.item_count++;
    }

  if (items >= RSSMAXITEM)
    {
      char buf[MAXBUFSIZE];

      sprintf (buf, "can write only RSS feeds with up to %d items (was %d items)\n",
        RSSMAXITEM, rsstool_get_item_count (rt));
      rsstool_log (rt, buf);
    }

  return rss_write (rsstool.output_file, &rss, version);
}


#if 0
//#ifdef  USE_XML2
int
rsstool_write_xml (st_rsstool_t *rt)
{
#define XMLPRINTF(s) xmlTextWriterWriteString(writer,BAD_CAST s)
  xmlTextWriterPtr writer;
  xmlBufferPtr buffer;
  char buf[RSSMAXBUFSIZE];
  unsigned int i = 0;

  if (!fp)
    return -1;

  if (!rss)
    return -1;

  if (!(buffer = xmlBufferCreate ()))
    return -1;

  if (!(writer = xmlNewTextWriterMemory (buffer, 0)))
    return -1;

  if (version != 1) // default to RSS 2.0
    version = 2;

  xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL);

//  xmlTextWriterWriteComment (writer, "comment");

  if (version == 2)
    {
      xmlTextWriterStartElement (writer, BAD_CAST "rss");  // <rss>

      xmlTextWriterWriteAttribute (writer, BAD_CAST "version", BAD_CAST "2.0");
    }
  else
    {
      xmlTextWriterStartElement (writer, BAD_CAST "rdf:RDF");  // <rdf:RDF>

      xmlTextWriterWriteAttribute (writer, BAD_CAST "xmlns", BAD_CAST "http://purl.org/rss/1.0/"); // specs?
    }

  XMLPRINTF("\n  ");

  xmlTextWriterStartElement (writer, BAD_CAST "channel"); // <channel>

  XMLPRINTF("\n    ");

  xmlTextWriterWriteElement (writer, BAD_CAST "title", BAD_CAST rss->title);

  XMLPRINTF("\n    ");

  xmlTextWriterWriteElement (writer, BAD_CAST "link", BAD_CAST rss->url);

  XMLPRINTF("\n    ");

  xmlTextWriterWriteElement (writer, BAD_CAST "description", BAD_CAST rss->desc);

#if 0
  XMLPRINTF("\n    ");

  xmlTextWriterWriteFormatElement (writer, BAD_CAST "dc:date", "%ld", BAD_CAST rss->date);
#endif

  if (version == 1)
    {
      XMLPRINTF("\n    ");

      xmlTextWriterStartElement (writer, BAD_CAST "items"); // <items>

      XMLPRINTF("\n      ");

      xmlTextWriterStartElement (writer, BAD_CAST "rdf:Seq"); // <rdf:Seq>

      for (i = 0; i < rss_item_count (rss); i++)
        {
          sprintf (buf, "\n        <rdf:li rdf:resource=\"%s\"/>", rss->item[i].url);
          XMLPRINTF(buf);
        }

      XMLPRINTF("\n      ");

      xmlTextWriterEndElement (writer); // </rdf:Seq>

      XMLPRINTF("\n    ");

      xmlTextWriterEndElement (writer); // </items>

      XMLPRINTF("\n  ");

      xmlTextWriterEndElement (writer); // </channel>
    }

  for (i = 0; i < rss_item_count (rss); i++)
    {
      XMLPRINTF("\n    ");

      xmlTextWriterStartElement (writer, BAD_CAST "item"); // <item>

      if (version == 1)
        xmlTextWriterWriteAttribute (writer, BAD_CAST "rdf:about", BAD_CAST rss->item[i].url);

      XMLPRINTF("\n      ");

      xmlTextWriterWriteElement (writer, BAD_CAST "title", BAD_CAST rss->item[i].title);

      XMLPRINTF("\n      ");

      xmlTextWriterWriteElement (writer, BAD_CAST "link", BAD_CAST rss->item[i].url);

      XMLPRINTF("\n      ");

      xmlTextWriterWriteElement (writer, BAD_CAST "description", BAD_CAST rss->item[i].desc);

      XMLPRINTF("\n      ");

//      xmlTextWriterWriteFormatElement (writer, BAD_CAST "dc:date", "%ld", rss->item[i].date);
      strftime (buf, RSSMAXBUFSIZE, "%a, %d %b %Y %H:%M:%S %Z", localtime (&rss->item[i].date));
      xmlTextWriterWriteElement (writer, BAD_CAST "pubDate", BAD_CAST buf);

      XMLPRINTF("\n      ");
#warning proper MRSS output
      xmlTextWriterWriteFormatElement (writer, BAD_CAST "media_duration", "%d", rss->item[i].media.duration);

      XMLPRINTF("\n    ");

      xmlTextWriterEndElement (writer); // </item>
    }

  if (version == 2)
    {
      XMLPRINTF("\n  ");

      xmlTextWriterEndElement (writer); // </channel>
    } 

  XMLPRINTF("\n");

  xmlTextWriterEndDocument (writer);  // </rss> or </rdf>

  xmlFreeTextWriter (writer);

  fputs ((const char *) buffer->content, fp);

  xmlBufferFree (buffer);

  return 0;
}
#else
int
rsstool_write_xml (st_rsstool_t *rt)
{
  st_rss_t rss;
  int i = 0;
  st_hash_t *dl_url_h = NULL;
  st_hash_t *url_h = NULL;
  st_hash_t *title_h = NULL;
  int items = rsstool_get_item_count (rt);
#define ENCODE(s) base64_enc(s,0)
//#define ENCODE(s) str_escape_xml(s)

  memset (&rss, 0, sizeof (st_rss_t));

  fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", rsstool.output_file);

  fputs ("<!--\n"
         "RSStool - read, parse, merge and write RSS and Atom feeds\n"
         "http://rsstool.berlios.de\n"
         "-->\n", rsstool.output_file);

  fputs ("<!--\n"
         "format:\n"
         "item[]\n"
         "  dl_url           \n"
         "  dl_url_md5\n"
         "  dl_url_crc32\n"
         "  dl_date\n"
         "  user             (base64 encoded)\n"
         "  site             (base64 encoded)\n"
         "  url              \n"
         "  url_md5\n"
         "  url_crc32\n"
         "  date\n"
         "  title            used by searches for related items (base64 encoded)\n"
         "  title_md5\n"
         "  title_crc32\n"
         "  desc             description (base64 encoded)\n"
         "  media_keywords   default: keywords from title and description\n"
         "  media_duration\n"
         "  media_thumbnail  path (base64 encoded)\n"
//         "  media_image      path (base64 encoded)\n"
//         "  event_start      default: date\n"
//         "  event_len        default: media_duration\n"
         "-->\n", rsstool.output_file);

  fputs ("<rsstool version=\"" RSSTOOL_VERSION_S "\">\n", rsstool.output_file);

  for (i = 0; i < items && i < RSSMAXITEM; i++)
//  for (i = 0; i < items; i++)
    {
      dl_url_h = hash_open (HASH_MD5|HASH_CRC32);
      url_h = hash_open (HASH_MD5|HASH_CRC32);
      title_h = hash_open (HASH_MD5|HASH_CRC32);

      dl_url_h = hash_update (dl_url_h, (const unsigned char *) rt->item[i]->feed_url, strlen (rt->item[i]->feed_url));
      url_h = hash_update (url_h, (const unsigned char *) rt->item[i]->url, strlen (rt->item[i]->url));
      title_h = hash_update (title_h, (const unsigned char *) rt->item[i]->title, strlen (rt->item[i]->title));

      fprintf (rsstool.output_file,
               "  <item>\n"
               "    <dl_url>%s</dl_url>\n"
               "    <dl_url_md5>%s</dl_url_md5>\n"
               "    <dl_url_crc32>%u</dl_url_crc32>\n"
               "    <dl_date>%ld</dl_date>\n"
               "    <user>%s</user>\n"
               "    <site>%s</site>\n"
               "    <url>%s</url>\n"
               "    <url_md5>%s</url_md5>\n"
               "    <url_crc32>%u</url_crc32>\n"
               "    <date>%ld</date>\n"
               "    <title>%s</title>\n"
               "    <title_md5>%s</title_md5>\n"
               "    <title_crc32>%u</title_crc32>\n"
               "    <desc>%s</desc>\n"
               "    <media_keywords>%s</media_keywords>\n"
               "    <media_duration>%d</media_duration>\n"
               "    <media_thumbnail>%s</media_thumbnail>\n"
//               "    <media_image>%s</media_image>\n"
//               "    <event_start>%u</event_start>\n"
//               "    <event_len>%u</event_len>\n"
               "  </item>\n",
        str_escape_xml (rt->item[i]->feed_url),
        hash_get_s (dl_url_h, HASH_MD5),
        hash_get_crc32 (dl_url_h),
        time (0),
        ENCODE (rt->item[i]->user),
        ENCODE (rt->item[i]->site),
        str_escape_xml (rt->item[i]->url),
        hash_get_s (url_h, HASH_MD5),
        hash_get_crc32 (url_h),
        rt->item[i]->date,
        ENCODE (rt->item[i]->title),
        hash_get_s (title_h, HASH_MD5),
        hash_get_crc32 (title_h),
        ENCODE (rt->item[i]->desc),
        ENCODE (rt->item[i]->media_keywords),
        rt->item[i]->media_duration,
        str_escape_xml (rt->item[i]->media_thumbnail)
//        str_escape_xml (rt->item[i]->media_image),
//        rt->item[i]->event_start,
//        rt->item[i]->event_len
);

      hash_close (dl_url_h);
      hash_close (url_h);
      hash_close (title_h);
    }

  fputs ("</rsstool>\n", rsstool.output_file);

  if (items >= RSSMAXITEM)
    {
      char buf[MAXBUFSIZE];

      sprintf (buf, "can write only RSS feeds with up to %d items (was %d items)\n",
        RSSMAXITEM, items);
      rsstool_log (rt, buf);
    }

  return 0;
}
#endif  // USE_XML2


