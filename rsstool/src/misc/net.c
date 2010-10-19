/*
net.c - miscellaneous network functions

Copyright (c) 2003 NoisyB (noisyb@gmx.net)

uses parts of mini_httpd 1999,2000 by Jef Poskanzer <jef@acme.com>.
           

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

#ifdef  USE_TCP
#include "misc.h"
#include "base64.h"
#include "string.h"
#endif
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
      {EAGAIN,        "O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted"},
      {EBADF,         "The socket argument is not a valid file descriptor"},
      {EINTR,         "The accept() function was interrupted by a signal that was caught before a valid connection arrived"},
      {EINVAL,        "The socket is not accepting connections"},
      {ENFILE,        "The maximum number of file descriptors in the system are already open"},
      {ENOMEM,        "There was insufficient memory available to complete the operation"},
#ifndef _WIN32
      {EADDRINUSE,    "Some other socket is already using the specified address"},
      {EADDRNOTAVAIL, "The specified address is not available on this machine"},
      {ECONNABORTED,  "A connection has been aborted"},
      {EDESTADDRREQ,  "The socket is not bound to a local address, and the protocol does not support listening on an unbound socket"},
      {ENOBUFS,       "Insufficient resources are available in the system to complete the call"},
      {ENOTSOCK,      "The socket argument does not refer to a socket"},
      {EOPNOTSUPP,    "The socket type of the specified socket does not support accepting connections"},
      {EPROTO,        "A protocol error has occurred; for example, the STREAMS protocol stack has not been initialized"},
      {EWOULDBLOCK,   "O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted"},
#endif
      {0, NULL}
    };
  for (; error_msg[i].msg; i++)
    if (e == error_msg[i].e)
      return error_msg[i].msg;
  return "Uknown error";
}
#endif


#if 0
typedef struct
{
  char *ext;
  char *val;
} st_mime_entry_t;


static st_mime_entry_t enc_tab[] = {
  {"Z", "compress"},
  {"gz", "gzip"},
  {"uu", "x-uuencode"},
};


static st_mime_entry_t typ_tab[] = {
  {"a", "application/octet-stream"},
  {"aab", "application/x-authorware-bin"},
  {"aam", "application/x-authorware-map"},
  {"aas", "application/x-authorware-seg"},
  {"ai", "application/postscript"},
  {"aif", "audio/x-aiff"},
  {"aifc", "audio/x-aiff"},
  {"aiff", "audio/x-aiff"},
  {"asc", "text/plain"},
  {"asf", "video/x-ms-asf"},
  {"asx", "video/x-ms-asf"},
  {"au", "audio/basic"},
  {"avi", "video/x-msvideo"},
  {"bcpio", "application/x-bcpio"},
  {"bin", "application/octet-stream"},
  {"bmp", "image/bmp"},
  {"cdf", "application/x-netcdf"},
  {"class", "application/x-java-vm"},
  {"cpio", "application/x-cpio"},
  {"cpt", "application/mac-compactpro"},
  {"crl", "application/x-pkcs7-crl"},
  {"crt", "application/x-x509-ca-cert"},
  {"csh", "application/x-csh"},
  {"css", "text/css"},
  {"dcr", "application/x-director"},
  {"dir", "application/x-director"},
  {"djv", "image/vnd.djvu"},
  {"djvu", "image/vnd.djvu"},
  {"dll", "application/octet-stream"},
  {"dms", "application/octet-stream"},
  {"doc", "application/msword"},
  {"dtd", "text/xml"},
  {"dump", "application/octet-stream"},
  {"dvi", "application/x-dvi"},
  {"dxr", "application/x-director"},
  {"eps", "application/postscript"},
  {"etx", "text/x-setext"},
  {"exe", "application/octet-stream"},
  {"ez", "application/andrew-inset"},
  {"fgd", "application/x-director"},
  {"fh", "image/x-freehand"},
  {"fh4", "image/x-freehand"},
  {"fh5", "image/x-freehand"},
  {"fh7", "image/x-freehand"},
  {"fhc", "image/x-freehand"},
  {"gif", "image/gif"},
  {"gtar", "application/x-gtar"},
  {"hdf", "application/x-hdf"},
  {"hqx", "application/mac-binhex40"},
  {"htm", "text/html; charset=%s"},
  {"html", "text/html; charset=%s"},
  {"ice", "x-conference/x-cooltalk"},
  {"ief", "image/ief"},
  {"iges", "model/iges"},
  {"igs", "model/iges"},
  {"iv", "application/x-inventor"},
  {"jar", "application/x-java-archive"},
  {"jfif", "image/jpeg"},
  {"jpe", "image/jpeg"},
  {"jpeg", "image/jpeg"},
  {"jpg", "image/jpeg"},
  {"js", "application/x-javascript"},
  {"kar", "audio/midi"},
  {"latex", "application/x-latex"},
  {"lha", "application/octet-stream"},
  {"lzh", "application/octet-stream"},
  {"m3u", "audio/x-mpegurl"},
  {"man", "application/x-troff-man"},
  {"mathml", "application/mathml+xml"},
  {"me", "application/x-troff-me"},
  {"mesh", "model/mesh"},
  {"mid", "audio/midi"},
  {"midi", "audio/midi"},
  {"mif", "application/vnd.mif"},
  {"mime", "message/rfc822"},
  {"mml", "application/mathml+xml"},
  {"mov", "video/quicktime"},
  {"movie", "video/x-sgi-movie"},
  {"mp2", "audio/mpeg"},
  {"mp3", "audio/mpeg"},
  {"mp4", "video/mp4"},
  {"mpe", "video/mpeg"},
  {"mpeg", "video/mpeg"},
  {"mpg", "video/mpeg"},
  {"mpga", "audio/mpeg"},
  {"ms", "application/x-troff-ms"},
  {"msh", "model/mesh"},
  {"mv", "video/x-sgi-movie"},
  {"mxu", "video/vnd.mpegurl"},
  {"nc", "application/x-netcdf"},
  {"o", "application/octet-stream"},
  {"oda", "application/oda"},
  {"ogg", "application/x-ogg"},
  {"pac", "application/x-ns-proxy-autoconfig"},
  {"pbm", "image/x-portable-bitmap"},
  {"pdb", "chemical/x-pdb"},
  {"pdf", "application/pdf"},
  {"pgm", "image/x-portable-graymap"},
  {"pgn", "application/x-chess-pgn"},
  {"png", "image/png"},
  {"pnm", "image/x-portable-anymap"},
  {"ppm", "image/x-portable-pixmap"},
  {"ppt", "application/vnd.ms-powerpoint"},
  {"ps", "application/postscript"},
  {"qt", "video/quicktime"},
  {"ra", "audio/x-realaudio"},
  {"ram", "audio/x-pn-realaudio"},
  {"ras", "image/x-cmu-raster"},
  {"rdf", "application/rdf+xml"},
  {"rgb", "image/x-rgb"},
  {"rm", "audio/x-pn-realaudio"},
  {"roff", "application/x-troff"},
  {"rpm", "audio/x-pn-realaudio-plugin"},
  {"rss", "application/rss+xml"},
  {"rtf", "text/rtf"},
  {"rtx", "text/richtext"},
  {"sgm", "text/sgml"},
  {"sgml", "text/sgml"},
  {"sh", "application/x-sh"},
  {"shar", "application/x-shar"},
  {"silo", "model/mesh"},
  {"sit", "application/x-stuffit"},
  {"skd", "application/x-koan"},
  {"skm", "application/x-koan"},
  {"skp", "application/x-koan"},
  {"skt", "application/x-koan"},
  {"smi", "application/smil"},
  {"smil", "application/smil"},
  {"snd", "audio/basic"},
  {"so", "application/octet-stream"},
  {"spl", "application/x-futuresplash"},
  {"src", "application/x-wais-source"},
  {"stc", "application/vnd.sun.xml.calc.template"},
  {"std", "application/vnd.sun.xml.draw.template"},
  {"sti", "application/vnd.sun.xml.impress.template"},
  {"stw", "application/vnd.sun.xml.writer.template"},
  {"sv4cpio", "application/x-sv4cpio"},
  {"sv4crc", "application/x-sv4crc"},
  {"svg", "image/svg+xml"},
  {"svgz", "image/svg+xml"},
  {"swf", "application/x-shockwave-flash"},
  {"sxc", "application/vnd.sun.xml.calc"},
  {"sxd", "application/vnd.sun.xml.draw"},
  {"sxg", "application/vnd.sun.xml.writer.global"},
  {"sxi", "application/vnd.sun.xml.impress"},
  {"sxm", "application/vnd.sun.xml.math"},
  {"sxw", "application/vnd.sun.xml.writer"},
  {"t", "application/x-troff"},
  {"tar", "application/x-tar"},
  {"tcl", "application/x-tcl"},
  {"tex", "application/x-tex"},
  {"texi", "application/x-texinfo"},
  {"texinfo", "application/x-texinfo"},
  {"tif", "image/tiff"},
  {"tiff", "image/tiff"},
  {"tr", "application/x-troff"},
  {"tsp", "application/dsptype"},
  {"tsv", "text/tab-separated-values"},
  {"txt", "text/plain; charset=%s"},
  {"ustar", "application/x-ustar"},
  {"vcd", "application/x-cdlink"},
  {"vrml", "model/vrml"},
  {"vx", "video/x-rad-screenplay"},
  {"wav", "audio/x-wav"},
  {"wax", "audio/x-ms-wax"},
  {"wbmp", "image/vnd.wap.wbmp"},
  {"wbxml", "application/vnd.wap.wbxml"},
  {"wm", "video/x-ms-wm"},
  {"wma", "audio/x-ms-wma"},
  {"wmd", "application/x-ms-wmd"},
  {"wml", "text/vnd.wap.wml"},
  {"wmlc", "application/vnd.wap.wmlc"},
  {"wmls", "text/vnd.wap.wmlscript"},
  {"wmlsc", "application/vnd.wap.wmlscriptc"},
  {"wmv", "video/x-ms-wmv"},
  {"wmx", "video/x-ms-wmx"},
  {"wmz", "application/x-ms-wmz"},
  {"wrl", "model/vrml"},
  {"wsrc", "application/x-wais-source"},
  {"wvx", "video/x-ms-wvx"},
  {"xbm", "image/x-xbitmap"},
  {"xht", "application/xhtml+xml"},
  {"xhtml", "application/xhtml+xml"},
  {"xls", "application/vnd.ms-excel"},
  {"xml", "text/xml"},
  {"xpm", "image/x-xpixmap"},
  {"xsl", "text/xml"},
  {"xwd", "image/x-xwindowdump"},
  {"xyz", "chemical/x-xyz"},
  {"zip", "application/zip"},
  {"tgz", "application/x-tar-gzip"},
  {"bz2", "application/x-bzip2"},
  {"gz", "application/x-gzip"},
  {"Z", "application/x-compress"},
};


const char *
net_get_mime_type_by_suffix (const char *suffix)
{
#warning TODO: net_get_mime_type_by_suffix()
  (void) suffix;
  return NULL;
}


/* qsort comparison routine - declared old-style on purpose, for portability. */
static int
ext_compare (a, b)
     struct mime_entry *a;
     struct mime_entry *b;
{
  return strcmp (a->ext, b->ext);
}


