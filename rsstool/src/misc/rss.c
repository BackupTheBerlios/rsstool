/*
rss.c - (M)RSS and Atom parser and generator (using libxml2)

Copyright (c) 2006-2010 NoisyB


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
//#include <libxml/parser.h>
//#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include "misc.h"
#include "string.h"
#include "xml.h"
#include "rss.h"


#define RSS_V0_91_S "RSS v0.91"
#define RSS_V0_92_S "RSS v0.92"
#define RSS_V0_93_S "RSS v0.93"
#define RSS_V0_94_S "RSS v0.94"
#define RSS_V1_0_S  "RSS v1.0"
#define RSS_V2_0_S  "RSS v2.0"
#define ATOM_V0_1_S "Atom v0.1"
#define ATOM_V0_2_S "Atom v0.2"
#define ATOM_V0_3_S "Atom v0.3"
#define ATOM_V1_0_S "Atom v1.0"
#define MRSS_V1_0_0_S "MRSS v1.0.0"
#define MRSS_V1_1_0_S "MRSS v1.1.0"
#define MRSS_V1_1_1_S "MRSS v1.1.1"
#define MRSS_V1_1_2_S "MRSS v1.1.2"
#define MRSS_V1_5_0_S "MRSS v1.5.0"


#ifdef  DEBUG
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
#endif


char *
rss_utf8_enc (const unsigned char *in, const char *encoding)
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

  temp = strlen ((const char *) in);
  size = sizeof (out);

  handler->input (out, &size, in, &temp);

  return (char *) out;
}


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
  {ATOM_V1_0, ATOM_V1_0_S, "1.0"},

// http://video.search.yahoo.com/mrss
  {MRSS_V1_0_0, MRSS_V1_0_0_S, "1.0.0"},
  {MRSS_V1_1_0, MRSS_V1_1_0_S, "1.1.0"},
  {MRSS_V1_1_1, MRSS_V1_1_1_S, "1.1.1"},
  {MRSS_V1_1_2, MRSS_V1_1_2_S, "1.1.2"},
  {MRSS_V1_5_0, MRSS_V1_5_0_S, "1.5.0"},
  {0, NULL, NULL}
};


const char *
rss_get_version_s_by_magic (const char *m)
{
  int i = 0;

  for (; rss_version[i].version; i++)
    if (!strcasecmp (rss_version[i].magic_s, m))
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
rss_demux (const char *fname, const char *encoding)
{
  xml_doc_t *doc = NULL;
  xml_node_t *node = NULL;
  int version = -1;
  char *p = NULL;

  if (!(doc = xml_parse (fname, encoding)))
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

  if (!strcasecmp (xml_get_name (node), "html")) // not xml
    return -1;

  if (!strcasecmp (xml_get_name (node), "feed")) // atom
    {
      version = ATOM_V0_1; // default

      if (!(p = (char *) xml_get_value (node, "version")))
        return version;

      if (!strcasecmp (p, "1.0"))
        version = ATOM_V1_0;
      else if (!strcasecmp (p, "0.3"))
        version = ATOM_V0_3;
      else if (!strcasecmp (p, "0.2"))
        version = ATOM_V0_2;
//      else if (!strcasecmp (p, "0.1"))
//        version = ATOM_V0_1;

      return version;
    }
  else if (!strcasecmp (xml_get_name (node), "rss")) // rss
    {
      version = RSS_V2_0; // default

      if (!(p = (char *) xml_get_value (node, "version")))
        return version;

      if (!strcasecmp (p, "0.91"))
        version = RSS_V0_91;
      else if (!strcasecmp (p, "0.92"))
        version = RSS_V0_92;
      else if (!strcasecmp (p, "0.93"))
        version = RSS_V0_93;
      else if (!strcasecmp (p, "0.94"))
        version = RSS_V0_94;
      else if (!strcasecmp (p, "2") || !strcasecmp (p, "2.0") || !strcasecmp (p, "2.00"))
        version = RSS_V2_0;

      return version;
    }
  else if (!strcasecmp (xml_get_name (node), "rdf"))
    {
#if 0
      if (!(p = xml_get_value (node, "xmlns")))
        return -1;

      // hack
      if (!strcasecmp (p, "http://my.netscape.com/rdf/simple/0.9/"))
        version = RSS_V0_90;
      else if (!strcasecmp (p, "http://purl.org/rss/1.0/"))
        version = RSS_V1_0;
#else
      version = RSS_V1_0;
#endif

      return version;
    }

  return -1;
}


static void
rss_read_copy (char *d, xml_node_t* n)
{
  const char *p = (const char *) xml_get_string (n);

  if (p)
    strncpy (d, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
  else
    *d = 0;
}


                static void
                rss_open_rss_mrss (xml_node_t *pnode, st_rss_item_t *item)
                {
                  const char *p = NULL;
                  xml_node_t *tnode = xml_get_childnode (pnode); 
                  while (tnode)
                    {
                      if (!tnode)
                        break;

//                      if (!strcasecmp (xml_get_name (tnode), "content")) // media:content
                      if (stristr (xml_get_name (tnode), "content")) // media:content
                        {
                          p = (const char *) xml_get_value (tnode, "duration");
                          if (p)
                            {
                              item->media.duration = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "filesize");
                          if (p)
                            {
                              item->media.filesize = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "width");
                          if (p)
                            {
                              item->media.width = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "height");
                          if (p)
                            {
                              item->media.height = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                        }
//                      else if (!strcasecmp (xml_get_name (tnode), "keywords")) // media:keywords
                      else if (stristr (xml_get_name (tnode), "keywords")) // media:keywords
                        {
                          rss_read_copy (item->media.keywords, xml_get_childnode (tnode));
                        }
//                      else if (!strcasecmp (xml_get_name (tnode), "thumbnail")) // media:thumbnail
                      else if (stristr (xml_get_name (tnode), "thumbnail")) // media:thumbnail
                        {
                          p = (const char *) xml_get_value (tnode, "url");
                          if (p)
                            if (!(item->media.thumbnail[0]))
                            {
                              strncpy (item->media.thumbnail, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
//                              found = 1;
//                              break;  
                            }
                        }
                      tnode = xml_get_nextnode (tnode);
                    }
                }


static st_rss_t *
rss_open_rss (st_rss_t *rss, const char *encoding)
{
  xml_doc_t *doc;
  xml_node_t *node;
  int rdf = 0;

  doc = xml_parse (rss->url, encoding);
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
  if (strcasecmp (xml_get_name (node), "rss") != 0 &&
      !strcasecmp (xml_get_name (node), "rdf"))
    rdf = 1;

  node = xml_get_childnode (node);
  while (node && xml_is_empty_node (node))
    node = xml_get_nextnode (node);

  if (!node)
    {
//      fprintf (stderr, "");
      return NULL;
    }

  if (strcasecmp (xml_get_name (node), "channel"))
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

      if (!strcasecmp (xml_get_name (node), "title"))
        rss_read_copy (rss->title, xml_get_childnode (node));
      else if (!strcasecmp (xml_get_name (node), "description"))
        rss_read_copy (rss->desc, xml_get_childnode (node));
//      else if (!strcasecmp (xml_get_name (node), "link"))
//        rss_read_copy (rss->url, xml_get_childnode (node));
      else if (!strcasecmp (xml_get_name (node), "date") ||
               !strcasecmp (xml_get_name (node), "pubDate") ||
               !strcasecmp (xml_get_name (node), "dc:date"))
        rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (node)));
      else if (!strcasecmp (xml_get_name (node), "channel") && rdf)
        {
          xml_node_t *pnode = xml_get_childnode (node);

          while (pnode)
            {
              if (!strcasecmp (xml_get_name (pnode), "title"))
                rss_read_copy (rss->title, xml_get_childnode (pnode));
              else if (!strcasecmp (xml_get_name (pnode), "description"))
                rss_read_copy (rss->desc, xml_get_childnode (pnode));
              else if (!strcasecmp (xml_get_name (pnode), "date") ||
                       !strcasecmp (xml_get_name (pnode), "pubDate") ||
                       !strcasecmp (xml_get_name (pnode), "dc:date"))
                rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));

              pnode = xml_get_nextnode (pnode);
            }

        }
      else if (!strcasecmp (xml_get_name (node), "item") || !strcasecmp (xml_get_name (node), "entry"))
        {
          xml_node_t *pnode = xml_get_childnode (node);
          st_rss_item_t *item = &rss->item[rss->item_count];
//          int found = 0;
          const char *p = NULL;
          char link[RSSMAXBUFSIZE], guid[RSSMAXBUFSIZE];

printf ("SHIT");
fflush (stdout);
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

              if (!strcasecmp (xml_get_name (pnode), "title"))
                {
                  rss_read_copy (item->title, xml_get_childnode (pnode));
//                  found = 1;
                }
              else if (!strcasecmp (xml_get_name (pnode), "link"))
                {
                  rss_read_copy (link, xml_get_childnode (pnode));
//                  found = 1;
                }
#if 0
              else if (!strcasecmp (xml_get_name (pnode), "enclosure"))
                {
                  p = (const char *) xml_get_value (pnode, "url");
                  if (p)
                    {
                      strncpy (link, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
//                      found = 1;
                    }
                }
#endif
              else if (!strcasecmp (xml_get_name (pnode), "guid") && (!(*link)))
                {
                  rss_read_copy (guid, xml_get_childnode (pnode));
//                  found = 1;
                }
              else if (!strcasecmp (xml_get_name (pnode), "description"))
                {
                  rss_read_copy (item->desc, xml_get_childnode (pnode));
//                  found = 1;
                }
              else if (!strcasecmp (xml_get_name (pnode), "date") ||
                       !strcasecmp (xml_get_name (pnode), "pubDate") ||
                       !strcasecmp (xml_get_name (pnode), "dc:date") ||
                       !strcasecmp (xml_get_name (pnode), "cropDate"))
                { 
                  item->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));
//                  found = 1;
                }
//              else if (!strcasecmp (xml_get_name (pnode), "duration")) // HACK yt:duration
              else if (stristr (xml_get_name (pnode), "duration")) // HACK yt:duration
                {
                  p = (const char *) xml_get_value (pnode, "seconds");
                  if (p)
                    {
                      item->media.duration = strtol (p, NULL, 10);
//                      found = 1;
//                      break;
                    }
                }
//              else if (!strcasecmp (xml_get_name (pnode), "group")) // media:group
              else if (stristr (xml_get_name (pnode), "group")) // media:group
                {
                  rss_open_rss_mrss (pnode, item);
                }
              else if (!strcasecmp (xml_get_name (pnode), "author") ||
                       !strcasecmp (xml_get_name (pnode), "dc:creator") ||
                       !strcasecmp (xml_get_name (pnode), "creator"))
                {
                    rss_read_copy (item->user, xml_get_childnode (pnode));
//                  found = 1;
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
rss_open_atom (st_rss_t *rss, const char *encoding)
{
  xml_doc_t *doc;
  xml_node_t *node;
  const char *p = NULL;

  doc = xml_parse (rss->url, encoding);
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
    node = xml_get_nextnode (node);
  if (!node)
    {
//      fprintf (stderr, "");
      return NULL;
    }

  while (node)
    {
      while (node && xml_is_empty_node (node))
        node = xml_get_nextnode (node);

      if (!node)
        break;

      if (!strcasecmp (xml_get_name (node), "title"))
        rss_read_copy (rss->title, xml_get_childnode (node));
      else if (!strcasecmp (xml_get_name (node), "description"))
        rss_read_copy (rss->desc, xml_get_childnode (node));
//      else if (!strcasecmp (xml_get_name (node), "link"))
//        rss_read_copy (rss->url, xml_get_childnode (node));
      else if (!strcasecmp (xml_get_name (node), "date") ||
               !strcasecmp (xml_get_name (node), "pubDate") ||
               !strcasecmp (xml_get_name (node), "dc:date") ||
               !strcasecmp (xml_get_name (node), "modified") ||
               !strcasecmp (xml_get_name (node), "updated"))
        rss->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (node)));
      else if ((!strcasecmp (xml_get_name (node), "entry")))
        {
          xml_node_t *pnode = xml_get_childnode (node);
          st_rss_item_t *item = &rss->item[rss->item_count];
//          int found = 0;
          char link[RSSMAXBUFSIZE];

          *link = 0;

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

              if (!strcasecmp (xml_get_name (pnode), "title"))
                {
                  rss_read_copy (item->title, xml_get_childnode (pnode));
//                  found = 1;
                }
#if 0
              else if (!strcasecmp (xml_get_name (pnode), "id"))
                {
                  rss_read_copy (link, xml_get_childnode (pnode));
//                  found = 1;
                }
#endif
              else if (!strcasecmp (xml_get_name (pnode), "link") && (!(*link)))
                {
#if 0
<link rel="alternate" type="text/html" href="http://edition.cnn.com/2006/POLITICS/11/01/kerry.remarks/"/>
#endif
                  p = (const char *) xml_get_value (pnode, "href");
                  if (p)
                    {
                      strncpy (link, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
//                      found = 1;
                    }
                }
              else if (!strcasecmp (xml_get_name (pnode), "content"))
                {
                  rss_read_copy (item->desc, xml_get_childnode (pnode));
//                  found = 1;
                }
              else if (!strcasecmp (xml_get_name (pnode), "author"))
                {
                  xml_node_t *tnode = xml_get_childnode (pnode); 
                  if (!strcasecmp (xml_get_name (tnode), "name"))
                    rss_read_copy (item->user, xml_get_childnode (tnode));
//                  found = 1;
                }
              else if (!strcasecmp (xml_get_name (pnode), "modified") ||
                       !strcasecmp (xml_get_name (pnode), "updated"))
                { 
                  item->date = strptime2 ((const char *) xml_get_string (xml_get_childnode (pnode)));
//                  found = 1;
                }
//              else if (!strcasecmp (xml_get_name (pnode), "duration")) // HACK yt:duration
              else if (stristr (xml_get_name (pnode), "duration")) // HACK yt:duration
                {
                  p = (const char *) xml_get_value (pnode, "seconds");
                  if (p)
                    {
                      item->media.duration = strtol (p, NULL, 10);
//                      found = 1;
//                      break;
                    }
                }
//              else if (!strcasecmp (xml_get_name (pnode), "group")) // media:group
              else if (stristr (xml_get_name (pnode), "group")) // media:group
#if 1
                {
                  rss_open_rss_mrss (pnode, item);
                }
#else
                {
                  xml_node_t *tnode = xml_get_childnode (pnode); 
                  while (tnode)
                    {
                      if (!tnode)
                        break;

//                      if (!strcasecmp (xml_get_name (tnode), "content")) // media:content
                      if (stristr (xml_get_name (tnode), "content")) // media:content
                        {
                          p = (const char *) xml_get_value (tnode, "duration");
                          if (p)
                            {
                              item->media.duration = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "filesize");
                          if (p)
                            {
                              item->media.filesize = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "width");
                          if (p)
                            {
                              item->media.width = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                          p = (const char *) xml_get_value (tnode, "height");
                          if (p)
                            {
                              item->media.height = strtol (p, NULL, 10);
//                              found = 1;
//                              break;
                            }
                        }
//                      else if (!strcasecmp (xml_get_name (tnode), "keywords")) // media:keywords
                      else if (stristr (xml_get_name (tnode), "keywords")) // media:keywords
                        {
                          rss_read_copy (item->media.keywords, xml_get_childnode (tnode));
                        }
//                      else if (!strcasecmp (xml_get_name (tnode), "thumbnail")) // media:thumbnail
                      else if (stristr (xml_get_name (tnode), "thumbnail")) // media:thumbnail
                        {
                          p = (const char *) xml_get_value (tnode, "url");
                          if (p)
                            if (!(item->media.thumbnail[0]))
                            {
                              strncpy (item->media.thumbnail, p, RSSMAXBUFSIZE)[RSSMAXBUFSIZE-1] = 0;
//                              found = 1;
//                              break;  
                            }
                        }
                      tnode = xml_get_nextnode (tnode);
                    }
                }
#endif
              pnode = xml_get_nextnode (pnode);
            }

          if (*link)
            strcpy (item->url, link);

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


st_rss_t *
rss_open (const char *fname, const char *encoding)
{
  st_rss_t *rss = NULL;

  if (!(rss = (st_rss_t *) malloc (sizeof (st_rss_t))))
    return NULL;

  memset (rss, 0, sizeof (st_rss_t));

  strncpy (rss->url, fname, RSSMAXBUFSIZE)[RSSMAXBUFSIZE - 1] = 0;
  rss->item_count = 0;

  rss->version = rss_demux (fname, encoding);

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
      case ATOM_V1_0:
        return rss_open_atom (rss, encoding);
      default:
        return rss_open_rss (rss, encoding);
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


int
generate_rss2 (FILE *fp, st_rss_t *rss, int type, int use_mrss, int use_rsscache, const char *xsl_stylesheet)
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

  if (type != 1) // default to RSS 2.0
    type = 2;

  xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL);

//  xmlTextWriterWriteComment (writer, "comment");

  if (type == 2)
    {
      xmlTextWriterStartElement (writer, BAD_CAST "rss");  // <rss>

      xmlTextWriterWriteAttribute (writer, BAD_CAST "type", BAD_CAST "2.0");
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

  if (type == 1)
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

      if (type == 1)
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
#warning proper Media RSS output
      xmlTextWriterWriteFormatElement (writer, BAD_CAST "media_duration", "%d", rss->item[i].media.duration);

      XMLPRINTF("\n    ");

      xmlTextWriterEndElement (writer); // </item>
    }

  if (type == 2)
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


int
rss_write (FILE *fp, st_rss_t *rss, int type)
{
  return generate_rss2 (fp, rss, type, 1, 1, NULL);
}


function
misc_xml_escape ($s)
{
  if ($s != htmlspecialchars ($s, ENT_QUOTES))
    return '<![CDATA['.$s.']]>';
  return $s;
}


function
generate_rss2_func ($item, $a, $whitespace = '      ')
{
  $p = '';
  for ($i = 0; isset ($a[$i]); $i++)
    if (isset ($item[$a[$i]]))
      {
        $t = $a[$i];

        // DEBUG
//        echo '<pre><tt>';
//        print_r ($item);
//        echo $t;

//        if (isset ($item[$t]))
          $p .= $whitespace.'<'.$t.'>'
               .(is_string ($item[$t]) ? misc_xml_escape ($item[$t]) : sprintf ("%u", $item[$t]))
               .'</'.$t.'>'."\n";
      }
  return $p;
}


//int
//rss_write (FILE *fp, st_rss_t *rss, int type)
//function
//generate_rss2 ($channel, $item, $use_mrss = 0, $use_rsscache = 0, $xsl_stylesheet = NULL)
int
generate_rss2 (FILE *fp, st_rss_t *rss, int type, int use_mrss, int use_rsscache, const char *xsl_stylesheet)
{
  // DEBUG
//  echo '<pre><tt>';
//  print_r ($channel);
//  print_r ($item);

  $use_cms = 0; 
  if ($use_rsscache == 1)
    $use_cms = 1;

  $p = '';

  $p .= '<?xml version="1.0" encoding="UTF-8"?>'."\n";

  if ($xsl_stylesheet)
    $p .= '<?xml-stylesheet href="'.$xsl_stylesheet.'" type="text/xsl"?>'."\n";

  $p .= '<rss version="2.0"';

  // base
//  $p .= ' xml:base="http://rsscache.a1.25u.com/"';

  if ($use_mrss == 1)
    $p .= ' xmlns:media="http://search.yahoo.com/mrss/"';

  if ($use_rsscache == 1)
    $p .= ' xmlns:rsscache="data:,rsscache"';

  if ($use_cms == 1)
    $p .= ' xmlns:cms="data:,cms"';

  $p .= '>'."\n";

  $p .= '  <channel>'."\n"
;

$a = array (
           'title',
           'link',
           'description',
           'docs',
);

   $p .= generate_rss2_func ($channel, $a, '    ');

  if (isset ($channel['lastBuildDate']))
    $p .= '    <lastBuildDate>'.strftime ("%a, %d %h %Y %H:%M:%S %z", $channel['lastBuildDate']).'</lastBuildDate>'."\n";

  if (isset ($channel['image']))
    $p .= ''
//       .'    <language>en</language>'."\n"
       .'    <image>'."\n"
       .'      <title>'.misc_xml_escape ($channel['title']).'</title>'."\n"
       .'      <url>'.$channel['image'].'</url>'."\n"
       .'      <link>'.misc_xml_escape ($channel['link']).'</link>'."\n"
//       .'      <width></width>'."\n"
//       .'      <height></height>'."\n"
       .'    </image>'."\n"
;

$a = array (
//           'rsscache:stats_category',
           'rsscache:stats_items',
           'rsscache:stats_days',
           'rsscache:stats_items_today',
           'rsscache:stats_items_7_days',
           'rsscache:stats_items_30_days',
);

   $p .= generate_rss2_func ($channel, $a, '    ');

  // items
//  for ($i = 0; isset ($item[$i]['link']); $i++)
  for ($i = 0; isset ($item[$i]); $i++)
    {
      $p .= '    <item>'."\n";

$a = array (
           'title',
           'link',
           'description',
           'category',
           'author',
           'comments',
);
   $p .= generate_rss2_func ($item[$i], $a);

//                <pubDate>Fri, 05 Aug 2011 15:03:02 +0200</pubDate>
      if (isset ($item[$i]['pubDate']))
        $p .= ''
             .'      <pubDate>'
             .strftime (
                "%a, %d %h %Y %H:%M:%S %z",
//                "%a, %d %h %Y %H:%M:%S %Z",
                $item[$i]['pubDate'])
             .'</pubDate>'."\n"
;
        if (isset ($item[$i]['enclosure']))
          {
            $suffix = strtolower (get_suffix ($item[$i]['enclosure']));

            // HACK
            if ($suffix == '.jpg') $suffix = '.jpeg';

            // TODO: get filesize from db
            $p .= '      <enclosure url="'.$item[$i]['enclosure'].'"'
//                 .' length=""'
                 .' type="image/'.substr ($suffix, 1).'" />'."\n"
;
          }

      // mrss
      if ($use_mrss == 1)
        {
//      $p .= '    <media:group>'."\n";

//      $p .= '      <media:category scheme="">'.'</media:category>';

      if (isset ($item[$i]['media:thumbnail']))
        $p .= '      <media:thumbnail url="'.$item[$i]['media:thumbnail'].'" media:url="'.$item[$i]['media:thumbnail'].'" />'."\n";

$a = array (
           'media:duration',
           'media:keywords',
           'media:embed',
);
  
   $p .= generate_rss2_func ($item[$i], $a);


/*
        <media:content 
               url="http://www.foo.com/movie.mov"
               fileSize="12216320" 
               type="video/quicktime"
               medium="video"
               isDefault="true"
               expression="full"
               bitrate="128"
               framerate="25"
               samplingrate="44.1"
               channels="2"
               duration="185"
               height="200"
               width="300"
               lang="en" />
*/ 

