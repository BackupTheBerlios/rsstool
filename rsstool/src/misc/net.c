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
#if     (defined USE_TCP || defined USE_UDP)
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

#ifdef  _WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "misc.h"
#include "base64.h"
#include "string.h"
#include "net.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


#ifdef  HAVE_ERRNO_H
static const char *
net_error (int e)
{
  int i = 0;
  struct
    {   
      int e;
      const char *msg;
    } error_msg[] =    
    {
      {EACCES,        "The calling process does not have the appropriate privileges"},
      {EADDRINUSE,    "Some other socket is already using the specified address"},
      {EADDRNOTAVAIL, "The specified address is not available on this machine"},
      {EAGAIN,        "O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted"},
      {EBADF,         "The socket argument is not a valid file descriptor"},
      {ECONNABORTED,  "A connection has been aborted"},
      {EDESTADDRREQ,  "The socket is not bound to a local address, and the protocol does not support listening on an unbound socket"},
      {EINTR,         "The accept() function was interrupted by a signal that was caught before a valid connection arrived"},
      {EINVAL,        "The socket is not accepting connections"},
      {EMFILE,        "{OPEN_MAX} file descriptors are currently open in the calling process"},
      {ENFILE,        "The maximum number of file descriptors in the system are already open"},
      {ENOBUFS,       "Insufficient resources are available in the system to complete the call"},
      {ENOMEM,        "There was insufficient memory available to complete the operation"},
      {ENOTSOCK,      "The socket argument does not refer to a socket"},
      {EOPNOTSUPP,    "The socket type of the specified socket does not support accepting connections"},
      {EPROTO,        "A protocol error has occurred; for example, the STREAMS protocol stack has not been initialized"},
      {EWOULDBLOCK,   "O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted"},
      {0, NULL}
    };
  for (; error_msg[i].msg; i++)
    if (e == error_msg[i].e)
      return error_msg[i].msg;
  return "Uknown error";
}
#endif


