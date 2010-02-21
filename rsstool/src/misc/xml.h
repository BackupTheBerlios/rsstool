/*
xml.h - wrapper for different XML parser

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
#ifndef MISC_XML_H
#define MISC_XML_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  USE_NXML
#include <nxml.h>
typedef nxml_t xml_doc_t;
typedef nxml_data_t xml_node_t;
#elif   defined USE_XML2
#include <libxml/tree.h>
typedef struct _xmlDoc xml_doc_t;
typedef struct _xmlNode xml_node_t;
#endif
#include "xml.h"


#if     (defined USE_NXML || defined USE_XML2)
/*
  xml_parse()
  xml_free()

  xml_get_rootnode()
  xml_get_childnode()
  xml_get_nextnode()

  xml_is_empty_node()
  xml_get_string()

  xml_get_name()

  xml_get_value()


<node>
        <node> ->parent
                <node>
                        <node> ->prev
                        </node>
                        <node>
                        </node>
                </node>
                <node> You Are Here
                        <node> ->children
                        </node>
                        <node>
                        </node>
                        <node> ->last
                        </node>
                </node>
                <node> ->next
                        <node>
                        </node>
                </node>
        </node>
</node>
*/
extern xml_doc_t *xml_parse (const char *fname, const char *encoding);
extern int xml_free (xml_doc_t *d);

extern xml_node_t *xml_get_rootnode (xml_doc_t *d);
extern xml_node_t *xml_get_childnode (xml_node_t *n);
extern xml_node_t *xml_get_nextnode (xml_node_t *n);

extern int xml_is_empty_node (xml_node_t *n);
extern const unsigned char *xml_get_string (xml_node_t *n);

extern const char *xml_get_name (xml_node_t *n);

extern const unsigned char *xml_get_value (xml_node_t *n, const char *name);


#ifdef  USE_XML2  
extern const char *xml_xpath_once (const char *fname, const char *xpath_expr);

extern xmlDocPtr xml_xpath_open (const char *fname);
extern const char *xml_xpath (xmlDocPtr ctx, const char *xpath_expr);
extern void xml_xpath_close (xmlDocPtr ctx);
#endif


#endif  // #if     (defined USE_NXML || defined USE_XML2)


/*
  Tag parse functions

  xml_tag_filter()          filter (html)tags in a string
                              continuous_flag must be set to 0 when starting at the beginning of
                                a new file with (html)tags; this makes sure that (html)tags which
                                start and end in different lines get parsed correctly when the
                                file is parsed line-wise; set this to 0 if str is the whole
                                file with (html) tags

                                Example (continuous_flag):
                                  st_tag_filter_t f[] = {{"a", a_remove},{NULL,NULL}};
                                  strcpy (buf, "<w><a");
                                  cf = xml_tag_filter (buf, f, 1, 0);
                                  printf ("%s", buf);
                                  strcpy (buf, "><w>");
                                  cf = xml_tag_filter (buf, f, 1, cf);
                                  printf ("%s", buf);
                                will output "<w><w>"

                            xml_tag_filter() returns -1 on ERROR (malloc failed) or the
                              continuous_flag to be used for the next call (see above)

  st_tag_filter_t
    start_tag                name of the tag to filter
                               if the name is "" then this filter will apply for all
                               tags - all following filters in the st_tag_filter_t
                               array will be ignored
    filter                   the actual filter
                               takes the string including '<' and '>'
                               and returns the replacement
                               can be used to pass, remove, and replace
                               (custom) tags

  xml_tag_arg()              splits a tag into an argv-like array and returns argc
*/
typedef struct
{
  const char *start_tag;
//  const char *end_tag;
  const char *(* filter) (const char *);
} st_tag_filter_t;
extern unsigned long xml_tag_filter (char *str, st_tag_filter_t *f, unsigned long continuous_flag);
extern const char *xml_tag_get_name (const char *tag);
extern const char *xml_tag_get_value (const char *tag, const char *value_name);
//extern int xml_tag_arg (char **argv, char *tag);


#endif
