/*
string.c - some string functions

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "string.h"


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#define MAXBUFSIZE 32768


#if 0
static int
is_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  /*
    Casting to unsigned char * is necessary to avoid differences between the
    different compilers' run-time environments. At least for isprint(). Without
    the cast the isprint() of (older versions of) DJGPP, MinGW, Cygwin and
    Visual C++ returns nonzero values for ASCII characters > 126.
  */
  for (; len >= 0; p++, len--)
    if (!func (*(unsigned char *) p))
      return 0;

  return 1;
}


static char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}
#endif


#if 0
unsigned char *
strutf8 (const char *s)
{
  static unsigned char *d = NULL;
  unsigned char *p = NULL;

  if (d)
    free (d);

  d = malloc ((strlen (s) + 1) * 3);

  if (!d)
    return NULL;

  p = d;

//  strcpy (p, s);
//  strrep (p, "\r\n", "\n");

  for (; *s; s++, p++)
    {
//      if (*s == '\r')
//        continue;

      if (*s < 128)
        *p = *s;
      else if (*s > 127 && *s < 2048)
        {
          *p = ((*s) >> 6) | 192;
          p++;
          *p = ((*s) & 63) | 128;
        }
      else
        {
          *p = ((*s) >> 12) | 224;
          p++;
          *p = (((*s) >> 6) & 63) | 128;
          p++;
          *p = ((*s) & 63) | 128;
        }
    }

  return d;
}


char *
utf8str (const unsigned char *s)
{
  static char *d = NULL;
  char *p = NULL;

  if (d)
    free (d);

  d = malloc (strlen (s));

  if (!d)
    return NULL;

  p = d;

  for (; *s; s++, p++)
    {
      if (*s < 128)
        *p = *s;
      else if (*s > 191 && *s < 224)
        {
          *p = ((*s) << 6) | ((*(s + 1)) & 63);
          s++;
        }
      else
        {
          *p = (((*s) & 15) << 12) | (((*(s + 1)) & 63) << 6) | ((*(s + 1)) & 63);
          s += 2;
        }
    }

  return d;
}
#endif


char *
strupr (char *s)
{
  char *p = s;

  for (; *p; p++)
    *p = toupper (*p);

  return s;
}


char *
strlwr (char *s)
{
  char *p = s;

  for (; *p; p++)
    *p = tolower (*p);

  return s;
}


char *
strmove (char *to, char *from)
{
  return (char *) memmove (to, from, strlen (from) + 1); // + 1 because of termination
}


char *
strins (char *dest, const char *ins)
{
  strmove (dest + strlen (ins), dest);
  memcpy (dest, ins, strlen (ins));

  return dest;
}


char *
strcat2 (const char *a, const char *b)
{
  char *p = (char *) malloc (strlen (a) + strlen (b) + 1);

  if (!p)
    return NULL;

  strcpy (p, a);
  strcat (p, b);

  return p;
}


char *
strcasestr2 (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;

  return (char *) memmem2 (str, strlen (str), search, strlen (search), MEMMEM2_CASE);
}

/*
char *
strrev (char *str)
{
  char *p1 = NULL, *p2 = NULL;

  if (!str)
    return str;
  if (!*str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
      *p1 ^= *p2;
      *p2 ^= *p1;
      *p1 ^= *p2;
    }
  return str;
}
*/

char *
strrstr (char *str, const char *search)
{
  unsigned int search_len = strlen (search);
  char *p = NULL;

  if (strlen (str) < search_len)
    return NULL;

  p = strchr (str, 0) - search_len;

  for (;; p--)
    {
      if (!strncmp (p, search, search_len))
        return p;

      if (p == str)
        return NULL;
    }

  return NULL;
}


char *
strristr (char *str, const char *search)
{
  unsigned int search_len = strlen (search);
  char *p = NULL;

  if (strlen (str) < search_len)
    return NULL;

  p = strchr (str, 0) - search_len;

  for (;; p--)
    {
      if (!strncasecmp (p, search, search_len))
        return p;

      if (p == str)
        return NULL;
    }

  return NULL;
}


char *
strtrim (char *str, int (*left) (int), int (*right) (int))
{
  if (left)
    {
      char *p = str;

      while (*p && left ((int) *p))
        p++;

      if (p - str)
        strmove (str, p);
    }

  if (right)
    {
      char *p = strchr (str, 0);

      while ((p - 1) - str && right ((int) *(p - 1)))
        p--;

      *p = 0;
    }

  return str;
}


