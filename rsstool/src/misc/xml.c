/*
xml.c - wrapper for different XML parser

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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef  USE_NXML
#include <nxml.h>
#elif   defined USE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#endif
#include "string.h"
#include "xml.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


#ifdef  USE_XML2
const char *
xml_xpath (const char *fname, const char *xpath_expr)
{
  int size;
  int i;
  const xmlChar *p = NULL;
  xmlNodeSetPtr nodes;
  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr xpathObj;

  xmlInitParser ();
  LIBXML_TEST_VERSION

  if (!(doc = xmlParseFile (fname)))
    {
      fprintf (stderr, "ERROR: unable to parse file \"%s\"\n", fname);
      return NULL;
    }

  if (!(xpathCtx = xmlXPathNewContext (doc)))
    {
      fprintf (stderr, "ERROR: unable to create new XPath context\n");
      xmlFreeDoc (doc);
      return NULL;
    }

  if (!(xpathObj = xmlXPathEvalExpression ((const xmlChar *) xpath_expr, xpathCtx)))
    {
      fprintf (stderr, "ERROR: unable to evaluate xpath expression \"%s\"\n", xpath_expr);
      xmlXPathFreeContext (xpathCtx);
      xmlFreeDoc (doc);
      return NULL;
    }

  nodes = xpathObj->nodesetval;
  size = (nodes) ? nodes->nodeNr : 0;

  for (i = size - 1; i >= 0; i--)
    {
      p = xmlNodeGetContent (nodes->nodeTab[i]);
#ifdef  DEBUG
      printf (p);
      fflush (stdout);
#endif
      if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
        nodes->nodeTab[i] = NULL;
    }

  xmlXPathFreeObject (xpathObj);
  xmlXPathFreeContext (xpathCtx);

//    xmlDocDump(stdout, doc);

  xmlFreeDoc (doc);
  xmlCleanupParser ();

  return (const char *) p;
}
#endif  // USE_XML2


xml_doc_t *
xml_parse (const char *fname, const char *encoding)
{
#ifdef  USE_NXML
  return nxmle_new_data_from_file ((char *) fname, NULL);
#elif   defined USE_XML2
//  return xmlParseFile (fname);
  return xmlReadFile (fname, encoding, 0); // XML_PARSE_RECOVER);
#endif
}


xml_node_t *
xml_get_rootnode (xml_doc_t *d)
{
#ifdef  USE_NXML
  if (d)
    return nxmle_root_element (d, NULL);
#elif   defined USE_XML2
  if (d)
    return xmlDocGetRootElement (d);
#endif
  return NULL;
}


xml_node_t *
xml_get_childnode (xml_node_t *n)
{
#ifdef  USE_NXML
  if (n)
    return n->children;
#elif   defined USE_XML2
  if (n)
    return n->xmlChildrenNode;
#endif
  return NULL;
}


xml_node_t *
xml_get_nextnode (xml_node_t *n)
{
#ifdef  USE_NXML
  if (n)
    return n->next;
#elif   defined USE_XML2
  if (n)
    return n->next;
#endif
  return NULL;
}


int
xml_is_empty_node (xml_node_t *n)
{
#ifdef  USE_NXML
  char *p = NULL;

  if (n)
    p = nxmle_get_string (n, NULL);

  if (p)
    while (*p)
     {
       if (!isspace (*p))
         return 0;
       p++;
     }

  return 1;
#elif   defined USE_XML2
  if (n)
    return xmlIsBlankNode (n);
  return 0;
#endif
}


const unsigned char *
xml_get_string (xml_node_t *n)
{
#ifdef  USE_NXML
  if (n)
    return (unsigned char *) nxmle_get_string (n, NULL);
#elif   defined USE_XML2
  if (n)
    return xmlNodeListGetString (n->doc, n, 1);
#endif
  return NULL;
}


const char *
xml_get_name (xml_node_t *n)
{
#ifdef  USE_NXML
  if (n)
    return n->value;
#elif   defined USE_XML2
  if (n)
    return (char *) n->name;
#endif
  return NULL;
}


const unsigned char *
xml_get_value (xml_node_t *n, const char *name)
{
#ifdef  USE_NXML
  if (n)
    return (unsigned char *) nxmle_find_attribute (n, (char *) name, NULL);
#elif   defined USE_XML2
  if (n)
    if (xmlHasProp (n, (const unsigned char *) name))
      return xmlGetProp (n, (const unsigned char *) name);
#endif
  return NULL;
}


int
xml_free (xml_doc_t *d)
{
#ifdef  USE_NXML
  nxmle_free (d);
#elif   defined USE_XML2
  (void) d;
#endif
  return 0;
}


const char *
xml_tag_get_name (const char *tag)
{
  static char buf[MAXBUFSIZE];
  char *p = NULL;

  p = strchr (tag, '<');
  if (!p)
    return NULL;

  strncpy (buf, p + 1, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  strtriml (buf);

  p = strchr (buf, '>');
  if (p)
    *p = 0;

  p = strchr (buf, ' ');
  if (p)
    *p = 0;
      
  strtrimr (buf);

  return buf;
}


const char *
xml_tag_get_value (const char *tag, const char *value_name)
{
  static char buf[MAXBUFSIZE];
  char *p = NULL;
  int quotes = 0;

  if (!stristr (tag, value_name))
    return NULL;

  strncpy (buf, tag, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  stritrim_s (buf, value_name, NULL);
  stritrim_s (buf, "=", NULL);
  strtriml (buf);

  if (*buf == '\"')
    {
      stritrim_s (buf, "\"", NULL);  // quotes are optional
      quotes = 1;
    }

  p = strchr (buf, '>');
  if (p)
    *p = 0;

  if (quotes)
    p = strchr (buf, '\"');
  else
    p = strchr (buf, ' ');

  if (p)
    *p = 0;

  strtrimr (buf);

  return buf;
}


unsigned long
xml_tag_filter (char *str, st_tag_filter_t *f, unsigned long continuous_flag)
{
  int in_tag = continuous_flag;
  char *bak = strdup (str);
  char *s = bak;
  char *d = str;
  int i = 0;

  if (!bak)
    return -1;

  for (; *s; s++)
    switch (*s)
      {
        case '>':
          if (in_tag)
            in_tag = 0;
          else
            {
              *d = *s;
              *(++d) = 0;
            }
          break;

        case '<':
          if (f)
            {
              char tag_full[MAXBUFSIZE];
              char *p = NULL;
              int found = 0;

              // nested tag?
              p = strpbrk (s + 1, "<>");
              if (p)
                if (*p == '<')
                  {
                    strncpy (d, s, p - s);
                    d += (p - s);
                    *d = 0;
                    s = p;
                  }

              strncpy (tag_full, s, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
              strtriml (tag_full);
              p = strchr (tag_full, '>');
              if (p)
                *(++p) = 0;
              else
                in_tag = 1;

#ifdef  DEBUG
              fprintf (stderr, "tag_full: %s\n", tag_full);
              fflush (stderr);
#endif

              // run filter
              for (i = 0; f[i].filter; i++)
                if (!(*(f[i].start_tag)) || // empty tag overrides all
                    !strcasecmp (xml_tag_get_name (tag_full), f[i].start_tag))
                  {
                    const char *rep = NULL;

#ifdef  DEBUG
                    fputs (f[i].start_tag, stderr);
                    fflush (stderr);
#endif

                    rep = f[i].filter (tag_full);
                    if (rep)
                      {
                        if (*rep)
                          {
                            strcpy (d, rep);
                            d = strchr (d, 0);
                          }
                        s += strlen (tag_full) - 1;
                      }

                    found = 1;
                    break;
                  }

              if (found)
                continue;
            }

        default:
          *d = *s;
          *(++d) = 0;
      }

  if (bak)
    free (bak);

  continuous_flag = in_tag;

  return continuous_flag;
}


#if 0
int
xml_tag_arg (char **argv, char *tag)
{
  static char buf[MAXBUFSIZE];

  // turn tag attributes into args
  strncpy (buf, tag, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  return strarg (argv, buf, " ", MAXBUFSIZE);
}
#endif


#ifdef  TEST
//#if 0
const char *
pass_filter (const char *s)
{
  return s;
}


const char *
remove_filter (const char *s)
{
  return "";
}


const char *
replace_filter (const char *s)
{
  return "bla";
}


int
main (int argc, char ** argv)
{
  char buf[MAXBUFSIZE];
  char *p = "< a 1234>abcd</a>< b 1234>abcd</b>< c 1234>abcd</c>< d 1234>abcd</d>";
  st_tag_filter_t f[] = {
    {
      "a",
      pass_filter
    },
    {
      "b",
      remove_filter
    },
    {
      "c",
      replace_filter
    },
    {
      NULL,
      NULL
    }
  };
  int cnt = 0;


  strcpy (buf, p);
  xml_tag_filter (buf, f, 0, 0);
  fputs (buf, stdout);

  strcpy (buf, p);
  xml_tag_filter (buf, f, 1, 0);
  fputs (buf, stdout);

  strcpy (buf, p);
  xml_tag_filter (buf, NULL, 0, 0);
  fputs (buf, stdout);

  strcpy (buf, p);
  xml_tag_filter (buf, NULL, 1, 0);
  fputs (buf, stdout);

  // using continuous_flag for multi-line tags
  strcpy (buf, "<w><b");
  cnt = xml_tag_filter (buf, f, 1, 0);
  printf ("%s (cnt: %d)\n", buf, cnt);

  strcpy (buf, "><w>");
  cnt = xml_tag_filter (buf, f, 1, cnt);
  printf ("%s (cnt: %d)\n", buf, cnt);


  return 0;
}
#endif  // TEST
