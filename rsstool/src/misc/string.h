/*
string.h - some string functions

Copyright (c) 1999 - 2009 NoisyB
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

TODO:  strutf8()     encode string to utf-8
TODO:  utf8str()     decode utf-8 to string

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
  strcasetrim_s()  same as strtrim_s() but case-insensitive

  strmove()     copy/move (overlapping) strings
  strins()      insert string in front of string
  strrep_once() replace string inside string
  strrep()      replace stringS inside string
  strcat2()     concat two strings as new string (malloc)

  str_escape_code()    turn string into code
  str_escape_html()    turn string into html
  str_unescape_html()  turn html into string
  stresc()             replace chars with %xx escape sequences
  strunesc()           replace %xx escape sequences with the char

  strrstr()     like strstr() but reverse
  strrcasestr()    like strrstr() but case-insensitive
  strcasestr2() strcasestr() clone for non-GNU platforms

  strtok2()     break string into array[max_args]
                  any of the characters of separators breaks the string
  explode()     like strtok2() but
                  only the whole separator_s breaks the string
  implode()     turn array into string

  str_getline() like fgets but works with strings instead with FILE

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
*/
//extern unsigned char *strutf8 (const char *s);
//extern char *utf8str (const unsigned char *s);

extern char *strlwr (char *str);
extern char *strupr (char *str);

extern char *strtrim (char *str, int (*left) (int), int (*right) (int));
extern char *strtriml (char *str);
extern char *strtrimr (char *str);
extern char *strtrim_s (char *str, const char *left, const char *right);
extern char *strcasetrim_s (char *str, const char *left, const char *right);
#define stritrim_s strcasetrim_s
//#define trim(s) strtriml(strtrimr(s))

extern char *strmove (char *to, char *from);
extern char *strins (char *str, const char *ins);
extern char *strrep_once (char *str, const char *orig, const char *rep);
extern char *strrep (char *str, const char *orig, const char *rep);
extern char *strcaserep (char *str, const char *orig, const char *rep);
#define str_replace(o,r,s) strrep(s,o,r)
#define str_ireplace(o,r,s) strcaserep(s,o,r)

extern char *strcat2 (const char *a, const char *b);

extern char *str_escape_code (char *str);
extern char *str_escape_html (char *str);
extern char *str_unescape_html (char *str);
extern char *str_escape_xml (char *str);
extern char *stresc (char *dest, const char *src);
extern char *strunesc (char *dest, const char *src); 

extern char *strrstr (char *str, const char *search);
extern char *strrcasestr (char *str, const char *search);
#define strristr strrcasestr
extern char *strcasestr2 (const char *str, const char *search);
//#ifndef __USE_GNU
//#define strcasestr strcasestr2
//#endif
#define stristr strcasestr2

extern int strtok2 (char **argv, char *str, const char *separators, int max_args);
extern int explode (char **argv, char *str, const char *separator_s, int max_args);
//extern int explode_csv (char **argv, char *str, const char *delimiter, // ','
//                                                const char *enclosure, // '"'
//                                                const char *escape);   // '\\'
extern const char *implode (const char *separator_s, char **argv);

extern int str_getline (char *line, int line_num, const char *buffer, int buffer_len);
extern const char *strline (const char *s, int line);


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


/*
  these are clones of PHP functions with the same name

  parse_url_component()  http://username:password@hostname/path?arg=value#anchor
  parse_url()            like parse_url_component() but returns all components

  RFC 2068 states:

  Servers should be cautious about depending on URI lengths above 255 bytes,
  because some older client or proxy implementations may not properly support
  these lengths.

  The spec for URL length does not dictate a minimum or maximum URL length,
  but implementation varies by browser.  On Windows: Opera supports ~4050
  characters, IE 4.0+ supports exactly 2083 characters, Netscape 3 -> 4.78
  support up to 8192 characters before causing errors on shut-down, and
  Netscape 6 supports ~2000 before causing errors on start-up.

  Note that there is no limit on the number of parameters you can stuff into a
  URL, but only on the length it can aggregate to.
*/
#define PARSE_URL_MAXBUFSIZE 8192 
enum {
  PHP_URL_SCHEME = 0,  // components
  PHP_URL_USER,
  PHP_URL_PASS,
  PHP_URL_HOST,
  PHP_URL_PORT,
  PHP_URL_PATH,
  PHP_URL_QUERY,
  PHP_URL_FRAGMENT,

  URL_REQUEST  // PHP_URL_PATH + PHP_URL_QUERY + PHP_URL_FRAGMENT
};
extern const char *parse_url_component (const char *url_s, int component);
typedef struct
{
  const char *scheme;    // http
  const char *user;
  const char *pass;
  const char *host;
  const char *port;      // do NOT add a port_s to the structure again
  const char *path;      // /path
  const char *query;     // arg=value
  const char *fragment;  // anchor
  char priv[PARSE_URL_MAXBUFSIZE];
} st_parse_url_t;
extern int parse_url (st_parse_url_t *url, const char *url_s);


/*
  parse_str()            parses query as if it were the query string passed via a URL
                           and sets variables as name and value pairs
                         TODO: support arrays arr[]=foo+bar&arr[]=baz
*/
typedef struct
{
  const char *name;
  const char *value;
} st_parse_str_pairs_t;
#define PARSE_STR_MAXPAIRS 255
#define PARSE_STR_MAXBUFSIZE 32768
typedef struct
{
  st_parse_str_pairs_t p[PARSE_STR_MAXPAIRS];
  char priv[PARSE_STR_MAXBUFSIZE];
} st_parse_str_t;
extern int parse_str (st_parse_str_t *pairs, const char *query);


/*
TODO:  strfilter()  filter string with implied boolean logic
                 + stands for AND
                 - stands for NOT
                 no operator implies OR
                 implied_boolean_logic == "+important -unimportant"
                 returns 1 true, 0 false
                 NOTE: works pretty much like "MATCH ... AGAINST" in ANSI SQL
*/
extern int strfilter (const char *s, const char *implied_boolean_logic);


//  misc_get_keywords()  extract keywords from a string
extern char *misc_get_keywords (char *s, int flag);


#ifdef  __cplusplus
}
#endif
#endif // MISC_STRING_H
