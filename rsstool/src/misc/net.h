/*
net.h - miscellaneous network functions

Copyright (c) 2006 NoisyB
           

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
#ifndef MISC_NET_H
#define MISC_NET_H
#ifdef  __cplusplus
extern "C" {
#endif
#if     (defined USE_TCP || defined USE_UDP)
#ifdef  _WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

/*
  Network functions

  net_init()     open TCP or UDP socket
  net_quit()     quit

  Flags
    NET_TCP        use TCP
    NET_CLIENT     use to connect to server
    NET_SERVER     use as server
    NET_UDP        use UDP
    NET_DEBUG      print DEBUG output
    NET_LOCALONLY  allow connections to/from localhost only

  Client (and server)
    net_open()     open connection to a server

    net_read()
    net_write()

    net_getc()
    net_putc()
    net_gets()
    net_puts()

    net_close()    close connection

    net_sync() 

  Server
    net_bind()
    net_listen()
    net_accept()
    net_server()   starts TCP server and runs callback function on connection/request
*/
#define NET_TCP        1
#define NET_CLIENT     (1<<1)
#define NET_SERVER     (1<<2)
#define NET_UDP        (1<<4)
#define NET_DEBUG      (1<<5)
#define NET_LOCALONLY  (1<<6)


typedef struct
{
  int flags;
  int timeout;
#define HOSTNAME_SIZE 4096
  char host[HOSTNAME_SIZE];
  int port;  

  int sock0;
  int socket;

  struct sockaddr_in addr;
  struct sockaddr_in udp_addr;
} st_net_t;


extern st_net_t *net_init (int flags, int timeout);
extern int net_quit (st_net_t *n);

// client
extern int net_open (st_net_t *n, const char *host_s, int port);
extern int net_close (st_net_t *n);

extern int net_read (st_net_t *n, void *buffer, int buffer_len);
extern int net_write (st_net_t *n, void *buffer, int buffer_len);
extern int net_getc (st_net_t *n);
extern int net_putc (st_net_t *n, int c);
extern char *net_gets (st_net_t *n, char *buffer, int buffer_len);
extern int net_puts (st_net_t *n, char *buffer);

//extern int net_sync (st_net_t *n);
extern int net_bind (st_net_t *n, int port);
extern int net_listen (st_net_t *n);
extern st_net_t *net_accept (st_net_t *n);

// server with callback
extern int net_server (st_net_t *n, int port, int (* callback_func) (const void *, int, void *, int *), int max_content_len);

extern int net_select (st_net_t *n, int (* ping_func) (st_net_t *),
                                    int (* read_func) (st_net_t *),
                                    int (* write_func) (st_net_t *),
                                    int timeout);


#ifdef  USE_HTTP
/*
  HTTP header build and read/parse functions

  net_get_http_header()     read and parse http header (from client or server)
  net_get_http_header_s()   read and parse http header (from string buffer)
                              both return the header size in bytes
  net_get_http_value()      get value from parsed http header

  net_build_http_request()  build http header (for client request)
  net_build_http_response() build http header (for server response) 
*/
typedef struct
{
  const char *name;
  const char *value;
} st_http_header_pairs_t;
#define HTTPHEADER_MAXPAIRS 255
#define HTTPHEADER_MAXBUFSIZE 32768
typedef struct
{
  st_http_header_pairs_t p[HTTPHEADER_MAXPAIRS];
  int pairs;
  char priv[HTTPHEADER_MAXBUFSIZE];
} st_http_header_t;
enum {
  NET_METHOD_GET = 0,
  NET_METHOD_POST
};
#define NAME_FIRSTLINE "0" // name for request/response line at top of http header
#define NAME_METHOD "1"    // name for the method
#define NAME_REQUEST "2"   // name for the request
extern int net_get_http_header (st_http_header_t *h, st_net_t *n); 
extern int net_http_get_header_s (st_http_header_t *h, const char *buffer, int buffer_len);
extern const char *net_http_get_value (st_http_header_t *h, const char *name, char *value);

extern int net_build_http_request (char *http_header, const char *url_s, const char *user_agent, int keep_alive, int method, int gzip);
extern int net_build_http_response (char *http_header, const char *user_agent, int keep_alive, unsigned int content_len, int gzip);


/*
  net_http_get_to_temp()    decide if url_or_fname is a url or fname
                              it will eventually download the file and
                              return the name of a temporary file
                              OR the fname when it was a local file
  Flags
    GET_USE_GZIP  use gzip compression (if compiled)
*/
#define GET_USE_GZIP (1<<1)
// curl is the default (if available) 
#define GET_NO_CURL  (1<<2) 
#define GET_VERBOSE  (1<<3)
extern const char *net_http_get_to_temp (const char *url_s, const char *user_agent, int flags);
                                              

/*
  net_get_mime_type_by_suffix() get the apropriate mime type for the specified suffix
                                  default: text/plain
*/
extern const char *net_get_mime_type_by_suffix (const char *suffix);
#endif  // USE_HTTP


/*
  net_cgi()  common gateway interface
*/
#ifdef  USE_CGI
extern int net_cgi (st_http_header_t *h, const char *filename, void *response, int *response_len, int max_content_len);
#endif  // USE_CGI


#endif  // (defined USE_TCP || defined USE_UDP)
#ifdef  __cplusplus
}
#endif
#endif  // MISC_NET_H
