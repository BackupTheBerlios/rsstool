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
#define RSSMAXBUFSIZE 4096


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


/*
very basic MRSS 1.0.0 specification (http://video.search.yahoo.com/mrss)

<media:group>

<media:group> is a sub-element of <item>. It allows grouping of
<media:content> elements that are effectively the same content, yet
different representations.  For instance: the same song recorded in both the
WAV and MP3 format.  It's an optional element that must only be used for
this purpose.

<media:content>

<media:content> is a sub-element of either <item> or <media:group>. Media
objects that are not the same content should not be included in the same
<media:group> element.  The sequence of these items implies the order of
presentation.  While many of the attributes appear to be audio/video
specific, this element can be used to publish any type of media.  It
contains 14 attributes, most of which are optional.

 
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

url
should specify the direct url to the media object. If not included, a
<media:player> element must be specified.

fileSize
is the number of bytes of the media object. It is an optional attribute.

type
is the standard MIME type of the object. It is an optional attribute.

medium
is the type of object (image | audio | video | document | executable). While
this attribute can at times seem redundant if type is supplied, it is
included because it simplifies decision making on the reader side, as well
as flushes out any ambiguities between MIME type and object type.  It is an
optional attribute.

isDefault
determines if this is the default object that should be used for the
<media:group>.  There should only be one default object per <media:group>. 
It is an optional attribute.

expression
determines if the object is a sample or the full version of the object, or
even if it is a continuous stream (sample | full | nonstop).  Default value
is 'full'.  It is an optional attribute.

bitrate
is the kilobits per second rate of media. It is an optional attribute.

framerate
is the number of frames per second for the media object. It is an optional
attribute.

samplingrate
is the number of samples per second taken to create the media object. It is
expressed in thousands of samples per second (kHz).  It is an optional
attribute.

channels
is number of audio channels in the media object. It is an optional
attribute.

duration
is the number of seconds the media object plays. It is an optional
attribute.

height
is the height of the media object. It is an optional attribute.

width
is the width of the media object. It is an optional attribute.

lang
is the primary language encapsulated in the media object. Language codes
possible are detailed in RFC 3066.  This attribute is used similar to the
xml:lang attribute detailed in the XML 1.0 Specification (Third Edition). 
It is an optional attribute.

These optional attributes, along with the optional elements below, contain
the primary metadata entries needed to index and organize media content. 
Additional supported attributes for describing images, audio, and video may
be added in future revisions of this document.

Note: While both <media:content> and <media:group> have no limitations on
the number of times they can appear, the general nature of RSS should be
preserved: an <item> represents a "story".  Simply stated, this is similar
to the blog style of syndication.  However, if one is using this module to
strictly publish media, there should be one <item> element for each media
object/group.  This is to allow for proper attribution for the origination
of the media content through the <link> element.  It also allows the full
benefit of the other RSS elements to be realized.
*/
typedef struct
{
  int duration; // default: 0
  int filesize;
  int width;
  int height;
  const char *keywords;
} st_mrss_item_t;


typedef struct
{
  char title[RSSMAXBUFSIZE];
  char url[RSSMAXBUFSIZE];
  char desc[RSSMAXBUFSIZE];
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
                        version can be 1 or 2

  rss_get_item()      get item n
  rss_item_count()    count items in st_rss_t
  rss_get_version_s() get version of feed as string
*/
extern int rss_demux (const char *fname, const char *encoding);

extern st_rss_t *rss_open (const char *fname, const char *encoding);
extern int rss_close (st_rss_t *rss);

extern int rss_write (FILE *fp, st_rss_t *rss, int version);

extern st_rss_item_t *rss_get_item (st_rss_t * rss, unsigned int n);
extern unsigned int rss_item_count (st_rss_t * rss);
extern const char *rss_get_version_s (st_rss_t * rss);
extern const char *rss_get_version_s_by_id (int version);
extern const char *rss_get_version_s_by_magic (const char *m);

extern char *rss_utf8_enc (const unsigned char *in, const char *encoding);


#endif //  RSS_H
