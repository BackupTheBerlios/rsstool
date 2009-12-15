/*
string.h - some string functions

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh


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
#ifndef MISC_STRING_H
#define MISC_STRING_H
#ifdef  __cplusplus
extern "C" {
#endif
#include <string.h>


#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif


/*
  String manipulation

  strutf8()     encode string to utf-8
  utf8str()     decode utf-8 to string

  strupr()      strupr() clone
  strlwr()      strlwr() clone

  strtrim()     strtrim (str, isspace, isspace) is the same as
                  strtriml (strtrimr (str))
  strtriml()    removes all leading blanks from a string
  strtrimr()    removes all trailing blanks from a string
                  Blanks are defined with isspace (blank, tab, newline,
                  return, formfeed, vertical tab = 0x09 - 0x0D + 0x20)
                  You can combine: strtriml (strtrimr ()) or
                  strtrimr (strtriml ())
  strtrim_s()   same as strtrim() but compares strings instead of chars
                  strtrim_s("123bla456", "23", "45) == "bla"
  stritrim_s()  same as strtrim_s() but case-insensitive

  strmove()     copy/move (overlapping) strings
  strins()      insert string in front of string
  strrep_once() replace string inside a string
  strrep()      replace stringS inside a string

  str_escape_code()    turn string into code
  str_escape_html()    turn string into html
  str_unescape_html()  turn html into string


  strrstr()     like strstr() but reverse
  strristr()    like strrstr() but case-insensitive

  explode()      break string into array[max_args]

  memcmp2()     memcmp() replacement with flags for wildcard and
                  relative/shifted similarities support
                  MEMCMP2_WCARD(SINGLE_WC, MULTI_WC)
                  SINGLE_WC is the wildcard for single chars
                  MULTI_WC is the wildcard for multiple chars
                  MEMCMP2_REL
                  look for relative/shifted similarities
                  MEMCMP2_CASE
                  ignore case of isalpha() bytes
  memmem2()     memmem() replacement with flags for wildcard and
                  relative/shifted similarities support
                  MEMMEM2_WCARD(SINGLE_WC, MULTI_WC)
                  SINGLE_WC is the wildcard for single bytes
                  MULTI_WC is the wildcard for multiple bytes
                  MEMMEM2_REL
                  look for relative/shifted similarities
                  MEMMEM2_CASE
                  ignore case of isalpha() bytes
  stristr()     same as strcasestr()
  stricmp()     same as strcasecmp()
  strnicmp()    same as strncasecmp()
  strcasestr2() strcasestr() clone for non-GNU platforms
*/
//extern unsigned char *strutf8 (const char *s);
//extern char *utf8str (const unsigned char *s);

extern char *strlwr (char *str);
extern char *strupr (char *str);

extern char *strtrim (char *str, int (*left) (int), int (*right) (int));
extern char *strtriml (char *str);
extern char *strtrimr (char *str);
extern char *strtrim_s (char *str, const char *left, const char *right);
extern char *stritrim_s (char *str, const char *left, const char *right);

extern char *strmove (char *to, char *from);
extern char *strins (char *str, const char *ins);
extern char *strrep_once (char *str, const char *orig, const char *rep);
extern char *strrep (char *str, const char *orig, const char *rep);
extern char *strcat2 (const char *a, const char *b);

extern char *str_escape_code (char *str);
extern char *str_escape_html (char *str);
extern char *str_unescape_html (char *str);
extern char *str_escape_xml (char *str);

extern char *strrstr (char *str, const char *search);
extern char *strristr (char *str, const char *search);

extern int explode (char **argv, char *str, const char *separator_s, int max_args);

#define MEMCMP2_WCARD(WC)                 ((1 << 17) | ((WC) & 0xff))
#define MEMCMP2_REL                       (1 << 18)
#define MEMCMP2_CASE                      (1 << 19)
extern int memcmp2 (const void *buffer,
                    const void *search, size_t searchlen, unsigned int flags);
#define MEMMEM2_WCARD     MEMCMP2_WCARD
#define MEMMEM2_REL       MEMCMP2_REL
#define MEMMEM2_CASE      MEMCMP2_CASE
extern const void *memmem2 (const void *buffer, size_t bufferlen,
                            const void *search, size_t searchlen, unsigned int flags);

extern char *strcasestr2 (const char *str, const char *search);
// #define strcasestr strcasestr2
#define stristr strcasestr2


/*
  strfilter()  filter string with implied boolean logic
                 + stands for AND
                 - stands for NOT
                 no operator implies OR
                 implied_boolean_logic == "+important -unimportant"
                 returns 1 true, 0 false
*/
//extern int strfilter (const char *s, const char *implied_boolean_logic);


#ifdef  __cplusplus
}
#endif
#endif // MISC_STRING_H