st_net_t *
net_init (int flags, int timeout)
{
  st_net_t *n = NULL;

  if (!(n = (st_net_t *) malloc (sizeof (st_net_t))))
    return NULL;

  memset (n, 0, sizeof (st_net_t));

  if (flags & NET_DEBUG)
    {
      fputs ("net_init(): NET_DEBUG", stderr);
      if (flags & NET_UDP)
        fputs ("|NET_UDP", stderr);
      if (flags & NET_TCP)
        fputs ("|NET_TCP", stderr);
      if (flags & NET_CLIENT)
        fputs ("|NET_CLIENT", stderr);
      if (flags & NET_SERVER)
        fputs ("|NET_SERVER", stderr);
      if (flags & NET_LOCALONLY)
        fputs ("|NET_LOCALONLY", stderr);
      fputs ("\n", stderr);
      fflush (stderr);
    }

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
  st_parse_url_t url;
//  int result; 
//  struct sockaddr_in addr; 
  struct hostent *host;
//  int valopt; 
//  long arg; 
//  fd_set myset; 
//  struct timeval tv; 
//  socklen_t lon; 

  if (!parse_url (&url, url_s)) // parse URL
    return -1;

  if (!port)
    port = url.port_s ? url.port : 80;
                    
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
      fflush (stderr);

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
net_close (st_net_t *n)
{
  if (n->socket)
    return close (n->socket);
  return 0;
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
            {
              fprintf (stderr, "WARNING: rejected packet (source port = 0)\n");
              fflush (stderr);
            }

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


#if 0
int   
net_sync (st_net_t *n)
{
#ifndef _WIN32
  return fsync (n->socket);
#else
  return 0;
#endif
}
#endif


int
net_build_http_request (char *http_header, const char *url_s, const char *user_agent, int keep_alive, int method, int gzip)
{
  char buf[MAXBUFSIZE];
  st_parse_url_t url;

  if (!parse_url (&url, url_s))
    return -1;

  *http_header = 0;
  sprintf (http_header, "%s ", method == NET_METHOD_POST ? "POST" : "GET");

  strcat (http_header, parse_url_component (url_s, URL_REQUEST)); // the request

  sprintf (strchr (http_header, 0), " HTTP/1.0\r\n"
    "Connection: %s\r\n"
    "User-Agent: %s\r\n"
    "Pragma: no-cache\r\n"
    "Host: %s\r\n" 
    "Accept: */*\r\n", // accept everything
    keep_alive ? "Keep-Alive" : "close",
    user_agent,
    url.host);

  if (gzip)
    strcpy (strchr (http_header, 0), "Accept-encoding: x-gzip\r\n");

  if (*url.user || *url.pass)
    {
      sprintf (buf, "%s:%s", url.user, url.pass);
      sprintf (strchr (http_header, 0), "Authorization: Basic %s\r\n", base64_enc (buf));
    } 

  strcat (http_header, "\r\n");

  // DEBUG 
//  fputs (http_header, stdout);
//  fflush (stdout);
    
  return 0;
}


int
net_build_http_response (char *http_header, const char *user_agent, int keep_alive, unsigned int content_len, int gzip)
{
  char buf[64];
  time_t t = time (0);

  *http_header = 0;
  strftime (buf, 64, "%a, %d %b %Y %H:%M:%S %Z", localtime (&t)); // "Sat, 20 Sep 2003 12:30:58 GMT"

  sprintf (http_header,
    "HTTP/1.0 302 Found\r\n"
    "Connection: %s\r\n"
    "Date: %s\r\n"
    "Content-Type: %s\r\n"
    "Server: %s\r\n",
    keep_alive ? "Keep-Alive" : "close",
    buf,
    "text/html",
    user_agent);

  if (gzip)
    strcpy (strchr (http_header, 0), "Content-encoding: x-gzip\r\n");  

  if (content_len)
    sprintf (strchr (http_header, 0), "Content-length: %d\r\n", content_len);

  strcat (http_header, "\r\n");

  // DEBUG
//  fputs (http_header, stdout);
//  fflush (stdout);

  return 0;
}


int
net_get_http_header (char *http_header, st_net_t *n)
{
  char buf[MAXBUFSIZE];

  *http_header = 0;

  while (net_gets (n, buf, MAXBUFSIZE))
    {
#ifdef  DEBUG
      fputs (buf, stdout);
      fflush (stdout);
#endif
      if (strlen (http_header) + strlen (buf) > NET_MAXHTTPHEADERSIZE - 1)
        {
          // too large http header
          break; 
        }

      if (!(*buf) || *buf == 0x0d || *buf == 0x0a)
        break;

      strcat (http_header, buf);
    }

  return 0;
}


int
net_parse_http_request (st_http_header_t *h, const char *http_header)
{
  char *p = NULL; 

  memset (h, 0, sizeof (st_http_header_t));
  strncpy (h->header, http_header, NET_MAXHTTPHEADERSIZE)[(NET_MAXHTTPHEADERSIZE) - 1] = 0;
#if 1
  p = strstr (h->header, "\n\n");
  if (p)
    *p = 0;
  p = strstr (h->header, "\n\r");
  if (p)
    *p = 0;
#endif

  // DEBUG
//  printf ("%s\n", http_header);
//  fflush (stdout);

  strncpy (h->request, http_header + strlen ("POST"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
  if ((p = strstr (h->request, "HTTP")))
    {
     if ((p = strpbrk (h->request, " \r\n")))
       *p = 0;
      strtriml (strtrimr (h->request));
    }
  else
    {
      return -1; // no http header
    }

  // fast-forward past the request line
  http_header = strpbrk (http_header, "\r\n"); 
  if (!http_header)
    return -1;

  if ((p = stristr (http_header, "Host:")))
    {
      strncpy (h->host, p + strlen ("Host:"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      if ((p = strpbrk (h->host, "\r\n")))
        *p = 0;
      strtriml (strtrimr (h->host));
    }
  else if ((p = stristr (http_header, "Accept-encoding:")))
    {
      if (stristr (p, "x-gzip"))
        h->gzip = 1;
#warning fix header parsing
printf ("SHIT");  
fflush (stdout);
    }
  else if ((p = stristr (http_header, "User-Agent:")))
    { 
      strncpy (h->user_agent, p + strlen ("User-Agent:"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      if ((p = strpbrk (h->user_agent, "\r\n")))
        *p = 0;
      strtriml (strtrimr (h->user_agent));
printf ("SHIT");  
printf (h->user_agent);
fflush (stdout);
    }

  return 0;
}


int
net_parse_http_response (st_http_header_t *h, const char *http_header)
{
  char *p = NULL;

  memset (h, 0, sizeof (st_http_header_t));
  strncpy (h->header, http_header, NET_MAXHTTPHEADERSIZE)[(NET_MAXHTTPHEADERSIZE) - 1] = 0;
#if 1
  p = strstr (h->header, "\n\n");   
  if (p)
    *p = 0;
  p = strstr (h->header, "\n\r");
  if (p)
    *p = 0;
#endif

  if ((p = stristr (http_header, "Host:")))
    {
      strncpy (h->host, p + strlen ("Host:"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      if ((p = strpbrk (h->host, "\r\n")))
        *p = 0;
      strtriml (strtrimr (h->host));
    }
  else if ((p = stristr (http_header, "Server:")))
    {
      strncpy (h->user_agent, p + strlen ("Server:"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      if ((p = strpbrk (h->user_agent, "\r\n")))
        *p = 0;
      strtriml (strtrimr (h->user_agent));
    }
  else if ((p = stristr (http_header, "Content-encoding:")))
    {
      if (stristr (p, "x-gzip"))
        h->gzip = 1;
    }
  else if ((p = stristr (http_header, "Content-type:")))
    {
      strncpy (h->content_type, p + strlen ("Content-type:"), NET_MAXBUFSIZE)[NET_MAXBUFSIZE - 1] = 0;
      if ((p = strpbrk (h->content_type, "\r\n")))
        *p = 0;
      strtriml (strtrimr (h->content_type));
    }

  return 0;
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
  char http_header_s[NET_MAXHTTPHEADERSIZE];
  char buf[MAXBUFSIZE];
  FILE *tmp = NULL;
  st_net_t *client = NULL;
  st_parse_url_t url;
  int len = 0;

  *tname = 0;
#ifdef  HAVE_ERRNO_H
  errno = 0;
#endif
  tmpnam3 (tname, 0);

  if (!(tmp = fopen (tname, "wb")))
    {
#ifdef  HAVE_ERRNO_H
      fprintf (stderr, "ERROR: could not write %s; %s\n", tname, strerror (errno));
#else
      fprintf (stderr, "ERROR: could not write %s\n", tname);
#endif
      fflush (stderr);

      return NULL;
    } 

#ifdef  USE_CURL
  if (!(flags & GET_NO_CURL))
    {
      CURL *curl = NULL;
      CURLcode result;

      curl = curl_easy_init ();
      if (!curl)
        {
          fprintf (stderr, "ERROR: curl_easy_init() failed\n");
          fflush (stderr);

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
#endif  // USE_CURL

  fprintf (stderr, "WARNING: compiled without cURL support, switching to workaround\n");
  fflush (stderr);

  if (!(client = net_init (NET_TCP|NET_CLIENT, 5)))
    {
      fprintf (stderr, "ERROR: net_http_get_to_temp()/net_init() failed\n");
      fflush (stderr);

      fclose (tmp);   
      remove (tname);
      return NULL;
    }

  if (parse_url (&url, url_s) != 0)
    {
      fprintf (stderr, "ERROR: net_http_get_to_temp()/parse_url() failed\n");
      fflush (stderr);

      fclose (tmp);  
      remove (tname);
      return NULL;
    }

  if (net_open (client, url.host, url.port_s ? url.port : 80) != 0)
    {
      fprintf (stderr, "ERROR: net_http_get_to_temp()/net_open() failed to open %s\n", url_s);
      fflush (stderr);

      fclose (tmp);  
      remove (tname);
      return NULL;
    }

  if (!net_build_http_request (http_header_s, url_s, user_agent, 0, NET_METHOD_GET, flags & GET_USE_GZIP))
    net_write (client, http_header_s, strlen (http_header_s));

  // skip http header
  if (!net_get_http_header (http_header_s, client))
    while ((len = net_read (client, buf, MAXBUFSIZE)))
      fwrite (buf, len, 1, tmp);

  net_quit (client);

  fclose (tmp);

  return tname;
}


int
net_bind (st_net_t *n, int port)
{
  if (!(n->flags & NET_SERVER))
    {
      fprintf (stderr, "ERROR: net_bind(): NET_SERVER flag not set\n");
      fflush (stderr);

      return -1;
    }

  if (n->flags & NET_UDP)
    {
      n->socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (n->socket < 0)
        {
#ifdef  HAVE_ERRNO_H
          fprintf (stderr, "ERROR: net_bind(): socket creation failed; %s\n", strerror (errno));
#else
          fprintf (stderr, "ERROR: net_bind(): socket creation failed\n");
#endif
          fflush (stderr);

          return -1;
        }

      memset (&n->addr, 0, sizeof (struct sockaddr_in));
      n->addr.sin_family = AF_INET;
      n->addr.sin_addr.s_addr = htonl (INADDR_ANY);
      n->addr.sin_port = htons (port);
#if 1
      if (bind (n->socket, (struct sockaddr *) &n->addr, sizeof (struct sockaddr)) < 0)
#else
      n->addr_len = strlen (n->addr.sin_data) + sizeof (n->addr.sin_family);
      if (bind (n->socket, (struct sockaddr *) &n->addr, n->addr_len) < 0)
#endif
        {
          fprintf (stderr, "ERROR: net_bind(): socket binding failed (%s)\n", strerror (errno));
          fflush (stderr);

          close (n->socket);

          return -1;
        }

      return 0;
    }

  n->sock0 = socket (AF_INET, SOCK_STREAM, 0);
  if (n->sock0 < 0)
    {
      fprintf (stderr, "ERROR: net_bind()/socket() failed\n");
      return -1; 
    }

  memset (&n->addr, 0, sizeof (struct sockaddr_in));
  n->addr.sin_family = AF_INET;
  if (n->flags & NET_LOCALONLY) // allow connections from localhost only
    {
      struct hostent *host = gethostbyname ("localhost");
      n->addr.sin_addr = *((struct in_addr *) host->h_addr);
    }
  else
    n->addr.sin_addr.s_addr = htonl (INADDR_ANY);
  n->addr.sin_port = htons (port);

  if (bind (n->sock0, (struct sockaddr *) &n->addr, sizeof (struct sockaddr)) < 0)
    {
#ifdef  HAVE_ERRNO_H   
      fprintf (stderr, "ERROR: net_bind(): %s\n", net_error (errno));
#else
      fprintf (stderr, "ERROR: net_bind()\n");
#endif
      fflush (stderr);

      close (n->sock0);

      return -1;
    }

  return 0;
}


int
net_listen (st_net_t *n)
{
  // wait for client connections
  if (listen (n->sock0, SOMAXCONN) < 0)
    {
#ifdef  HAVE_ERRNO_H
      fprintf (stderr, "ERROR: net_listen(): %s\n", net_error (errno));
#else
      fprintf (stderr, "ERROR: net_listen()\n");
#endif
      fflush (stderr);

      return -1;
    }

  return 0;
}


st_net_t *
net_accept (st_net_t *n)
{
#if 1
  if ((n->socket = accept (n->sock0, 0, 0)) < 0)
#else
  if ((n->socket = accept (n->sock0, &n->addr, sizeof (struct sockaddr)) < 0)
#endif
    {
#ifdef  HAVE_ERRNO_H
      fprintf (stderr, "ERROR: net_accept(): %s\n", net_error (errno));
#else
      fprintf (stderr, "ERROR: net_accept()\n");
#endif
      fflush (stderr);

      return NULL;
    }

  return n;
}


int
net_server (st_net_t *n, int port, int (* callback_func) (const void *, int, void *, int *), int max_content_len)
{
  unsigned char *request = NULL;
  unsigned char *response = NULL;
  int request_len = 0;
  int response_len = 0;

  if (net_bind (n, port) != 0)
    return -1;

  if (!(request = (unsigned char *) malloc (max_content_len + 1)))
    {
      fprintf (stderr, "ERROR: net_server()/malloc() failed\n");
      return -1;
    }

  if (!(response = (unsigned char *) malloc (max_content_len + 1)))
    {
      fprintf (stderr, "ERROR: net_server()/malloc() failed\n");
      free (request);
      return -1;
    }

#if 1
  if (net_listen (n) == -1)
    {
      net_close (n);

      free (request);
      free (response);

      return -1;
    }
#endif

  // ignore child process termination
//  signal (SIGCHLD, SIG_IGN);

  while (1)
    {
      int pid = 0;

#if 0
      if (net_listen (n) == -1)
        break;
#endif

      if (!net_accept (n))
        {
          return -1;
//          exit (1);
        }

      if ((pid = fork()) < 0)
        {
          fprintf (stderr, "ERROR: net_server()/fork() failed\n");
          fflush (stderr);
        }
      else if (pid == 0) // child 
        {
          close (n->sock0); // do not need listen socket in child

          request_len = recv (n->socket, request, max_content_len, 0);

          if (callback_func (request, request_len, response, &response_len) == -1)
            {
              fprintf (stderr, "ERROR: net_server()/callback_func() failed\n");
              fflush (stderr);
#warning causes strange problem
//              response_len = 0;
            }

          // DEBUG
//          fprintf (stderr, "DEBUG: net_server()/request_len: %d, response_len: %d\n", request_len, response_len);
//          fflush (stderr);

          send (n->socket, response, response_len, 0);

          close (n->socket);

          exit (0);
        }
      else
        {
          close (n->socket);
        }
    }

  net_close (n);

  free (request);
  free (response);

  return 0;
}


#ifdef  TEST
//#if 0
int
net_server_cb (const void *request, int request_len, void *response, int *response_len)
{
  char http_header_s[NET_MAXHTTPHEADERSIZE];
  st_http_header_t http_header;

  net_parse_http_request (&http_header, request);
  printf ("%s", http_header.request);
  fflush (stdout);
   
  net_build_http_response (http_header_s, "example", 0, 0, 0);
 
  sprintf (response, "%sHello World!", http_header_s);
  *response_len = strlen (response);

  return 0;
}


int
main (int argc, char ** argv)
{
#if 0
  // client test
  st_net_t *net = net_init (NET_TCP|NET_SERVER, 5);
  if (!net_open (net, "http://www.google.de", 80))
    {
      char *p = net_build_http_request ("http://www.google.de/index.html", "example", 0);

      net_write  (net, p, strlen (p));

      while (net_gets (net, buf, MAXBUFSIZE))
        fputs (buf, stdout);

      net_close (net);
      net_quit (net);
    }
#else
  // server test
  st_net_t *net = net_init (NET_TCP|NET_SERVER, 5);
  net_server (net, 80, &net_server_cb, MAXBUFSIZE * 2);
  net_quit (net); 
#endif

  return 0;
}
#endif  // TEST


#endif  // #if     (defined USE_TCP || defined USE_UDP
