/*
net.c - miscellaneous network functions

Copyright (c) 2003 Dirk (d_i_r_k_@gmx.net)
           

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#ifdef  HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#include <time.h>

#ifdef  USE_CURL
#include <curl/curl.h>
#endif

#if     (defined USE_TCP || defined USE_UDP)
#ifdef  _WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#endif  // #if     (defined USE_TCP || defined USE_UDP)

#include "defines.h"
#include "misc.h"
#include "base64.h"
#include "string.h"
#include "net.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


#ifdef  DEBUG
static void
st_strurl_t_sanity_check (st_strurl_t *url)
{
  printf ("url_s:    %s\n", url->url_s);
  printf ("user:     %s\n", url->user);
  printf ("pass:     %s\n", url->pass);
  printf ("protocol: %s\n", url->protocol);
  printf ("hostname: %s\n", url->host);
  printf ("port:     %d\n", url->port);
  printf ("request:  %s\n", url->request);
        
  fflush (stdout);
}
#endif



#if     (defined USE_TCP || defined USE_UDP)
st_net_t *
net_init (int flags, int timeout)
{
  st_net_t *n = NULL;

  if (!(n = (st_net_t *) malloc (sizeof (st_net_t))))
    return NULL;

  memset (n, 0, sizeof (st_net_t));

  n->flags = flags;
  n->timeout = timeout;

  return n;
}


int
net_quit (st_net_t *n)
{
  if (n->flags & NET_SERVER)
    if (n->socket)
      {
        shutdown (n->socket, 2);
        wait2 (100);
        close (n->socket);
      }

  free (n);
  n = NULL;
 
  return 0;
}


int
net_open (st_net_t *n, const char *url_s, int port)
{
  st_strurl_t url;
//  int result; 
//  struct sockaddr_in addr; 
  struct hostent *host;
//  int valopt; 
//  long arg; 
//  fd_set myset; 
//  struct timeval tv; 
//  socklen_t lon; 

  if (!strurl (&url, url_s)) // parse URL
    return -1;

  if (!port)
    port = url.port > -1 ? url.port : 80;
                    
  if (!(host = gethostbyname (url.host)))
    return -1;

  if (n->flags & NET_UDP)
    {
      n->socket = socket (AF_INET, SOCK_DGRAM, 0);
//      n->socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (n->socket < 0)
        return -1;

      memset (&n->addr, 0, sizeof (struct sockaddr_in));
      n->addr.sin_family = AF_INET;
      n->addr.sin_addr = *((struct in_addr *) host->h_addr);
      n->addr.sin_port = htons (port);

      return 0;
    }

  n->socket = socket (AF_INET, SOCK_STREAM, 0);
  if (n->socket < 0)
    return -1;

  memset (&n->addr, 0, sizeof (struct sockaddr_in));
  n->addr.sin_family = AF_INET;
  n->addr.sin_addr = *((struct in_addr *) host->h_addr);
  n->addr.sin_port = htons (port);

#if 1
  if (connect (n->socket, (struct sockaddr *) &n->addr, sizeof (struct sockaddr)) < 0)
    {
      fprintf (stderr, "ERROR: connect()\n");
      return -1;
    }
#else
  // set non-blocking 
  if ((arg = fcntl (n->socket, F_GETFL, NULL)) < 0)
    return -1;
  arg |= O_NONBLOCK;
  if (fcntl (n->socket, F_SETFL, arg) < 0)
    return -1;

  // trying to connect with timeout 
  result = connect (n->socket, (struct sockaddr *) &n->addr, sizeof (struct sockaddr));
  if (result < 0)
    {
      if (errno == EINPROGRESS)
        {
          do
            {
              tv.tv_sec = n->timeout;
              tv.tv_usec = 0;
              FD_ZERO (&myset);
              FD_SET (n->socket, &myset);

              result = select (n->socket + 1, NULL, &myset, NULL, &tv);

              if (result < 0 && errno != EINTR)
                {
                  // error connecting
                  return -1;
                }
              else if (result > 0)
                {
                  lon = sizeof (int);
                  if (getsockopt (n->socket, SOL_SOCKET, SO_ERROR, (void *) (&valopt), &lon) < 0)
                    {
                      // error in getsockopt()
                      return -1;
                    }

                  if (valopt)
                    {
                      // error in delayed connection()
                      return -1;
                    }
                  break;
                }
              else
                {
                  // timeout in select()
                  return -1;
                }
            }
          while (1);
        }
      else
        {
          // error connecting
          return -1;
        }
    }

  // set to blocking mode again
  if ((arg = fcntl (n->socket, F_GETFL, NULL)) < 0)
    return -1;
  arg &= (~O_NONBLOCK);
  if (fcntl (n->socket, F_SETFL, arg) < 0)
    return -1;
#endif

  return 0;
}


int
net_bind (st_net_t *n, int port)
{
  struct sockaddr_in addr;
  n->flags |= NET_SERVER;

  if (n->flags & NET_UDP)
    {
      n->socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (n->socket < 0)
        {
          fprintf (stderr, "ERROR: socket creation failed (%s)\n", strerror (errno));
          return -1;
        }

      memset (&addr, 0, sizeof (addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl (INADDR_ANY);
      addr.sin_port = htons (port);

      if (bind (n->socket, (struct sockaddr *) &addr, sizeof (struct sockaddr)) < 0)
        {
          fprintf (stderr, "ERROR: socket binding failed (%s)\n",
                   strerror (errno));

          close (n->socket);

          return -1;
        }

      return 0;
    }

  n->sock0 = socket (AF_INET, SOCK_STREAM, 0);
  if (n->sock0 < 0)
    return -1; 

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addr.sin_port = htons (port);

  if (bind (n->sock0, (struct sockaddr *) &addr, sizeof (struct sockaddr)) < 0)
    {
#ifdef  HAVE_ERRNO_H
      int e = 0;
      struct
        {
           int e;
           const char *msg;
        } error_msg[] = 
        {
          {EBADF,         "The argument is not a valid file descriptor"},
          {ENOTSOCK,      "The descriptor is not a socket"},
          {EADDRNOTAVAIL, "The specified address is not available on this machine"},
          {EADDRINUSE,    "Some other socket is already using the specified address"},
          {EINVAL,        "The socket already has an address"},
          {EACCES,        "You do not have permission to access the requested address"},
          {0, NULL}
        };
      for (; error_msg[e].msg; e++)
        if (errno == error_msg[e].e)
          break;

      fprintf (stderr, "ERROR: bind() %s\n", error_msg[e].msg ? error_msg[e].msg : "");
#else
      fprintf (stderr, "ERROR: bind()\n");
#endif
      fflush (stderr);

      close (n->socket);

      return -1;
    }

  return 0;
}


int
net_listen (st_net_t *n)
{
  // wait for client connections
  if (listen (n->sock0, 5) < 0)
    {
#ifdef  HAVE_ERRNO_H
      fprintf (stderr, "ERROR: listen() %s\n",
        errno == EOPNOTSUPP ? "The socket does not support this operation" :
          "The argument is not a valid file descriptor/is not a socket");
#else
      fprintf (stderr, "ERROR: listen()\n");
#endif
      fflush (stderr);

      return -1;
    }

  return 0;
}


st_net_t *
net_accept (st_net_t *n)
{
  // TODO: fork()

  // accept waits and "dupes" the socket
  if ((n->socket = accept (n->sock0, 0, 0)) < 0)
    {
      fprintf (stderr, "ERROR: accept()\n");
      fflush (stderr);

      return NULL;
    }

  return n;
}


int
net_close (st_net_t *n)
{
  return close (n->socket);
}


int
net_read (st_net_t *n, void *buffer, int buffer_len)
{
  if (n->flags & NET_UDP)
    {
      fd_set readset;  
      struct timeval t;
      int result;
      unsigned int dummy = 0;

      if (n->flags & NET_SERVER)
        {
          socklen_t addrlen;
          int result = 0;

          addrlen = sizeof (n->udp_addr);
          result = recvfrom (n->socket, buffer, buffer_len, 0, (struct sockaddr *) &n->udp_addr, &addrlen);

          if (!ntohs (n->udp_addr.sin_port))       
            fprintf (stderr, "WARNING: rejected packet (source port = 0)\n");

          return result;
        }

      t.tv_sec = n->timeout;
      t.tv_usec = 0;

      FD_ZERO (&readset);
      FD_SET (n->socket, &readset);

      result = select (n->socket + 1, &readset, NULL, NULL, &t);

      if (result < 0)
        return -1;

      if (!result)
        return 0;

      if (FD_ISSET (n->socket, &readset))
        return recvfrom (n->socket, buffer, buffer_len, 0, (struct sockaddr *) &n->addr, &dummy);
    }

  return recv (n->socket, buffer, buffer_len, 0);
}


int
net_write (st_net_t *n, void *buffer, int buffer_len)
{
  if (n->flags & NET_UDP)
    {
      fd_set writeset;
      struct timeval t;
      int result;

      if (n->flags & NET_SERVER)
        return sendto (n->socket, buffer, buffer_len, 0, (struct sockaddr *) &n->udp_addr, sizeof (struct sockaddr));

      t.tv_sec = n->timeout;
      t.tv_usec = 0;

      FD_ZERO (& writeset);
      FD_SET (n->socket, & writeset);

      result = select (n->socket + 1, NULL, &writeset, NULL, &t);

      if (result < 0)
        return -1;

      if (!result)
        return 0;

//      if (FD_ISSET (n->socket, &writeset))
        return sendto (n->socket, buffer, buffer_len, 0, (struct sockaddr *) &n->addr, sizeof (struct sockaddr));
    }

  return send (n->socket, buffer, buffer_len, 0);
}


int
net_getc (st_net_t *n)
{
  char buf[2];

  if (recv (n->socket, (void *) buf, 1, 0) == 1)
    return *buf;
  else
    return -1;
}


int
net_putc (st_net_t *n, int c)
{
  unsigned char buf[2];

  *buf = (unsigned char) c & 0xff;

  if (send (n->socket, (void *) buf, 1, 0) == 1)
    return *buf;
  else
    return EOF;
}


char *
net_gets (st_net_t *n, char *buffer, int buffer_len)
{
  int c = 0, count = 0;
  char *dst = buffer;

  while (count < buffer_len)
    {
      c = net_getc (n);

      if (c < 1)
        {
          if (count)
            {
              *dst = 0;
              return buffer;
            }
          return NULL;
        }

      if (c == '\n')
        {
          *dst = '\n';
          *(dst + 1) = 0;

#ifdef  DEBUG
          printf (buffer);
          fflush (stdout);
#endif

          return buffer;
        }

      if (c == '\r')
        continue;
      else
        {
          *dst++ = c;
          count++;
        }
    }
  *dst = 0;

#ifdef  DEBUG
  printf (buffer);
  fflush (stdout);
#endif
  
  return buffer;
}


int
net_puts (st_net_t *n, char *buffer)
{
  return send (n->socket, buffer, strlen (buffer), 0);
}


int   
net_sync (st_net_t *n)
{
#ifndef _WIN32
  return fsync (n->socket);
#else
  return 0;
#endif
}
#endif  // #if     (defined USE_TCP || defined USE_UDP)


char *
net_build_http_request (const char *url_s, const char *user_agent, int keep_alive, int method, int gzip)
{
  static char buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE];
  st_strurl_t url;

  if (!strurl (&url, url_s))
    return NULL;

#ifdef  DEBUG
  st_strurl_t_sanity_check (&url);
#endif

  sprintf (buf, "%s ", method == NET_METHOD_POST ? "POST" : "GET");

  if (*url.request)
    strcat (buf, url.request);
  else
    strcat (buf, "/");

  sprintf (strchr (buf, 0), " HTTP/1.0\r\n"
    "Connection: %s\r\n"
    "User-Agent: %s\r\n"
    "Pragma: no-cache\r\n"
    "Host: %s\r\n" 
    "Accept: */*\r\n", // accept everything
    keep_alive ? "Keep-Alive" : "close",
    user_agent,
    url.host);

  if (gzip)
    strcpy (strchr (buf, 0), "Accept-encoding: x-gzip\r\n");

  if (*url.user || *url.pass)
    {
      sprintf (buf2, "%s:%s", url.user, url.pass);
      sprintf (strchr (buf, 0), "Authorization: Basic %s\r\n", base64_enc (buf2));
    } 

  strcat (buf, "\r\n");

