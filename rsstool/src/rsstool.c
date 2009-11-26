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
#include "misc/defines.h"
#include "misc/string.h"
#include "misc/getopt2.h"
#include "misc/misc.h"
#include "misc/net.h"
#include "misc/rss.h"
#include "misc/hash_crc.h"
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

  if (*(rsstool.pipe_command) && rsstool.output_file != stdout)
    {
      pclose (rsstool.output_file);
      rsstool.output_file = NULL;
    }
  else if (rsstool.output_file != stdout && rsstool.output_file != stderr)
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
      "rsstool " RSSTOOL_VERSION_S " " CURRENT_OS_S " 2006-2009 by NoisyB\n"
      "This may be freely redistributed under the terms of the GNU Public License\n\n"
      "Usage: rsstool [OPTION] FILE(S)... URL(S)...\n"
    },
    {
      NULL,      0, 0, 0,
      NULL,   "Read, parse, merge and write RSS and Atom feeds\n"
              "\nSupports: RSS 0.9x, 1.0, 2.0 and Atom 0.1, 0.2, 0.3, 1.0\n"
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
      "FILE|URL", "generate RSS feed from random HTML document"
    },
    {
      "log", 1, 0, RSSTOOL_LOG,
      "FILE", "write a log to FILE (including HTTP headers)"
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
      "fixdate", 0, 0, RSSTOOL_FIXDATE,
      NULL,  "missing dates will be replaced with the current date"
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
      NULL,   "\nStrip & Sort"
    },
    {
      "shtml",        0, 0, RSSTOOL_SHTML,
      NULL,   "strip HTML code from descriptions"
    },
    {
      "shtml2",       0, 0, RSSTOOL_SHTML2,
      NULL,   "like " OPTION_LONG_S "shtml but keeps links intact"
    },
    {
      "s",        0, 0, RSSTOOL_SHTML,
      NULL, NULL
    },
    {
      "swhite",        0, 0, RSSTOOL_SWHITE,
      NULL,   "strip whitespace from descriptions"
    },
    {
      "slf",        0, 0, RSSTOOL_SLF,
      NULL,   "strip line feeds/carriage returns from descriptions"
    },
#if 0
    {
      "stitle",        0, 0, RSSTOOL_STITLE,
      NULL,   "strip the title from feeds"
    },
