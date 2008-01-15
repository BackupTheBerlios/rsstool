/*
rss.c - RSS (and Atom) parser and generator (using libxml2)

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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "misc.h"
#include "xml.h"
#include "rss.h"


#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


#define RSS_V0_91_S "RSS v0.91"
#define RSS_V0_92_S "RSS v0.92"
#define RSS_V0_93_S "RSS v0.93"
#define RSS_V0_94_S "RSS v0.94"
#define RSS_V1_0_S  "RSS v1.0"
#define RSS_V2_0_S  "RSS v2.0"
#define ATOM_V0_1_S "ATOM v0.1"
#define ATOM_V0_2_S "ATOM v0.2"
#define ATOM_V0_3_S "ATOM v0.3"


//#ifdef  DEBUG
void
rss_st_rss_t_sanity_check (st_rss_t *rss)
{
  int i = 0;

  for (; i < rss->item_count; i++)
    printf ("pos:%d\n"
            "title: %s\n"
            "url: %s\n"
            "date: %ld\n"
            "desc: %s\n\n",
      i,
      rss->item[i].title,
      rss->item[i].url,
      rss->item[i].date,
      rss->item[i].desc);

  printf ("rss->item_count: %d\n\n", rss->item_count);
}
//#endif


#if 0
char *
rss_utf8_enc (const char *in, const char *encoding)
{
  static xmlChar out[RSSMAXBUFSIZE * 2];
  int temp = 0;
  int size = 0;
  xmlCharEncodingHandlerPtr handler;

  if (!in)
    return NULL;

  handler = xmlFindCharEncodingHandler (encoding);

  if (!handler)
    return NULL;

  temp = strlen (in);
  size = sizeof (out);

  handler->input (out, &size, in, &temp);

  return (char *) out;
}
#endif


typedef struct
{
  int version;
  const char *version_s;

  const char *magic_s;
} st_rss_version_t;


static st_rss_version_t rss_version[] = {
  {RSS_V0_91, RSS_V0_91_S, "0.91"},
  {RSS_V0_92, RSS_V0_92_S, "0.92"},
  {RSS_V0_93, RSS_V0_93_S, "0.93"},
  {RSS_V0_94, RSS_V0_94_S, "0.94"},
  {RSS_V1_0,  RSS_V1_0_S,  "1.0"},
  {RSS_V2_0,  RSS_V2_0_S,  "2.0"},
  {RSS_V2_0,  RSS_V2_0_S,  "2"},
  {ATOM_V0_1, ATOM_V0_1_S, "0.1"},
  {ATOM_V0_2, ATOM_V0_2_S, "0.2"},
  {ATOM_V0_3, ATOM_V0_3_S, "0.3"},
  {0, NULL, NULL}
};


const char *
rss_get_version_s_by_magic (const char *m)
{
  int i = 0;

  for (; rss_version[i].version; i++)
    if (!strcmp (rss_version[i].magic_s, m))
      return rss_version[i].version_s;
  return NULL;
}


const char *
rss_get_version_s_by_id (int version)
{
  int i = 0;

  for (; rss_version[i].version; i++)
    if (rss_version[i].version == version)
      return rss_version[i].version_s;
  return NULL;
}

const char *
rss_get_version_s (st_rss_t * rss)
{
  return rss_get_version_s_by_id (rss->version);
}


unsigned int
rss_item_count (st_rss_t * rss)
{
  return rss->item_count;
}


st_rss_item_t *
rss_get_item (st_rss_t * rss, unsigned int n)
{
  return &rss->item[n];
}


#if 0
void
rsstool_set_version (const char *version)
{
  const st_rss_version_t *v = NULL;

  if (!(*version))
    fprintf (stderr, "WARNING: no version in %d\n", rsstool.article_count);

  v = rsstool_get_rss_version_by_magic (version);
  if (v)
    rsstool.rss[rsstool.article_count].version = v->version;

#ifdef  DEBUG
  printf ("link[%d]: %s\n", rsstool.article_count, rsstool.rss[rsstool.article_count].version);
  fflush (stdout);
#endif
}


void
rsstool_set_site (const char *site)
{
  if (!(*site))
    fprintf (stderr, "WARNING: no site discovered\n");

  strncpy (rsstool.rss[rsstool.article_count].site, site, RSSMAXBUFSIZE)[RSSMAXBUFSIZE - 1] = 0;

#ifdef  DEBUG
  printf ("site[%d]: %s\n", rsstool.article_count, rsstool.rss[rsstool.article_count].site);
  fflush (stdout);
#endif
}
#endif


int
rss_demux (const char *fname)
{
  xml_doc_t *doc = NULL;
  xml_node_t *node = NULL;
  int version = -1;
  char *p = NULL;

  if (!(doc = xml_parse (fname)))
    {
      fprintf (stderr, "ERROR: cannot read %s\n", fname);
      return -1;
    }

  node = xml_get_rootnode (doc);

  if (!node)
    return -1;

  if (!xml_get_name (node))
    return -1;

#ifdef  DEBUG
  printf ("%s\n", xml_get_name (node));
  fflush (stdout);
#endif

  if (!strcmp (xml_get_name (node), "html")) // not xml
    return -1;

  if (!strcmp (xml_get_name (node), "feed")) // atom
    {
      version = ATOM_V0_1; // default

      if (!(p = (char *) xml_get_value (node, "version")))
        return version;

      if (!strcmp (p, "0.3"))
        version = ATOM_V0_3;
      else if (!strcmp (p, "0.2"))
        version = ATOM_V0_2;
//      else if (!strcmp (p, "0.1"))
//        version = ATOM_V0_1;

      return version;
    }
  else if (!strcmp (xml_get_name (node), "rss")) // rss
    {
      if (!(p = (char *) xml_get_value (node, "version")))
        return -1;

      if (!strcmp (p, "0.91"))
        version = RSS_V0_91;
      else if (!strcmp (p, "0.92"))
        version = RSS_V0_92;
      else if (!strcmp (p, "0.93"))
        version = RSS_V0_93;
      else if (!strcmp (p, "0.94"))
        version = RSS_V0_94;
      else if (!strcmp (p, "2") || !strcmp (p, "2.0") || !strcmp (p, "2.00"))
        version = RSS_V2_0;

      return version;
    }
  else if (!strcmp (xml_get_name (node), "rdf") ||
           !strcmp (xml_get_name (node), "RDF")) // rdf
    {
#if 0
      if (!(p = xml_get_value (node, "xmlns")))
        return -1;

      // hack
      if (!strcmp (p, "http://my.netscape.com/rdf/simple/0.9/"))
        version = RSS_V0_90;
      else if (!strcmp (p, "http://purl.org/rss/1.0/"))
        version = RSS_V1_0;
#else
      version = RSS_V1_0;
#endif

      return version;
    }

  return -1;
}


static void
rss_read_copy (char *d, xml_doc_t* doc, xml_node_t* n)
{
#ifndef _WIN32
  (void) doc;
#endif
  const char *p = (const char *) xml_get_string (n);

  if (p)
    strncpy (d, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
  else
    *d = 0;
}



static st_rss_t *
rss_open_rss (st_rss_t *rss)
{
  xml_doc_t *doc;
  xml_node_t *node;
  int rdf = 0;

  doc = xml_parse (rss->url);
  if (!doc)
    {
      fprintf (stderr, "ERROR: cannot read %s\n", rss->url);
      return NULL;
    }

  node = xml_get_rootnode (doc);
  if (!node)
    {
      fprintf (stderr, "ERROR: empty document %s\n", rss->url);
      xml_free (doc);
      return NULL;
    }

  // rdf?
  // TODO: move this to rss_demux()
  if (strcmp (xml_get_name (node), "rss") != 0 &&
      (!strcmp (xml_get_name (node), "rdf") ||
       !strcmp (xml_get_name (node), "RDF")))
    rdf = 1;

  node = xml_get_childnode (node);
  while (node && xml_is_empty_node (node))
    node = xml_get_nextnode (node);

  if (!node)
    {
//      fprintf (stderr, "");
      return NULL;
    }

  if (strcmp (xml_get_name (node), "channel"))
    {
      fprintf (stderr, "ERROR: bad document: did not immediately find the RSS element\n");
      return NULL;
    }

  if (!rdf) // document is RSS
    node = xml_get_childnode (node);

  while (node)
    {
      while (node && xml_is_empty_node (node))
        node = xml_get_nextnode (node);

      if (!node)
        break;

      if (!strcmp (xml_get_name (node), "title"))
        rss_read_copy (rss->title, doc, xml_get_childnode (node));
      else if (!strcmp (xml_get_name (node), "description"))
        rss_read_copy (rss->desc, doc, xml_get_childnode (node));
//      else if (!strcmp (xml_get_name (node), "link"))
//        rss_read_copy (rss->url, doc, xml_get_childnode (node));
      else if (!strcmp (xml_get_name (node), "date") ||
               !strcmp (xml_get_name (node), "pubDate") ||
               !strcmp (xml_get_name (node), "dc:date"))
        rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (node)));
      else if (!strcmp (xml_get_name (node), "channel") && rdf)
        {
          xml_node_t *pnode = xml_get_childnode (node);

          while (pnode)
            {
              if (!strcmp (xml_get_name (pnode), "title"))
                rss_read_copy (rss->title, doc, xml_get_childnode (pnode));
              else if (!strcmp (xml_get_name (pnode), "description"))
                rss_read_copy (rss->desc, doc, xml_get_childnode (pnode));
              else if (!strcmp (xml_get_name (pnode), "date") ||
                       !strcmp (xml_get_name (pnode), "pubDate") ||
                       !strcmp (xml_get_name (pnode), "dc:date"))
                rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));

              pnode = xml_get_nextnode (pnode);
            }

        }
      else if (!strcmp (xml_get_name (node), "item") || !strcmp (xml_get_name (node), "entry"))
        {
          xml_node_t *pnode = xml_get_childnode (node);
          st_rss_item_t *item = &rss->item[rss->item_count];
          int found = 0;
//          const char *p = NULL;
          char link[RSSMAXBUFSIZE], guid[RSSMAXBUFSIZE];

          *link = *guid = 0;

          while (pnode)
            {
              while (pnode && xml_is_empty_node (pnode))
                pnode = xml_get_nextnode (pnode);

              if (!pnode)
                break;

#ifdef  DEBUG
              printf ("%s\n", xml_get_name (pnode));
              fflush (stdout);
#endif

              if (!strcmp (xml_get_name (pnode), "title"))
                {
                  rss_read_copy (item->title, doc, xml_get_childnode (pnode));
                  found = 1;
                }
              else if (!strcmp (xml_get_name (pnode), "link"))
                {
                  rss_read_copy (link, doc, xml_get_childnode (pnode));
                  found = 1;
                }
#if 0
              else if (!strcmp (xml_get_name (pnode), "enclosure"))
                {
                  p = (const char *) xml_get_value (pnode, "url");
                  if (p)
                    {
                      strncpy (link, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
                      found = 1;
                    }
                }
#endif
              else if (!strcmp (xml_get_name (pnode), "guid") && (!(*link)))
                {
                  rss_read_copy (guid, doc, xml_get_childnode (pnode));
                  found = 1;
                }
              else if (!strcmp (xml_get_name (pnode), "description"))
                {
                  rss_read_copy (item->desc, doc, xml_get_childnode (pnode));
                  found = 1;
                }
              else if (!stricmp (xml_get_name (pnode), "date") ||
                       !stricmp (xml_get_name (pnode), "pubDate") ||
                       !stricmp (xml_get_name (pnode), "dc:date") ||
                       !stricmp (xml_get_name (pnode), "cropDate"))
                { 
                  item->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));
                  found = 1;
                }
#if 0
              else
                {
                  if (!found) // possibly malformed feed
                    break;
                  else
                    found = 0;
                }
#endif

              pnode = xml_get_nextnode (pnode);
            }

          // some feeds use the guid tag for the link
          if (*link)
            strcpy (item->url, link);
          else if (*guid)
            strcpy (item->url, guid);
          else
            *(item->url) = 0;

          rss->item_count++;

          if (rss->item_count == RSSMAXITEM)
            break;
        }

//      rss->item_count++;

      node = xml_get_nextnode (node);
    }

#ifdef  DEBUG
  rss_st_rss_t_sanity_check (rss);
  fflush (stdout);
#endif

  return rss;
}


static st_rss_t *
rss_open_atom (st_rss_t *rss)
{
  xml_doc_t *doc;
  xml_node_t *node;
  const char *p = NULL;

  doc = xml_parse (rss->url);
  if (!doc)
    {
      fprintf (stderr, "ERROR: cannot read %s\n", rss->url);
      return NULL;
    }

  node = xml_get_rootnode (doc);
  if (!node)
    {
      fprintf (stderr, "ERROR: empty document %s\n", rss->url);
      xml_free (doc);
      return NULL;
    }

  node = xml_get_childnode (node);
  while (node && xml_is_empty_node (node))
    node = node->next;
  if (!node)
    {
//      fprintf (stderr, "");
      return NULL;
    }

  while (node)
    {
      while (node && xml_is_empty_node (node))
        node = node->next;

      if (!node)
        break;

      if (!strcmp (xml_get_name (node), "title"))
        rss_read_copy (rss->title, doc, xml_get_childnode (node));
      else if (!strcmp (xml_get_name (node), "description"))
        rss_read_copy (rss->desc, doc, xml_get_childnode (node));
//      else if (!strcmp (xml_get_name (node), "link"))
//        rss_read_copy (rss->url, doc, xml_get_childnode (node));
      else if (!strcmp (xml_get_name (node), "date") ||
               !strcmp (xml_get_name (node), "pubDate") ||
               !strcmp (xml_get_name (node), "dc:date") ||
               !strcmp (xml_get_name (node), "modified") ||
               !strcmp (xml_get_name (node), "updated"))
        rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (node)));
      else if ((!strcmp (xml_get_name (node), "entry")))
        {
          xml_node_t *pnode = xml_get_childnode (node);
          st_rss_item_t *item = &rss->item[rss->item_count];
          int found = 0;
          char link[RSSMAXBUFSIZE];

          *link = 0;

          while (pnode)
            {
              while (pnode && xml_is_empty_node (pnode))
                pnode = pnode->next;

              if (!pnode)
                break;

#ifdef  DEBUG
              printf ("%s\n", xml_get_name (pnode));
              fflush (stdout);
#endif

              if (!strcmp (xml_get_name (pnode), "title"))
                {
                  rss_read_copy (item->title, doc, xml_get_childnode (pnode));
                  found = 1;
                }
#if 0
              else if (!strcmp (xml_get_name (pnode), "id"))
                {
                  rss_read_copy (item->url, doc, xml_get_childnode (pnode));
                  found = 1;
                }
#endif
              else if (!strcmp (xml_get_name (pnode), "link") && (!(*link)))
                {
#if 0
<link rel="alternate" type="text/html" href="http://edition.cnn.com/2006/POLITICS/11/01/kerry.remarks/"/>
#endif
                  p = (const char *) xml_get_value (pnode, "href");
                  if (p)
                    {
                      strncpy (link, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
                      found = 1;
                    }
                }
              else if (!strcmp (xml_get_name (pnode), "content"))
                {
                  rss_read_copy (item->desc, doc, xml_get_childnode (pnode));
                  found = 1;
                }
              else if (!strcmp (xml_get_name (pnode), "modified") ||
                       !strcmp (xml_get_name (pnode), "updated"))
                { 
                  item->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));
                  found = 1;
                }

              pnode = pnode->next;
            }

          if (*link)
            strcpy (item->url, link);

          rss->item_count++;

          if (rss->item_count == RSSMAXITEM)
            break;
        }

//      rss->item_count++;

      node = node->next;
    }

#ifdef  DEBUG
  rss_st_rss_t_sanity_check (rss);
  fflush (stdout);
#endif

  return rss;
}


st_rss_t *
rss_open (const char *fname)
{
  st_rss_t *rss = NULL;

  if (!(rss = malloc (sizeof (st_rss_t))))
    return NULL;

  memset (rss, 0, sizeof (st_rss_t));

  strncpy (rss->url, fname, RSSMAXBUFSIZE)[RSSMAXBUFSIZE - 1] = 0;
  rss->item_count = 0;

  rss->version = rss_demux (fname);

#ifdef  DEBUG
  fprintf (stderr, "version: %s\n", rss_get_version_s_by_id (rss->version));
  fflush (stderr);
#endif

  if (rss->version == -1)
    {
      fprintf (stderr, "ERROR: unknown feed format %s\n", rss->url);
      return NULL;
    }

  switch (rss->version)
    {
      case ATOM_V0_1:
      case ATOM_V0_2:
      case ATOM_V0_3:
        return rss_open_atom (rss);
      default:
        return rss_open_rss (rss);
    }

  free (rss);
  rss = NULL;

  return NULL;
}


int
rss_close (st_rss_t *rss)
{
  if (rss)
    {
      free (rss);
      rss = NULL;
    }

  return 0;
}


#if 0
int
rss_write (FILE *fp, st_rss_t *rss, int version)
{
// TODO: escape html code in desc
  unsigned int i = 0;

  if (!fp)
    return -1;

  if (!rss)
    return -1;

  if (version != 1) // default to RSS 2.0
    version = 2;

  fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);

  if (version == 1)
    fputs ("<rdf:RDF xmlns=\"http://purl.org/rss/1.0/\">\n", fp);
  else
    fputs ("<rss version=\"2.0\">\n", fp);

  fputs ("  <channel>\n"
         "    <title>RSStool</title>\n"
         "    <link>http://rsstool.berlios.de</link>\n"
         "    <description>read, parse, merge and write RSS (and Atom) feeds</description>\n"
//         "    <dc:date>%ld</dc:date>"
         , fp);


  if (version == 1)
    {
      fputs ("<items>\n"
             "<rdf:Seq>\n", fp);

      for (i = 0; i < rss_item_count (rss); i++)
        fprintf (fp, "\n        <rdf:li rdf:resource=\"%s\"/>", rss->item[i].url);

      fputs ("</rdf:Seq>\n"
             "</items>\n"
             "</channel>\n", fp);
    }

  for (i = 0; i < rss_item_count (rss); i++)
    {
      if (version == 1)
        fprintf (fp, "<item rdf:about=\"%s\">\n", rss->item[i].url);
      else
        fputs ("    <item>\n", fp);

      fprintf (fp, "      <title>%s</title>\n", rss->item[i].title);
      fprintf (fp, "      <link>%s</link>\n", rss->item[i].url);
      fprintf (fp, "      <description>%s</description>\n", rss->item[i].desc);
      fprintf (fp, "      <dc:date>%ld</dc:date>\n", rss->item[i].date);

      fputs ("    </item>\n", fp);
    }

  if (version == 2)
    fputs ("  </channel>\n", fp);

  fputs ("</rss>\n", fp);

  return 0;
}
#else
//#include <libxml/parser.h>
//#include <libxml/tree.h>
#include <libxml/xmlwriter.h>


int
rss_write (FILE *fp, st_rss_t *rss, int version)
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
#endif
