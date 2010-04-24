/*
property.h - configfile handling

Copyright (c) 2004 NoisyB


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
#ifndef MISC_PROPERTY_H
#define MISC_PROPERTY_H
#ifdef  __cplusplus
extern "C" {
#endif


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined AMIGA || defined __APPLE__      // Mac OS X actually
// GNU/Linux, Solaris, FreeBSD, OpenBSD, Cygwin, BeOS, Amiga, Mac (OS X)
#define FILE_SEPARATOR '/'  
#define FILE_SEPARATOR_S "/"
#else // DJGPP, Win32
#define FILE_SEPARATOR '\\'  
#define FILE_SEPARATOR_S "\\"
#endif


typedef struct
{
  const char *name;                             // property name
  const char *value_s;                          // property value
  const char *comment_s;                        // property comment, # COMMENT
} st_property_t;                                // NAME=VALUE


#define PROPERTY_ESCAPE '\\'
#define PROPERTY_SEPARATOR '='
#define PROPERTY_SEPARATOR_S "="
//#define PROPERTY_ENCLOSURE '"'
//#define PROPERTY_ENCLOSURE_S "\""
#define PROPERTY_COMMENT '#'
#define PROPERTY_COMMENT_S "#"


/*
  property_check()       makes sure the configfile with filename exists and
                           has the right version.
                           returns:
                           -1 == failure (property file could not be written)
                           0 == everything is OK
                           1 == property file needs update; set_property() and
                                set_property_array() must be used to update it

  get_property()       get value of propname from filename
  get_property_int()   like get_property() but returns an integer which is 0
                         if the value of propname was 0, [Nn] or [Nn][Oo]
                         and an integer or at least 1 for every other case

  set_property()       set propname with value in filename
  set_property_array() set an array of properties (st_property_t) at once

  DELETE_PROPERTY()    like set_property but when value of propname is NULL
                         the whole property will disappear from filename
*/
#ifdef  __MSDOS__
#define PROPERTY_HOME_RC(n) "~" FILE_SEPARATOR_S n ".cfg"
#else
#define PROPERTY_HOME_RC(n) "~" FILE_SEPARATOR_S "." n "rc"
#endif

#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
#define PROPERTY_MODE_GLOBAL_RC(n)  n ".cfg"
#else
#define PROPERTY_MODE_GLOBAL_RC(n)  FILE_SEPARATOR_S "etc" FILE_SEPARATOR_S n
#endif

#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
#define PROPERTY_MODE_DIR(n) "~" FILE_SEPARATOR_S 
#elif   defined __unix__ || defined __BEOS__ || defined __APPLE__ // Mac OS X actually
#define PROPERTY_MODE_DIR(n) "~" FILE_SEPARATOR_S "." n FILE_SEPARATOR_S
#else
#define PROPERTY_MODE_DIR(n) ""
#endif


extern int property_check (const char *filename, int version, int verbose);

extern const char *get_property (const char *filename, const char *propname);
extern signed long int get_property_int (const char *filename, const char *propname);

extern int set_property (const char *filename, const char *propname,
                         const char *value, const char *comment);
extern int set_property_int (const char *filename, const char *propname,
                             signed long int value, const char *comment);
extern int set_property_array (const char *filename, const st_property_t *prop);

#define DELETE_PROPERTY(a,b) (set_property(a, b, NULL, NULL))


#ifdef  __cplusplus
}
#endif
#endif // MISC_PROPERTY_H