for ($j = 0; isset ($item[$i]['media:content_'.$j.'_url']); $j++)
  {
    if (isset ($item[$i]['media:content_'.$j.'_url']))
      $p .= '      <media:content url="'.$item[$i]['media:content_'.$j.'_url'].'"'
//           .' media:url="'.$item[$i]['media:content_'.$j.'_url'].'"'
           .' medium="'.$item[$i]['media:content_'.$j.'_medium'].'"'
           .' />'."\n";
  }

//      $p .= '      </media:group>'."\n";
        }

      // rsscache
      if ($use_rsscache == 1)
        {
//      $p .= '    <rsscache:group>'."\n";

      if (isset ($item[$i]['pubDate']))
        $p .= '      <rsscache:pubDate>'.sprintf ("%u", $item[$i]['pubDate']).'</rsscache:pubDate>'."\n";

$a = array (
           'rsscache:category_title',
           'rsscache:dl_date',
           'rsscache:related_id',
           'rsscache:event_start',
           'rsscache:event_end',
           'rsscache:url_crc32',
           'rsscache:movable',
           'rsscache:reportable',
           'rsscache:votable',
           'rsscache:table_suffix',
//           'rsscache:download',
);

   $p .= generate_rss2_func ($item[$i], $a);
  // DEBUG
//  echo '<pre><tt>';
//  print_r ($channel);
//  print_r ($item);

for ($j = 0; isset ($item[$i]['rsscache:feed_'.$j.'_link']); $j++)
  {
    $p .= '      <rsscache:feed>'."\n";
    if (isset ($item[$i]['rsscache:feed_'.$j.'_exec']))
      $p .= '        <rsscache:exec>'.misc_xml_escape ($item[$i]['rsscache:feed_'.$j.'_exec']).'</rsscache:exec>'."\n";
    if (isset ($item[$i]['rsscache:feed_'.$j.'_link']))
      $p .= '        <rsscache:link>'.misc_xml_escape ($item[$i]['rsscache:feed_'.$j.'_link']).'</rsscache:link>'."\n";
    $p .= '      </rsscache:feed>'."\n";
  }

//      $p .= '      </rsscache:group>'."\n";
        }