static void
init_mime (void)
{
  int i;

  /* Sort the tables so we can do binary search. */
  qsort (enc_tab, n_enc_tab, sizeof (*enc_tab), ext_compare);
  qsort (typ_tab, n_typ_tab, sizeof (*typ_tab), ext_compare);

  /* Fill in the lengths. */
  for (i = 0; i < n_enc_tab; ++i)
    {
      enc_tab[i].ext_len = strlen (enc_tab[i].ext);
      enc_tab[i].val_len = strlen (enc_tab[i].val);
    }
  for (i = 0; i < n_typ_tab; ++i)
    {
      typ_tab[i].ext_len = strlen (typ_tab[i].ext);
      typ_tab[i].val_len = strlen (typ_tab[i].val);
    }
}


/* Figure out MIME encodings and type based on the filename.  Multiple
** encodings are separated by commas, and are listed in the order in
** which they were applied to the file.
*/
static const char *
figure_mime (char *name, char *me, size_t me_size)
{
  char *prev_dot;
  char *dot;
  char *ext;
  int me_indexes[100], n_me_indexes;
  size_t ext_len, me_len;
  int i, top, bot, mid;
  int r;
  const char *default_type = "text/plain; charset=%s";
  const char *type;

  /* Peel off encoding extensions until there aren't any more. */
  n_me_indexes = 0;
  for (prev_dot = &name[strlen (name)];; prev_dot = dot)
    {
      for (dot = prev_dot - 1; dot >= name && *dot != '.'; --dot)
        ;
      if (dot < name)
        {
          /* No dot found.  No more encoding extensions, and no type
           ** extension either.
           */
          type = default_type;
          goto done;
        }
      ext = dot + 1;
      ext_len = prev_dot - ext;
      /* Search the encodings table.  Linear search is fine here, there
       ** are only a few entries.
       */
      for (i = 0; i < n_enc_tab; ++i)
        {
          if (ext_len == enc_tab[i].ext_len
              && strncasecmp (ext, enc_tab[i].ext, ext_len) == 0)
            {
              if (n_me_indexes < sizeof (me_indexes) / sizeof (*me_indexes))
                {
                  me_indexes[n_me_indexes] = i;
                  ++n_me_indexes;
                }
              goto next;
            }
        }
      /* No encoding extension found.  Break and look for a type extension. */
      break;

    next:;
    }

  /* Binary search for a matching type extension. */
  top = n_typ_tab - 1;
  bot = 0;
  while (top >= bot)
    {
      mid = (top + bot) / 2;
      r = strncasecmp (ext, typ_tab[mid].ext, ext_len);
      if (r < 0)
        top = mid - 1;
      else if (r > 0)
        bot = mid + 1;
      else if (ext_len < typ_tab[mid].ext_len)
        top = mid - 1;
      else if (ext_len > typ_tab[mid].ext_len)
        bot = mid + 1;
      else
        {
          type = typ_tab[mid].val;
          goto done;
        }
    }
  type = default_type;

done:

  /* The last thing we do is actually generate the mime-encoding header. */
  me[0] = '\0';
  me_len = 0;
  for (i = n_me_indexes - 1; i >= 0; --i)
    {
      if (me_len + enc_tab[me_indexes[i]].val_len + 1 < me_size)
        {
          if (me[0] != '\0')
            {
              (void) strcpy (&me[me_len], ",");
              ++me_len;
            }
          (void) strcpy (&me[me_len], enc_tab[me_indexes[i]].val);
          me_len += enc_tab[me_indexes[i]].val_len;
        }
    }

  return type;
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
//        wait2 (100);
        close (n->socket);
      }