#endif
    {
      "sdesc",        0, 0, RSSTOOL_SDESC,
      NULL,   "strip the whole descriptions from feeds"
    },
    {
      "sd",        0, 0, RSSTOOL_SDESC,
      NULL, NULL
    },
    {
      "1",       0, 0, RSSTOOL_SDESC,
      NULL, NULL
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
      "pipe",       1, 0, RSSTOOL_PIPE,
      "CMD",   "pipe the output for a _single_ item to CMD\n"
               "for used with " OPTION_LONG_S "html, " OPTION_LONG_S "txt, " OPTION_LONG_S "href or " OPTION_LONG_S "template2, only"
    },
    {
      "rss",       2, 0, RSSTOOL_RSS,
      "VERSION",   "output as RSS feed\n"
                   "VERSION=1 will write RSS v1.0\n"
                   "VERSION=2 will write RSS v2.0 (default)"
    },
    {
      "php",       0, 0, RSSTOOL_PHP,
      NULL,   "output as PHP code (array ())"
    },
    {
      "html",       0, 0, RSSTOOL_HTML,
      NULL,   "output as html"
    },
    {
      "bookmarks",       0, 0, RSSTOOL_BOOKMARKS,
      NULL,   "output as bookmarks.html for use with Mozilla or Firefox"
    },
    {
      "href", 0, 0, RSSTOOL_HREF,
      NULL, "output only the links as plain text"
    },
    {
      "txt",        0, 0, RSSTOOL_TXT,
      NULL,   "output as plain text"
    },
    {
      "csv",       2, 0, RSSTOOL_CSV,
      "SEPARATOR",   "output as comma-separated values CSV"
    },
    {
      "prop", 0, 0, RSSTOOL_PROP,
      NULL,   "output as properties\n"
                    "Layout: NAME_NUM=VALUE\n"
                    "        NAME  will be \"title\", \"url\", etc..\n"
                    "        NUM   will be an integer counting the items\n"
                    "        VALUE will be the content"
    },
    {
      "property",       2, 0, RSSTOOL_PROPERTY,
      NULL, NULL
    },
    {
      "template", 1, 0, RSSTOOL_TEMPLATE,
      "FILE|URL",   "parse template file and replace tags with content\n"  
                    "Tags: \"<rsstool:url item=NUM>\"   url/link of item NUM\n"
                    "      \"<rsstool:title item=NUM>\" title of item NUM\n"
                    "      \"<rsstool:desc item=NUM>\"  description of item\n"
                    "                                   NUM\n"
                    "      \"<rsstool:date item=NUM>\"  date of item NUM\n"
                    "      \"<rsstool:site item=NUM>\"  site where item NUM\n"
                    "                                   came from\n"
                    "      \"<rsstool:start item=NUM>\" start of item NUM\n"
                    "                                   will be replaced with\n"
                    "                                   \"<!--\" if item NUM is\n"
                    "                                   empty\n"
                    "      \"<rsstool:end item=NUM>\"   end of item NUM\n"
                    "                                   will be replaced with\n"
                    "                                   \"-->\" if item NUM is\n"
                    "                                   empty\n"
                    "      \"<rsstool:rsstool>\"        rsstool notice with\n"
                    "                                   version\n"  
                    "      \"<rsstool:updated>\"        rsstool notice with\n"
                    "                                   version and date"
    },
    {
      "template2", 1, 0, RSSTOOL_TEMPLATE2,
      "FILE|URL",   "same as " OPTION_LONG_S "template but repeats the whole\n"
                    "template for every single item"
    },
    {
      "sql", 2, 0, RSSTOOL_SQL,
      "VALUE",   "output as ANSI SQL script"
#if 0
"\n"
                 // deprecated
                 "VALUE=092       RSStool 0.9.2 db format\n"
                 "VALUE=094       RSStool 0.9.4 db format\n"
                 "VALUE=095       RSStool 0.9.5 db format\n"
                 "VALUE=\"current\" use current db format (default)"
#endif
    },
    {
      "sqlold", 0, 0, RSSTOOL_SQLOLD,
      NULL,   NULL
    },
    {
      NULL, 0, 0, 0,
      NULL,   "\nDeprecated"
    },
    {
      "joomla", 0, 0, RSSTOOL_JOOMLA,
      NULL,   "output as ANSI SQL script for import into Joomla! CMS"
    },
    {
      "dragonfly", 0, 0, RSSTOOL_DRAGONFLY,
      NULL,   "output as ANSI SQL script for import into Dragonfly CMS"
    },
    {
      NULL,       0, 0, 0,
      NULL, "\nReport problems/comments/ideas/whinge to noisyb@gmx.net\n"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

#if 0
  realpath2 (PROPERTY_HOME_RC ("quh"), quh.configfile);

  result = property_check (quh.configfile, QUH_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = set_property_array (quh.configfile, props);
  if (result == -1) // property_check() or update failed
    return -1;
#endif

  if (argc < 2)
    {
      getopt2_usage (options);
      exit (-1);
    }

  atexit (rsstool_exit);

  memset (&rsstool, 0, sizeof (st_rsstool_t));

  strncpy (rsstool.user_agent, RSSTOOL_USER_AGENT_S, sizeof (rsstool.user_agent))[sizeof (rsstool.user_agent) - 1] = 0;
  rsstool.start_time = time (0);
  rsstool.output_file = stdout;
  rsstool.csv_separator = ',';
  rsstool.timeout = 2; // default

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
            rsstool_get_links (p);
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

        case RSSTOOL_PIPE:
          p = optarg;
          rsstool.output_file = stdout; // default (changed later)
          if (p)
            strncpy (rsstool.pipe_command, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
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
          break;

        case RSSTOOL_SHTML2:
          rsstool.strip_html = 2;
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

        case RSSTOOL_U:
          p = optarg;
          if (p)
            strncpy (rsstool.user_agent, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
          break;

        case RSSTOOL_HTML:
          rsstool.output = RSSTOOL_OUTPUT_HTML;
          break;

        case RSSTOOL_PHP:
          rsstool.output = RSSTOOL_OUTPUT_PHP;
          rsstool.strip_html = 1;
          rsstool.strip_whitespace = 1;
          rsstool.strip_lf = 1;
          break;

        case RSSTOOL_TXT:
          rsstool.output = RSSTOOL_OUTPUT_TXT;
          rsstool.strip_html = 1;
          break;

        case RSSTOOL_HREF:
          rsstool.output = RSSTOOL_OUTPUT_HREF;
          break;

        case RSSTOOL_BOOKMARKS:
          rsstool.output = RSSTOOL_OUTPUT_BOOKMARKS;
          break;

        case RSSTOOL_CSV:
          rsstool.output = RSSTOOL_OUTPUT_CSV;
          rsstool.strip_html = 1;
          rsstool.strip_whitespace = 1;
          rsstool.strip_lf = 1;
          p = optarg;
          if (p)   
            rsstool.csv_separator = *p;
          break;

        case RSSTOOL_RSS:
          rsstool.rss_version = 2;
          p = optarg;
          if (p)
            rsstool.rss_version = strtol (optarg, NULL, 10);
          rsstool.output = RSSTOOL_OUTPUT_RSS;
          break;

        case RSSTOOL_PROP:
          rsstool.output = RSSTOOL_OUTPUT_PROPERTY;
          rsstool.strip_html = 1;
          rsstool.strip_whitespace = 1;
          rsstool.strip_lf = 1;
          break;

        case RSSTOOL_PROPERTY:
          fputs ("WARNING: " OPTION_LONG_S "property has bee deprecated; use " OPTION_LONG_S "prop and " OPTION_S "o instead\n", stderr);
          break;

        case RSSTOOL_TEMPLATE:
          p = optarg;
          if (p)
            if (access (p, R_OK) != 0)
              p = net_http_get_to_temp (p, rsstool.user_agent, rsstool.get_flags);

          if (p)
            {
              rsstool.output = RSSTOOL_OUTPUT_TEMPLATE;
              strncpy (rsstool.template_file, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
            }
          else
            fputs ("ERROR: template not found\n", stderr);
          break;

        case RSSTOOL_TEMPLATE2:
          p = optarg;
          if (p)
            if (access (p, R_OK) != 0)
              p = net_http_get_to_temp (p, rsstool.user_agent, rsstool.get_flags);

          if (p)
            {
              rsstool.output = RSSTOOL_OUTPUT_TEMPLATE2;
              strncpy (rsstool.template_file, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
            }
          else
            fputs ("ERROR: template not found\n", stderr);
          break;

        case RSSTOOL_SQL:
          rsstool.ansisql_version = 0;
          p = optarg;
          if (p)
            rsstool.ansisql_version = strtol (p, NULL, 10);
          rsstool.output = RSSTOOL_OUTPUT_SQL;
          break;

        case RSSTOOL_SQLOLD:
          rsstool.output = RSSTOOL_OUTPUT_SQLOLD;
          break;

        case RSSTOOL_JOOMLA:
          rsstool.output = RSSTOOL_OUTPUT_JOOMLA;
          break;

        case RSSTOOL_DRAGONFLY:
          rsstool.output = RSSTOOL_OUTPUT_DRAGONFLY;
          break;

#ifdef  USE_ODBC
        case RSSTOOL_ODBC:
          p = optarg;
          if (p)
            {
              rsstool.output = RSSTOOL_OUTPUT_ODBC;
              strncpy (rsstool.dburl, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
            }
          else
            fputs ("ERROR: db not found\n", stderr);
          break;
#endif
#ifdef  USE_MYSQL
        case RSSTOOL_MYSQL:
          p = optarg;
          if (p)
            {
              rsstool.output = RSSTOOL_OUTPUT_MYSQL;
              strncpy (rsstool.dburl, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
            }
          else
            fputs ("ERROR: db not found\n", stderr);
          break;
#endif

        default:
          fputs ("Try 'rsstool " OPTION_LONG_S "help' for more information\n\n", stdout);
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
              s = strchr (buf2, '\n');
              if (s)
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

      if (p)
        {
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
      else
        {
          sprintf (buf, "could not open/download %s", s);
          rsstool_log (&rsstool, buf);
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
        case RSSTOOL_OUTPUT_HTML:
          rsstool_write_html (&rsstool);
          break;

        case RSSTOOL_OUTPUT_TXT:
          rsstool_write_txt (&rsstool);
          break;

        case RSSTOOL_OUTPUT_HREF:
          rsstool_write_href (&rsstool);
          break;

        case RSSTOOL_OUTPUT_SQL:
          switch (rsstool.ansisql_version)
            {
#if 0
              // deprecated
              case 92:
                rsstool_write_ansisql_092 (&rsstool);
                break;

              case 94:
                rsstool_write_ansisql_094 (&rsstool);
                break;

              case 95:
                rsstool_write_ansisql_095 (&rsstool);
                break;
#endif
              default:
                rsstool_write_ansisql (&rsstool);
            }
          break;

#if 0
        // deprecated
        case RSSTOOL_OUTPUT_SQLOLD:
          rsstool_write_ansisql_095 (&rsstool);
          break;
#endif
        case RSSTOOL_OUTPUT_JOOMLA:
          rsstool_write_ansisql_joomla (&rsstool);
          break;

        case RSSTOOL_OUTPUT_DRAGONFLY:
          rsstool_write_ansisql_dragonfly (&rsstool);
          break;

        case RSSTOOL_OUTPUT_BOOKMARKS:
          rsstool_write_bookmarks (&rsstool);
          break;

        case RSSTOOL_OUTPUT_CSV:
          rsstool_write_csv (&rsstool, rsstool.csv_separator);
          break;

        case RSSTOOL_OUTPUT_PHP:
          rsstool_write_php (&rsstool);
          break;

        case RSSTOOL_OUTPUT_RSS:
          rsstool_write_rss (&rsstool, rsstool.rss_version);
          break;

#ifdef  USE_ODBC
        case RSSTOOL_OUTPUT_ODBC:
          rsstool_write_odbc (&rsstool);
          break;
#endif

#ifdef  USE_MYSQL
        case RSSTOOL_OUTPUT_MYSQL:
          rsstool_write_mysql (&rsstool);
          break;
#endif

        case RSSTOOL_OUTPUT_TEMPLATE:
          if (*rsstool.template_file)
            rsstool_write_template (&rsstool, rsstool.template_file);
          else
            fputs ("ERROR: no input template specified\n", stderr);
          break;

        case RSSTOOL_OUTPUT_TEMPLATE2:
          if (*rsstool.template_file)
            rsstool_write_template2 (&rsstool, rsstool.template_file);
          else
            fputs ("ERROR: no input template specified\n", stderr);
          break;

        case RSSTOOL_OUTPUT_PROPERTY:
          rsstool_write_property (&rsstool);
          break;
    }
 
  return 0;
}