char *
strtriml (char *str)
{
  return strtrim (str, isspace, NULL);
}


char *
strtrimr (char *str)
{
  return strtrim (str, NULL, isspace);
}


char *
strtrim_s (char *str, const char *left, const char *right)
{
  if (left)
    {
      char *p = strstr (str, left);

      if (p)
        strmove (str, p + strlen (left));
    }

  if (right)
    {
      char *p = strrstr (str, right);

      if (p)
        *p = 0;
    }

  return str;
}


char *
strcasetrim_s (char *str, const char *left, const char *right)
{
  if (left)
    {
      char *p = strcasestr2 (str, left);

      if (p)
        strmove (str, p + strlen (left));
    }

  if (right)
    {
      char *p = strristr (str, right);

      if (p)
        *p = 0;
    }

  return str;
}


char *
strrep_once (char *str, const char *orig, const char *rep)
{
  int o_len = strlen (orig);
  int r_len = strlen (rep);
  char *p = str;

  if ((p = strstr (p, orig)))
    {
      strmove (p + r_len, p + o_len);
      memcpy (p, rep, r_len);
      p += r_len;
    }

  return str;
}


char *
strrep (char *str, const char *orig, const char *rep)
{
  int o_len = strlen (orig);
  int r_len = strlen (rep);
  char *p = str;

  while ((p = strstr (p, orig)))
    {
      strmove (p + r_len, p + o_len);
      memcpy (p, rep, r_len);
      p += r_len;
    }

  return str;
}


char *
strcaserep (char *str, const char *orig, const char *rep)
{
  int o_len = strlen (orig);
  int r_len = strlen (rep);
  char *p = str;

  while ((p = strcasestr2 (p, orig)))
    {
      strmove (p + r_len, p + o_len);
      memcpy (p, rep, r_len);
      p += r_len;
    }

  return str;
}


char *
str_escape_code (char *str)
{
//  strrep (str, "~", "\\~");
  strrep (str, "%", "\\%\\%");
#if 0
  strrep (str, "|", "\\|");
  strrep (str, "'", "\\'");
  strrep (str, "&", "\\&");
  strrep (str, ";", "\\;");
  strrep (str, "?", "\\?");
  strrep (str, "!", "\\!");
  strrep (str, "*", "\\*");
  strrep (str, "[", "\\[");
  strrep (str, "]", "\\]");
  strrep (str, "{", "\\{");
  strrep (str, "}", "\\}");
  strrep (str, "(", "\\(");
  strrep (str, ")", "\\)");
  strrep (str, "<", "\\<");
  strrep (str, ">", "\\>");
#endif
  strrep (str, "\"", "\\\"");

  strrep (str, "\n", "\\n");

  return str;
}


char *
str_escape_html (char *str)
{
  strrep (str, "<", "&lt;");
  strrep (str, ">", "&gt;");
  strrep (str, "  ", "&nbsp; ");

  return str;
}


char *
str_unescape_html (char *str)
{
  strrep (str, "&lt;", "<");
  strrep (str, "&gt;", ">");
  strrep (str, "&nbsp;", " ");

  return str;
}


char *
str_escape_xml (char *str)
{
  strrep (str, "&", "&amp;");
  strrep (str, "<", "&lt;");
  strrep (str, ">", "&gt;");
  strrep (str, "'", "&apos;");
  strrep (str, "!", "&#33;");
//  strrep (str, "\"", "\\\"");
//  strrep (str, "\\", "\\\\");

  return str;
}


char *
strunesc (char *dest, const char *src)
{
  unsigned int c;
  char *p = dest;

  if (!src)
    return NULL;
  if (!(*src))
    {
      *dest = 0;
      return dest;
    }

  while ((c = *src++))
    {
      if (c == '%')
        {
          unsigned char buf[4];

          buf[0] = *src++;
          buf[1] = *src++;
          buf[2] = 0;
        
          sscanf ((const char *) buf, "%x", &c);
        }
      else
        if (c == '+')
          c = ' ';

       *p++ = c;
     }
  *p = 0;
  
  return dest;
}