//  free (n);
 
  return 0;
}


int
net_open (st_net_t *n, const char *host_s, int port)
{
#ifdef  USE_TCP
  st_parse_url_t url;
#endif
//  int result; 
//  struct sockaddr_in addr; 
  struct hostent *host;
//  int valopt; 
//  long arg; 
//  fd_set myset; 
//  struct timeval tv; 
//  socklen_t lon; 

#ifdef  USE_TCP
  parse_url (&url, host_s); // parse in case host is a url

  if (!port)
    port = url.port ? strtol (url.port, NULL, 10) : 80;
                    
  if (!(host = gethostbyname (url.host)))
    return -1;
  // store host and port
  if (n->flags & NET_TCP)
    strncpy (n->host, url.host, HOSTNAME_SIZE)[HOSTNAME_SIZE - 1] = 0;
  else
#endif
    {
      strncpy (n->host, host_s, HOSTNAME_SIZE)[HOSTNAME_SIZE - 1] = 0;
      if (!(host = gethostbyname (host_s)))
        return -1;
    }
  n->port = port;

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
#ifdef  _WIN32
          int addrlen;
#else
          socklen_t addrlen;
#endif
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

//      if (c == '\r')
//        continue;
//      else
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


#ifdef  USE_HTTP
static void 
net_http_get_header_normalize (st_http_header_t *h)
{
  int i;
  char *arg[HTTPHEADER_MAXPAIRS];
  char *v = NULL;

  // split into lines
  h->pairs = strtok2 (arg, h->priv, "\r\n", HTTPHEADER_MAXPAIRS);

  // split into pairs
  for (i = 1; i < h->pairs; i++)
    {
      h->p[i - 1].name = arg[i];
      v = strchr (arg[i], ':');
      if (v)
        {
          *v = 0;
          v++;
          if (*v == ' ')
            v++;
          h->p[i - 1].value = v;
        }

      // DEBUG
//      printf ("%d: name: \"%s\" value: \"%s\"\n", i, h->p[i - 1].name, h->p[i - 1].value);
//      fflush (stdout);
    }
}


int
net_get_http_header (st_http_header_t *h, st_net_t *n)
{
  int len = 0;
  char buf[MAXBUFSIZE];

  memset (h, 0, sizeof (st_http_header_t));

  while (net_gets (n, buf, MAXBUFSIZE))
    {
      // DEBUG
//      printf (buf); 
//      fflush (stdout);

      len += strlen (buf);
      if (len > HTTPHEADER_MAXBUFSIZE - 1)
        break;  // too large http header

      strcat (h->priv, buf);

      if (!(*buf) || strchr ("\r\n", *buf))
        break;
    }

  net_http_get_header_normalize (h);

  return len;
}


int
net_http_get_header_s (st_http_header_t *h, const char *buffer, int buffer_len)
{
  int len = 0;
  int line = 0;
  char buf[MAXBUFSIZE];

  memset (h, 0, sizeof (st_http_header_t));

  while (str_getline (buf, line++, buffer, buffer_len) > 0)
    {
      // DEBUG  
//      printf (buf);
//      fflush (stdout);

      len += strlen (buf);
      if (len > HTTPHEADER_MAXBUFSIZE - 1)
        break;  // too large http header
 
      strcat (h->priv, buf);          
 
      if (!(*buf) || strchr ("\r\n", *buf))
        break;
    }

  net_http_get_header_normalize (h);

  return len;
}


const char *
net_http_get_value (st_http_header_t *h, const char *name, char *value)
{
  int i = 0;
  char buf[MAXBUFSIZE];
  char *p = NULL;
  char *e = NULL;

  if (!strcmp (name, NAME_FIRSTLINE))
    {
      if (str_getline (value, 0, h->priv, strlen (h->priv)) > 0)
        return value;
      return NULL;
    }
  if (!strcmp (name, NAME_METHOD))
    {
      if (str_getline (value, 0, h->priv, strlen (h->priv)) > 0)
        {
          if (strcasestr2 (value, "GET"))
            return "GET";
          else if (strcasestr2 (value, "POST"))
            return "POST";
        }
      return NULL;
    }
  if (!strcmp (name, NAME_REQUEST))
    {
      if (str_getline (buf, 0, h->priv, strlen (h->priv)) > 0)
        {
          p = strchr (buf, ' ');
#if 1
          if (p)   
            e = strchr (p + 1, ' ');

          if (e)
            strncpy (value, p, (e - p) + 1)[e - p] = 0;
          else
            strcpy (value, p);
#else
          if (p)
            {
              p++;
              e = strchr (p, ' ');
            }

          if (e)
            strncpy (value, p, e - p)[(e - p) - 1] = 0;
          else
            strcpy (value, p);
#endif
          strtriml (strtrimr (value));  
 
          return value; 
        }
      return NULL;
    }

  for (i = 0; i < h->pairs; i++)
    {
      // DEBUG
//      printf ("%d: \"%s\" \"%s\"\n", h->p[i].name, h->p[i].value);
//      fflush (stdout);

      if (!strcasecmp (h->p[i].name, name))
        { 
          strcpy (value, h->p[i].value);
          return value;
        }
    }

  return NULL;
}


int
net_build_http_request (char *http_header, const char *url_s, const char *user_agent, int keep_alive, int method, int gzip)
{
  char buf[MAXBUFSIZE];
  st_parse_url_t url;

  parse_url (&url, url_s);

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

  if (url.user && url.pass)
    {
      sprintf (buf, "%s:%s", url.user, url.pass);
      sprintf (strchr (http_header, 0), "Authorization: Basic %s\r\n", (char *) base64_enc (buf, 0));
    } 

  strcat (http_header, "\r\n");

  // DEBUG 
//  printf (http_header);
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
//  printf (http_header);
//  fflush (stdout);

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
  st_http_header_t h;
  char http_header_s[HTTPHEADER_MAXBUFSIZE];
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

      curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1); // follow redirects
      curl_easy_setopt (curl, CURLOPT_MAXREDIRS, 10); 
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

  if (net_open (client, url.host, url.port ? strtol (url.port, NULL, 10) : 80) != 0)
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
  net_get_http_header (&h, client);

  while ((len = net_read (client, buf, MAXBUFSIZE)))
    fwrite (buf, len, 1, tmp);

  net_quit (client);

  fclose (tmp);

  return tname;
}
#endif  // USE_HTTP


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

