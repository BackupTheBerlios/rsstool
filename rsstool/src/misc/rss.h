/*
rss.h - (M)RSS and Atom parser and generator (using libxml2)

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
#ifndef RSS_H
#define RSS_H


#define RSSMAXITEM 512
#define RSSMAXBUFSIZE 1024


// version id's
enum {
  RSS_V0_90 = 1,
  RSS_V0_91,
  RSS_V0_92,
  RSS_V0_93,
  RSS_V0_94,
  RSS_V1_0,
  RSS_V2_0,

  ATOM_V0_1,
  ATOM_V0_2,
  ATOM_V0_3,
  ATOM_V1_0,

  MRSS_V1_0_0,
  MRSS_V1_1_0,
  MRSS_V1_1_1,
  MRSS_V1_1_2,
  MRSS_V1_5_0
};


typedef struct
{
  int duration; // default: 0
  int filesize;
  int width;
  int height;
  char keywords[RSSMAXBUFSIZE];
//  char image[RSSMAXBUFSIZE];
  char thumbnail[RSSMAXBUFSIZE];
} st_mrss_item_t;


typedef struct
{
  char title[RSSMAXBUFSIZE];
  char url[RSSMAXBUFSIZE];
  char desc[RSSMAXBUFSIZE];
  char user[RSSMAXBUFSIZE];
  time_t date;
  st_mrss_item_t media;
} st_rss_item_t;


typedef struct
{
  int version;               // version of the feed

  // feed information
  char title[RSSMAXBUFSIZE];
  char url[RSSMAXBUFSIZE];
  char desc[RSSMAXBUFSIZE];
  time_t date;

  st_rss_item_t item[RSSMAXITEM];
  int item_count;
} st_rss_t;


/*
  rss_demux()         check if it is a valid RSS or Atom feed
                        returns: version id == success
                                 -1 == failed

  rss_read()          read and parse RSS or Atom feed
  rss_write()         create XML and write to file
                        TYPE=1 will write RSS v1.0
                        TYPE=2 will write RSS v2.0 (default)
                        TYPE=3 will write Media RSS v1.5.0

  rss_get_item()      get item n
  rss_item_count()    count items in st_rss_t
  rss_get_version_s() get version of feed as string
*/
extern int rss_demux (const char *fname, const char *encoding);

extern st_rss_t *rss_open (const char *fname, const char *encoding);
extern int rss_close (st_rss_t *rss);

extern int rss_write (FILE *fp, st_rss_t *rss, int type);

extern st_rss_item_t *rss_get_item (st_rss_t * rss, unsigned int n);
extern unsigned int rss_item_count (st_rss_t * rss);
extern const char *rss_get_version_s (st_rss_t * rss);
extern const char *rss_get_version_s_by_id (int version);
extern const char *rss_get_version_s_by_magic (const char *m);

extern char *rss_utf8_enc (const unsigned char *in, const char *encoding);


#endif //  RSS_H