char *
stresc (char *dest, const char *src)
{
//TODO: what if the src was already escaped?
  unsigned char c;
  char *p = dest;
  const char *positiv =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_.!~"                     // mark characters
    "*\\()%"                    // do not touch escape character
    ";/?:@"                     // reserved characters
    "&=+$,"                     // see RFC 2396
//  "\x7f ... \xff"    far east languages(Chinese, Korean, Japanese)
//    "+" // "/"
    ;

  if (!src)
    return NULL;
  if (!(*src))
    {
      *dest = 0;
      return dest;
    }
            
  while ((c = *src++))
    if (strchr (positiv, c) != NULL || c >= 0x7f)
      *p++ = c;
    else
      {
        sprintf (p, "%%%02X", c);
        p += 3;
      }
  *p = 0;

  return dest;
}


#ifdef  DEBUG
static int
explode_debug (int argc, char **argv)
{
  int pos;
  fprintf (stdout, "argc:     %d\n", argc);
  for (pos = 0; pos < argc; pos++)
    fprintf (stdout, "argv[%d]:  %s\n", pos, argv[pos]);

  fflush (stdout);

  return 0;
}
#endif


int
strtok2 (char **argv, char *str, const char *separators, int max_args)
{
  // any of the characters of separators breaks the string
  int argc = 0;

  if (str)
    if (*str)
      for (; (argv[argc] = (char *) strtok (!argc ? str : NULL, separators)) &&
           (argc < (max_args - 1)); argc++)
        ;

  // DEBUG
//  explode_debug (argc, argv);

  return argc;
}


int
explode (char **argv, char *str, const char *separator_s, int max_args)
{
  // only the whole separator_s breaks the string
  char *s = NULL, *e = NULL; // start, end
  int argc = 0;

  if (!str)
    return 0;
    
  if (!(*str))
    return 0;
    
  s = str;
  for (; argc < (max_args - 1); argc++)
    {
      e = strstr (s, separator_s);
      if (e)
        {
          argv[argc] = s;
          *e = 0;
          s = e + strlen (separator_s);
        }
      else
        {
          argv[argc] = s;
          break;
        }
    }

  argc++;

  // DEBUG
//  explode_debug (argc, argv);

  return argc;
}


const char *
implode (const char *separator_s, char **argv)
{
  int i = 0;
  static char *buffer = NULL;
  int buffer_len = 0;

  for (; argv[i]; i++)
    buffer_len += strlen (argv[i]);
  buffer_len += ((i + 1) * strlen (separator_s));

  if (buffer)
    free (buffer);

  if (!(buffer = (char *) malloc (buffer_len)))
    return NULL;

  *buffer = 0;
  for (i = 0; argv[i]; i++)
    sprintf (strchr (buffer, 0), "%s%s", (i > 0 ? separator_s : ""), argv[i]);

  return buffer;
}


