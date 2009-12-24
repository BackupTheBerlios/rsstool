/*
getopt2.h - getopt() extension
              new structure for defining options, build-in CGI support

Copyright (c) 2004 - 2009 NoisyB
Copyright (c) 2005        dbjh


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
#include "getopt.h"                             // struct option
#include "getopt2.h"
#include "string.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


const st_getopt2_t *
getopt2_get_option (const st_getopt2_t *option, int val)
{
  int i = 0;

  for (; option[i].name || option[i].help; i++)
    if (option[i].name && // it IS an option
        option[i].val == val)
      return &option[i];

  return NULL;
}


#if 0
int
getopt2_cgi (char **argv, const st_getopt2_t *option, int args_max)
{
  int argc = 0;
  char buf[MAXBUFSIZE];
  const char *query_string = getenv ("QUERY_STRING");

  if (!query_string)
    return 0; // no args

  // DEBUG
//  printf ("query_string: %s\n", query_string);

  strncpy (buf, query_string, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
//  argc = explode (char **argv, buf, "?&", args_max);

  return argc;
}


const char *
getopt_cgi_ui (const st_getopt2_t *option)
{
  getopt2_usage (option); // writes to stdout
  return "";
}
#endif


void
getopt2_usage (const st_getopt2_t *usage)
{
  int i = 0;
  char buf[MAXBUFSIZE];

  for (i = 0; usage[i].name || usage[i].help; i++)
    if (usage[i].help) // hidden options ARE allowed
      {
        if (usage[i].name)
          {
            sprintf (buf, "%s%s%s%s%s%s ",
              // long or short name?
              (usage[i].name[1] ? "  " OPTION_LONG_S : "   " OPTION_S),
              usage[i].name,
              (usage[i].arg_name && usage[i].has_arg == 2) ? "[" : "", // == 2 arg is optional
              usage[i].arg_name ? OPTARG_S : "",
              usage[i].arg_name ? usage[i].arg_name : "",
              (usage[i].arg_name && usage[i].has_arg == 2) ? "]" : ""); // == 2 arg is optional

            if (strlen (buf) < 16)
              {
                strcat (buf, "                             ");
                buf[16] = 0;
              }
            fputs (buf, stdout);
          }

        if (usage[i].help)
          {
            char c, *p = buf, *p2 = NULL;

            strcpy (buf, usage[i].help);

            if (usage[i].name)
              for (; (p2 = strchr (p, '\n')); p = p2 + 1)
                {
                  c = p2[1];
                  p2[1] = 0;
                  fputs (p, stdout);
                  fputs ("                  ", stdout);
                  p2[1] = c;
                }

            fputs (p, stdout);
            fputc ('\n', stdout);
          }
      }
}


static int
getopt2_long_internal (struct option *long_option, const st_getopt2_t *option,
                       int args_max, int long_only)
{
  int i = 0, j = 0, x = 0;

  memset (long_option, 0, sizeof (struct option) * args_max);

  for (; option[i].name || option[i].help; i++)
    if (option[i].name) // IS option
      if (long_only || option[i].name[1]) // IS long
                                          // if (long_only) also one char options are long
        {
          for (j = 0; j < i; j++)
            if (option[j].name)
              if (!strcmp (option[i].name, option[j].name))
                break; // no dupes

          if (j == i && x < args_max)
            {
              long_option[x].name = option[i].name;
              long_option[x].has_arg = option[i].has_arg;
              long_option[x].flag = option[i].flag;
              long_option[x++].val = option[i].val;
            }
      }

  return (x < args_max) ? (x + 1) : 0;
}


int
getopt2_long (struct option *long_option, const st_getopt2_t *option, int args_max)
{
  return getopt2_long_internal (long_option, option, args_max, 0);
}


int
getopt2_long_only (struct option *long_option, const st_getopt2_t *option, int args_max)
{
  return getopt2_long_internal (long_option, option, args_max, 1);
}


int
getopt2_short (char *short_option, const st_getopt2_t *option, int args_max)
{
  int i = 0;
  char *p = short_option;

  *p = 0;
  for (; option[i].name || option[i].help; i++)
    if ((int) strlen (short_option) + 3 < args_max && option[i].name) // IS option
      if (!option[i].name[1]) // IS short
        if (!strchr (short_option, option[i].name[0])) // no dupes
          {
            *p++ = option[i].name[0];
            switch (option[i].has_arg)
              {
              case 2:
                *p++ = ':';
              case 1:                           // falling through
                *p++ = ':';
              case 0:
                break;
              default:
                fprintf (stderr, "ERROR: getopt2_short(): unexpected has_arg value (%d)\n", option[i].has_arg);
              }
            *p = 0;
          }

#ifdef  DEBUG
  printf ("getopt2_short(): short_option: %s\n", short_option);
  fflush (stdout);
#endif

  return (int) strlen (short_option) + 3 < args_max ? (int) strlen (short_option) : 0;
}


#if TEST
// compile with -DTEST to build an executable

enum
{
  OPTION_HELP = 1,
  OPTION_BBB,
  OPTION_C,
  OPTION_D
};


#define MAX_OPTIONS 256


int
main (int argc, char **argv)
{
  const char *str = "test";
  int c, digit_optind = 0;
  st_getopt2_t option[] =
    {
      {
        "help", 0, 0, OPTION_HELP,
        NULL, "show this output and exit"
      },
      {
        "bbb", 0, 0, OPTION_BBB,
        NULL, "option bbb"
      },
      {
        "c", 1, 0, OPTION_C,
        "ARG", "option c with required ARG"
      },
      {
        "d", 2, 0, OPTION_D,
        "ARG", "option d with optional ARG"
      },
      {NULL, 0, 0, 0, NULL, NULL}
    };
  struct option option_long[MAX_OPTIONS];
  char option_short[MAX_OPTIONS*3];

  // turn st_getopt2_t into struct option
  getopt2_long (option_long, option, MAX_OPTIONS);
  getopt2_short (option_short, option, MAX_OPTIONS);

//  printf ("option_short: \"%s\"\n", option_short);

  optind = 0;
  while ((c = getopt_long (argc, argv, option_short, option_long, NULL)) != -1)
    {
      if (c == '?') // getopt() returns 0x3f ('?') when an unknown option was given
        {
          printf ("Try '%s " OPTION_LONG_S "help' for more information.\n",
            argv[0]);
          exit (1);
        }

      if (c == -1)
        break;

      switch (c)
        {
        case OPTION_BBB:
          printf ("option bbb\n");
          break;

        case OPTION_C:
          printf ("option c with required value '%s'\n", optarg);
          break;

        case OPTION_D:
          printf ("option d with optional value '%s'\n", optarg);
          break;

        case OPTION_HELP:
          getopt2_usage (option);
          break;

        default:
          printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

  if (optind < argc)
    {
      printf ("files: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      printf ("\n");
    }

  exit (0);
}
#endif