#ifndef _WIN32
#ifdef  HAVE_ERRNO_H
      if (errno == EADDRINUSE)
        return -2; // try another port
      else
#endif
#endif
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
  (void) port;
  unsigned char *request = NULL;
  unsigned char *response = NULL;
  int request_len = 0;
  int response_len = 0;
#if 0
  int result = 0;

  result = net_bind (n, port);

  if (result != 0)
    return result;

  if (net_listen (n) == -1)
    return -1;
#endif

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

  // ignore child process termination
//  signal (SIGCHLD, SIG_IGN);

  while (1)
    {
      int pid = 0;

      if (!net_accept (n))
        {
          fprintf (stderr, "ERROR: net_server()/net_accept() failed\n");
          return -1;
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


int
net_select (st_net_t *n, int (* ping_func) (st_net_t *),
                         int (* read_func) (st_net_t *),
                         int (* write_func) (st_net_t *),
                         int timeout)
{
  int result;
  struct timeval tv;
  fd_set rd;
  time_t response = 0;

  setbuf (stdout, NULL);        // unbuffered stdout
  while (1)
    {
      FD_ZERO (&rd);
      FD_SET (0, &rd);
      FD_SET (n->socket, &rd);

      tv.tv_sec = 120;
      tv.tv_usec = 0;

      result = select (n->socket + 1, &rd, 0, 0, &tv);
      if (result < 0)
        {
          if (errno == EINTR)
            continue;
          // error on select()
          return -1;
        }
      else if (result == 0)
        {
          if (time (0) - response >= timeout)
            {
              fprintf (stderr, "ERROR: net_select() timeout\n");
              return -1; // parse timeout
            }

          if (ping_func)
            if (ping_func (n) == -1) // read message from server
              {
                fprintf (stderr, "ERROR: net_select()/ping_func() failed\n");
                return -1;
              }

          continue;
        }

      if (FD_ISSET (n->socket, &rd))
        {
          if (read_func)
            if (read_func (n) == -1) // read message from server
              {
                fprintf (stderr, "ERROR: net_select()/read_func() failed\n");
                return -1;
              }

          response = time (0);
        }

      if (FD_ISSET (0, &rd))
        {
          if (write_func)
            if (write_func (n) == -1) // write message to server
              {
                fprintf (stderr, "ERROR: net_select()/write_func() failed\n");
                return -1;
              }
        }
    }
}


#ifdef  DEBUG
#define setenv2(n,v,i) printf("%s=%s\n", n, v);fflush(stdout);setenv(n, v, i);
#else
#define setenv2 setenv
#endif


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


#ifdef  USE_CGI
int
net_cgi (st_http_header_t *h, const char *filename, void *response, int *response_len, int max_content_len)
{
#define CGI_PATH "/usr/local/bin:/bin:/usr/bin"
#define CGI_LD_LIBRARY_PATH "/usr/local/lib:/usr/lib"
  const char *p = NULL;
  FILE *cgi = NULL;
  char buf[HTTPHEADER_MAXBUFSIZE];
  st_parse_url_t url;

  strcpy (buf, "http://");
  net_http_get_value (h, "Host", strchr (buf, 0));
  strcpy (strchr (buf, 0), "/");
  net_http_get_value (h, NAME_REQUEST, strchr (buf, 0));
  parse_url (&url, buf);

  // DEBUG
//  printf ("%s\n", buf);
//  fflush (stdout);
//  st_parse_url_t_sanity_check (&url);

#if 0
/*

_ENV["SERVER_SOFTWARE"]	lighttpd/1.4.19
_ENV["SERVER_NAME"]	debian2
_ENV["GATEWAY_INTERFACE"]	CGI/1.1
_ENV["SERVER_PROTOCOL"]	HTTP/1.1
_ENV["SERVER_PORT"]	80
_ENV["SERVER_ADDR"]	0.0.0.0
_ENV["REQUEST_METHOD"]	GET
_ENV["REDIRECT_STATUS"]	200
_ENV["REQUEST_URI"]	/test/test10.php
_ENV["REMOTE_ADDR"]	192.168.2.21
_ENV["REMOTE_PORT"]	42388
_ENV["CONTENT_LENGTH"]	0
_ENV["SCRIPT_FILENAME"]	/var/www/test/test10.php
_ENV["SCRIPT_NAME"]	/test/test10.php
_ENV["DOCUMENT_ROOT"]	/var/www/
_ENV["HTTP_HOST"]	debian2
_ENV["HTTP_USER_AGENT"]	Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.6) Gecko/20091206 Firefox/3.6a1pre
_ENV["HTTP_ACCEPT"]	text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
_ENV["HTTP_ACCEPT_ENCODING"]	gzip,deflate
_ENV["HTTP_KEEP_ALIVE"]	300
_ENV["HTTP_CONNECTION"]	keep-alive
_ENV["HTTP_REFERER"]	http://debian2/test/

*/
#endif

  setenv2 ("PATH",              CGI_PATH, 1);
  setenv2 ("LD_LIBRARY_PATH",   CGI_LD_LIBRARY_PATH, 1);

  setenv2 ("SERVER_SOFTWARE",   net_http_get_value (h, "User-Agent", buf), 1);
  setenv2 ("SERVER_NAME",       net_http_get_value (h, "Host", buf), 1);
  setenv2 ("GATEWAY_INTERFACE", "CGI/1.1", 1);
  setenv2 ("SERVER_PROTOCOL",   "HTTP/1.0", 1);
  setenv2 ("SERVER_PORT",       url.port, 1);
  setenv2 ("REQUEST_METHOD",    net_http_get_value (h, NAME_METHOD, buf), 1);
  // basename()
  p = strrchr (filename, '/');
  p = p ? p + 1 : filename;
  setenv2 ("SCRIPT_NAME",       (char *) p, 1);
  setenv2 ("SCRIPT_FILENAME",   (char *) realpath (filename, buf), 1);
  setenv2 ("PATH_INFO",         "/", 1);
  if (realpath (filename, buf))
    {
//      sprintf (buf, "%s/", (char *) realpath (filename, buf));
      setenv2 ("PATH_TRANSLATED",   buf, 1);
    }
  setenv2 ("QUERY_STRING",      url.query ? url.query : "?", 1);
  setenv2 ("REMOTE_ADDR",       url.host, 1);
//  setenv2 ("HTTP_REFERER",    net_http_get_value (h, "Referer", buf), 1);
  setenv2 ("HTTP_USER_AGENT",   net_http_get_value (h, "User-Agent", buf), 1);
//  setenv2 ("HTTP_COOKIE",     net_http_get_value (h, "Cookie", buf), 1);
  sprintf (buf, "%s:%s",        url.host, url.port);
  setenv2 ("HTTP_HOST",         buf, 1);
#warning fix this net_http_get_value() should return NULL here
//  if (net_http_get_value (h, "Content-Type", buf))
//    setenv2 ("CONTENT_TYPE",      buf, 1);
//  if (net_http_get_value (h, "Content-Length", buf))
//    setenv2 ("CONTENT_LENGTH",    buf, 1);
  setenv2 ("REMOTE_USER",       "", 1);
  // php env
  setenv2 ("PHP_AUTH_USER",     "", 1);
  setenv2 ("PHP_AUTH_PW",       "", 1);
  setenv2 ("AUTH_USER",         "", 1);
  setenv2 ("AUTH_PW",           "", 1);
  setenv2 ("AUTH_TYPE",         "Basic", 1);
  if (getenv ("TZ"))
    setenv2 ("TZ",              getenv ("TZ"), 1);

//  dirname2 (filename, buf);
//  chdir (buf);
//  p = basename2 (filename);

  if (!(cgi = popen (filename, "r")))
    {
      fprintf (stderr, "ERROR: could not run %s\n", filename);
      fflush (stderr);
      return -1;
    }

  *response_len = fread (response, 1, max_content_len, cgi);
  pclose (cgi);

  // DEBUG
//  printf (response);
//  fflush (stdout);

  return 0;
}
#endif


#ifdef  TEST
//#if 0
int
net_server_cb (const void *request, int request_len, void *response, int *response_len)
{
  char buf[MAXBUFSIZE];
  char http_header_s[HTTPHEADER_MAXBUFSIZE];
  st_http_header_t http_header;

  net_http_get_header_s (&http_header, request, request_len);
  printf ("%s", net_http_get_value (&http_header, NAME_REQUEST, buf));
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