#if 0
const char *
get_property_from_string (char *str, const char *propname, const char prop_sep, const char comment_sep)
{
  static char value_s[MAXBUFSIZE];
  char str_end[8], *p = NULL, buf[MAXBUFSIZE];
  int len = strlen (str);

  if (len >= MAXBUFSIZE)
    len = MAXBUFSIZE - 1;
  memcpy (buf, str, len);
  buf[len] = 0;

  p = strtriml (buf);
  if (*p == comment_sep || *p == '\n' || *p == '\r')
    return NULL;                                // text after comment_sep is comment

  strcpy (str_end, "\r\n");
  if ((p = strpbrk (buf, str_end)))             // strip *any* returns and comments
    *p = 0;

  // terminate at unescaped '#'
  for (p = buf + 1; *p; p++)
    if (*p == '#')
      {
        if (*(p - 1) == '\\')
          p = strmove (p - 1, p);
        else
          {
            *p = 0;
            break;
          }
      }

  p = strchr (buf, prop_sep);
  if (p)
    {
      *p = 0;                                   // note that this "cuts" _buf_ ...
      p++;
    }
  strtriml (strtrimr (buf));

  if (!strcasecmp (buf, propname))                 // ...because we do _not_ use strncasecmp()
    {
      // if no divider was found the propname must be a bool config entry
      //  (present or not present)
      if (p)
        {
          strncpy (value_s, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

          // terminate at unescaped '#'
          for (p = value_s + 1; *p; p++)
          if (*p == '#')
            {
              if (*(p - 1) == '\\')
                p = strmove (p - 1, p);
              else
                {
                  *p = 0;
                  break;
                }
            }

//          strtriml (strtrimr (value_s));
        }
      else
        strcpy (value_s, "1");
    }
  else
    return NULL;

  return value_s;
}
#endif


int
str_getline (char *line, int line_num, const char *buffer, int buffer_len)
{
  (void) buffer_len;
  int i = 0;
  const char *s = NULL, *e = NULL;

  s = buffer;
  for (; i < line_num; i++)
    {
      s = strchr (s, '\n');
      if (s)
        s++;
      else
        break;
    }

  if (i < line_num)
    return -1;

  if ((e = strchr (s, '\n')))
    {
      e++;
      strncpy (line, s, e - s)[e - (s + 1)] = 0;
    }
  else
    strcpy (line, s);

  return strlen (line);
}


const char *
strline (const char *s, int line)
{
#warning replace string.c/str_getline() with this and fix/check it everywhere
#if 0
  static char buf[MAXBUFSIZE];
  str_getline (buf, line, s, strlen (s));
  return buf;
#else
  int l = 0;
  static char buf[MAXBUFSIZE];
  char *p = NULL;

  if (!s)
    return NULL;

  if (!(*s))
    return NULL;

  for (l = 0; l < line; l++)
    {
      if (!(s = strchr (s, '\n')))
        return NULL;
      s++;
    }

  if (l < line)   
    return NULL;   

  strncpy (buf, s, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  if ((p = strchr (buf, '\n')))
    *p = 0;

#ifdef  DEBUG
  printf ("[%s]\n", buf);
  fflush (stdout);
#endif

  return buf;
#endif 
}


int
memcmp2 (const void *buffer, const void *search, size_t searchlen, unsigned int flags)
{
#define WILDCARD(f)  (f & 0xff)
  size_t i = 0, j = 0;
  const unsigned char *b = (const unsigned char *) buffer,
                      *s = (const unsigned char *) search;

#ifdef  DEBUG
  if (flags & MEMMEM2_WCARD (0))
    printf ("wildcard: %c\n", WILDCARD (flags));
#endif

  if (!flags)
    return memcmp (buffer, search, searchlen);

  if (flags & MEMMEM2_REL)
    {
      searchlen--;
      if (searchlen < 1)
        return -1;
    }

  for (i = j = 0; i < searchlen; i++, j++)
    {
      if (flags & MEMMEM2_WCARD (0))
        {
          if (*(s + i) == WILDCARD (flags))
            continue;
        }

      if (flags & MEMMEM2_REL)
        {
          if ((*(b + j) - *(b + j + 1)) != (*(s + i) - *(s + i + 1)))
            break;
        }
      else
        {
          if (flags & MEMMEM2_CASE && isalpha (*(s + i)))
            {
              if (tolower (*(b + j)) != tolower (*(s + i)))
                break;
            }
          else
            if (*(b + j) != *(s + i))
              break;
        }
    }

  return i == searchlen ? 0 : -1;
}


const void *
memmem2 (const void *buffer, size_t bufferlen,
         const void *search, size_t searchlen, unsigned int flags)
{
  size_t i;

  if (bufferlen >= searchlen)
    for (i = 0; i <= bufferlen - searchlen; i++)
      if (!memcmp2 ((const unsigned char *) buffer + i, search, searchlen, flags))
        return (const unsigned char *) buffer + i;

  return NULL;
}


#ifdef  DEBUG
static void
st_parse_url_t_sanity_check (st_parse_url_t *url)
{
  printf ("scheme:   %s\n", url->scheme);
  printf ("user:     %s\n", url->user);
  printf ("pass:     %s\n", url->pass);
  printf ("host:     %s\n", url->host);
  printf ("port:     %s\n", url->port);
  printf ("path:     %s\n", url->path);
  printf ("query:    %s\n", url->query);
  printf ("fragment: %s\n", url->fragment);
  fflush (stdout);   
}
#endif


int
parse_url (st_parse_url_t *url, const char *url_s)
{
  /*
    foo://username:password@example.com:8042/over/there/index.dtb;type=animal?name=ferret#nose
    \ /   \________________/\_________/ \__/            \___/ \_/ \_________/ \_________/ \__/
     |           |               |       |                |    |       |           |       |
     |       userinfo         hostname  port              |    |       parameter query  fragment
     |    \_______________________________/ \_____________|____|____________/
  scheme                  |                               | |  |
     |                authority                           |path|
     |                                                    |    |
     |            path                       interpretable as filename
     |   ___________|____________                              |
    / \ /                        \                             |
    urn:example:animal:ferret:nose               interpretable as extension   
  */
  char *s = NULL, *e = NULL; // start, end
  char *p = NULL;

  memset (url, 0, sizeof (st_parse_url_t));
  strncpy (url->priv, url_s, PARSE_URL_MAXBUFSIZE)[PARSE_URL_MAXBUFSIZE - 1] = 0;

  s = url->priv;

  // scheme
  e = strstr (s, "://");
  if (e)
    {
      url->scheme = s;
      *e = 0;
      s = e + 3;
    }

//  if (url->scheme)
//    if (strcasecmp (url->scheme, "file") != 0)
    {
      e = strchr (s, '@');
      if (e) // has user (and pass)
        {
          p = strchr (s, ':');
          if (p)
            {
              if (p < e) // is pass
                {
                  // user
                  e = strchr (s, ':');
                  if (e)
                    {
                      url->user = s;
                      *e = 0;
                      s = e + 1;
                    }
    
                  // pass
                  e = strchr (s, '@');
                  if (e)
                    {
                      url->pass = s;
                      *e = 0;
                      s = e + 1;
                    }
                }
              else
                {
                  // user
                  url->user = s;
                  *e = 0;
                  s = e + 1;
                }
            }
          else
            { 
              // user
              url->user = s;
              *e = 0;
              s = e + 1;
            }
        }
    
      // host (w/ port)
      e = strchr (s, ':');
      if (e)
        {
          url->host = s;
          *e = 0;   
          s = e + 1;
    
          // port
          e = strchr (s, '/');
          if (!e)
            e = strchr (s, 0);
          if (e)
            {
              url->port = s;
              *e = 0;
              s = e + 1;
            }
        }
      else
        {
          // host (w/o port)
          e = strchr (s, '/');
          if (!e)
            e = strchr (s, 0);
          if (e)
            {
              url->host = s;
              *e = 0;
              s = e + 1;
            }
        }
    }

  // path
  if (s)
    {
      // path
      url->path = s;

      // fragment
      e = strchr (s, '?');
      if (e)
        {
          url->query = e + 1;
          *e = 0;
          s = e + 1;
        }

      // fragment
      e = strchr (s, '#');
      if (e)
        {
          url->fragment = e + 1;
          *e = 0;
        }
    }
  
#ifdef  DEBUG
  st_parse_url_t_sanity_check (url);
#endif

  return 0;
}


const char *
parse_url_component (const char *url_s, int component) 
{
  static char request[PARSE_URL_MAXBUFSIZE];
  static st_parse_url_t url;

  if (!parse_url (&url, url_s))
    switch (component)
      {
        case PHP_URL_SCHEME:
          return url.scheme;
        case PHP_URL_USER:
          return url.user;
        case PHP_URL_PASS:
          return url.pass;
        case PHP_URL_HOST:
          return url.host;
        case PHP_URL_PORT:
          return url.port;
        case PHP_URL_PATH:
          return url.path;
        case PHP_URL_QUERY:
          return url.query;
        case PHP_URL_FRAGMENT:
          return url.fragment;
        case URL_REQUEST:
          sprintf (request, "%s%s%s%s%s", url.path ? url.path : "",
                            url.query ? "?" : "",
                            url.query ? url.query : "",
                            url.fragment ? "#" : "",
                            url.fragment ? url.fragment : "");
          return (*request) ? request : NULL;
      }

  return NULL;
}


int
parse_str (st_parse_str_t *pairs, const char *query)
{
  int i = 0, argc = 0;
  char *argv[PARSE_STR_MAXPAIRS], *p = NULL;

  strncpy (pairs->priv, query, PARSE_STR_MAXBUFSIZE)[PARSE_STR_MAXBUFSIZE - 1] = 0;

  argc = strtok2 (argv, pairs->priv, "&", PARSE_STR_MAXPAIRS);

  for (; i < argc; i++)
    { 
      pairs->p[i].name = argv[i];
      p = strchr (argv[i], '=');
      if (p)
        {
          *p = 0;
          pairs->p[i].value = p + 1;
        }
        
      // DEBUG
//      printf ("%s=%s\n", pairs->p[i].name, pairs->p[i].value);
//      fflush (stdout);
    }

  return 0;
}


int
strfilter (const char *s, const char *implied_boolean_logic)
{
  char *against = strdup (implied_boolean_logic);
  char *arg[512];
  int res = 0;
  int i;
  int argc;

  str_replace ("OR", " ", against);

  argc = explode (arg, against, " ", 512);

  for (i = 0; i < argc; i++)
    if (*arg[i])
      {
        if (*arg[i] == '+')
          {
            if (!stristr (s, &arg[i][1]))
              {
                res = 0;
                break;
              }
            else
              res = 1;
          }
        else if (*arg[i] == '-')
          {
            if (stristr (s, &arg[i][1]))
              {
                res = 0;
                break;
              }
            else
              res = 1;
          }
      }

  if (!res)
    for (i = 0; i < argc; i++)
      if (*arg[i])
        if (*arg[i] != '+' && *arg[i] != '-') // OR
          if (stristr (s, arg[i]))
            res = 1;

  free (against);

  return res;
}


char *
misc_get_keywords (char *s, int flag) // default: isalnum()
{
#warning misc_get_keywords()
#if 0
  unsigned int i = 0;
  unsigned int keyword_size = 3;
  int (*func) (int) = isalnum;

  // normalize
  $s = str_replace (array ('. ', ',', ';', '!', '?', '"'), ' ', strtolower ($s));

  // remove punctuation
  for ($i = 0; $i < strlen ($s); $i++)
    if (ispunct ($s[$i]) && $s[$i] != '_' && $s[$i] != '.')
      $s[$i] = ' ';

  // remove eventual html tags
  $s = strip_tags2 ($s);

  // explode and trim
  $a = explode (' ', $s);
  for ($i = 0; isset ($a[$i]); $i++)
    $a[$i] = trim ($a[$i], ' .');

  // stemmer.php (english only)
//  if (class_exists (stemmer))
//    {
//      $s = new stemmer;
//
//      for ($i = 0; isset ($a[$i]); $i++)
//        $a[$i] = $s->stem ($a[$i]);
//    }

  $p = '';
  $func = $flag ? 'isalpha' : 'isalnum';
  for ($i = 0; isset ($a[$i]); $i++)
    {
      $s = $a[$i];

      if (strlen ($s) < $keyword_size)
        continue;

      for ($j = 0; $j < strlen ($s); $j++)
        if (!$func ($s[$j]) && $s[$j] != '_' && $s[$j] != '.')
          continue;

      $p .= trim ($s).' ';
    }

  // DEBUG
//  echo $p;

  return trim ($p);

  if (flag == 1)
    func = isalpha;

  if (strlen (strtriml (strtrimr (s))) < (unsigned) keyword_size)
    return 0;

  for (i = 0; i < strlen (s); i++)
    if (!func (s[i]) && s[i] != '_' && s[i] != '.')
      return 0;

  return 1;
#endif
  return "";
}


//#if 0
#ifdef  TEST
int
main (int argc, char **argv)
{
#define MAXBUFSIZE 32768
  st_parse_url_t url;
  st_parse_str_t pairs;
  char buf[MAXBUFSIZE];
  const char *b = "123(123.32.21.44)214";
  const char *s = "(xxx.xx.xx.xx)";
  char *p = "aaaa bbbb cccc";

  // test strfilter()
  printf ("%d\n", strfilter (p, "+aaaa -bbbb OR cccc"));

  strcpy (buf, b);

#if 0
//  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('*', 'x'));
  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('x'));
  printf ("%s\n", p);

  strcpy (buf, "1234567890");
  strins (buf + 2, 6, "abc");
  printf ("%s\n", buf);

  strcpy (buf, "12434akjgkjh56453fdsg");
  strcasetrim_s (buf, "sg", "xx");
  printf (buf);
#endif  

#if 0
  printf ("%s", strrep (buf, "1", "X"));
#endif

#if 0
  printf ("%s", implode ("&", argv));
#endif

#if 0
/*
foo://username:password@example.com:8042/over/there/index.dtb;type=animal?name=ferret#nose
foo://username@example.com:8042/over/there/index.dtb;type=animal?name=ferret#nose
foo://example.com:8042/over/there/index.dtb;type=animal?name=ferret#nose
foo://example.com/over/there/index.dtb;type=animal?name=ferret#nose
foo://example.com/over/there/index.dtb;type=animal
foo://example.com/?name=ferret#nose     
foo://example.com?name=ferret#nose                                     
foo://example.com?name=ferret
foo://example.com#nose                                      
foo://example.com
*/
  parse_url (&url, argv[1]);
  st_parse_url_t_sanity_check (&url);
#endif

#if 1
  parse_str (&pairs, argv[1]);
#endif

  return 0;
}
#endif