#ifdef  DEBUG 
  fputs (buf, stdout);
  fflush (stdout);
#endif
    
  return buf;
}


char *
net_build_http_response (const char *user_agent, int keep_alive, unsigned int content_len, int gzip)
{
  static char buf[MAXBUFSIZE];
  char buf2[64];
  time_t t = time (0);

  strftime (buf2, 64, "%a, %d %b %Y %H:%M:%S %Z", localtime (&t)); // "Sat, 20 Sep 2003 12:30:58 GMT"

  sprintf (buf,
    "HTTP/1.0 302 Found\r\n"
    "Connection: %s\r\n"
    "Date: %s\r\n"
    "Content-Type: %s\r\n"
    "Server: %s\r\n",
    keep_alive ? "Keep-Alive" : "close",
    buf2,
    "text/html",
    user_agent);

  if (gzip)
    strcpy (strchr (buf, 0), "Content-encoding: x-gzip\r\n");  

  if (content_len)
    sprintf (strchr (buf, 0), "Content-length: %d\r\n", content_len);

  strcat (buf, "\r\n");

#ifdef  DEBUG
  fputs (buf, stdout);
  fflush (stdout);
#endif

  return buf;
}


#if     (defined USE_TCP || defined USE_UDP)
st_http_header_t *
net_parse_http_request (st_net_t *n)
{
  char buf[MAXBUFSIZE];
  char *p = NULL; 
  static st_http_header_t h;
  int line = 0;

  memset (&h, 0, sizeof (st_http_header_t));

  while (net_gets (n, buf, MAXBUFSIZE))
    {
#ifdef  DEBUG
      fputs (buf, stdout);
      fflush (stdout);
#endif

      p = strchr (buf, '\n');
      if (p)
        *p = 0;

      sprintf (strchr (h.header, 0), "%s\n", buf);

      if (!line)
        {
          p = strstr (buf, "HTTP");
          if (p)
            p += 4;
          strncpy (h.request, p, NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
          p = strchr (h.request, ' ');
          if (p)
            *p = 0;
          strtriml (strtrimr (h.request));
        }
      else if (stristr (buf, "Host: "))
        strncpy (h.host, buf + strlen ("Host: "), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      else if (stristr (buf, "Accept-encoding: "))
        {
          if (stristr (buf, "x-gzip"))
            h.gzip = 1;
        }
      else if (stristr (buf, "User-Agent: "))
        strncpy (h.user_agent, buf + strlen ("User-Agent: "), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      else if (!(*buf) || *buf == 0x0d || *buf == 0x0a)
        return &h;

      line++;
    }

  return NULL;
}


st_http_header_t *
net_parse_http_response (st_net_t *n)
{
  char buf[MAXBUFSIZE];
  char *p = NULL;
  static st_http_header_t h;
  int line = 0;

  memset (&h, 0, sizeof (st_http_header_t));

  while (net_gets (n, buf, MAXBUFSIZE))
    {
#ifdef  DEBUG
      fputs (buf, stdout);
      fflush (stdout);
#endif

      p = strchr (buf, '\n');
      if (p)
        *p = 0;

      sprintf (strchr (h.header, 0), "%s\n", buf);

      if (stristr (buf, "Host: "))
        strncpy (h.host, buf + strlen ("Host: "), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      else if (stristr (buf, "Server: "))
        strncpy (h.user_agent, buf + strlen ("User-Agent: "), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      else if (stristr (buf, "Content-encoding: "))
        {
          if (stristr (buf, "x-gzip"))
            h.gzip = 1;
        }
      else if (stristr (buf, "Content-type: "))
        strncpy (h.content_type, buf + strlen ("Content-type: "), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      else if (!(*buf) || *buf == 0x0d || *buf == 0x0a)
        return &h;

      line++;
    }

  return NULL;
}


#ifdef  USE_CURL
static size_t
curl_write_cb (void *ptr, size_t size, size_t nmemb, void *stream)
{
  return fwrite (ptr, size, nmemb, (FILE *) stream);
}
#endif


const char *
net_http_get_to_temp (const char *url_s, const char *user_agent, int flags)
{
  static char tname[FILENAME_MAX];
  char buf[MAXBUFSIZE];
  FILE *tmp = NULL;
  st_net_t *client = NULL;
  char *p = NULL;
  st_strurl_t url;
  int len = 0;

  *tname = 0;
#ifdef  HAVE_ERRNO_H
  errno = 0;
#endif
  tmpnam3 (tname, 0);

#ifdef  USE_CURL
  if (!(flags & GET_NO_CURL))
    {
      CURL *curl = NULL;
      CURLcode result;

      if (!(tmp = fopen (tname, "wb")))
        {
#ifdef  HAVE_ERRNO_H
          fprintf (stderr, "ERROR: could not write %s; %s\n", tname, strerror (errno));
#else
          fprintf (stderr, "ERROR: could not write %s\n", tname);
#endif
          return NULL;
        } 

      curl = curl_easy_init ();
      if (!curl)
        {
          fprintf (stderr, "ERROR: curl_easy_init() failed\n");
          return NULL;
        }

      curl_easy_setopt (curl, CURLOPT_URL, url_s);

      if (flags & GET_VERBOSE)
        curl_easy_setopt (curl, CURLOPT_VERBOSE, 1);
      else
        {
          curl_easy_setopt (curl, CURLOPT_VERBOSE, 0);
          curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 1);
        }

      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
      curl_easy_setopt (curl, CURLOPT_WRITEDATA, tmp);

      result = curl_easy_perform (curl);

      curl_easy_cleanup (curl);

      fclose (tmp);

      if (!result)
        return tname;

      remove (tname);

      return NULL;
    }
  else
#endif  // USE_CURL

  if (!(tmp = fopen (tname, "wb")))
    {
#ifdef  HAVE_ERRNO_H
      fprintf (stderr, "ERROR: could not write %s; %s\n", tname, strerror (errno));
#else
      fprintf (stderr, "ERROR: could not write %s\n", tname);
#endif
      return NULL;
    } 

  if (!(client = net_init (0, 5)))
    {
      fprintf (stderr, "ERROR: net_http_get_to_temp()/net_init() failed\n");
      fclose (tmp);   
      remove (tname);
      return NULL;
    }

  strurl (&url, url_s);
  if (net_open (client, url.host, (url.port > -1) ? url.port : 80) != 0)
    {
      fprintf (stderr, "ERROR: net_http_get_to_temp()/net_open() failed to open %s\n", url_s);
      fclose (tmp);  
      remove (tname);
      return NULL;
    }

  p = net_build_http_request (url_s, user_agent, 0, NET_METHOD_GET, flags & GET_USE_GZIP);
  net_write (client, (char *) p, strlen (p));

  // skip http header
  if (net_parse_http_request (client))
    while ((len = net_read (client, buf, MAXBUFSIZE)))
      fwrite (buf, len, 1, tmp);

  net_quit (client);

  fclose (tmp);

  return tname;
}
#endif  // #if     (defined USE_TCP || defined USE_UDP


char *
strunesc (char *dest, const char *src)
{
  unsigned int c;
  char *p = dest;

  if (!src)
    return NULL;
  if (!src[0])
    return (char *) "";

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
  if (!src[0])
    return (char *) "";
            
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


st_strurl_t *
strurl (st_strurl_t *url, const char *url_s)
{
  int pos = 0, pos2 = 0;
  char *p = NULL, *p2 = NULL, *p3 = NULL;

#ifdef  DEBUG
  printf ("strurl() url_s: %s\n\n", url_s);
  fflush (stdout);
#endif

  if (!url)
    return NULL;
  if (!url_s)
    return NULL;
  if (!url_s[0])
    return NULL;

  memset (url, 0, sizeof (st_strurl_t));
  strcpy (url->url_s, url_s);
  url->port = -1;

  // look for "://"
  if ((p = strstr (url_s, "://")))
    {
      // extract the protocol
      pos = p - url_s;
      strncpy (url->protocol, url_s, MIN (pos, NET_MAXBUFSIZE))[MIN (pos, NET_MAXBUFSIZE - 1)] = 0;

      // jump the "://"
      p += 3;
      pos += 3;
    }
  else
    p = (char *) url_s;

  // check if a user:pass is given
  if ((p2 = strchr (p, '@')))
    {
      int len = p2 - p;
      strncpy (url->user, p, MIN (len, NET_MAXBUFSIZE))[MIN (len, NET_MAXBUFSIZE - 1)] = 0;

      p3 = strchr (p, ':');
      if (p3 != NULL && p3 < p2)
        {
          int len2 = p2 - p3 - 1;

          url->user[p3 - p] = 0;
          strncpy (url->pass, p3 + 1, MIN (len2, NET_MAXBUFSIZE))[MIN (len2, NET_MAXBUFSIZE - 1)] = 0;
        }
      p = p2 + 1;
      pos = p - url_s;
    }

  // look if the port is given
  p2 = strchr (p, ':');                 // If the : is after the first / it isn't the port
  p3 = strchr (p, '/');
  if (p3 && p3 - p2 < 0)
    p2 = NULL;
  if (!p2)
    {
      pos2 =
        (p2 = strchr (p, '/')) ?        // Look if a path is given
        (p2 - url_s) :                  // We have an URL like http://www.hostname.com/file.txt
        (int) strlen (url_s);           // No path/filename
                                        // So we have an URL like http://www.hostname.com
    }
  else
    {
      // We have an URL beginning like http://www.hostname.com:1212
      url->port = atoi (p2 + 1);  // Get the port number
      pos2 = p2 - url_s;
    }

  // copy the hostname into st_strurl_t
  strncpy (url->host, p, MIN (pos2 - pos, NET_MAXBUFSIZE))[MIN (pos2 - pos, NET_MAXBUFSIZE - 1)] = 0;

  // look if a path is given
  if ((p2 = strchr (p, '/')))
    if (strlen (p2) > 1)                // A path/filename is given check if it's not a trailing '/'
      strcpy (url->request, p2);           // copy the path/filename into st_strurl_t

#ifdef  DEBUG
  st_strurl_t_sanity_check (url);
#endif

#if 0
  // turn request into args
  strncpy (url->priv, url->request, NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
  // argc < 2
  if (!strcmp (url->priv, "/"))
    *(url->priv) = 0;
  url->argc = strarg (url->argv, url->priv, "?&", NET_MAXBUFSIZE);
#endif

  return url;
}


//#ifdef  TEST
#if 0
int
main (int argc, char ** argv)
{
  char buf[MAXBUFSIZE];
  st_net_t *net = net_init (0, 5);

#if 0
  // client test
  if (!net_open (net, "http://www.google.de", 80))
    {
      char *p = net_build_http_request ("http://www.google.de/index.html", "example", 0);

      net_write  (net, p, strlen (p));

      while (net_gets (net, buf, MAXBUFSIZE))
        fputs (buf, stdout);
    }
#else
  // server test
  while (1)
    {
      if (!net_bind (net, 80))
        { 
          char *p = net_build_http_response ("example", 0);
          
          net_read (net, buf, MAXBUFSIZE);
          fputs (buf, stdout);
          
          net_write (net, p, strlen (p));
          net_puts (net, "Hello World!");
          
          break;
        }
      else break;
    }
#endif
  net_close (net);
  net_quit (net);

  return 0;
}
#endif  // TEST