$a = array (
           'rsscache:stats_category',
           'rsscache:stats_items',
           'rsscache:stats_days',
           'rsscache:stats_items_today',
           'rsscache:stats_items_7_days',
           'rsscache:stats_items_30_days',
);

   $p .= generate_rss2_func ($item[$i], $a);

      // CMS
      if ($use_cms == 1)
        {
//          $p .= '    <cms:group>'."\n";

            $a = array (
            'cms:separate',
            'cms:button_only',
            'cms:status',
            'cms:select',
            'cms:local',
            'cms:iframe',
            'cms:proxy',
//            'cms:query',
            'cms:demux',
);

   $p .= generate_rss2_func ($item[$i], $a);

//          $p .= '    </cms:group>'."\n";
        }

      $p .= '    </item>'."\n";
    }

  $p .= '  </channel>'."\n";

  $p .= '</rss>'."\n";

  return $p;
}


#if 0
// TODO: escape html code in desc
  unsigned int i = 0;

  if (!fp)
    return -1;

  if (!rss)
    return -1;

  if (type != 1) // default to RSS 2.0
    type = 2;

  fputs ("<?xml type=\"1.0\" encoding=\"UTF-8\"?>\n", fp);

  if (type == 1)
    fputs ("<rdf:RDF xmlns=\"http://purl.org/rss/1.0/\">\n", fp);
  else
    fputs ("<rss type=\"2.0\">\n", fp);

  fputs ("  <channel>\n"
         "    <title>RSStool</title>\n"
         "    <link>http://rsstool.berlios.de</link>\n"
         "    <description>read, parse, merge and write RSS and Atom feeds</description>\n"
//         "    <dc:date>%ld</dc:date>"
         , fp);


  if (type == 1)
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
      if (type == 1)
        fprintf (fp, "<item rdf:about=\"%s\">\n", rss->item[i].url);
      else
        fputs ("    <item>\n", fp);

      fprintf (fp, "      <title>%s</title>\n", rss->item[i].title);
      fprintf (fp, "      <link>%s</link>\n", rss->item[i].url);
      fprintf (fp, "      <description>%s</description>\n", rss->item[i].desc);
      fprintf (fp, "      <dc:date>%ld</dc:date>\n", rss->item[i].date);

      fputs ("    </item>\n", fp);
    }

  if (type == 2)
    fputs ("  </channel>\n", fp);

  fputs ("</rss>\n", fp);
#endif


