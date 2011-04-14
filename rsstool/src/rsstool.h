/*
rsstool.h - RSStool reading, parsing and writing RSS and Atom feeds

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
#ifndef RSSTOOL_H
#define RSSTOOL_H


typedef struct
{
  int version;  // rss? atom?

  time_t date;
  char url[RSSTOOL_MAXBUFSIZE];
  char title[RSSTOOL_MAXBUFSIZE]; // used by searches for related items
  char desc[RSSTOOL_MAXBUFSIZE];
#warning user/author
  char user[RSSTOOL_MAXBUFSIZE];
  char site[RSSTOOL_MAXBUFSIZE];
  char feed_url[RSSTOOL_MAXBUFSIZE];

#warning media_keywords
  char media_keywords[RSSTOOL_MAXBUFSIZE]; // default: keywords from title and description
#warning fix media_duration
  int media_duration; //  default: 0
#warning media_thumbnail
  char media_thumbnail[FILENAME_MAX];
#warning media_image
  char media_image[FILENAME_MAX];

#warning event_start
  int event_start;    // default: date
#warning event_len
  int event_len;  // default: event_start + media_duration

//  int private;   // used by sort
} st_rsstool_item_t;


typedef struct
{
  int quiet;
  int timeout;   // timeout for net operations
  time_t start_time;
  char encoding[MAXBUFSIZE];
  char user_agent[MAXBUFSIZE];
  const char *optarg;

  int output;       // what kind of output
  FILE *output_file; // output pointer (default: stdout)
  char pipe_command[MAXBUFSIZE]; // name of command to pipe the output to,
                                 // empty string if --pipe not used
//  int sql_update;   // update existing rows
  int nosort;         // sort?
  int reverse;      // sort reverse before output
  int rss_version;  // version of RSS
  time_t since;     // no feed items older than since
  int fixdate;
  int csv_separator;

  int strip_html;
  int strip_desc;
  int strip_lf;
  int strip_whitespace;
  const char *strip_filter;

//  char property[FILENAME_MAX];
  char template_file[FILENAME_MAX];
  char temp_file[FILENAME_MAX];
  FILE *input_file;
  FILE *log;
  int get_flags;

  st_rsstool_item_t *item[RSSTOOL_MAXITEM];
  int item_count;
} st_rsstool_t;


extern st_rsstool_t rsstool;


#endif  // RSSTOOL_H
