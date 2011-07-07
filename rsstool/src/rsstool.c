/*
rsstool.c - RSStool reading, parsing and writing RSS and Atom feeds

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
#include <string.h>
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>
#include "misc/string.h"
#include "misc/getopt2.h"
#include "misc/misc.h"
#include "misc/net.h"
#include "misc/rss.h"
#include "misc/crc32.h"
#include "rsstool_defines.h"
#include "rsstool.h"
#include "rsstool_misc.h"
#include "rsstool_write.h"


st_rsstool_t rsstool;


static void
rsstool_exit (void)
{
  int i = 0;

  if (rsstool.log)
    {
//      rsstool_log (&rsstool, "rsstool_exit()");

      if (rsstool.log != stderr && rsstool.log != stdout)
        {
          fclose (rsstool.log);
          rsstool.log = NULL;
        }
    }

  if (rsstool.output_file != stdout && rsstool.output_file != stderr)
    {
      fclose (rsstool.output_file);
      rsstool.output_file = NULL;
    }

  for (; rsstool.item[i]; i++)
    {
      free (rsstool.item[i]);
      rsstool.item[i] = NULL;
    }

  if (*(rsstool.temp_file))
    {
      remove (rsstool.temp_file);
      *(rsstool.temp_file) = 0;
    }
}


int
main (int argc, char **argv)
{
//  char short_options[ARGS_MAX];
  struct option long_only_options[ARGS_MAX];
  int option_index = 0;
  int c = 0;
  char buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE];
  const char *p = NULL;
  FILE *fh = NULL;
#if 0
  st_property_t props[] = {
    {NULL, NULL, NULL}
  };
#endif
  const st_getopt2_t options[] = {
    {
      NULL,      0, 0, 0, NULL,
      "rsstool " RSSTOOL_VERSION_S " " CURRENT_OS_S " 2006-2011 by NoisyB\n"
      "This may be freely redistributed under the terms of the GNU Public License\n\n"
      "Usage: rsstool [OPTION] FILE(S)|URL(S)...\n"
    },
    {
      NULL,      0, 0, 0,
      NULL,   "Read, parse, merge and write RSS and Atom feeds\n"
              "\n"
              "Supported feeds:\n"
              " RW xml             RSS 0.9x, 1.0, 2.0\n"
//              " RW                 Media RSS 1.0, 1.1.x, 1.5.0\n"
              " R  xml             Atom 0.1, 0.2, 0.3, 1.0\n"
              "\nThe RSS or Atom feeds will be merged and re-sorted if you enter more than\n"
              "one URL or FILEname\n"
    },
    {
      NULL,      0, 0, 0,
      NULL,   "Options"
    },
    {
      "u", 1, 0, RSSTOOL_U,
      "AGENT", "use user-AGENT for connecting"
    },
#ifdef  USE_ZLIB
    {
      "gzip", 0, 0, RSSTOOL_GZIP,
      NULL, "use gzip compression for downloading"
    },
#endif
#ifdef  USE_CURL  
    { 
      "curl", 0, 0, RSSTOOL_CURL,
      NULL, NULL
    },
#endif
    {
      "input-file", 1, 0, RSSTOOL_INPUT_FILE,
      "FILE", "download feeds found in FILE"
    },
    {
      "i", 1, 0, RSSTOOL_INPUT_FILE,
      "FILE", "same as " OPTION_LONG_S "input-file"
    },
    {
      "parse",   1, 0, RSSTOOL_PARSE,
      "FILE|URL", "parse and structure non-RSS content (e.g. HTML document)"
    },
    {
      "log", 1, 0, RSSTOOL_LOG,
      "FILE", "write a log to FILE (including HTTP headers)"
    },
    {
      "enc", 1, 0, RSSTOOL_ENC,
      "ENCODING", "overrides the encoding specified in RSS header"
    },
    {
      "version", 0, 0, RSSTOOL_VER,
      NULL,   "output version information and exit"
    },
    {
      "ver",     0, 0, RSSTOOL_VER,
      NULL,   NULL
    },
    {
      "help",    0, 0, RSSTOOL_HELP,
      NULL,   "display this help and exit"
    },
    {
      "h",       0, 0, RSSTOOL_HELP,
      NULL,   NULL
    },
#if 0
    {
      "q", 0, 0, RSSTOOL_QUIET,
      NULL,   "be quiet/less verbose"
    },
#endif
    {
      NULL, 0, 0, 0,
      NULL,   "\nStrip, Sort & Repair"
    },
    {
      "sbin",        0, 0, RSSTOOL_SBIN,
      NULL,   "strip all unprintable characters from feed (before parsing)"
    },
#warning test --filter
    {
      "filter", 1, 0, RSSTOOL_FILTER,
      "LOGIC", "sometimes referred to as implied Boolean LOGIC, in which\n"
               "+ stands for AND\n"
               "- stands for NOT\n"
               "no operator implies OR\n"
               "use this to remove items from the RSS feed before output\n"
               "(nested) parentheses are not supported\n"
               "Example: --filter=\"+INCLUDE -EXCLUDE\""
    },
    {
      "keywords", 1, 0, RSSTOOL_KEYWORDS,
      "OPTION", "generate KEYWORDS from title and/or description\n"
             "OPTION=0 from both title and description (default)\n"
             "OPTION=1 from title only\n"
             "OPTION=2 from description only"
    },
#warning test --shtml
    {
      "shtml",        2, 0, RSSTOOL_SHTML,
      "ALLOW",   "strip HTML tags from description\n"
                 "(default: " OPTION_LONG_S "shtml=\"a,br\" strip all tags except A and BR)"
    },
    {
      "swhite",        0, 0, RSSTOOL_SWHITE,
      NULL,   "strip whitespaces from description"
    },
    {
      "slf",        0, 0, RSSTOOL_SLF,
      NULL,   "strip line feeds/carriage returns from descriptions"
    },
    {
      "sdesc",        0, 0, RSSTOOL_SDESC,
      NULL,   "strip the whole description"
    },
#warning --hack-google
    {
      "hack-google",        0, 0, RSSTOOL_HACK_GOOGLE,
      NULL,   NULL // remove un-escaped html
    },
#warning --hack-event
    {
      "hack-event",        0, 0, RSSTOOL_HACK_EVENT,
      NULL,   NULL // gather a start time and length from description
                     // for temporary events like broadcast shedules
    },
    {
      "fixdate", 0, 0, RSSTOOL_FIXDATE,
      NULL,  "missing dates will be replaced with the current date"
    },
    {
      "since", 1, 0, RSSTOOL_SINCE,
      "DATE",  "pass only items (of feeds) newer than DATE\n"
               "DATE can have the following formats\n"
               "\"Thu, 01 Jan 1970 01:00:00 +0100\" (RFC 822),\n"
               "\"YYYY-MM-DDTHH:MM\", \"DD MMM YYYY HH:MM\",\n"
               "or \"YYYY-MM-DD\""
    },
    {
      "nosort",        0, 0, RSSTOOL_NOSORT,
      NULL,   "do not sort items by date (default: on)"
    },
    {
      "r",        0, 0, RSSTOOL_REVERSE,
      NULL,   "sort reverse"
    },
    {
      NULL, 0, 0, 0,
      NULL,   "\nOutput"
    },
    {
      "o",       1, 0, RSSTOOL_O,
      "FILE",   "output into FILE (default: stdout)"
    },
    {
      "rss",       2, 0, RSSTOOL_RSS,
      "TYPE",   "output as new feed\n"
                   "TYPE=1 will write RSS v1.0\n"
                   "TYPE=2 will write RSS v2.0 (default)" // "\n"
#warning finish support for writing proper Media RSS
//                   "TYPE=3 will write Media RSS v1.5.0"
    },
    {
      "xml",       0, 0, RSSTOOL_XML,
      NULL,   "output as normalized (proprietary) XML"
    },
    {
      NULL,       0, 0, 0,
      NULL, "\nReport problems/comments/ideas/whinge to noisyb@gmx.net\n"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

#if 0
  realpath2 (PROPERTY_HOME_RC ("rsstool"), rsstool.configfile);

  result = property_check (rsstool.configfile, QUH_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = set_property_array (rsstool.configfile, props);
  if (result == -1) // property_check() or update failed
    return -1;
#endif

  if (argc < 2)
    {
      getopt2_usage (options);
      exit (-1);
    }

  atexit (rsstool_exit);

  // defaults
  memset (&rsstool, 0, sizeof (st_rsstool_t));
  strncpy (rsstool.user_agent, RSSTOOL_USER_AGENT_S, sizeof (rsstool.user_agent))[sizeof (rsstool.user_agent) - 1] = 0;
  rsstool.start_time = time (0);
  rsstool.output_file = stdout;
  rsstool.timeout = 2; // default
#ifdef  USE_CURL
//  rsstool.get_flags = GET_NO_CURL; // curl is always the default
#endif

//  getopt2_short (short_options, options, ARGS_MAX);
  getopt2_long_only (long_only_options, options, ARGS_MAX);

  while ((c = getopt_long_only (argc, argv, "", long_only_options, &option_index)) != -1)
    switch (c)
      {
        case RSSTOOL_QUIET:
          rsstool.quiet = 1;
          break;

        case RSSTOOL_VER:
          printf ("rsstool version: %s\n", RSSTOOL_VERSION_S);
          exit (0);

        case RSSTOOL_HELP:
          getopt2_usage (options);
          exit (0);

        case RSSTOOL_FILTER:
          p = optarg;
          if (p)
            rsstool.strip_filter = p;
          break;

        case RSSTOOL_KEYWORDS:
          p = optarg;
          if (p)
            rsstool.strip_keywords = strtol (p, NULL, 10);
          break;

        case RSSTOOL_NOSORT:
          rsstool.nosort = 1;
          break;

        case RSSTOOL_REVERSE:
          rsstool.reverse = 1;
          break;

        case RSSTOOL_PARSE:
          if (!rsstool.output)
            rsstool.output = RSSTOOL_OUTPUT_RSS;
          p = optarg;
          if (p)
            if (access (p, F_OK) != 0)
              p = net_http_get_to_temp (p, rsstool.user_agent, rsstool.get_flags);

          if (p)
            rsstool_parse (p);
          else
            fputs ("ERROR: HTML document not found\n", stderr);
          break;

        case RSSTOOL_LOG:
          p = optarg;
          if (p)
            rsstool.log = fopen (p, "a");
//          rsstool_log (&rsstool, "start");
          break;

        case RSSTOOL_INPUT_FILE:
          p = optarg;
          if (p)
            rsstool.input_file = fopen (p, "r");
          if (!rsstool.input_file)
            fputs ("ERROR: input file not found\n", stderr);
          break;

        case RSSTOOL_ENC:
          p = optarg;
          if (p)
            strncpy (rsstool.encoding, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
          break;

        case RSSTOOL_O:
          p = optarg;
          if (p)
            rsstool.output_file = fopen (p, "w");
          if (!rsstool.output_file)
            {
              fprintf (stderr, "ERROR: could not open output file %s (using stdout)\n", p);
              rsstool.output_file = stdout;
            }
          break;

#ifdef  USE_ZLIB
        case RSSTOOL_GZIP:
          rsstool.get_flags |= GET_USE_GZIP;
          break;
#endif

#ifdef  USE_CURL
        case RSSTOOL_CURL:
//          rsstool.get_flags |= GET_USE_CURL;
          fprintf (stderr, "NOTE: "OPTION_LONG_S "curl has been deprecated; it is the default now\n");
          break;
#endif

        case RSSTOOL_SINCE:
          p = optarg;
          if (p)
            rsstool.since = strptime2 (p);
          break;

        case RSSTOOL_FIXDATE:
          rsstool.fixdate = 1;
          break;

        case RSSTOOL_SHTML:
          rsstool.strip_html = 1;
          p = optarg;
          if (p)
            rsstool.strip_html_allow = p;
          break;

        case RSSTOOL_SDESC:
          rsstool.strip_desc = 1;
          break;

        case RSSTOOL_SWHITE:
          rsstool.strip_whitespace = 1;
          break;

        case RSSTOOL_SLF:
          rsstool.strip_lf = 1;
          break;

        case RSSTOOL_SBIN:
          rsstool.strip_bin = 1;
          break;

        case RSSTOOL_HACK_GOOGLE:
          rsstool.hack_google = 1;
          break;

        case RSSTOOL_HACK_EVENT:
          rsstool.hack_event = 1;
          break;

        case RSSTOOL_U:
          p = optarg;
          if (p)
            strncpy (rsstool.user_agent, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
          break;

        case RSSTOOL_RSS:
          rsstool.rss_version = 2;
          p = optarg;
          if (p)
            rsstool.rss_version = strtol (p, NULL, 10);
          rsstool.output = RSSTOOL_OUTPUT_RSS;
          break;

        case RSSTOOL_XML:
          rsstool.output = RSSTOOL_OUTPUT_XML;
          break;

        default:
          fputs ("NOTE: some options have been deprecated\n\n"
                 "rsstool writes proprietary XML which can converted into SQL, CSV, etc.\n"
                 "using rsstool2sql and other scripts\n\n"
                 "Try 'rsstool " OPTION_LONG_S "help' for more information\n\n", stderr);
          exit (-1);
      }

  if (!optind)
    {
      getopt2_usage (options);
      exit (-1);
    }

  // get and parse the standard feeds
  while (1)
    {
      char *s = NULL;
      const char *feed_url = NULL;
      int feeds = rsstool_get_item_count (&rsstool);

      if (optind == argc && !rsstool.input_file) // no more feeds
        break;

      p = s = NULL;

      if (!p && optind < argc)
        p = s = argv[optind++];

      if (!p && rsstool.input_file)
        {
          if (fgets (buf2, MAXBUFSIZE, rsstool.input_file))
            {
              if ((s = strpbrk (buf2, "\r\n")))
                *s = 0;
              p = s = buf2;
            }
          else
            {
              fclose (rsstool.input_file);
              rsstool.input_file = NULL;
            }
        }

      if (!p) // no more feeds
        break;

      if (access (p, F_OK) != 0)
        {
          feed_url = p;
          p = net_http_get_to_temp (p, rsstool.user_agent, rsstool.get_flags);

          if (p)
            strncpy (rsstool.temp_file, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
        }

      if (!p)
        {
          sprintf (buf, "could not open/download %s", s);
          rsstool_log (&rsstool, buf);
          continue;
        }

      // normalize feed as string
      p = rsstool_normalize_feed (&rsstool, p);

      if (!p)
        {
          sprintf (buf, "could not normalize feed %s", s);
          rsstool_log (&rsstool, buf);
          continue;
        }

      if (!rsstool.output) // just print
        {
          if ((fh = fopen (p, "r")))
            {
              while (fgets (buf, MAXBUFSIZE, fh))
                fputs (buf, rsstool.output_file);

              fclose (fh);
            }
        }
      else
        {
          rsstool_parse_rss (&rsstool, feed_url, p);

          sprintf (buf, "%d feeds: ", rsstool_get_item_count (&rsstool) - feeds);
          strcat (buf, s);

          rsstool_log (&rsstool, buf);
        }

      if (*(rsstool.temp_file))
        {
          remove (rsstool.temp_file);
          *(rsstool.temp_file) = 0;
        }
    }

  if (!rsstool.output)
    return 0;

  if (!rsstool_get_item_count (&rsstool))
    {
      rsstool_log (&rsstool, "no feeds");
      return -1;
    }

  sprintf (buf, "%d feeds total", rsstool_get_item_count (&rsstool));
  rsstool_log (&rsstool, buf);

  // sort rss feed
  if (!rsstool.nosort) 
    rsstool_sort (&rsstool);

  if (rsstool.output)
    switch (rsstool.output)
      {
        case RSSTOOL_OUTPUT_RSS:
          rsstool_write_rss (&rsstool, rsstool.rss_version);
          break;

        case RSSTOOL_OUTPUT_XML:
          rsstool_write_xml (&rsstool);
          break;
    }
 
  return 0;
}
