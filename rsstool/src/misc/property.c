/*
property.c - configfile handling

Copyright (c) 2004 - 2005 NoisyB


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
#include <sys/stat.h>                           // for struct stat
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include "string.h"
#include "property.h"


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif 


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


// TODO: make update of configfiles more intelligent
//         allow property with quotes at start and end?

int
property_check (const char *filename, int version, int verbose)
{
  char buf[MAXBUFSIZE];
  const char *p = NULL;
  int result = 0;

  if (access (filename, F_OK) != 0)
    {
      FILE *fh = NULL;

      if (verbose) 
        {
          fprintf (stderr, "NOTE: %s not found: creating...", filename);
          fflush (stderr);
        }

      // create new config file
      if (!(fh = fopen (filename, "w")))
        {
          printf ("FAILED\n\n");
          return -1;
        }
      fclose (fh);                              // we'll use set_property() from now
    }
  else
    {
      p = get_property (filename, "version");

      if (strtol (p ? p : "0", NULL, 10) >= version)
        return 0; // OK

      strcpy (buf, filename);
      strcat (buf, "_bak");

      if (verbose)
        {
          fprintf (stderr, "NOTE: updating config: will be renamed to %s...", buf);
          fflush (stderr);
        }

      rename (filename, buf);
    }

  // store new version
  sprintf (buf, "%d", version);
  result = set_property (filename, "version", buf, "configfile version (do NOT edit)");

  if (result > 0)
    {
      if (verbose)
        fprintf (stderr, "OK\n\n");
    }
  else
    {
      if (verbose)
        fprintf (stderr, "FAILED\n\n");

      // remove the crap
      remove (filename);
    }

  if (verbose)
    fflush (stderr);

  return 1;
}


static const char *
get_property_from_line (const char *line, const char *propname)
{
  static char buf[MAXBUFSIZE];
  char *p = NULL;

  strncpy (buf, line, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  if ((p = strpbrk (buf, "\r\n" PROPERTY_COMMENT_S)))  // strip any returns and comments
    *p = 0;

  p = strtriml (buf);
  if (!(*p))
    return NULL;                                // text after comment_sep is comment

  p = strchr (buf, PROPERTY_SEPARATOR);
  if (p)
    *p = 0;

  strtriml (strtrimr (buf));
  if (strcasecmp (buf, propname) != 0) // check property name
    return NULL;

  if (!p) // no prop_sep so empty value
    return "";

  p++;
  strncpy (buf, line + (p - buf), MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  for (p = buf + 1; *p; p++)
    if (*p == PROPERTY_COMMENT)
      {
        if (*(p - 1) == PROPERTY_ESCAPE)
          {
            // unescape escaped PROPERTY_COMMENT
            p = strmove (p - 1, p);
          }
        else
          {
            // break at first not escaped PROPERTY_COMMENT
            *p = 0;
            break;
          }
      }

  return buf;
}


const char *
get_property (const char *filename, const char *propname /*, int mode */)
{
  char line[MAXBUFSIZE]; // , *p = NULL;
  FILE *fh;
  const char *value_s = NULL;

  if (!(fh = fopen (filename, "r")))
    return NULL;

  while (fgets (line, sizeof line, fh) != NULL)
    if ((value_s = get_property_from_line (line, propname)))
      break;

  fclose (fh);

  return value_s;
}


signed long int
get_property_int (const char *filename, const char *propname)
{
  signed long int value = 0;
  const char *value_s = get_property (filename, propname);

  if (!value_s)
    return 0;

  if (!(*value_s))
    return 0;

  if (strchr ("n0", tolower (*value_s))) // 0 or [Nn]o
    return 0;
 
  value = strtol (value_s, NULL, 10);
  return MAX (1, value);     //  we'll return at least 1
//  return value;
}


int
set_property (const char *filename, const char *propname,
              const char *value_s, const char *comment_s)
{
  int found = 0, result = 0;
  unsigned long int file_size = 0;
  char line[MAXBUFSIZE], line2[MAXBUFSIZE], *str = NULL, *p = NULL;
  FILE *fh;
  struct stat fstate;

  if (stat (filename, &fstate) == 0)
    file_size = fstate.st_size;

  if (!(str = (char *) malloc (file_size + MAXBUFSIZE)))
    return -1;

  *str = 0;
  if ((fh = fopen (filename, "r")) != 0)
    {
      // update existing properties
      while (fgets (line, sizeof line, fh) != NULL)
        {
          strcpy (line2, line);
          if ((p = strpbrk (line2, "\r\n" PROPERTY_COMMENT_S)))
            *p = 0;
          p = strchr (line2, PROPERTY_SEPARATOR);
          if (p)
            *p = 0;

          strtriml (strtrimr (line2));

          if (!strcasecmp (line2, propname))
            {
              found = 1;
              if (value_s)
                sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s ? value_s : "1");
            }
          strcat (str, line);
        }
      fclose (fh);
    }

  // completely new properties are added at the bottom
  if (!found && value_s)
    {
      if (comment_s)
        {
          sprintf (strchr (str, 0), "%c\n%c ", PROPERTY_COMMENT, PROPERTY_COMMENT);

          for (p = strchr (str, 0); *comment_s; comment_s++)
            switch (*comment_s)
              {
              case '\r':
                break;
              case '\n':
                sprintf (strchr (str, 0), "\n%c ", PROPERTY_COMMENT);
                break;

              default:
                p = strchr (str, 0);
                *p = *comment_s;
                *(++p) = 0;
                break;
              }

          sprintf (strchr (str, 0), "\n%c\n", PROPERTY_COMMENT);
        }

      sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s);
      strcat (str, line);
    }

  if ((fh = fopen (filename, "w")) == NULL)
    {
      free (str);
      return -1;
    }

  result = fwrite (str, 1, strlen (str), fh);
  fclose (fh);

  free (str);

  return result;
}


int
set_property_array (const char *filename, const st_property_t *prop)
{
  int i = 0, result = 0;

  for (; prop[i].name; i++)
    {
      result = set_property (filename, prop[i].name, prop[i].value_s,
                             prop[i].comment_s);

      if (result == -1) // failed
        break;
    }

  return result;
}


int
set_property_int (const char *filename, const char *propname,
                  signed long int value, const char *comment)
{
  char buf[64];
  sprintf (buf, "%ld", value);
  return set_property (filename, propname, buf, comment);
}
